/*
 * Copyright (C) 2011-2013 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "graphic/render/gamerenderer_gl.h"

#include <memory>

#include <SDL_image.h>

#include "graphic/graphic.h"
#include "graphic/image_io.h"
#include "graphic/render/gl_surface.h"
#include "graphic/render/gl_utils.h"
#include "graphic/rendertarget.h"
#include "graphic/surface_cache.h"
#include "graphic/texture.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/editor_game_base.h"
#include "logic/player.h"
#include "logic/world/terrain_description.h"
#include "logic/world/world.h"
#include "wui/mapviewpixelconstants.h"
#include "wui/mapviewpixelfunctions.h"
#include "wui/overlay_manager.h"

using namespace Widelands;

namespace  {

constexpr int kAttribVertexPosition = 0;
constexpr int kAttribVertexColor = 1;
constexpr int kAttribVertexHeight = 2;

struct TerrainProgramData {
	float x;
	float y;
	float height;
	float r;
	float g;
	float b;
};

static_assert(sizeof(TerrainProgramData) == 24, "Wrong padding.");

// NOCOM(#sirver): should not load from a file
GLuint load_shader(GLenum type, const std::string& source_file) {
	std::string source;
	{
		FileRead fr;
		fr.Open(*g_fs, source_file);
		source = fr.CString();
	}

	const GLuint shader = glCreateShader(type);
	if (!shader) {
		throw wexception("Could not create shader from %s.", source_file.c_str());
	}

	const char* source_ptr = source.c_str();
	glShaderSource(shader, 1, &source_ptr, nullptr);

	glCompileShader(shader);
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		GLint infoLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

		if (infoLen > 1) {
			std::unique_ptr<char[]> infoLog(new char[infoLen]);
			glGetShaderInfoLog(shader, infoLen, NULL, infoLog.get());
			throw wexception("Error compiling shader %s:\n%s", source_file.c_str(), infoLog.get());
		}
		// NOCOM(#sirver): cleanup in any case.
		// glDeleteShader(shader);
		// exit(EXIT_FAILURE);
		// return 0;
	}

	return shader;
}

GLuint compile_gl_program(const std::string& vertex_shader_file,
                          const std::string& fragment_shader_file) {
	const GLuint program_object = glCreateProgram();
	if (!program_object) {
		throw wexception("Could not create GL program.");
	}

	log("#sirver Compiling vertex_shader\n");
	const GLuint vertex_shader = load_shader(GL_VERTEX_SHADER, vertex_shader_file);
	glAttachShader(program_object, vertex_shader);

	log("#sirver Compiling fragment_shader\n");
	const GLuint fragment_shader = load_shader(GL_FRAGMENT_SHADER, fragment_shader_file);
	glAttachShader(program_object, fragment_shader);
	return program_object;
}

void link_gl_program(const GLuint program_object) {
	glLinkProgram(program_object);

	// Check the link status
	GLint linked;
	glGetProgramiv(program_object, GL_LINK_STATUS, &linked);
	if (!linked) {
		GLint infoLen = 0;
		glGetProgramiv(program_object, GL_INFO_LOG_LENGTH, &infoLen);

		if (infoLen > 1) {
			std::unique_ptr<char[]> infoLog(new char[infoLen]);
			glGetProgramInfoLog(program_object, infoLen, NULL, infoLog.get());
			throw wexception("Error linking:\n%s", infoLog.get());
		}
		// NOCOM(#sirver): cleanup in any case.
		// glDeleteProgram(program_object);
		// return 0;
	}
}
}  // namespace

static const uint32_t PatchSize = 4;

GameRendererGL::GameRendererGL() :
	m_patch_vertices_size(0),
	m_patch_indices_size(0),
	m_edge_vertices_size(0),
	m_road_vertices_size(0)
{
}

GameRendererGL::~GameRendererGL()
{
}

const GLSurfaceTexture * GameRendererGL::get_dither_edge_texture()
{
	const std::string fname = "world/pics/edge.png";
	const std::string cachename = std::string("gltex#") + fname;

	if (Surface* surface = g_gr->surfaces().get(cachename))
		return dynamic_cast<GLSurfaceTexture *>(surface);

	SDL_Surface* sdlsurf = load_image_as_sdl_surface(fname, g_fs);
	GLSurfaceTexture* edgetexture = new GLSurfaceTexture(sdlsurf, true);
	g_gr->surfaces().insert(cachename, edgetexture, false);
	return edgetexture;
}

uint32_t GameRendererGL::patch_index(const Coords & f) const
{
	uint32_t x = f.x - m_patch_size.x;
	uint32_t y = f.y - m_patch_size.y;

	assert(x < m_patch_size.w);
	assert(y < m_patch_size.h);

	uint32_t outerx = x / PatchSize;
	uint32_t outery = y / PatchSize;
	uint32_t innerx = x % PatchSize;
	uint32_t innery = y % PatchSize;

	return
		(outery * (m_patch_size.w / PatchSize) + outerx) * (PatchSize * PatchSize) +
		innery * PatchSize + innerx;
}

uint8_t GameRendererGL::field_brightness(const FCoords & coords) const
{
	uint32_t brightness;
	brightness = 144 + coords.field->get_brightness();
	brightness = (brightness * 255) / 160;
	if (brightness > 255)
		brightness = 255;

	if (m_player && !m_player->see_all()) {
		const Map & map = m_egbase->map();
		const Player::Field & pf = m_player->fields()[Map::get_index(coords, map.get_width())];
		if (pf.vision == 0) {
			return 0;
		} else if (pf.vision == 1) {
			static const uint32_t DecayTime = 20000;
			Duration time_ago = m_egbase->get_gametime() - pf.time_node_last_unseen;
			if (time_ago < DecayTime)
				brightness = (brightness * (2 * DecayTime - time_ago)) / (2 * DecayTime);
			else
				brightness = brightness / 2;
		}
	}

	return brightness;
}

void GameRendererGL::initialize() {
	// Setup Buffers;
	handle_glerror();
	glGenBuffers(1, &terrain_program_data_);
	handle_glerror();
	glGenBuffers(1, &indices_buffer_);
	handle_glerror();

	terrain_program_ =
	   compile_gl_program("src/graphic/render/terrain.vs", "src/graphic/render/terrain.fs");

	handle_glerror();
	glBindAttribLocation(terrain_program_, kAttribVertexPosition, "position");
	glBindAttribLocation(terrain_program_, kAttribVertexColor, "color");
	glBindAttribLocation(terrain_program_, kAttribVertexHeight, "height");
	handle_glerror();

	link_gl_program(terrain_program_);

	handle_glerror();

	do_initialize_ = false;
}

void GameRendererGL::draw_terrain_triangles() {
	const Map & map = m_egbase->map();
	const World& world = m_egbase->world();

	std::vector<TerrainProgramData> vertices;
	std::vector<uint16_t> indices;

	int current_index = 0;
	const auto add_vertex = [this, &vertices, &indices, &map, &current_index](
	   int fx, int fy, const RGBColor& vertex_color) {
		Coords coords(fx, fy);

		TerrainProgramData v;
		int x, y;
		MapviewPixelFunctions::get_basepix(coords, x, y);
		v.x = x + m_surface_offset.x;
		v.y = y + m_surface_offset.y;

		map.normalize_coords(coords);
		const FCoords fcoords = map.get_fcoords(coords);
		v.height = fcoords.field->get_height();
		v.r = vertex_color.r / 255.;
		v.g = vertex_color.g / 255.;
		v.b = vertex_color.b / 255.;

		indices.push_back(current_index++);
		vertices.push_back(v);
	};

	for (int32_t fy = m_minfy; fy <= m_maxfy; ++fy) {
		for (int32_t fx = m_minfx; fx <= m_maxfx; ++fx) {
			Coords ncoords(fx, fy);
			map.normalize_coords(ncoords);
			const FCoords fcoords = map.get_fcoords(ncoords);

			const int8_t brightness = fcoords.field->get_brightness();

			// Right triangle.
			const RGBColor color_r =
			   g_gr->get_maptexture_data(world.terrain_descr(fcoords.field->terrain_r()).get_texture())
			      ->get_minimap_color(brightness);
			add_vertex(fx, fy, color_r);
			add_vertex(fx + 1, fy, color_r); // right neighbor
			add_vertex(fx + (fy & 1), fy + 1, color_r); // bottom right neighbor

			// Bottom triangle.
			const RGBColor color_d =
			   g_gr->get_maptexture_data(world.terrain_descr(fcoords.field->terrain_d()).get_texture())
			      ->get_minimap_color(brightness);
			add_vertex(fx, fy, color_d);
			add_vertex(fx + (fy & 1), fy + 1, color_d); // bottom right neighbor
			add_vertex(fx + (fy & 1) - 1, fy + 1, color_d); // bottom left neighbor
		}
	}

	// Use the program object
	glUseProgram(terrain_program_);

	assert(vertices.size() == indices.size());

	glBindBuffer(GL_ARRAY_BUFFER, terrain_program_data_);
	handle_glerror();
	glBufferData(
		GL_ARRAY_BUFFER, sizeof(TerrainProgramData) * vertices.size(), vertices.data(), GL_STREAM_DRAW);

	// Setup vertex position.
	glEnableVertexAttribArray(kAttribVertexPosition);
	glVertexAttribPointer(kAttribVertexPosition,
	                      2,
	                      GL_FLOAT,
	                      GL_FALSE,
	                      sizeof(TerrainProgramData),
	                      reinterpret_cast<void*>(offsetof(TerrainProgramData, x)));

	// Color.
	glEnableVertexAttribArray(kAttribVertexColor);
	glVertexAttribPointer(kAttribVertexColor,
	                      3,
	                      GL_FLOAT,
	                      GL_FALSE,
	                      sizeof(TerrainProgramData),
	                      reinterpret_cast<void*>(offsetof(TerrainProgramData, r)));

	// Height.
	glEnableVertexAttribArray(kAttribVertexHeight);
	glVertexAttribPointer(kAttribVertexHeight,
	                      1,
	                      GL_FLOAT,
	                      GL_FALSE,
	                      sizeof(TerrainProgramData),
	                      reinterpret_cast<void*>(offsetof(TerrainProgramData, height)));

	// Which triangles to draw?
	handle_glerror();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_buffer_);
	handle_glerror();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
					 sizeof(GL_UNSIGNED_SHORT) * indices.size(),
					 indices.data(),
					 GL_STREAM_DRAW);
	handle_glerror();

	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, 0 /* offset */);
	handle_glerror();

	// Release Program object.
	glUseProgram(0);
}

