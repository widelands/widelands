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

// Useful: http://www.cs.unh.edu/~cs770/docs/glsl-1.20-quickref.pdf
const char kTerrainVertexShader[] = R"(
#version 120

// Attributes.
attribute vec2 attr_position;
attribute float attr_brightness;
attribute vec2 attr_texture_position;

// Output of vertex shader.
varying float var_brightness;
varying vec2 var_texture_position;

void main() {
	var_texture_position = attr_texture_position;
	var_brightness = attr_brightness;
	vec4 p = vec4(attr_position, 0., 1.);
	gl_Position = gl_ProjectionMatrix * p;
}
)";

const char kTerrainFragmentShader[] = R"(
#version 120

uniform sampler2D u_terrain_texture;

varying float var_brightness;
varying vec2 var_texture_position;

void main() {
	gl_FragColor = texture2D(u_terrain_texture, var_texture_position) * var_brightness;
}
)";

const char kDitherVertexShader[] = R"(
#version 120

// Attributes.
attribute vec2 attr_position;
attribute vec2 attr_monotonically_increasing;
attribute vec2 attr_texture_position;

// Output of vertex shader.
varying vec2 var_texture_position;
varying vec2 var_dither_texture_position;

void main() {
	var_texture_position = attr_texture_position;
	var_dither_texture_position = attr_monotonically_increasing;
	vec4 p = vec4(attr_position, 0., 1.);
	gl_Position = gl_ProjectionMatrix * p;
}
)";

const char kDitherFragmentShader[] = R"(
#version 120

uniform sampler2D u_dither_texture;
uniform sampler2D u_dither_terrain_texture;

varying vec2 var_texture_position;
varying vec2 var_dither_texture_position;

void main() {
	gl_FragColor = texture2D(u_dither_texture, var_dither_texture_position);
}
)";

// Helper struct that contains the minimum and maximum fields for rendering.
struct Patch {
	Patch(int init_minfx, int init_maxfx, int init_minfy, int init_maxfy) {
		minfx = init_minfx;
		maxfx = init_maxfx;
		minfy = init_minfy;
		maxfy = init_maxfy;
		w = (maxfx - minfx + 1);
		h = (maxfy - minfy + 1);
	}

	// Calculates the index of the given field with (x, y) being geometric
	// coordinates in the map. Returns -1 if this field is not in the patch.
	inline int calculate_index(int x, int y) const {
		uint16_t xidx = x - minfx;
		if (xidx >= w) {
			return -1;
		}
		uint16_t yidx = y - minfy;
		if (yidx >= h) {
			return -1;
		}
		return yidx * w + xidx;
	}

	// Minimum and maximum field coordinates (geometric) to render. Can be negative.
	int minfx;
	int maxfx;
	int minfy;
	int maxfy;

	// Width and height in number of fields.
	int w, h;
};

// NOCOM(#sirver): only do this once.
const GLSurfaceTexture * get_dither_edge_texture()
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


}  // namespace

// The workhorse that doe the actual rendering.
// Schematic of triangle neighborhood:
//
//               *
//              / \
//             / u \
//         (f)/     \
//    *------*------* (r)
//     \  l / \  r / \
//      \  /   \  /   \
//       \/  d  \/ rr  \
//       *------*------* (br)
//        \ dd /
//         \  /
//          \/
//          *
//
// Each field (f) owns two triangles: (r)ight & (d)own. When we look at the
// field, we have to make sure to schedule drawing the triangles.
//
// To draw dithered edges, we have to look at the neighboring triangles for the
// two triangles too: If a neighboring triangle has another texture and our our
// dither layer is larger, we have to draw a dithering triangle too. The dither
// triangle is the triangle that should be partially overdrawn (i.e. if r and d
// have different textures and r.dither_layer > d.dither_layer, then we will
// repaint d with the dither texture as mask.
class TerrainProgram {
	public:
	// Compiles the program. Throws on errors.
	TerrainProgram();

