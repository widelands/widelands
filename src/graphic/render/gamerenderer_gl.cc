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

// QuickRef:
// http://www.cs.unh.edu/~cs770/docs/glsl-1.20-quickref.pdf
// Full specification:
// http://www.opengl.org/registry/doc/GLSLangSpec.Full.1.20.8.pdf
// We target OpenGL 2.1 for the desktop here.
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
	vec4 clr = texture2D(u_terrain_texture, var_texture_position);
	clr.rgb *= var_brightness;
	gl_FragColor = clr;
}
)";

const char kDitherVertexShader[] = R"(
#version 120

// Attributes.
attribute float attr_brightness;
attribute vec2 attr_position;
attribute vec2 attr_texture_position;
attribute vec2 attr_dither_texture_position;

// Output of vertex shader.
varying vec2 var_texture_position;
varying vec2 var_dither_texture_position;
varying float var_brightness;

void main() {
	var_texture_position = attr_texture_position;
	var_dither_texture_position = attr_dither_texture_position;
	var_brightness = attr_brightness;
	vec4 p = vec4(attr_position, 0., 1.);
	gl_Position = gl_ProjectionMatrix * p;
}
)";

const char kDitherFragmentShader[] = R"(
#version 120

uniform sampler2D u_dither_texture;
uniform sampler2D u_terrain_texture;

varying float var_brightness;
varying vec2 var_texture_position;
varying vec2 var_dither_texture_position;

