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

#include "graphic/gl/fill_rect_program.h"

#include <vector>

#include "base/log.h"
#include "base/wexception.h"

namespace  {

const char kFillRectVertexShader[] = R"(
#version 120

// Attributes.
attribute vec3 attr_position;

void main() {
	gl_Position = vec4(attr_position, 1.);
}
)";

const char kFillRectFragmentShader[] = R"(
#version 120

uniform ivec4 u_color;

void main() {
	gl_FragColor = vec4(u_color) / 255.;
}
)";

}  // namespace

// static
FillRectProgram& FillRectProgram::instance() {
	static FillRectProgram fill_rect_program;
	return fill_rect_program;
}

FillRectProgram::FillRectProgram() {
	gl_program_.build(kFillRectVertexShader, kFillRectFragmentShader);

	attr_position_ = glGetAttribLocation(gl_program_.object(), "attr_position");
	u_color_ = glGetUniformLocation(gl_program_.object(), "u_color");
}

void FillRectProgram::draw(const FloatRect& destination_rect,
                           const float z_value,
                           const RGBAColor& color,
                           const BlendMode blend_mode) {
	// This method does 3 things:
	// - if blend_mode is Copy, we will copy color into the destination
	// pixels without blending.
	// - if blend_mode is Alpha and color.r < 0, we will
	// GL_FUNC_REVERSE_SUBTRACT color.r from all RGB values in the
	// destination buffer. color.a should be 0 for this.
	// - if blend_mode is Alpha and color.r > 0, we will
	// GL_ADD color.r to all RGB values in the destination buffer.
	// color.a should be 0 for this.

	// The simple trick here is to fill the rect, but using a different glBlendFunc that will sum
	// src and target (or subtract them if factor is negative).
	switch (blend_mode) {
	case BlendMode::Subtract:
		glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
		/* fallthrough intended */
	case BlendMode::UseAlpha:
		glBlendFunc(GL_ONE, GL_ONE);
		break;

	case BlendMode::Copy:
		glDisable(GL_BLEND);
		break;

	default:
		break;
	}

	glUseProgram(gl_program_.object());

	std::vector<PerVertexData> vertices;
	vertices.reserve(4);

	vertices.emplace_back(destination_rect.x, destination_rect.y, z_value);
	vertices.emplace_back(destination_rect.x + destination_rect.w, destination_rect.y, z_value);
	vertices.emplace_back(destination_rect.x, destination_rect.y + destination_rect.h, z_value);
	vertices.emplace_back(
	   destination_rect.x + destination_rect.w, destination_rect.y + destination_rect.h, z_value);

	glBindBuffer(GL_ARRAY_BUFFER, gl_array_buffer_.object());
	glBufferData(
	   GL_ARRAY_BUFFER, sizeof(PerVertexData) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);

	glVertexAttribPointer(attr_position_,
								 3,
								 GL_FLOAT,
								 GL_FALSE,
								 sizeof(PerVertexData),
								 reinterpret_cast<void*>(0));

	glEnableVertexAttribArray(attr_position_);

	glUniform4i(u_color_, color.r, color.g, color.b, color.a);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(attr_position_);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	switch (blend_mode) {
	case BlendMode::Subtract:
		glBlendEquation(GL_FUNC_ADD);
		/* fallthrough intended */
	case BlendMode::UseAlpha:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;

	case BlendMode::Copy:
		glEnable(GL_BLEND);
		break;

	default:
		break;
	}
}