void GameRendererGL::draw() {
	if (do_initialize_) {
		// NOCOM(#sirver): this is done for each gamerenderer. This is not really necessary.
		initialize();
	}

	// NOCOM(#sirver): Use this quantity to draw as many triangles as possible.
	// glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);

	const World& world = m_egbase->world();
	if (m_terrain_freq.size() < world.terrains().get_nitems()) {
		m_terrain_freq.resize(world.terrains().get_nitems());
		m_terrain_edge_freq.resize(world.terrains().get_nitems());
	}

	m_surface = dynamic_cast<GLSurface *>(m_dst->get_surface());
	if (!m_surface)
		return;
	m_rect = m_dst->get_rect();
	m_surface_offset = m_dst_offset + m_rect.top_left() + m_dst->get_offset();

	// NOCOM(#sirver): maiyb?
	// glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// Clear the color buffer
	// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//


	m_patch_size.x = m_minfx - 1;
	m_patch_size.y = m_minfy;
	m_patch_size.w = ((m_maxfx - m_minfx + 2 + PatchSize) / PatchSize) * PatchSize;
	m_patch_size.h = ((m_maxfy - m_minfy + 1 + PatchSize) / PatchSize) * PatchSize;

	glScissor
		(m_rect.x, m_surface->height() - m_rect.y - m_rect.h,
		 m_rect.w, m_rect.h);
	glEnable(GL_SCISSOR_TEST);

	draw_terrain_triangles();

	// prepare_terrain_base();
	// draw_terrain_base();

	// if (g_gr->caps().gl.multitexture && g_gr->caps().gl.max_tex_combined >= 2) {
		// prepare_terrain_dither();
		// draw_terrain_dither();
	// }
	// prepare_roads();
	// draw_roads();
	// draw_objects();

	glDisable(GL_SCISSOR_TEST);
}