	// Draws the terrain.
	void draw_terrain(const uint32_t gametime,
	                  const Map& map,
	                  const Player* player,
	                  const DescriptionMaintainer<TerrainDescription>& terrains,
	                  const Patch& patch,
	                  const Point& surface_offset);

private:
	void draw_terrain_triangles(int num_vertices,
	                            const DescriptionMaintainer<TerrainDescription>& terrains);
	void draw_terrain_dither(int num_vertices,
	                         const DescriptionMaintainer<TerrainDescription>& terrains);

	static constexpr int kAttribVertexPosition = 0;
	static constexpr int kAttribVertexBrightness = 1;
	static constexpr int kAttribMonotonicallyIncreasing = 1;
	static constexpr int kAttribVertexTexturePosition = 2;
	// NOCOM(#sirver): split into more arrays to not copy unneded information.
	struct PerVertexData {
		float x;
		float y;
		float brightness;
		float texture_x;
		float texture_y;
	};
	static_assert(sizeof(PerVertexData) == 20, "Wrong padding.");

	// Returns the brightness value in [0, 1.] for 'fcoords' at 'gametime' for
	// 'player' (which can be nullptr).
	float field_brightness(const FCoords& fcoords,
	                         const uint32_t gametime,
	                         const Map& map,
	                         const Player* player) const;

	// The buffer that will contain 'vertices_' for rendering.
	Gl::Buffer gl_program_data_buffer_;

	// The program used for drawing the terrain.
	Gl::Program terrain_gl_program_;

	// Uniforms.
	GLint u_terrain_texture_;

	// The program used for drawing the terrain.
	Gl::Program dither_gl_program_;

	// Uniforms.
	GLint u_dither_terrain_texture_;
	GLint u_dither_texture_;

	// Objects below are kept around to avoid memory allocations on each frame.
	// They could theoretically also be recreated.

	// All vertices that are going to get rendered this frame.
	std::vector<TerrainProgram::PerVertexData> vertices_;

	// A map from terrain index in world.terrains() to indices in 'vertices_'
	// that have this terrain type.
	std::vector<std::vector<uint16_t>> terrains_to_indices_;
	std::vector<std::vector<uint16_t>> dither_layer_terrains_to_indices_;
	Gl::Buffer monotonously_increasing_buffer_;


	DISALLOW_COPY_AND_ASSIGN(TerrainProgram);
};

TerrainProgram::TerrainProgram() {
	{  // terrain program
		terrain_gl_program_.compile(kTerrainVertexShader, kTerrainFragmentShader);

		glBindAttribLocation(terrain_gl_program_.object(), kAttribVertexPosition, "attr_position");
		glBindAttribLocation(
		   terrain_gl_program_.object(), kAttribVertexBrightness, "attr_brightness");
		glBindAttribLocation(
		   terrain_gl_program_.object(), kAttribVertexTexturePosition, "attr_texture_position");

		terrain_gl_program_.link();

		glEnableVertexAttribArray(kAttribVertexPosition);
		glEnableVertexAttribArray(kAttribVertexBrightness);
		glEnableVertexAttribArray(kAttribVertexTexturePosition);

		u_terrain_texture_ = glGetUniformLocation(terrain_gl_program_.object(), "u_terrain_texture");
	}

	{ // dither program
		dither_gl_program_.compile(kDitherVertexShader, kDitherFragmentShader);

		glBindAttribLocation(dither_gl_program_.object(), kAttribVertexPosition, "attr_position");
		glBindAttribLocation(dither_gl_program_.object(), kAttribMonotonicallyIncreasing, "attr_monotonically_increasing");
		glBindAttribLocation(
		   terrain_gl_program_.object(), kAttribVertexTexturePosition, "attr_texture_position");

		dither_gl_program_.link();

		glEnableVertexAttribArray(kAttribVertexPosition);
		glEnableVertexAttribArray(kAttribMonotonicallyIncreasing);
		glEnableVertexAttribArray(kAttribVertexTexturePosition);

		u_dither_texture_ = glGetUniformLocation(dither_gl_program_.object(), "u_dither_texture");
		u_dither_terrain_texture_ = glGetUniformLocation(dither_gl_program_.object(), "u_dither_terrain_texture");
	}
}

