/*
 * Copyright (C) 2006-2015 by the Widelands Development Team
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

#include <algorithm>
#include <cassert>
#include <vector>

#include "base/log.h"

namespace  {

const char kDrawLineVertexShader[] = R"(
#version 120

// Attributes.
attribute vec3 attr_position;
attribute vec3 attr_color;

varying vec3 var_color;

void main() {
	var_color = attr_color;
	gl_Position = vec4(attr_position, 1.);
}
)";

const char kDrawLineFragmentShader[] = R"(
#version 120

varying vec3 var_color;

void main() {
	gl_FragColor = vec4(var_color.rgb, 1.);
}
)";

struct DrawBatch {
	int offset;
	int count;
	int line_width;
};

}  // namespace

// static
DrawLineProgram& DrawLineProgram::instance() {
	static DrawLineProgram draw_line_program;
	return draw_line_program;
}

DrawLineProgram::DrawLineProgram() {
	gl_program_.build(kDrawLineVertexShader, kDrawLineFragmentShader);

	attr_position_ = glGetAttribLocation(gl_program_.object(), "attr_position");
	attr_color_ = glGetAttribLocation(gl_program_.object(), "attr_color");
}

void DrawLineProgram::draw(const FloatPoint& start,
                           const FloatPoint& end,
                           const float z_value,
                           const RGBColor& color,
									int line_width) {
	draw({Arguments{FloatRect(start.x, start.y, end.x - start.x, end.y - start.y),
	                z_value,
	                color,
						 static_cast<uint8_t>(line_width),
	                BlendMode::Copy}});
}

void DrawLineProgram::draw(std::vector<Arguments> arguments) {
	size_t i = 0;

	glUseProgram(gl_program_.object());
	glEnableVertexAttribArray(attr_position_);
	glEnableVertexAttribArray(attr_color_);

	gl_array_buffer_.bind();

	Gl::vertex_attrib_pointer(attr_position_, 3, sizeof(PerVertexData), offsetof(PerVertexData, gl_x));
	Gl::vertex_attrib_pointer(attr_color_, 3, sizeof(PerVertexData), offsetof(PerVertexData, color_r));

	vertices_.clear();

	std::vector<DrawBatch> draw_batches;
	int offset = 0;
	while (i < arguments.size()) {
		const Arguments& template_args = arguments[i];

		while (i < arguments.size()) {
			const Arguments& current_args = arguments[i];
			if (current_args.line_width != template_args.line_width) {
				break;
			}
			// We do not support anything else for drawing lines, really.
			assert(current_args.blend_mode == BlendMode::Copy);

			vertices_.emplace_back(current_args.destination_rect.x,
			                       current_args.destination_rect.y,
			                       current_args.z_value,
			                       current_args.color.r / 255.,
			                       current_args.color.g / 255.,
			                       current_args.color.b / 255.);

			vertices_.emplace_back(current_args.destination_rect.x + current_args.destination_rect.w,
			                       current_args.destination_rect.y + current_args.destination_rect.h,
			                       current_args.z_value,
			                       current_args.color.r / 255.,
			                       current_args.color.g / 255.,
			                       current_args.color.b / 255.);
			++i;
		}

		draw_batches.emplace_back(
		   DrawBatch{offset, static_cast<int>(vertices_.size() - offset), template_args.line_width});
		offset = vertices_.size();
	}

	gl_array_buffer_.update(vertices_);

	// Now do the draw calls.
	for (const auto& draw_arg : draw_batches) {
		glLineWidth(draw_arg.line_width);
		glDrawArrays(GL_LINES, draw_arg.offset, draw_arg.count);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(attr_position_);
	glDisableVertexAttribArray(attr_color_);

	glUseProgram(0);
}