template<typename vertex>
void GameRendererGL::compute_basevertex(const Coords & coords, vertex & vtx) const
{
	const Map & map = m_egbase->map();
	Coords ncoords(coords);
	map.normalize_coords(ncoords);
	FCoords fcoords = map.get_fcoords(ncoords);
	Point pix;
	MapviewPixelFunctions::get_basepix(coords, pix.x, pix.y);
	pix.y -= fcoords.field->get_height() * HEIGHT_FACTOR;
	pix += m_surface_offset;
	vtx.x = pix.x;
	vtx.y = pix.y;
	Point tex;
	MapviewPixelFunctions::get_basepix(coords, tex.x, tex.y);
	vtx.tcx = float(tex.x) / TEXTURE_WIDTH;
	vtx.tcy = float(tex.y) / TEXTURE_HEIGHT;
	uint8_t brightness = field_brightness(fcoords);
	vtx.color[0] = vtx.color[1] = vtx.color[2] = brightness;
	vtx.color[3] = 255;
}

void GameRendererGL::count_terrain_base(TerrainIndex ter)
{
	if (ter >= m_terrain_freq.size())
		m_terrain_freq.resize(ter + 1);
	m_terrain_freq[ter] += 1;
}

void GameRendererGL::add_terrain_base_triangle
	(TerrainIndex ter, const Coords & p1, const Coords & p2, const Coords & p3)
{
	uint32_t index = m_patch_indices_indexs[ter];
	m_patch_indices[index++] = patch_index(p1);
	m_patch_indices[index++] = patch_index(p2);
	m_patch_indices[index++] = patch_index(p3);
	m_patch_indices_indexs[ter] = index;
}

