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

#include "graphic/gl/blit_program.h"

#include "graphic/blit_mode.h"
#include "graphic/gl/blit_data.h"
#include "graphic/gl/coordinate_conversion.h"
#include "graphic/gl/utils.h"

namespace {

// While drawing we put all draw calls into a buffer, so that we have to
// transfer the buffer to the GPU only once, even though we might need to do
// many glDraw* calls. This structure represents the parameters for one glDraw*
// call.
struct DrawBatch {
	int offset;
	int count;
	uint32_t texture;
	uint32_t mask;
	BlendMode blend_mode;
};

}  // namespace

BlitProgram::BlitProgram() {
	gl_program_.build("blit");

	attr_blend_ = glGetAttribLocation(gl_program_.object(), "attr_blend");
	attr_mask_texture_position_ =
	   glGetAttribLocation(gl_program_.object(), "attr_mask_texture_position");
	attr_position_ = glGetAttribLocation(gl_program_.object(), "attr_position");
	attr_texture_position_ = glGetAttribLocation(gl_program_.object(), "attr_texture_position");
	attr_program_flavor_ = glGetAttribLocation(gl_program_.object(), "attr_program_flavor");

	u_texture_ = glGetUniformLocation(gl_program_.object(), "u_texture");
	u_mask_ = glGetUniformLocation(gl_program_.object(), "u_mask");
}

void BlitProgram::draw(const std::vector<Arguments>& arguments) {
	glUseProgram(gl_program_.object());

	auto& gl_state = Gl::State::instance();

	gl_state.enable_vertex_attrib_array({attr_blend_, attr_mask_texture_position_, attr_position_,
	                                     attr_texture_position_, attr_program_flavor_});

	gl_array_buffer_.bind();

	Gl::vertex_attrib_pointer(
	   attr_blend_, 4, sizeof(PerVertexData), offsetof(PerVertexData, blend_r));
	Gl::vertex_attrib_pointer(attr_mask_texture_position_, 2, sizeof(PerVertexData),
	                          offsetof(PerVertexData, mask_texture_x));
	Gl::vertex_attrib_pointer(
	   attr_position_, 3, sizeof(PerVertexData), offsetof(PerVertexData, gl_x));
	Gl::vertex_attrib_pointer(
	   attr_texture_position_, 2, sizeof(PerVertexData), offsetof(PerVertexData, texture_x));
	Gl::vertex_attrib_pointer(
	   attr_program_flavor_, 1, sizeof(PerVertexData), offsetof(PerVertexData, program_flavor));

	glUniform1i(u_texture_, 0);
	glUniform1i(u_mask_, 1);

	// Prepare the buffer for many draw calls.
	std::vector<DrawBatch> draw_batches;
	int offset = 0;
	vertices_.clear();

	size_t i = 0;
	while (i < arguments.size()) {
		const auto& template_args = arguments[i];

		// Batch common blit operations up.
		while (i < arguments.size()) {
			const auto& current_args = arguments[i];
			if (current_args.blend_mode != template_args.blend_mode ||
			    current_args.texture.texture_id != template_args.texture.texture_id ||
			    (current_args.mask.texture_id != 0 &&
			     current_args.mask.texture_id != template_args.mask.texture_id)) {
				break;
			}

			const float blend_r = current_args.blend.r / 255.;
			const float blend_g = current_args.blend.g / 255.;
			const float blend_b = current_args.blend.b / 255.;
			const float blend_a = current_args.blend.a / 255.;

			const Rectf texture_rect = to_gl_texture(current_args.texture);
			const Rectf mask_rect = to_gl_texture(current_args.mask);
			float program_flavor = 0;
			switch (current_args.blit_mode) {
			case BlitMode::kDirect:
				program_flavor = 0.;
				break;

			case BlitMode::kMonochrome:
				program_flavor = 1.;
				break;

			case BlitMode::kBlendedWithMask:
				program_flavor = 2.;
				break;
			}

			vertices_.emplace_back(current_args.destination_rect.x, current_args.destination_rect.y,
			                       current_args.z_value, texture_rect.x, texture_rect.y, mask_rect.x,
			                       mask_rect.y, blend_r, blend_g, blend_b, blend_a, program_flavor);

			vertices_.emplace_back(current_args.destination_rect.x + current_args.destination_rect.w,
			                       current_args.destination_rect.y, current_args.z_value,
			                       texture_rect.x + texture_rect.w, texture_rect.y,
			                       mask_rect.x + mask_rect.w, mask_rect.y, blend_r, blend_g, blend_b,
			                       blend_a, program_flavor);

			vertices_.emplace_back(
			   current_args.destination_rect.x,
			   current_args.destination_rect.y + current_args.destination_rect.h, current_args.z_value,
			   texture_rect.x, texture_rect.y + texture_rect.h, mask_rect.x, mask_rect.y + mask_rect.h,
			   blend_r, blend_g, blend_b, blend_a, program_flavor);

			vertices_.emplace_back(vertices_.at(vertices_.size() - 2));
			vertices_.emplace_back(vertices_.at(vertices_.size() - 2));

			vertices_.emplace_back(current_args.destination_rect.x + current_args.destination_rect.w,
			                       current_args.destination_rect.y + current_args.destination_rect.h,
			                       current_args.z_value, texture_rect.x + texture_rect.w,
			                       texture_rect.y + texture_rect.h, mask_rect.x + mask_rect.w,
			                       mask_rect.y + mask_rect.h, blend_r, blend_g, blend_b, blend_a,
			                       program_flavor);
			++i;
		}

		draw_batches.emplace_back(DrawBatch{offset, static_cast<int>(vertices_.size() - offset),
		                                    template_args.texture.texture_id,
		                                    template_args.mask.texture_id, template_args.blend_mode});
		offset = vertices_.size();
	}
	gl_array_buffer_.update(vertices_);

	// Now do the draw calls.
	for (const auto& draw_arg : draw_batches) {
		gl_state.bind(GL_TEXTURE0, draw_arg.texture);
		gl_state.bind(GL_TEXTURE1, draw_arg.mask);

		if (draw_arg.blend_mode == BlendMode::Copy) {
			glBlendFunc(GL_ONE, GL_ZERO);
		}
		glDrawArrays(GL_TRIANGLES, draw_arg.offset, draw_arg.count);

		if (draw_arg.blend_mode == BlendMode::Copy) {
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
	}
}

void BlitProgram::draw(const Rectf& gl_dest_rect,
                       const float z_value,
                       const BlitData& texture,
                       const BlitData& mask,
                       const RGBAColor& blend,
                       const BlendMode& blend_mode) {
	draw({Arguments{gl_dest_rect, z_value, texture, mask, blend, blend_mode,
	                mask.texture_id != 0 ? BlitMode::kBlendedWithMask : BlitMode::kDirect}});
}

void BlitProgram::draw_monochrome(const Rectf& dest_rect,
                                  const float z_value,
                                  const BlitData& texture,
                                  const RGBAColor& blend) {
	draw({Arguments{dest_rect, z_value, texture, BlitData{0, 0, 0, Rectf()}, blend,
	                BlendMode::UseAlpha, BlitMode::kMonochrome}});
}

// static
BlitProgram& BlitProgram::instance() {
	static BlitProgram blit_program;
	return blit_program;
}