float TerrainProgram::field_brightness(const FCoords& fcoords,
                                       const uint32_t gametime,
                                       const Map& map,
                                       const Player* const player) const {
	uint32_t brightness = 144 + fcoords.field->get_brightness();
	brightness = std::min<uint32_t>(255, (brightness * 255) / 160);

	if (player && !player->see_all()) {
		const Player::Field& pf = player->fields()[Map::get_index(fcoords, map.get_width())];
		if (pf.vision == 0) {
			return 0.;
		} else if (pf.vision == 1) {
			static const uint32_t kDecayTimeInMs = 20000;
			const Duration time_ago = gametime - pf.time_node_last_unseen;
			if (time_ago < kDecayTimeInMs) {
				brightness = (brightness * (2 * kDecayTimeInMs - time_ago)) / (2 * kDecayTimeInMs);
			} else {
				brightness = brightness / 2;
			}
		}
	}
	return brightness / 255.;
}

struct Coordinate {
	float x;
	float y;
};

void
TerrainProgram::draw_terrain_dither(int num_vertices, const DescriptionMaintainer<TerrainDescription>& terrains) {
	glUseProgram(dither_gl_program_.object());

	// NOCOM(#sirver): Name makes no sense anymore.
	std::vector<Coordinate> monotonically_increasing(num_vertices);
	for (int i = 0; i < num_vertices; ++i) {
		switch (i % 3) {
			case 0:
				monotonically_increasing[i] = Coordinate{0., 0.};
				break;
			case 1:
				monotonically_increasing[i] = Coordinate{1., 0.};
				break;
			case 2:
				monotonically_increasing[i] = Coordinate{0.5, 1.};
				break;
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, monotonously_increasing_buffer_.object());
	glBufferData(GL_ARRAY_BUFFER,
	             sizeof(Coordinate) * num_vertices,
	             monotonically_increasing.data(),
	             GL_STREAM_DRAW);

	glVertexAttribPointer(
	   kAttribMonotonicallyIncreasing, 2, GL_FLOAT, GL_FALSE, sizeof(Coordinate), 0);

	glBindBuffer(GL_ARRAY_BUFFER, gl_program_data_buffer_.object());
	const auto set_attrib_pointer = [](const int vertex_index, int num_items, int offset) {
		glVertexAttribPointer(vertex_index,
		                      num_items,
		                      GL_FLOAT,
		                      GL_FALSE,
		                      sizeof(TerrainProgram::PerVertexData),
		                      reinterpret_cast<void*>(offset));
	};
	set_attrib_pointer(kAttribVertexPosition, 2, offsetof(PerVertexData, x));
	set_attrib_pointer(kAttribVertexTexturePosition, 2, offsetof(PerVertexData, texture_x));

	// Set the sampler texture unit to 0
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(u_dither_texture_, 0);
	glBindTexture(GL_TEXTURE_2D, get_dither_edge_texture()->get_gl_texture());

	glActiveTexture(GL_TEXTURE1);
	glUniform1i(u_dither_terrain_texture_, 1);

	// Which triangles to draw?
	for (size_t i = 0; i < dither_layer_terrains_to_indices_.size(); ++i) {
		const auto& indices = dither_layer_terrains_to_indices_[i];
		if (indices.empty()) {
			continue;
		}
		glBindTexture(
			GL_TEXTURE_2D,
			g_gr->get_maptexture_data(terrains.get_unmutable(i).get_texture())->getTexture());
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, indices.data());
	}

	// Release Program object.
	glUseProgram(0);
}