void GameRendererGL::collect_terrain_base(bool onlyscan)
{
	const Map & map = m_egbase->map();

	uint32_t index = 0;
	for (uint32_t outery = 0; outery < m_patch_size.h / PatchSize; ++outery) {
		for (uint32_t outerx = 0; outerx < m_patch_size.w / PatchSize; ++outerx) {
			for (uint32_t innery = 0; innery < PatchSize; ++innery) {
				for (uint32_t innerx = 0; innerx < PatchSize; ++innerx) {
					Coords coords
						(m_patch_size.x + outerx * PatchSize + innerx,
						 m_patch_size.y + outery * PatchSize + innery);

					if (onlyscan) {
						assert(index == patch_index(coords));
						compute_basevertex(coords, m_patch_vertices[index]);
						++index;
					}

					if (coords.x >= m_minfx && coords.y >= m_minfy && coords.x <= m_maxfx && coords.y <= m_maxfy) {
						Coords ncoords(coords);
						map.normalize_coords(ncoords);
						FCoords fcoords = map.get_fcoords(ncoords);
						TerrainIndex ter_d = fcoords.field->get_terrains().d;
						TerrainIndex ter_r = fcoords.field->get_terrains().r;

						if (onlyscan) {
							count_terrain_base(ter_d);
							count_terrain_base(ter_r);
						} else {
							Coords brn(coords.x + (coords.y & 1), coords.y + 1);
							Coords bln(brn.x - 1, brn.y);
							Coords rn(coords.x + 1, coords.y);

							add_terrain_base_triangle(ter_d, coords, bln, brn);
							add_terrain_base_triangle(ter_r, coords, brn, rn);
						}
					}
				}
			}
		}
	}
}

