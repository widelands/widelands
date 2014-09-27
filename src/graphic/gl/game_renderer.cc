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

#include "graphic/gl/game_renderer.h"

#include <memory>

#include <SDL_image.h>

#include "graphic/gl/surface.h"
#include "graphic/gl/utils.h"
#include "graphic/graphic.h"
#include "graphic/image_io.h"
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

namespace {

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

// Helper struct that contains the data needed for drawing all fields.
class FieldsToDraw {
public:
	struct Field {
		int fx, fy;  // geometric coordinates (i.e. map coordinates that can be out of bounds).
		float x, y;  // Pixel position relative to top left.
		float texture_x, texture_y;  // Texture coordinates.
		float brightness;            // brightness of the pixel
		uint8_t ter_r, ter_d;        // Texture index of the right and down triangle.
	};

	FieldsToDraw(int minfx, int maxfx, int minfy, int maxfy)
	   : min_fx_(minfx),
	     max_fx_(maxfx),
	     min_fy_(minfy),
	     max_fy_(maxfy),
	     w_(max_fx_ - min_fx_ + 1),
	     h_(max_fy_ - min_fy_ + 1) {
		fields_.resize(w_ * h_);
	}

	// Calculates the index of the given field with ('fx', 'fy') being geometric
	// coordinates in the map. Returns -1 if this field is not in the fields_to_draw.
	inline int calculate_index(int fx, int fy) const {
		uint16_t xidx = fx - min_fx_;
		if (xidx >= w_) {
			return -1;
		}
		uint16_t yidx = fy - min_fy_;
		if (yidx >= h_) {
			return -1;
		}
		return yidx * w_ + xidx;
	}

	// The number of fields to draw.
	inline size_t size() const {
		return fields_.size();
	}

	// Get the field at 'index' which must be in bound.
	inline const Field& at(const int index) const {
		return fields_.at(index);
	}

	// Returns a mutable field at 'index' which must be in bound.
	inline Field* mutable_field(const int index) {
		return &fields_[index];
	}

private:
	// Minimum and maximum field coordinates (geometric) to render. Can be negative.
	const int min_fx_;
	const int max_fx_;
	const int min_fy_;
	const int max_fy_;

	// Width and height in number of fields.
	const int w_;
	const int h_;