void main() {
	vec4 clr = texture2D(u_terrain_texture, var_texture_position);
	clr.rgb *= var_brightness;
	clr.a = 1. - texture2D(u_dither_texture, var_dither_texture_position).a;
	gl_FragColor = clr;
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

	// Returns true if the geometric coordinates (x, y) are inside this field.
	inline int contains(int x, int y) const {
		return calculate_index(x, y) != -1;
	}

	// Minimum and maximum field coordinates (geometric) to render. Can be negative.
	int minfx;
	int maxfx;
	int minfy;
	int maxfy;

	// Width and height in number of fields.
	int w, h;

	struct Field {
		Coords geometric_coords;
		FCoords fcoords;
		float x, y;  // Pixel relative to top left.
		float texture_x, texture_y;  // Texture coordinates.
		float brightness; // brightness of the pixel
	};
	std::vector<Field> fields;
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

// NOCOM(#sirver): comment
float field_brightness(const FCoords& fcoords,
                                       const uint32_t gametime,
                                       const Map& map,
                                       const Player* const player) {
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


}  // namespace

// Explanation of how drawing works:
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
// field, we have to make sure to schedule drawing the triangles. This is done
// by of these triangles is done by TerrainProgram.
//
// To draw dithered edges, we have to look at the neighboring triangles for the
// two triangles too: If a neighboring triangle has another texture and our
// dither layer is smaller, we have to draw a dithering triangle too - this lets the neighboring texture
// bleed into our triangle.
//
// The dither triangle is the triangle that should be partially (either r or
// d). Example: if r and d have different textures and r.dither_layer >
// d.dither_layer, then we will repaint d with the dither texture as mask.

class DitherProgram {
	public:
		DitherProgram();

	// Draws the terrain.
	void draw(const uint32_t gametime,
	                  const Map& map,
	                  const Player* player,
	                  const DescriptionMaintainer<TerrainDescription>& terrains,
	                  const Patch& patch,
	                  const Point& surface_offset);

private:
	// NOCOM(#sirver): comment
	Gl::Buffer gl_array_buffer_;

	// The program used for drawing the terrain.
	Gl::Program dither_gl_program_;

	// Attributes.
	GLint attr_position_;
	GLint attr_texture_position_;
	GLint attr_dither_texture_position_;
	GLint attr_brightness_;

	// Uniforms.
	GLint u_terrain_texture_;
	GLint u_dither_texture_;
};

class TerrainProgram {
	public:
	// Compiles the program. Throws on errors.
	TerrainProgram();

	// Draws the terrain.
	void draw(const DescriptionMaintainer<TerrainDescription>& terrains, const Patch& patch);

private:
	void gl_draw(int num_vertices,
	                            const DescriptionMaintainer<TerrainDescription>& terrains);

	// NOCOM(#sirver): split into more arrays to not copy unneded information.
	struct PerVertexData {
		float x;
		float y;
		float brightness;
		float texture_x;
		float texture_y;
	};
	static_assert(sizeof(PerVertexData) == 20, "Wrong padding.");

	// The buffer that will contain 'vertices_' for rendering.
	Gl::Buffer gl_program_data_buffer_;

	// The program used for drawing the terrain.
	Gl::Program terrain_gl_program_;

	// Attributes.
	GLint attr_position_;
	GLint attr_texture_position_;
	GLint attr_brightness_;

	// Uniforms.
	GLint u_terrain_texture_;

	// Objects below are kept around to avoid memory allocations on each frame.
	// They could theoretically also be recreated.

	// All vertices that are going to get rendered this frame.
	std::vector<TerrainProgram::PerVertexData> vertices_;

	// A map from terrain index in world.terrains() to indices in 'vertices_'
	// that have this terrain type.
	std::vector<std::vector<uint16_t>> terrains_to_indices_;

	DISALLOW_COPY_AND_ASSIGN(TerrainProgram);
};

TerrainProgram::TerrainProgram() {
		// NOCOM(#sirver): merge compile & link
		terrain_gl_program_.compile(kTerrainVertexShader, kTerrainFragmentShader);
		terrain_gl_program_.link();

		attr_position_ = glGetAttribLocation(terrain_gl_program_.object(), "attr_position");
		attr_texture_position_ = glGetAttribLocation(terrain_gl_program_.object(), "attr_texture_position");
		attr_brightness_ = glGetAttribLocation(terrain_gl_program_.object(), "attr_brightness");

		u_terrain_texture_ = glGetUniformLocation(terrain_gl_program_.object(), "u_terrain_texture");
}

void
TerrainProgram::gl_draw(int num_vertices,
                                       const DescriptionMaintainer<TerrainDescription>& terrains) {
	glUseProgram(terrain_gl_program_.object());

	glEnableVertexAttribArray(attr_position_);
	glEnableVertexAttribArray(attr_texture_position_);
	glEnableVertexAttribArray(attr_brightness_);

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
	set_attrib_pointer(attr_position_, 2, offsetof(PerVertexData, x));
	set_attrib_pointer(attr_brightness_, 1, offsetof(PerVertexData, brightness));
	set_attrib_pointer(attr_texture_position_, 2, offsetof(PerVertexData, texture_x));

	glBindBuffer(GL_ARRAY_BUFFER, 0);

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

	glDisableVertexAttribArray(attr_position_);
	glDisableVertexAttribArray(attr_texture_position_);
	glDisableVertexAttribArray(attr_brightness_);

	glUseProgram(0);
}

void TerrainProgram::draw(const DescriptionMaintainer<TerrainDescription>& terrains,
                          const Patch& patch) {
	const size_t num_vertices = patch.w * patch.h;
	if (vertices_.size() < num_vertices) {
		vertices_.resize(num_vertices);
		terrains_to_indices_.resize(terrains.size());
	}
	for (auto& container : terrains_to_indices_) {
		container.clear();
		container.reserve(num_vertices);
	}

	for (size_t current_index = 0; current_index < num_vertices; ++current_index) {
		const Patch::Field& field = patch.fields[current_index];

		PerVertexData& vertex = vertices_[current_index];
		vertex.texture_x = field.texture_x;
		vertex.texture_y = field.texture_y;
		vertex.x = field.x;
		vertex.y = field.y;
		vertex.brightness = field.brightness;

		// The bottom right neighbor patch is needed for both triangles
		// associated with this field. If it is not in patch, there is no need to
		// draw any triangles.
		int brn_index = patch.calculate_index(
		   field.geometric_coords.x + (field.geometric_coords.y & 1), field.geometric_coords.y + 1);
		if (brn_index == -1) {
			continue;
		}

		// Down triangle.
		int bln_index =
		   patch.calculate_index(field.geometric_coords.x + (field.geometric_coords.y & 1) - 1,
		                         field.geometric_coords.y + 1);
		const int terrain_d = field.fcoords.field->terrain_d();
		if (bln_index != -1) {
			terrains_to_indices_[terrain_d].push_back(current_index);
			terrains_to_indices_[terrain_d].push_back(bln_index);
			terrains_to_indices_[terrain_d].push_back(brn_index);
		}

		// Right triangle.
		int rn_index = patch.calculate_index(field.geometric_coords.x + 1, field.geometric_coords.y);
		const int terrain_r = field.fcoords.field->terrain_r();
		if (rn_index != -1) {
			terrains_to_indices_[terrain_r].push_back(current_index);
			terrains_to_indices_[terrain_r].push_back(brn_index);
			terrains_to_indices_[terrain_r].push_back(rn_index);
		}
	}

	gl_draw(num_vertices, terrains);
}

// NOCOM(#sirver): move into program.
struct DitherData {
	float x;
	float y;
	float texture_x;
	float texture_y;
	float brightness;
	float dither_texture_x;
	float dither_texture_y;
};

struct Vertex {
	Coords geometric_coords;
	FCoords fcoords;
	int patch_index;
};

enum {
	kThis,
	kRight,
	kBottomRight,
	kBottomLeft,
	kLeft,
	kTopLeft,
	kTopRight,
};

DitherProgram::DitherProgram() {
	dither_gl_program_.compile(kDitherVertexShader, kDitherFragmentShader);
	dither_gl_program_.link();

	attr_brightness_ = glGetAttribLocation(dither_gl_program_.object(), "attr_brightness");
	attr_dither_texture_position_ = glGetAttribLocation(dither_gl_program_.object(), "attr_dither_texture_position");
	attr_position_ = glGetAttribLocation(dither_gl_program_.object(), "attr_position");
	attr_texture_position_ = glGetAttribLocation(dither_gl_program_.object(), "attr_texture_position");

	u_dither_texture_ = glGetUniformLocation(dither_gl_program_.object(), "u_dither_texture");
	u_terrain_texture_ =
	   glGetUniformLocation(dither_gl_program_.object(), "u_terrain_texture");
}

void DitherProgram::draw(const uint32_t gametime,
                         const Map& map,
                         const Player* player,
                         const DescriptionMaintainer<TerrainDescription>& terrains,
                         const Patch& patch,
                         const Point& surface_offset) {
	glUseProgram(dither_gl_program_.object());

	glEnableVertexAttribArray(attr_brightness_);
	glEnableVertexAttribArray(attr_dither_texture_position_);
	glEnableVertexAttribArray(attr_position_);
	glEnableVertexAttribArray(attr_texture_position_);

	Vertex verts[7];

	// NOCOM(#sirver): do not recreate everytime.
	std::vector<std::vector<DitherData>> data;
	data.resize(terrains.size());

	const auto add_vertex =
	   [&verts, &data, &surface_offset, &map, &player, &gametime](int index, int order_index, int terrain) {
		const Vertex& v = verts[index];

		data[terrain].emplace_back();
		DitherData& back = data[terrain].back();

		int x, y;
		MapviewPixelFunctions::get_basepix(v.geometric_coords, x, y);
		back.texture_x = float(x) / TEXTURE_WIDTH;
		back.texture_y = float(y) / TEXTURE_HEIGHT;
		back.x = x + surface_offset.x;
		back.y = y + surface_offset.y;

		// Figure out the brightness of the field.
		back.brightness = field_brightness(v.fcoords, gametime, map, player);

		// Correct for the height of the field.
		back.y -= v.fcoords.field->get_height() * HEIGHT_FACTOR;
		switch (order_index) {
		case 0:
			back.dither_texture_x = 0.;
			back.dither_texture_y = 0.;
			break;
		case 1:
			back.dither_texture_x = 1.;
			back.dither_texture_y = 0.;
			break;
		case 2:
			back.dither_texture_x = 0.5;
			back.dither_texture_y = 1.;
			break;
		}
	};

	const auto potentially_add_dithering_triangle = [&terrains, &verts, &add_vertex](
	   int idx1, int idx2, int idx3, int my_terrain, int other_terrain) {
		const Vertex& v1 = verts[idx1];
		const Vertex& v2 = verts[idx2];
		const Vertex& v3 = verts[idx3];
		if (v1.patch_index == -1 || v2.patch_index == -1 || v3.patch_index == -1 ||
		    my_terrain == other_terrain) {
			return;
		}
		if (terrains.get_unmutable(my_terrain).dither_layer() <
		    terrains.get_unmutable(other_terrain).dither_layer()) {
			add_vertex(idx1, 0, other_terrain);
			add_vertex(idx2, 1, other_terrain);
			add_vertex(idx3, 2, other_terrain);
		}
	};

	const auto fill_vertex = [&verts, &map, &patch](int index, const Coords& geometric_coords) {
		Vertex& vertex = verts[index];
		vertex.geometric_coords = geometric_coords;

		Coords normalized_coords(geometric_coords);
		map.normalize_coords(normalized_coords);
		vertex.fcoords = map.get_fcoords(normalized_coords);

		vertex.patch_index = patch.calculate_index(geometric_coords.x, geometric_coords.y);
	};

	Coords geometric_coords;
	for (geometric_coords.y = patch.minfy; geometric_coords.y <= patch.maxfy; ++geometric_coords.y) {
		for (geometric_coords.x = patch.minfx; geometric_coords.x <= patch.maxfx;
		     ++geometric_coords.x) {
			fill_vertex(kThis, geometric_coords);
			fill_vertex(kRight, Coords(geometric_coords.x + 1, geometric_coords.y));
			fill_vertex(kBottomRight,
			            Coords(geometric_coords.x + (geometric_coords.y & 1), geometric_coords.y + 1));
			fill_vertex(
			   kBottomLeft,
			   Coords(geometric_coords.x + (geometric_coords.y & 1) - 1, geometric_coords.y + 1));
			fill_vertex(kLeft, Coords(geometric_coords.x - 1, geometric_coords.y));
			fill_vertex(
			   kTopLeft,
			   Coords(geometric_coords.x + (geometric_coords.y & 1) - 1, geometric_coords.y - 1));
			fill_vertex(kTopRight,
			            Coords(geometric_coords.x + (geometric_coords.y & 1), geometric_coords.y - 1));

			// Dithering triangles for Right triangle.
			const int terrain_r = verts[kThis].fcoords.field->terrain_r();
			const int terrain_d = verts[kThis].fcoords.field->terrain_d();
			potentially_add_dithering_triangle(kThis, kBottomRight, kRight, terrain_r, terrain_d);
			if (verts[kTopRight].patch_index != -1) {
				int terrain_u = verts[kTopRight].fcoords.field->terrain_d();
				potentially_add_dithering_triangle(kRight, kThis, kBottomRight, terrain_r, terrain_u);
			}
			if (verts[kRight].patch_index != -1) {
				int terrain_rr = verts[kRight].fcoords.field->terrain_d();
				potentially_add_dithering_triangle(kBottomRight, kRight, kThis, terrain_r, terrain_rr);
			}

			// Dithering triangles for Down triangle.
			potentially_add_dithering_triangle(kBottomRight, kThis, kBottomLeft, terrain_d, terrain_r);
			if (verts[kBottomLeft].patch_index != -1) {
				const int terrain_dd = verts[kBottomLeft].fcoords.field->terrain_r();
				potentially_add_dithering_triangle(
				   kBottomLeft, kBottomRight, kThis, terrain_d, terrain_dd);
			}
			if (verts[kLeft].patch_index != -1) {
				const int terrain_l = verts[kLeft].fcoords.field->terrain_r();
				potentially_add_dithering_triangle(
				   kThis, kBottomLeft, kBottomRight, terrain_d, terrain_l);
			}
		}
	}

	// Set the sampler texture unit to 0
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(u_dither_texture_, 0);
	glBindTexture(GL_TEXTURE_2D, get_dither_edge_texture()->get_gl_texture());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glActiveTexture(GL_TEXTURE1);
	glUniform1i(u_terrain_texture_, 1);

	// Which triangles to draw?
	glBindBuffer(GL_ARRAY_BUFFER, gl_array_buffer_.object());
	for (size_t i = 0; i < data.size(); ++i) {
		const auto& current_data = data[i];
		if (current_data.empty()) {
			continue;
		}
		glBindTexture(
		   GL_TEXTURE_2D,
		   g_gr->get_maptexture_data(terrains.get_unmutable(i).get_texture())->getTexture());

		glBufferData(GL_ARRAY_BUFFER,
		             sizeof(DitherData) * current_data.size(),
		             current_data.data(),
		             GL_STREAM_DRAW);

		const auto set_attrib_pointer = [](const int vertex_index, int num_items, int offset) {
			glVertexAttribPointer(vertex_index,
			                      num_items,
			                      GL_FLOAT,
			                      GL_FALSE,
			                      sizeof(DitherData),
			                      reinterpret_cast<void*>(offset));
		};
		set_attrib_pointer(attr_brightness_, 1, offsetof(DitherData, brightness));
		set_attrib_pointer(attr_dither_texture_position_, 2, offsetof(DitherData, dither_texture_x));
		set_attrib_pointer(attr_position_, 2, offsetof(DitherData, x));
		set_attrib_pointer(attr_texture_position_, 2, offsetof(DitherData, texture_x));

		glDrawArrays(GL_TRIANGLES, 0, current_data.size());
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(attr_brightness_);
	glDisableVertexAttribArray(attr_dither_texture_position_);
	glDisableVertexAttribArray(attr_position_);
	glDisableVertexAttribArray(attr_texture_position_);

	glActiveTexture(GL_TEXTURE0);

	// Release Program object.
	glUseProgram(0);
}


std::unique_ptr<TerrainProgram> GameRendererGL::terrain_program_;
std::unique_ptr<DitherProgram> GameRendererGL::dither_program_;

GameRendererGL::GameRendererGL() :
	m_road_vertices_size(0)
{
}

GameRendererGL::~GameRendererGL()
{
}


// Returns the brightness value in [0, 1.] for 'fcoords' at 'gametime' for
// 'player' (which can be nullptr).
uint8_t GameRendererGL::field_brightness(const FCoords& coords) const {
	uint32_t brightness = 144 + coords.field->get_brightness();
	brightness = std::min<uint32_t>(255, (brightness * 255) / 160);

	if (m_player && !m_player->see_all()) {
		const Map& map = m_egbase->map();
		const Player::Field& pf = m_player->fields()[Map::get_index(coords, map.get_width())];
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
		dither_program_.reset(new DitherProgram());
	}

	m_surface = dynamic_cast<GLSurface *>(m_dst->get_surface());
	if (!m_surface)
		return;
	m_rect = m_dst->get_rect();
	m_surface_offset = m_dst_offset + m_rect.top_left() + m_dst->get_offset();

	glClear(GL_COLOR_BUFFER_BIT);

	glScissor
		(m_rect.x, m_surface->height() - m_rect.y - m_rect.h,
		 m_rect.w, m_rect.h);
	glEnable(GL_SCISSOR_TEST);

	Map& map = m_egbase->map();
	const uint32_t gametime = m_egbase->get_gametime();

	Patch patch(m_minfx, m_maxfx, m_minfy, m_maxfy);
	for (int32_t fy = m_minfy; fy <= m_maxfy; ++fy) {
		for (int32_t fx = m_minfx; fx <= m_maxfx; ++fx) {
			patch.fields.emplace_back();
			Patch::Field& f = patch.fields.back();

			f.geometric_coords = Coords(fx, fy);

			Coords ncoords(f.geometric_coords);
			map.normalize_coords(ncoords);
			f.fcoords = map.get_fcoords(ncoords);

			int x, y;
			MapviewPixelFunctions::get_basepix(f.geometric_coords, x, y);

			f.texture_x = float(x) / TEXTURE_WIDTH;
			f.texture_y = float(y) / TEXTURE_HEIGHT;
			f.x = x + m_surface_offset.x;
			f.y = y + m_surface_offset.y;

			// Correct for the height of the field.
			// NOCOM(#sirver): how often is fcoords needed after this?
			f.y -= f.fcoords.field->get_height() * HEIGHT_FACTOR;

			// Figure out the brightness of the field.
			f.brightness = ::field_brightness(f.fcoords, gametime, map, m_player);
		}
	}

	const World& world = m_egbase->world();
	terrain_program_->draw(world.terrains(), patch);
	// dither_program_->draw(
		// gametime, map, m_player, world.terrains(), patch, m_surface_offset);

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