void GameRendererGL::prepare_terrain_base()
{
	static_assert(sizeof(BaseVertex) == 32, "assert(sizeof(basevertex) == 32) failed.");

	uint32_t reqsize = m_patch_size.w * m_patch_size.h;
	if (reqsize > 0x10000)
		throw wexception("Too many vertices; decrease screen resolution");

	if (reqsize > m_patch_vertices_size) {
		m_patch_vertices.reset(new BaseVertex[reqsize]);
		m_patch_vertices_size = reqsize;
	}

	m_terrain_freq.assign(m_terrain_freq.size(), 0);

	collect_terrain_base(true);

	m_terrain_freq_cum.resize(m_terrain_freq.size());
	uint32_t nrtriangles = 0;
	for (uint32_t idx = 0; idx < m_terrain_freq.size(); ++idx) {
		m_terrain_freq_cum[idx] = nrtriangles;
		nrtriangles += m_terrain_freq[idx];
	}

	if (3 * nrtriangles > m_patch_indices_size) {
		m_patch_indices.reset(new uint16_t[3 * nrtriangles]);
		m_patch_indices_size = 3 * nrtriangles;
	}

	m_patch_indices_indexs.resize(m_terrain_freq.size());
	for (TerrainIndex ter = 0; ter < m_terrain_freq.size(); ++ter)
		m_patch_indices_indexs[ter] = 3 * m_terrain_freq_cum[ter];

	collect_terrain_base(false);

	for (TerrainIndex ter = 0; ter < m_terrain_freq.size(); ++ter) {
		assert(m_patch_indices_indexs[ter] == 3 * (m_terrain_freq_cum[ter] + m_terrain_freq[ter]));
	}
}

void GameRendererGL::draw_terrain_base()
{
	const World & world = m_egbase->world();

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glVertexPointer(2, GL_FLOAT, sizeof(BaseVertex), &m_patch_vertices[0].x);
	glTexCoordPointer(2, GL_FLOAT, sizeof(BaseVertex), &m_patch_vertices[0].tcx);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(BaseVertex), &m_patch_vertices[0].color);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glColor3f(1.0, 1.0, 1.0);
	glDisable(GL_BLEND);

	for (TerrainIndex ter = 0; ter < m_terrain_freq.size(); ++ter) {
		if (!m_terrain_freq[ter])
			continue;

		const Texture & texture =
				*g_gr->get_maptexture_data
					(world.terrain_descr(ter).get_texture());
		glBindTexture(GL_TEXTURE_2D, texture.getTexture());
		glDrawRangeElements
			(GL_TRIANGLES,
			 0, m_patch_size.w * m_patch_size.h - 1,
			 3 * m_terrain_freq[ter], GL_UNSIGNED_SHORT,
			 &m_patch_indices[3 * m_terrain_freq_cum[ter]]);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}

void GameRendererGL::add_terrain_dither_triangle
	(bool onlyscan, TerrainIndex ter, const Coords & edge1, const Coords & edge2, const Coords & opposite)
{
	if (onlyscan) {
		assert(ter < m_terrain_edge_freq.size());
		m_terrain_edge_freq[ter] += 1;
	} else {
		static const float TyZero = 1.0 / TEXTURE_HEIGHT;
		static const float TyOne = 1.0 - TyZero;

		uint32_t index = m_terrain_edge_indexs[ter];
		compute_basevertex(edge1, m_edge_vertices[index]);
		m_edge_vertices[index].edgex = 0.0;
		m_edge_vertices[index].edgey = TyZero;
		++index;
		compute_basevertex(edge2, m_edge_vertices[index]);
		m_edge_vertices[index].edgex = 1.0;
		m_edge_vertices[index].edgey = TyZero;
		++index;
		compute_basevertex(opposite, m_edge_vertices[index]);
		m_edge_vertices[index].edgex = 0.5;
		m_edge_vertices[index].edgey = TyOne;
		++index;
		m_terrain_edge_indexs[ter] = index;
	}
}

