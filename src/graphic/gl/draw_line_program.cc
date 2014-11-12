/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#include "graphic/gl/draw_line_program.h"

#include <vector>

#include "base/log.h"

namespace  {

const char kDrawLineVertexShader[] = R"(
#version 120

// Attributes.
attribute vec2 attr_position;

void main() {
	gl_Position = vec4(attr_position, 0., 1.);
}
)";

const char kDrawLineFragmentShader[] = R"(
#version 120

uniform ivec3 u_color;

void main() {
	gl_FragColor = vec4(vec3(u_color) / 255., 1.);
}
)";

}  // namespace

// static
DrawLineProgram& DrawLineProgram::instance() {
	static DrawLineProgram draw_line_program;
	return draw_line_program;
}

DrawLineProgram::DrawLineProgram() {
	gl_program_.build(kDrawLineVertexShader, kDrawLineFragmentShader);

	attr_position_ = glGetAttribLocation(gl_program_.object(), "attr_position");
	u_color_ = glGetUniformLocation(gl_program_.object(), "u_color");

}

void DrawLineProgram::draw(const float x1,
                           const float y1,
                           const float x2,
                           const float y2,
                           const RGBColor& color,
                           const int line_width) {
	glUseProgram(gl_program_.object());
	glEnableVertexAttribArray(attr_position_);

	const std::vector<PerVertexData> vertices = {{x1, y1}, {x2, y2}};

	glBindBuffer(GL_ARRAY_BUFFER, gl_array_buffer_.object());
	glBufferData(
	   GL_ARRAY_BUFFER, sizeof(PerVertexData) * vertices.size(), vertices.data(), GL_STREAM_DRAW);
	glVertexAttribPointer(attr_position_,
								 2,
								 GL_FLOAT,
								 GL_FALSE,
								 sizeof(PerVertexData),
								 reinterpret_cast<void*>(0));

	glUniform3i(u_color_, color.r, color.g, color.b);

	glLineWidth(line_width);
	glDrawArrays(GL_LINES, 0, 2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(attr_position_);
	glUseProgram(0);
}
