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

#include "graphic/gl/draw_line_program.h"

#include <cassert>
#include <iterator>

// static
DrawLineProgram& DrawLineProgram::instance() {
	static DrawLineProgram draw_line_program;
	return draw_line_program;
}

DrawLineProgram::DrawLineProgram() {
	gl_program_.build("draw_line");

	attr_position_ = glGetAttribLocation(gl_program_.object(), "attr_position");
	attr_color_ = glGetAttribLocation(gl_program_.object(), "attr_color");
}

void DrawLineProgram::draw(std::vector<Arguments> arguments) {
	glUseProgram(gl_program_.object());

	auto& gl_state = Gl::State::instance();
	gl_state.enable_vertex_attrib_array({
	   attr_position_,
	   attr_color_,
	});

	gl_array_buffer_.bind();

	Gl::vertex_attrib_pointer(
	   attr_position_, 3, sizeof(PerVertexData), offsetof(PerVertexData, gl_x));
	Gl::vertex_attrib_pointer(
	   attr_color_, 4, sizeof(PerVertexData), offsetof(PerVertexData, color_r));

	vertices_.clear();

	for (Arguments& current_args : arguments) {
		// We do not support anything else for drawing lines, really.
		assert(current_args.blend_mode == BlendMode::UseAlpha);

		for (auto& vertice : current_args.vertices) {
			vertice.gl_z = current_args.z_value;
		}
		std::move(
		   current_args.vertices.begin(), current_args.vertices.end(), std::back_inserter(vertices_));
	}
	gl_array_buffer_.update(vertices_);
	glDrawArrays(GL_TRIANGLES, 0, vertices_.size());
}