void GameRendererGL::collect_terrain_dither(bool onlyscan)
{
	const Map & map = m_egbase->map();
	const World & world = m_egbase->world();

	for (int32_t fy = m_minfy; fy <= m_maxfy; ++fy) {
		for (int32_t fx = m_minfx; fx <= m_maxfx; ++fx) {
			Coords ncoords(fx, fy);
			map.normalize_coords(ncoords);
			FCoords fcoords = map.get_fcoords(ncoords);

			TerrainIndex ter_d = fcoords.field->get_terrains().d;
			TerrainIndex ter_r = fcoords.field->get_terrains().r;
			TerrainIndex ter_u = map.tr_n(fcoords).field->get_terrains().d;
			TerrainIndex ter_rr = map.r_n(fcoords).field->get_terrains().d;
			TerrainIndex ter_l = map.l_n(fcoords).field->get_terrains().r;
			TerrainIndex ter_dd = map.bl_n(fcoords).field->get_terrains().r;
			int32_t lyr_d = world.terrain_descr(ter_d).dither_layer();
			int32_t lyr_r = world.terrain_descr(ter_r).dither_layer();
			int32_t lyr_u = world.terrain_descr(ter_u).dither_layer();
			int32_t lyr_rr = world.terrain_descr(ter_rr).dither_layer();
			int32_t lyr_l = world.terrain_descr(ter_l).dither_layer();
			int32_t lyr_dd = world.terrain_descr(ter_dd).dither_layer();

			Coords f(fx, fy);
			Coords rn(fx + 1, fy);
			Coords brn(fx + (fy & 1), fy + 1);
			Coords bln(brn.x - 1, brn.y);

			if (lyr_r > lyr_d) {
				add_terrain_dither_triangle(onlyscan, ter_r, brn, f, bln);
			} else if (ter_d != ter_r) {
				add_terrain_dither_triangle(onlyscan, ter_d, f, brn, rn);
			}
			if ((lyr_u > lyr_r) || (lyr_u == lyr_r && ter_u != ter_r)) {
				add_terrain_dither_triangle(onlyscan, ter_u, rn, f, brn);
			}
			if (lyr_rr > lyr_r) {
				add_terrain_dither_triangle(onlyscan, ter_rr, brn, rn, f);
			}
			if ((lyr_l > lyr_d) || (lyr_l == lyr_d && ter_l != ter_d)) {
				add_terrain_dither_triangle(onlyscan, ter_l, f, bln, brn);
			}
			if (lyr_dd > lyr_d) {
				add_terrain_dither_triangle(onlyscan, ter_dd, bln, brn, f);
			}
		}
	}
}

/*
 * Schematic of triangle neighborhood:
 *
 *               *
 *              / \
 *             / u \
 *         (f)/     \
 *    *------*------* (r)
 *     \  l / \  r / \
 *      \  /   \  /   \
 *       \/  d  \/ rr  \
 *       *------*------* (br)
 *        \ dd /
 *         \  /
 *          \/
 *          *
 */
void GameRendererGL::prepare_terrain_dither()
{
	static_assert(sizeof(DitherVertex) == 32, "assert(sizeof(dithervertex) == 32) failed.");

	m_terrain_edge_freq.assign(m_terrain_edge_freq.size(), 0);

	collect_terrain_dither(true);

	uint32_t nrtriangles = 0;
	m_terrain_edge_freq_cum.resize(m_terrain_edge_freq.size());
	for (TerrainIndex ter = 0; ter < m_terrain_edge_freq.size(); ++ter) {
		m_terrain_edge_freq_cum[ter] = nrtriangles;
		nrtriangles += m_terrain_edge_freq[ter];
	}

	if (3 * nrtriangles > m_edge_vertices_size) {
		m_edge_vertices.reset(new DitherVertex[3 * nrtriangles]);
		m_edge_vertices_size = 3 * nrtriangles;
	}

	m_terrain_edge_indexs.resize(m_terrain_edge_freq_cum.size());
	for (TerrainIndex ter = 0; ter < m_terrain_edge_freq.size(); ++ter)
		m_terrain_edge_indexs[ter] = 3 * m_terrain_edge_freq_cum[ter];

	collect_terrain_dither(false);

	for (TerrainIndex ter = 0; ter < m_terrain_edge_freq.size(); ++ter) {
		assert(m_terrain_edge_indexs[ter] == 3 * (m_terrain_edge_freq_cum[ter] + m_terrain_edge_freq[ter]));
	}
}

