/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "graphic/gl/grid_program.h"

#include "graphic/gl/fields_to_draw.h"
#include "graphic/gl/utils.h"

GridProgram::GridProgram() {
	gl_program_.build("grid");

	attr_position_ = glGetAttribLocation(gl_program_.object(), "attr_position");

	u_z_value_ = glGetUniformLocation(gl_program_.object(), "u_z_value");
}

void GridProgram::gl_draw(int gl_texture, float z_value) {
	glUseProgram(gl_program_.object());

	auto& gl_state = Gl::State::instance();

	gl_array_buffer_.bind();
	gl_array_buffer_.update(vertices_);

	Gl::vertex_attrib_pointer(
	   attr_position_, 2, sizeof(PerVertexData), offsetof(PerVertexData, gl_x));

	gl_state.bind(GL_TEXTURE0, gl_texture);

	glUniform1f(u_z_value_, z_value);

	glDrawArrays(GL_LINES, 0, vertices_.size());
}

void GridProgram::add_vertex(const FieldsToDraw::Field& field) {
	vertices_.emplace_back();
	PerVertexData& back = vertices_.back();
	back.gl_x = field.gl_position.x;
	back.gl_y = field.gl_position.y;
}

void GridProgram::draw(uint32_t texture_id, const FieldsToDraw& fields_to_draw, float z_value) {
	vertices_.clear();
	vertices_.reserve(fields_to_draw.size() * 2);

	for (size_t current_index = 0; current_index < fields_to_draw.size(); ++current_index) {
		const FieldsToDraw::Field& field = fields_to_draw.at(current_index);

		// Southwestern edge
		if (field.bln_index != FieldsToDraw::kInvalidIndex) {
			add_vertex(fields_to_draw.at(current_index));
			add_vertex(fields_to_draw.at(field.bln_index));
		}

		// Southeastern edge
		if (field.brn_index != FieldsToDraw::kInvalidIndex) {
			add_vertex(fields_to_draw.at(current_index));
			add_vertex(fields_to_draw.at(field.brn_index));
		}

		// Eastern edge
		if (field.rn_index != FieldsToDraw::kInvalidIndex) {
			add_vertex(fields_to_draw.at(current_index));
			add_vertex(fields_to_draw.at(field.rn_index));
		}
	}

	gl_draw(texture_id, z_value);
}
