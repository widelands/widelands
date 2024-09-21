/*
 * Copyright (C) 2006-2024 by the Widelands Development Team
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

std::vector<FillRectProgram::Arguments> FillRectProgram::make_arguments_for_rect(const Rectf& destination_rect,
                           const float z_value,
                           const RGBAColor& color,
                           const BlendMode blend_mode) {
	const float r = color.r / 255.f;
	const float g = color.g / 255.f;
	const float b = color.b / 255.f;
	const float a = color.a / 255.f;

	Arguments::Vertex vbr = {Vector2f(destination_rect.x + destination_rect.w, destination_rect.y + destination_rect.h), r, g, b, a};
	Arguments::Vertex vtr = {Vector2f(destination_rect.x + destination_rect.w, destination_rect.y), r, g, b, a};
	Arguments::Vertex vbl = {Vector2f(destination_rect.x, destination_rect.y + destination_rect.h), r, g, b, a};
	Arguments::Vertex vtl = {Vector2f(destination_rect.x, destination_rect.y), r, g, b, a};

	return {
		Arguments{{vbr, vtl, vtr}, z_value, blend_mode},
		Arguments{{vbr, vtl, vbl}, z_value, blend_mode},
	};
}

void FillRectProgram::draw(const Rectf& destination_rect,
                           const float z_value,
                           const RGBAColor& color,
                           const BlendMode blend_mode) {
	draw(make_arguments_for_rect(destination_rect, z_value, color, blend_mode));
}

void FillRectProgram::draw_height_heat_map_overlays(const FieldsToDraw& fields_to_draw, const float z_value) {
	constexpr float kAlpha = 0.9f;
	std::vector<Arguments> arguments;

	for (size_t current_index = 0; current_index < fields_to_draw.size(); ++current_index) {
		const FieldsToDraw::Field& field = fields_to_draw.at(current_index);
		if (field.brn_index == FieldsToDraw::kInvalidIndex) {
			continue;
		}

		const FieldsToDraw::Field& field_brn = fields_to_draw.at(field.brn_index);

		float val1 = field.fcoords.field->get_height();
		float val2 = field_brn.fcoords.field->get_height();
		val1 /= MAX_FIELD_HEIGHT;
		val2 /= MAX_FIELD_HEIGHT;
		assert(val1 >= 0.f && val1 <= 1.f);
		assert(val2 >= 0.f && val2 <= 1.f);

		Arguments arg;
		arg.z_value = z_value;
		arg.blend_mode = BlendMode::Default;
		arg.triangle[0].point = field.gl_position;
		arg.triangle[0].color_r = val1;
		arg.triangle[0].color_g = 0.f;
		arg.triangle[0].color_b = 1.f - val1;
		arg.triangle[0].color_a = kAlpha;
		arg.triangle[1].point = field_brn.gl_position;
		arg.triangle[1].color_r = val2;
		arg.triangle[1].color_g = 0.f;
		arg.triangle[1].color_b = 1.f - val2;
		arg.triangle[1].color_a = kAlpha;

		if (field.rn_index != FieldsToDraw::kInvalidIndex) {
			const FieldsToDraw::Field& field_rn = fields_to_draw.at(field.rn_index);
			float val3 = field_rn.fcoords.field->get_height();
			val3 /= MAX_FIELD_HEIGHT;
			assert(val3 >= 0.f && val3 <= 1.f);

			arg.triangle[2].point = field_rn.gl_position;
			arg.triangle[2].color_r = val3;
			arg.triangle[2].color_g = 0.f;
			arg.triangle[2].color_b = 1.f - val3;
			arg.triangle[2].color_a = kAlpha;

			arguments.push_back(arg);
		}

		if (field.bln_index != FieldsToDraw::kInvalidIndex) {
			const FieldsToDraw::Field& field_bln = fields_to_draw.at(field.bln_index);
			float val3 = field_bln.fcoords.field->get_height();
			val3 /= MAX_FIELD_HEIGHT;
			assert(val3 >= 0.f && val3 <= 1.f);

			arg.triangle[2].point = field_bln.gl_position;
			arg.triangle[2].color_r = val3;
			arg.triangle[2].color_g = 0.f;
			arg.triangle[2].color_b = 1.f - val3;
			arg.triangle[2].color_a = kAlpha;

			arguments.push_back(arg);
		}
	}

	draw(arguments);
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

		default:
			NEVER_HERE();
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

			for (const Arguments::Vertex& vertex : current_args.triangle) {
				vertices_.emplace_back(vertex.point.x, vertex.point.y, current_args.z_value, vertex.color_r, vertex.color_g, vertex.color_b, vertex.color_a);
			}

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

		default:
			NEVER_HERE();
		}
	}
}