void GameRendererGL::draw_terrain_dither()
{
	if (m_edge_vertices_size == 0)
		return;

	glVertexPointer(2, GL_FLOAT, sizeof(DitherVertex), &m_edge_vertices[0].x);
	glTexCoordPointer(2, GL_FLOAT, sizeof(DitherVertex), &m_edge_vertices[0].tcx);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(DitherVertex), &m_edge_vertices[0].color);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glActiveTextureARB(GL_TEXTURE1_ARB);
	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glTexCoordPointer(2, GL_FLOAT, sizeof(DitherVertex), &m_edge_vertices[0].edgex);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	GLuint edge = get_dither_edge_texture()->get_gl_texture();
	glBindTexture(GL_TEXTURE_2D, edge);
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);

	for (TerrainIndex ter = 0; ter < m_terrain_freq.size(); ++ter) {
		if (!m_terrain_edge_freq[ter])
			continue;

		const Texture & texture =
				*g_gr->get_maptexture_data
					(m_egbase->world().terrain_descr(ter).get_texture());
		glBindTexture(GL_TEXTURE_2D, texture.getTexture());
		glDrawArrays
			(GL_TRIANGLES,
			 3 * m_terrain_edge_freq_cum[ter], 3 * m_terrain_edge_freq[ter]);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_TEXTURE_2D);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
}

uint8_t GameRendererGL::field_roads(const FCoords & coords) const
{
	uint8_t roads;
	const Map & map = m_egbase->map();
	if (m_player && !m_player->see_all()) {
		const Player::Field & pf = m_player->fields()[Map::get_index(coords, map.get_width())];
		roads = pf.roads | map.overlay_manager().get_road_overlay(coords);
	} else {
		roads = coords.field->get_roads();
	}
	roads |= map.overlay_manager().get_road_overlay(coords);
	return roads;
}