void
TerrainProgram::draw_terrain_triangles(int num_vertices,
                                       const DescriptionMaintainer<TerrainDescription>& terrains) {
	glUseProgram(terrain_gl_program_.object());

	glBindBuffer(GL_ARRAY_BUFFER, gl_program_data_buffer_.object());
	glBufferData(GL_ARRAY_BUFFER,
	             sizeof(TerrainProgram::PerVertexData) * num_vertices,
	             vertices_.data(),
	             GL_STREAM_DRAW);

	const auto set_attrib_pointer = [](const int vertex_index, int num_items, int offset) {
		glVertexAttribPointer(vertex_index,
		                      num_items,
		                      GL_FLOAT,
		                      GL_FALSE,
		                      sizeof(TerrainProgram::PerVertexData),
		                      reinterpret_cast<void*>(offset));
	};
	set_attrib_pointer(kAttribVertexPosition, 2, offsetof(PerVertexData, x));
	set_attrib_pointer(kAttribVertexBrightness, 1, offsetof(PerVertexData, brightness));
	set_attrib_pointer(kAttribVertexTexturePosition, 2, offsetof(PerVertexData, texture_x));

	// Set the sampler texture unit to 0
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(u_terrain_texture_, 0);

	// Which triangles to draw?
	for (size_t i = 0; i < terrains_to_indices_.size(); ++i) {
		const auto& indices = terrains_to_indices_[i];
		if (indices.empty()) {
			continue;
		}
		glBindTexture(
		   GL_TEXTURE_2D,
		   g_gr->get_maptexture_data(terrains.get_unmutable(i).get_texture())->getTexture());
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, indices.data());
	}

	// Release Program object.
	glUseProgram(0);
}

void TerrainProgram::draw_terrain(const uint32_t gametime,
                                  const Map& map,
                                  const Player* player,
                                  const DescriptionMaintainer<TerrainDescription>& terrains,
                                  const Patch& patch,
                                  const Point& surface_offset) {
	const size_t num_vertices = patch.w * patch.h;
	if (vertices_.size() < num_vertices) {
		vertices_.resize(num_vertices);
		terrains_to_indices_.resize(terrains.size());
		dither_layer_terrains_to_indices_.resize(terrains.size());
	}
	for (auto& container : terrains_to_indices_) {
		container.clear();
		container.reserve(num_vertices);
	}
	for (auto& container : dither_layer_terrains_to_indices_) {
		container.clear();
		container.reserve(num_vertices);
	}

	const auto& potentially_add_dithering_triangle =
	   [&terrains, this](int idx1, int idx2, int idx3, int my_terrain, int other_terrain) {
		if (idx1 == -1 || idx2 == -1 || idx3 == -1 || my_terrain == other_terrain) {
			return;
		}
		if (terrains.get_unmutable(my_terrain).dither_layer() <
		    terrains.get_unmutable(other_terrain).dither_layer()) {
			dither_layer_terrains_to_indices_[other_terrain].push_back(idx1);
			dither_layer_terrains_to_indices_[other_terrain].push_back(idx2);
			dither_layer_terrains_to_indices_[other_terrain].push_back(idx3);
		}
	};

	const auto& get_field = [&map](const int fx, const int fy) {
		Coords coords(fx, fy);
		map.normalize_coords(coords);
		return map.get_fcoords(coords);
	};

	int current_index = 0;
	Coords geometric_coords;
	for (geometric_coords.y = patch.minfy; geometric_coords.y <= patch.maxfy; ++geometric_coords.y) {
		for (geometric_coords.x = patch.minfx; geometric_coords.x <= patch.maxfx;
		     ++geometric_coords.x, ++current_index) {
			Coords normalized_coords(geometric_coords);
			map.normalize_coords(normalized_coords);
			const FCoords fcoords = map.get_fcoords(normalized_coords);

			{
				PerVertexData& vertex = vertices_[current_index];
				int x, y;
				MapviewPixelFunctions::get_basepix(geometric_coords, x, y);
				vertex.texture_x = float(x) / TEXTURE_WIDTH;
				vertex.texture_y = float(y) / TEXTURE_HEIGHT;
				vertex.x = x + surface_offset.x;
				vertex.y = y + surface_offset.y;

				// Correct for the height of the field.
				vertex.y -= fcoords.field->get_height() * HEIGHT_FACTOR;

				// Figure out the brightness of the field.
				vertex.brightness = field_brightness(fcoords, gametime, map, player);
			}

			// The bottom right neighbor patch is needed for both triangles
			// associated with this field. If it is not in patch, there is no need
			// to draw any triangles.
			int brn_index = patch.calculate_index(
			   geometric_coords.x + (geometric_coords.y & 1), geometric_coords.y + 1);
			if (brn_index == -1) {
				continue;
			}

			// Down triangle.
			int bln_index = patch.calculate_index(
			   geometric_coords.x + (geometric_coords.y & 1) - 1, geometric_coords.y + 1);
			const int terrain_d = fcoords.field->terrain_d();
			if (bln_index != -1) {
				terrains_to_indices_[terrain_d].push_back(current_index);
				terrains_to_indices_[terrain_d].push_back(brn_index);
				terrains_to_indices_[terrain_d].push_back(bln_index);
			}

			// Right triangle.
			int rn_index = patch.calculate_index(geometric_coords.x + 1, geometric_coords.y);
			const int terrain_r = fcoords.field->terrain_r();
			if (rn_index != -1) {
				terrains_to_indices_[terrain_r].push_back(current_index);
				terrains_to_indices_[terrain_r].push_back(rn_index);
				terrains_to_indices_[terrain_r].push_back(brn_index);
			}

			// Dithering triangles.
			potentially_add_dithering_triangle(
					current_index, brn_index, rn_index, terrain_r, terrain_d);
			// int trn_index = patch.calculate_index(
				// geometric_coords.x + (geometric_coords.y & 1), geometric_coords.y - 1);
			// if (trn_index != -1) {
				// // NOCOM(#sirver): duplicated logic (index math)
				// potentially_add_dithering_triangle(
					// current_index,
					// rn_index,
					// brn_index,
					// terrain_r,
					// get_field(geometric_coords.x + (geometric_coords.y & 1), geometric_coords.y - 1)
						// .field->terrain_d());
			// }
			// if (rn_index != -1) {
				// potentially_add_dithering_triangle(
					// rn_index,
					// brn_index,
					// current_index,
					// terrain_r,
					// get_field(geometric_coords.x + 1, geometric_coords.y).field->terrain_d());
			// }

			// NOCOM(#sirver): add the triangles for d

		}
	}

	draw_terrain_triangles(num_vertices, terrains);
	draw_terrain_dither(num_vertices, terrains);
}