	std::vector<Field> fields_;
};

// NOCOM(#sirver): only do this once.
const GLSurfaceTexture* get_dither_edge_texture() {
	const std::string fname = "world/pics/edge.png";
	const std::string cachename = std::string("gltex#") + fname;

	if (Surface* surface = g_gr->surfaces().get(cachename))
		return dynamic_cast<GLSurfaceTexture*>(surface);

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
// dither layer is smaller, we have to draw a dithering triangle too - this lets the neighboring
// texture
// bleed into our triangle.
//
// The dither triangle is the triangle that should be partially (either r or
// d). Example: if r and d have different textures and r.dither_layer >
// d.dither_layer, then we will repaint d with the dither texture as mask.

// NOCOM(#sirver): Move the programs into their own files in graphic/gl/.
class DitherProgram {
public:
	DitherProgram();

	// Draws the terrain.
	void draw(const DescriptionMaintainer<TerrainDescription>& terrains,
	          const FieldsToDraw& fields_to_draw);

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
	void draw(const DescriptionMaintainer<TerrainDescription>& terrains,
	          const FieldsToDraw& fields_to_draw);

private:
	void gl_draw(int num_vertices, const DescriptionMaintainer<TerrainDescription>& terrains);

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
	terrain_gl_program_.build(kTerrainVertexShader, kTerrainFragmentShader);

	attr_position_ = glGetAttribLocation(terrain_gl_program_.object(), "attr_position");
	attr_texture_position_ =
	   glGetAttribLocation(terrain_gl_program_.object(), "attr_texture_position");
	attr_brightness_ = glGetAttribLocation(terrain_gl_program_.object(), "attr_brightness");

	u_terrain_texture_ = glGetUniformLocation(terrain_gl_program_.object(), "u_terrain_texture");
}

void TerrainProgram::gl_draw(int num_vertices,
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
                          const FieldsToDraw& fields_to_draw) {
	if (vertices_.size() < fields_to_draw.size()) {
		vertices_.resize(fields_to_draw.size());
		terrains_to_indices_.resize(terrains.size());
	}
	for (auto& container : terrains_to_indices_) {
		container.clear();
		container.reserve(fields_to_draw.size());
	}

	for (size_t current_index = 0; current_index < fields_to_draw.size(); ++current_index) {
		const FieldsToDraw::Field& field = fields_to_draw.at(current_index);

		PerVertexData& vertex = vertices_[current_index];
		vertex.texture_x = field.texture_x;
		vertex.texture_y = field.texture_y;
		vertex.x = field.x;
		vertex.y = field.y;
		vertex.brightness = field.brightness;

		// The bottom right neighbor fields_to_draw is needed for both triangles
		// associated with this field. If it is not in fields_to_draw, there is no need to
		// draw any triangles.
		const int brn_index = fields_to_draw.calculate_index(field.fx + (field.fy & 1), field.fy + 1);
		if (brn_index == -1) {
			continue;
		}

		// Down triangle.
		const int bln_index =
		   fields_to_draw.calculate_index(field.fx + (field.fy & 1) - 1, field.fy + 1);
		if (bln_index != -1) {
			terrains_to_indices_[field.ter_d].push_back(current_index);
			terrains_to_indices_[field.ter_d].push_back(bln_index);
			terrains_to_indices_[field.ter_d].push_back(brn_index);
		}

		// Right triangle.
		const int rn_index = fields_to_draw.calculate_index(field.fx + 1, field.fy);
		if (rn_index != -1) {
			terrains_to_indices_[field.ter_r].push_back(current_index);
			terrains_to_indices_[field.ter_r].push_back(brn_index);
			terrains_to_indices_[field.ter_r].push_back(rn_index);
		}
	}

	gl_draw(fields_to_draw.size(), terrains);
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

DitherProgram::DitherProgram() {
	dither_gl_program_.build(kDitherVertexShader, kDitherFragmentShader);

	attr_brightness_ = glGetAttribLocation(dither_gl_program_.object(), "attr_brightness");
	attr_dither_texture_position_ =
	   glGetAttribLocation(dither_gl_program_.object(), "attr_dither_texture_position");
	attr_position_ = glGetAttribLocation(dither_gl_program_.object(), "attr_position");
	attr_texture_position_ =
	   glGetAttribLocation(dither_gl_program_.object(), "attr_texture_position");

	u_dither_texture_ = glGetUniformLocation(dither_gl_program_.object(), "u_dither_texture");
	u_terrain_texture_ = glGetUniformLocation(dither_gl_program_.object(), "u_terrain_texture");
}

void DitherProgram::draw(const DescriptionMaintainer<TerrainDescription>& terrains,
                         const FieldsToDraw& fields_to_draw) {
	glUseProgram(dither_gl_program_.object());

	glEnableVertexAttribArray(attr_brightness_);
	glEnableVertexAttribArray(attr_dither_texture_position_);
	glEnableVertexAttribArray(attr_position_);
	glEnableVertexAttribArray(attr_texture_position_);

	// NOCOM(#sirver): do not recreate everytime.
	std::vector<std::vector<DitherData>> data;
	data.resize(terrains.size());

	const auto add_vertex = [&fields_to_draw, &data](int index, int order_index, int terrain) {
		const FieldsToDraw::Field& field = fields_to_draw.at(index);

		data[terrain].emplace_back();
		DitherData& back = data[terrain].back();

		back.x = field.x;
		back.y = field.y;
		back.texture_x = field.texture_x;
		back.texture_y = field.texture_y;
		back.brightness = field.brightness;

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

	const auto potentially_add_dithering_triangle = [&terrains, &add_vertex, &fields_to_draw](
	   int idx1, int idx2, int idx3, int my_terrain, int other_terrain) {
		if (my_terrain == other_terrain) {
			return;
		}
		if (terrains.get_unmutable(my_terrain).dither_layer() <
		    terrains.get_unmutable(other_terrain).dither_layer()) {
			add_vertex(idx1, 0, other_terrain);
			add_vertex(idx2, 1, other_terrain);
			add_vertex(idx3, 2, other_terrain);
		}
	};

	for (size_t current_index = 0; current_index < fields_to_draw.size(); ++current_index) {
		const FieldsToDraw::Field& field = fields_to_draw.at(current_index);

		// The bottom right neighbor fields_to_draw is needed for both triangles
		// associated with this field. If it is not in fields_to_draw, there is no need to
		// draw any triangles.
		const int brn_index = fields_to_draw.calculate_index(field.fx + (field.fy & 1), field.fy + 1);
		if (brn_index == -1) {
			continue;
		}

		// Dithering triangles for Down triangle.
		const int bln_index =
		   fields_to_draw.calculate_index(field.fx + (field.fy & 1) - 1, field.fy + 1);
		if (bln_index != -1) {
			potentially_add_dithering_triangle(
			   brn_index, current_index, bln_index, field.ter_d, field.ter_r);

			const int terrain_dd = fields_to_draw.at(bln_index).ter_r;
			potentially_add_dithering_triangle(
			   bln_index, brn_index, current_index, field.ter_d, terrain_dd);

			const int ln_index = fields_to_draw.calculate_index(field.fx - 1, field.fy);
			if (ln_index != -1) {
				const int terrain_l = fields_to_draw.at(ln_index).ter_r;
				potentially_add_dithering_triangle(
				   current_index, bln_index, brn_index, field.ter_d, terrain_l);
			}
		}

		// Dithering for right triangle.
		const int rn_index = fields_to_draw.calculate_index(field.fx + 1, field.fy);
		if (rn_index != -1) {
			potentially_add_dithering_triangle(
			   current_index, brn_index, rn_index, field.ter_r, field.ter_d);
			int terrain_rr = fields_to_draw.at(rn_index).ter_d;
			potentially_add_dithering_triangle(
			   brn_index, rn_index, current_index, field.ter_r, terrain_rr);

			const int trn_index =
			   fields_to_draw.calculate_index(field.fx + (field.fy & 1), field.fy - 1);
			if (trn_index != -1) {
				const int terrain_u = fields_to_draw.at(trn_index).ter_d;
				potentially_add_dithering_triangle(
				   rn_index, current_index, brn_index, field.ter_r, terrain_u);
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

std::unique_ptr<TerrainProgram> GlGameRenderer::terrain_program_;
std::unique_ptr<DitherProgram> GlGameRenderer::dither_program_;

GlGameRenderer::GlGameRenderer() : m_road_vertices_size(0) {
}

GlGameRenderer::~GlGameRenderer() {
}

// Returns the brightness value in [0, 1.] for 'fcoords' at 'gametime' for
// 'player' (which can be nullptr).
uint8_t GlGameRenderer::field_brightness(const FCoords& coords) const {
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

void GlGameRenderer::draw() {
	if (terrain_program_ == nullptr) {
		terrain_program_.reset(new TerrainProgram());
		dither_program_.reset(new DitherProgram());
	}

	m_surface = dynamic_cast<GLSurface*>(m_dst->get_surface());
	if (!m_surface)
		return;
	m_rect = m_dst->get_rect();
	m_surface_offset = m_dst_offset + m_rect.top_left() + m_dst->get_offset();

	glClear(GL_COLOR_BUFFER_BIT);

	glScissor(m_rect.x, m_surface->height() - m_rect.y - m_rect.h, m_rect.w, m_rect.h);
	glEnable(GL_SCISSOR_TEST);

	Map& map = m_egbase->map();
	const uint32_t gametime = m_egbase->get_gametime();

	FieldsToDraw fields_to_draw(m_minfx, m_maxfx, m_minfy, m_maxfy);
	for (int32_t fy = m_minfy; fy <= m_maxfy; ++fy) {
		for (int32_t fx = m_minfx; fx <= m_maxfx; ++fx) {
			FieldsToDraw::Field& f =
			   *fields_to_draw.mutable_field(fields_to_draw.calculate_index(fx, fy));

			f.fx = fx;
			f.fy = fy;

			Coords coords(fx, fy);
			int x, y;
			MapviewPixelFunctions::get_basepix(coords, x, y);

			map.normalize_coords(coords);
			const FCoords& fcoords = map.get_fcoords(coords);

			f.texture_x = float(x) / TEXTURE_WIDTH;
			f.texture_y = float(y) / TEXTURE_HEIGHT;
			f.x = x + m_surface_offset.x;
			f.y = y + m_surface_offset.y - fcoords.field->get_height() * HEIGHT_FACTOR;

			f.ter_d = fcoords.field->terrain_d();
			f.ter_r = fcoords.field->terrain_r();

			f.brightness = ::field_brightness(fcoords, gametime, map, m_player);
		}
	}

	const World& world = m_egbase->world();
	terrain_program_->draw(world.terrains(), fields_to_draw);
	dither_program_->draw(world.terrains(), fields_to_draw);

	// prepare_roads();
	// draw_roads();
	// draw_objects();

	glDisable(GL_SCISSOR_TEST);
}

template <typename vertex>
void GlGameRenderer::compute_basevertex(const Coords& coords, vertex& vtx) const {
	const Map& map = m_egbase->map();
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

uint8_t GlGameRenderer::field_roads(const FCoords& coords) const {
	uint8_t roads;
	const Map& map = m_egbase->map();
	if (m_player && !m_player->see_all()) {
		const Player::Field& pf = m_player->fields()[Map::get_index(coords, map.get_width())];
		roads = pf.roads | map.overlay_manager().get_road_overlay(coords);
	} else {
		roads = coords.field->get_roads();
	}
	roads |= map.overlay_manager().get_road_overlay(coords);
	return roads;
}

void GlGameRenderer::prepare_roads() {
	const Map& map = m_egbase->map();

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

void GlGameRenderer::draw_roads() {
	if (!m_road_freq[0] && !m_road_freq[1])
		return;

	GLuint rt_normal = dynamic_cast<const GLSurfaceTexture&>(
	                      g_gr->get_road_texture(Widelands::Road_Normal)).get_gl_texture();
	GLuint rt_busy = dynamic_cast<const GLSurfaceTexture&>(
	                    g_gr->get_road_texture(Widelands::Road_Busy)).get_gl_texture();

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