void GameRendererGL::prepare_roads()
{
	const Map & map = m_egbase->map();

	m_road_freq[0] = 0;
	m_road_freq[1] = 0;

	for (int32_t fy = m_minfy; fy <= m_maxfy; ++fy) {
		for (int32_t fx = m_minfx; fx <= m_maxfx; ++fx) {
			Coords ncoords(fx, fy);
			map.normalize_coords(ncoords);
			FCoords fcoords = map.get_fcoords(ncoords);
			uint8_t roads = field_roads(fcoords);

			for (int dir = 0; dir < 3; ++dir) {
				uint8_t road = (roads >> (2 * dir)) & Road_Mask;
				if (road >= Road_Normal && road <= Road_Busy) {
					++m_road_freq[road - Road_Normal];
				}
			}
		}
	}

	uint32_t nrquads = m_road_freq[0] + m_road_freq[1];
	if (4 * nrquads > m_road_vertices_size) {
		m_road_vertices.reset(new BaseVertex[4 * nrquads]);
		m_road_vertices_size = 4 * nrquads;
	}

	uint32_t indexs[2];
	indexs[0] = 0;
	indexs[1] = 4 * m_road_freq[0];

	for (int32_t fy = m_minfy; fy <= m_maxfy; ++fy) {
		for (int32_t fx = m_minfx; fx <= m_maxfx; ++fx) {
			Coords ncoords(fx, fy);
			map.normalize_coords(ncoords);
			FCoords fcoords = map.get_fcoords(ncoords);
			uint8_t roads = field_roads(fcoords);

			uint8_t road = (roads >> Road_East) & Road_Mask;
			if (road >= Road_Normal && road <= Road_Busy) {
				uint32_t index = indexs[road - Road_Normal];
				BaseVertex start, end;
				compute_basevertex(Coords(fx, fy), start);
				compute_basevertex(Coords(fx + 1, fy), end);
				m_road_vertices[index] = start;
				m_road_vertices[index].y -= 2;
				m_road_vertices[index].tcy -= 2.0 / TEXTURE_HEIGHT;
				++index;
				m_road_vertices[index] = start;
				m_road_vertices[index].y += 2;
				m_road_vertices[index].tcy += 2.0 / TEXTURE_HEIGHT;
				++index;
				m_road_vertices[index] = end;
				m_road_vertices[index].y += 2;
				m_road_vertices[index].tcy += 2.0 / TEXTURE_HEIGHT;
				++index;
				m_road_vertices[index] = end;
				m_road_vertices[index].y -= 2;
				m_road_vertices[index].tcy -= 2.0 / TEXTURE_HEIGHT;
				++index;
				indexs[road - Road_Normal] = index;
			}

			road = (roads >> Road_SouthEast) & Road_Mask;
			if (road >= Road_Normal && road <= Road_Busy) {
				uint32_t index = indexs[road - Road_Normal];
				BaseVertex start, end;
				compute_basevertex(Coords(fx, fy), start);
				compute_basevertex(Coords(fx + (fy & 1), fy + 1), end);
				m_road_vertices[index] = start;
				m_road_vertices[index].x += 3;
				m_road_vertices[index].tcx += 3.0 / TEXTURE_HEIGHT;
				++index;
				m_road_vertices[index] = start;
				m_road_vertices[index].x -= 3;
				m_road_vertices[index].tcx -= 3.0 / TEXTURE_HEIGHT;
				++index;
				m_road_vertices[index] = end;
				m_road_vertices[index].x -= 3;
				m_road_vertices[index].tcx -= 3.0 / TEXTURE_HEIGHT;
				++index;
				m_road_vertices[index] = end;
				m_road_vertices[index].x += 3;
				m_road_vertices[index].tcx += 3.0 / TEXTURE_HEIGHT;
				++index;
				indexs[road - Road_Normal] = index;
			}

			road = (roads >> Road_SouthWest) & Road_Mask;
			if (road >= Road_Normal && road <= Road_Busy) {
				uint32_t index = indexs[road - Road_Normal];
				BaseVertex start, end;
				compute_basevertex(Coords(fx, fy), start);
				compute_basevertex(Coords(fx + (fy & 1) - 1, fy + 1), end);
				m_road_vertices[index] = start;
				m_road_vertices[index].x += 3;
				m_road_vertices[index].tcx += 3.0 / TEXTURE_HEIGHT;
				++index;
				m_road_vertices[index] = start;
				m_road_vertices[index].x -= 3;
				m_road_vertices[index].tcx -= 3.0 / TEXTURE_HEIGHT;
				++index;
				m_road_vertices[index] = end;
				m_road_vertices[index].x -= 3;
				m_road_vertices[index].tcx -= 3.0 / TEXTURE_HEIGHT;
				++index;
				m_road_vertices[index] = end;
				m_road_vertices[index].x += 3;
				m_road_vertices[index].tcx += 3.0 / TEXTURE_HEIGHT;
				++index;
				indexs[road - Road_Normal] = index;
			}
		}
	}

	assert(indexs[0] == 4 * m_road_freq[0]);
	assert(indexs[1] == 4 * nrquads);
}

void GameRendererGL::draw_roads()
{
	if (!m_road_freq[0] && !m_road_freq[1])
		return;

	GLuint rt_normal =
		dynamic_cast<const GLSurfaceTexture &>
		(g_gr->get_road_texture(Widelands::Road_Normal)).get_gl_texture();
	GLuint rt_busy =
		dynamic_cast<const GLSurfaceTexture &>
		(g_gr->get_road_texture(Widelands::Road_Busy)).get_gl_texture();

	glVertexPointer(2, GL_FLOAT, sizeof(BaseVertex), &m_road_vertices[0].x);
	glTexCoordPointer(2, GL_FLOAT, sizeof(BaseVertex), &m_road_vertices[0].tcx);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(BaseVertex), &m_road_vertices[0].color);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glDisable(GL_BLEND);

	if (m_road_freq[0]) {
		glBindTexture(GL_TEXTURE_2D, rt_normal);
		glDrawArrays(GL_QUADS, 0, 4 * m_road_freq[0]);
	}

	if (m_road_freq[1]) {
		glBindTexture(GL_TEXTURE_2D, rt_busy);
		glDrawArrays(GL_QUADS, 4 * m_road_freq[0], 4 * m_road_freq[1]);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}