static const uint32_t PatchSize = 4;

std::unique_ptr<TerrainProgram> GameRendererGL::terrain_program_;

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
	uint32_t brightness = 144 + coords.field->get_brightness();
	brightness = std::min<uint32_t>(255, (brightness * 255) / 160);

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


void GameRendererGL::draw() {
	if (terrain_program_ == nullptr) {
		terrain_program_.reset(new TerrainProgram());
	}

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

	m_patch_size.x = m_minfx - 1;
	m_patch_size.y = m_minfy;
	m_patch_size.w = ((m_maxfx - m_minfx + 2 + PatchSize) / PatchSize) * PatchSize;
	m_patch_size.h = ((m_maxfy - m_minfy + 1 + PatchSize) / PatchSize) * PatchSize;

	glClear(GL_COLOR_BUFFER_BIT);

	glScissor
		(m_rect.x, m_surface->height() - m_rect.y - m_rect.h,
		 m_rect.w, m_rect.h);
	glEnable(GL_SCISSOR_TEST);

	const uint32_t gametime = m_egbase->get_gametime();
	const Patch patch(m_minfx, m_maxfx, m_minfy, m_maxfy);
	terrain_program_->draw_terrain(
		gametime, m_egbase->map(), m_player, world.terrains(), patch, m_surface_offset);

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
	const FCoords fcoords = map.get_fcoords(ncoords);
	Point pix;
	MapviewPixelFunctions::get_basepix(coords, pix.x, pix.y);
	{
		vtx.tcx = float(pix.x) / TEXTURE_WIDTH;
		vtx.tcy = float(pix.y) / TEXTURE_HEIGHT;
	}
	{
		pix.y -= fcoords.field->get_height() * HEIGHT_FACTOR;
		pix += m_surface_offset;
		vtx.x = pix.x;
		vtx.y = pix.y;
	}
	{
		uint8_t brightness = field_brightness(fcoords);
		vtx.color[0] = vtx.color[1] = vtx.color[2] = brightness;
		vtx.color[3] = 255;
	}
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
	// glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
	// glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_REPLACE);
	// glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS);
	// glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
	// glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_REPLACE);
	// glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE);
	// glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);

	// glEnable(GL_BLEND);
	// glBlendFunc(GL_ONE_MINUS_SR_ALPHA, GL_SRC_ALPHA);

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
