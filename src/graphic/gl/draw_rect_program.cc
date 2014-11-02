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

#include "graphic/gl/draw_rect_program.h"

#include <vector>

#include "base/log.h"

namespace  {

const char kDrawRectVertexShader[] = R"(
#version 120

// Attributes.
attribute vec2 attr_position;

// Uniforms.
uniform vec4 u_rect;


void main() {
	float x = u_rect.x + attr_position.x * u_rect.z;
	float y = u_rect.y + attr_position.y * u_rect.w;
	gl_Position = vec4(x, y, 0., 1.);
}
)";

const char kDrawRectFragmentShader[] = R"(
#version 120

uniform ivec3 u_color;

void main() {
	gl_FragColor = vec4(vec3(u_color) / 255., 1.);
}
)";

}  // namespace

// static
DrawRectProgram& DrawRectProgram::instance() {
	static DrawRectProgram draw_rect_program;
	return draw_rect_program;
}

DrawRectProgram::DrawRectProgram() {
	gl_program_.build(kDrawRectVertexShader, kDrawRectFragmentShader);

	attr_position_ = glGetAttribLocation(gl_program_.object(), "attr_position");

	u_color_ = glGetUniformLocation(gl_program_.object(), "u_color");
	u_rect_ = glGetUniformLocation(gl_program_.object(), "u_rect");

	std::vector<PerVertexData> vertices;
	vertices.push_back(PerVertexData
			{0., 0.});
	vertices.push_back(PerVertexData
			{0., 1.});
	vertices.push_back(PerVertexData
			{1., 1.});
	vertices.push_back(PerVertexData
			{1., 0.});

	glBindBuffer(GL_ARRAY_BUFFER, gl_array_buffer_.object());
	glBufferData(
	   GL_ARRAY_BUFFER, sizeof(PerVertexData) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(attr_position_,
								 2,
								 GL_FLOAT,
								 GL_FALSE,
								 sizeof(PerVertexData),
								 reinterpret_cast<void*>(0));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DrawRectProgram::draw(const FloatRect& gl_dst_rect, const RGBColor& color) {
	glUseProgram(gl_program_.object());
	glEnableVertexAttribArray(attr_position_);
	glBindBuffer(GL_ARRAY_BUFFER, gl_array_buffer_.object());

	glVertexAttribPointer(attr_position_,
								 2,
								 GL_FLOAT,
								 GL_FALSE,
								 sizeof(PerVertexData),
								 reinterpret_cast<void*>(0));

	glUniform4f(u_rect_, gl_dst_rect.x, gl_dst_rect.y, gl_dst_rect.w, gl_dst_rect.h);
	glUniform3i(u_color_, color.r, color.g, color.b);

	glLineWidth(1.);
	glDrawArrays(GL_LINE_LOOP, 0, 4);

	glDisableVertexAttribArray(attr_position_);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}
