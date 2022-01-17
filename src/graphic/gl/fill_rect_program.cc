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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "graphic/gl/fill_rect_program.h"

#include "base/macros.h"
#include "base/wexception.h"

// static
FillRectProgram& FillRectProgram::instance() {
	static FillRectProgram fill_rect_program;
	return fill_rect_program;
}

FillRectProgram::FillRectProgram() {
	gl_program_.build("fill_rect");

	attr_position_ = glGetAttribLocation(gl_program_.object(), "attr_position");
	attr_color_ = glGetAttribLocation(gl_program_.object(), "attr_color");
}

void FillRectProgram::draw(const Rectf& destination_rect,
                           const float z_value,
                           const RGBAColor& color,
                           const BlendMode blend_mode) {
	draw({Arguments{destination_rect, z_value, color, blend_mode}});
}

void FillRectProgram::draw(const std::vector<Arguments>& arguments) {
	size_t i = 0;

	while (i < arguments.size()) {
		vertices_.clear();
		const Arguments& template_args = arguments[i];

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
		switch (template_args.blend_mode) {
		case BlendMode::Subtract:
			glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
			FALLS_THROUGH;
		case BlendMode::UseAlpha:
			glBlendFunc(GL_ONE, GL_ONE);
			break;

		case BlendMode::Copy:
			glDisable(GL_BLEND);
			break;

		case BlendMode::Default:
			break;
		}

		glUseProgram(gl_program_.object());

		gl_array_buffer_.bind();

		auto& gl_state = Gl::State::instance();
		gl_state.enable_vertex_attrib_array({
		   attr_position_,
		   attr_color_,
		});

		// Batch common rectangles up.
		while (i < arguments.size()) {
			const Arguments& current_args = arguments[i];
			if (current_args.blend_mode != template_args.blend_mode) {
				break;
			}

			const float r = current_args.color.r / 255.;
			const float g = current_args.color.g / 255.;
			const float b = current_args.color.b / 255.;
			const float a = current_args.color.a / 255.;

			// First triangle.
			vertices_.emplace_back(current_args.destination_rect.x, current_args.destination_rect.y,
			                       current_args.z_value, r, g, b, a);
			vertices_.emplace_back(current_args.destination_rect.x + current_args.destination_rect.w,
			                       current_args.destination_rect.y, current_args.z_value, r, g, b, a);
			vertices_.emplace_back(current_args.destination_rect.x,
			                       current_args.destination_rect.y + current_args.destination_rect.h,
			                       current_args.z_value, r, g, b, a);

			// Second triangle.
			vertices_.emplace_back(current_args.destination_rect.x + current_args.destination_rect.w,
			                       current_args.destination_rect.y, current_args.z_value, r, g, b, a);
			vertices_.emplace_back(current_args.destination_rect.x,
			                       current_args.destination_rect.y + current_args.destination_rect.h,
			                       current_args.z_value, r, g, b, a);
			vertices_.emplace_back(current_args.destination_rect.x + current_args.destination_rect.w,
			                       current_args.destination_rect.y + current_args.destination_rect.h,
			                       current_args.z_value, r, g, b, a);
			++i;
		}

		gl_array_buffer_.update(vertices_);

		Gl::vertex_attrib_pointer(
		   attr_position_, 3, sizeof(PerVertexData), offsetof(PerVertexData, gl_x));
		Gl::vertex_attrib_pointer(attr_color_, 4, sizeof(PerVertexData), offsetof(PerVertexData, r));

		glDrawArrays(GL_TRIANGLES, 0, vertices_.size());

		switch (template_args.blend_mode) {
		case BlendMode::Subtract:
			glBlendEquation(GL_FUNC_ADD);
			FALLS_THROUGH;
		case BlendMode::UseAlpha:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;

		case BlendMode::Copy:
			glEnable(GL_BLEND);
			break;

		case BlendMode::Default:
			break;
		}
	}
}
