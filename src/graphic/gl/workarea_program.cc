/*
 * Copyright (C) 2006-2019 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "graphic/gl/workarea_program.h"

#include "graphic/gl/coordinate_conversion.h"
#include "graphic/gl/fields_to_draw.h"
#include "graphic/gl/utils.h"
#include "graphic/texture.h"

WorkareaProgram::WorkareaProgram() {
	gl_program_.build("workarea");

	attr_position_ = glGetAttribLocation(gl_program_.object(), "attr_position");
	attr_overlay_ = glGetAttribLocation(gl_program_.object(), "attr_overlay");

	u_z_value_ = glGetUniformLocation(gl_program_.object(), "u_z_value");
}

void WorkareaProgram::gl_draw(int gl_texture, float z_value) {
	glUseProgram(gl_program_.object());

	auto& gl_state = Gl::State::instance();
	gl_state.enable_vertex_attrib_array({attr_position_, attr_overlay_});

	gl_array_buffer_.bind();
	gl_array_buffer_.update(vertices_);

	Gl::vertex_attrib_pointer(
	   attr_position_, 2, sizeof(PerVertexData), offsetof(PerVertexData, gl_x));
	Gl::vertex_attrib_pointer(
	   attr_overlay_, 4, sizeof(PerVertexData), offsetof(PerVertexData, overlay_r));

	gl_state.bind(GL_TEXTURE0, gl_texture);

	glUniform1f(u_z_value_, z_value);

	glDrawArrays(GL_TRIANGLES, 0, vertices_.size());
}

constexpr uint8_t kWorkareaTransparency = 127;
static RGBAColor workarea_colors[]  // Comment to prevent clang-format from breaking codecheck
   {
      RGBAColor(63, 31, 127, kWorkareaTransparency),  // All three circles
      RGBAColor(127, 63, 0, kWorkareaTransparency),   // Medium and outer circle
      RGBAColor(0, 127, 0, kWorkareaTransparency),    // Outer circle
      RGBAColor(63, 0, 127, kWorkareaTransparency),   // Inner and medium circle
      RGBAColor(127, 0, 0, kWorkareaTransparency),    // Medium circle
      RGBAColor(0, 0, 127, kWorkareaTransparency),    // Inner circle
   };
static inline RGBAColor apply_color(RGBAColor c1, RGBAColor c2) {
	uint8_t r = (c1.r * c1.a + c2.r * c2.a) / (c1.a + c2.a);
	uint8_t g = (c1.g * c1.a + c2.g * c2.a) / (c1.a + c2.a);
	uint8_t b = (c1.b * c1.a + c2.b * c2.a) / (c1.a + c2.a);
	uint8_t a = (c1.a + c2.a) / 2;
	return RGBAColor(r, g, b, a);
}

void WorkareaProgram::add_vertex(const FieldsToDraw::Field& field, RGBAColor overlay) {
	vertices_.emplace_back();
	PerVertexData& back = vertices_.back();

	back.gl_x = field.gl_position.x;
	back.gl_y = field.gl_position.y;
	back.overlay_r = overlay.r / 255.f;
	back.overlay_g = overlay.g / 255.f;
	back.overlay_b = overlay.b / 255.f;
	back.overlay_a = overlay.a / 255.f;
}

void WorkareaProgram::draw(uint32_t texture_id,
                           Workareas workarea,
                           const FieldsToDraw& fields_to_draw,
                           float z_value) {
	vertices_.clear();
	vertices_.reserve(fields_to_draw.size() * 3);

	for (size_t current_index = 0; current_index < fields_to_draw.size(); ++current_index) {
		const FieldsToDraw::Field& field = fields_to_draw.at(current_index);

		// The bottom right neighbor fields_to_draw is needed for both triangles
		// associated with this field. If it is not in fields_to_draw, there is no need to
		// draw any triangles.
		if (field.brn_index == FieldsToDraw::kInvalidIndex) {
			continue;
		}

		// Down triangle.
		if (field.bln_index != FieldsToDraw::kInvalidIndex) {
			RGBAColor color(0, 0, 0, 0);
			for (const std::map<Widelands::TCoords<>, uint8_t>& wa_map : workarea) {
				const auto it =
				   wa_map.find(Widelands::TCoords<>(field.fcoords, Widelands::TriangleIndex::D));
				if (it != wa_map.end()) {
					color = apply_color(color, workarea_colors[it->second]);
				}
			}
			add_vertex(fields_to_draw.at(current_index), color);
			add_vertex(fields_to_draw.at(field.bln_index), color);
			add_vertex(fields_to_draw.at(field.brn_index), color);
		}

		// Right triangle.
		if (field.rn_index != FieldsToDraw::kInvalidIndex) {
			RGBAColor color(0, 0, 0, 0);
			for (const std::map<Widelands::TCoords<>, uint8_t>& wa_map : workarea) {
				const auto it =
				   wa_map.find(Widelands::TCoords<>(field.fcoords, Widelands::TriangleIndex::R));
				if (it != wa_map.end()) {
					color = apply_color(color, workarea_colors[it->second]);
				}
			}
			add_vertex(fields_to_draw.at(current_index), color);
			add_vertex(fields_to_draw.at(field.brn_index), color);
			add_vertex(fields_to_draw.at(field.rn_index), color);
		}
	}

	gl_draw(texture_id, z_value);
}
