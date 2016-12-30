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

#include "graphic/gl/blit_program.h"

#include <memory>
#include <vector>

#include "base/log.h"
#include "graphic/blit_mode.h"
#include "graphic/gl/blit_data.h"
#include "graphic/gl/coordinate_conversion.h"
#include "graphic/gl/streaming_buffer.h"
#include "graphic/gl/utils.h"
#include "profile/profile.h"

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

class BlitProgramGl2 : public BlitProgram {
public:
	BlitProgramGl2();

	void draw(const std::vector<Arguments>& arguments) override;

private:
	struct PerVertexData {
		PerVertexData(float init_gl_x,
		              float init_gl_y,
		              float init_gl_z,
		              float init_texture_x,
		              float init_texture_y,
		              float init_mask_texture_x,
		              float init_mask_texture_y,
		              float init_blend_r,
		              float init_blend_g,
		              float init_blend_b,
		              float init_blend_a,
		              float init_program_flavor)
		   : gl_x(init_gl_x),
		     gl_y(init_gl_y),
		     gl_z(init_gl_z),
		     texture_x(init_texture_x),
		     texture_y(init_texture_y),
		     mask_texture_x(init_mask_texture_x),
		     mask_texture_y(init_mask_texture_y),
		     blend_r(init_blend_r),
		     blend_g(init_blend_g),
		     blend_b(init_blend_b),
		     blend_a(init_blend_a),
		     program_flavor(init_program_flavor) {
		}

		float gl_x, gl_y, gl_z;
		float texture_x, texture_y;
		float mask_texture_x, mask_texture_y;
		float blend_r, blend_g, blend_b, blend_a;
		float program_flavor;
	};
	static_assert(sizeof(PerVertexData) == 48, "Wrong padding.");

	// The buffer that will contain the quad for rendering.
	Gl::Buffer<PerVertexData> gl_array_buffer_;

	// The program.
	Gl::Program gl_program_;

	// Attributes.
	GLint attr_blend_;
	GLint attr_mask_texture_position_;
	GLint attr_position_;
	GLint attr_texture_position_;
	GLint attr_program_flavor_;

	// Uniforms.
	GLint u_texture_;
	GLint u_mask_;

	// Cached for efficiency.
	std::vector<PerVertexData> vertices_;
};

class BlitProgramGl4 : public BlitProgram {
public:
	BlitProgramGl4();

	static bool supported();

	void draw(const std::vector<Arguments>& arguments) override;

private:
	struct PerRectData {
		float dst_x, dst_y, dst_width, dst_height;
		uint32_t src_x, src_y, src_width, src_height;
		uint32_t src_parent_width, src_parent_height;
		uint32_t mask_x, mask_y, mask_width, mask_height;
		uint32_t mask_parent_width, mask_parent_height;
		uint32_t blend_r, blend_g, blend_b, blend_a;
		float program_flavor, z;

		// Standard OpenGL packing aligns arrays to a multiple of 16 bytes.
		float padding[2];
	};
	static_assert(sizeof(PerRectData) == 96, "Wrong padding.");

	void setup_index_buffer(unsigned num_rects);

	// The index buffer.
	Gl::Buffer<uint16_t> gl_index_buffer_;
	unsigned num_index_rects_;

	// The per-rect data buffer.
	Gl::StreamingBuffer<PerRectData> gl_rects_buffer_;

	// The program.
	Gl::Program gl_program_;

	// Uniform locations.
	GLint u_texture_;
	GLint u_mask_;
};

}  // namespace

BlitProgram::BlitProgram() {
}

BlitProgram::~BlitProgram() {
}

BlitProgramGl2::BlitProgramGl2() {
	log("Using GL2 rendering path\n");

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

void BlitProgramGl2::draw(const std::vector<Arguments>& arguments) {
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

BlitProgramGl4::BlitProgramGl4()
  : gl_rects_buffer_(GL_ARRAY_BUFFER) {
	log("Using GL4 rendering path\n");

	gl_program_.build_vp_fp({"blit_gl4"}, {"blit"});

	u_texture_ = glGetUniformLocation(gl_program_.object(), "u_texture");
	u_mask_ = glGetUniformLocation(gl_program_.object(), "u_mask");

	num_index_rects_ = 0;
}

bool BlitProgramGl4::supported() {
	// TODO(nha): implement this
	// GLSL >= 1.30
	// ARB_separate_shader_objects
	// ARB_shader_storage_buffer_object
	return !g_options.pull_section("global").get_bool("disable_gl4", false);
}

void BlitProgramGl4::draw(const std::vector<Arguments>& arguments) {
	glUseProgram(gl_program_.object());

	auto& gl_state = Gl::State::instance();

	gl_state.enable_vertex_attrib_array({});

	glUniform1i(u_texture_, 0);
	glUniform1i(u_mask_, 1);

	// Prepare the buffer for many draw calls.
	std::vector<DrawBatch> draw_batches;
	auto rects = gl_rects_buffer_.stream(arguments.size());

	size_t i = 0;
	while (i < arguments.size()) {
		const auto& template_args = arguments[i];
		const int start = i;

		// Batch common blit operations up.
		while (i < arguments.size()) {
			const auto& current_args = arguments[i];
			if (current_args.blend_mode != template_args.blend_mode ||
			    current_args.texture.texture_id != template_args.texture.texture_id ||
			    (current_args.mask.texture_id != 0 &&
			     current_args.mask.texture_id != template_args.mask.texture_id)) {
				break;
			}

			rects.emplace_back();
			auto& rect = rects.back();
			rect.dst_x = current_args.destination_rect.x;
			rect.dst_y = current_args.destination_rect.y;
			rect.dst_width = current_args.destination_rect.w;
			rect.dst_height = current_args.destination_rect.h;

			rect.src_x = current_args.texture.rect.x;
			rect.src_y = current_args.texture.rect.y;
			rect.src_width = current_args.texture.rect.w;
			rect.src_height = current_args.texture.rect.h;
			rect.src_parent_width = current_args.texture.parent_width;
			rect.src_parent_height = current_args.texture.parent_height;

			rect.mask_x = current_args.mask.rect.x;
			rect.mask_y = current_args.mask.rect.y;
			rect.mask_width = current_args.mask.rect.w;
			rect.mask_height = current_args.mask.rect.h;
			rect.mask_parent_width = current_args.mask.parent_width;
			rect.mask_parent_height = current_args.mask.parent_height;

			rect.blend_r = current_args.blend.r;
			rect.blend_g = current_args.blend.g;
			rect.blend_b = current_args.blend.b;
			rect.blend_a = current_args.blend.a;

			switch (current_args.blit_mode) {
			case BlitMode::kDirect:
				rect.program_flavor = 0.;
				break;

			case BlitMode::kMonochrome:
				rect.program_flavor = 1.;
				break;

			case BlitMode::kBlendedWithMask:
				rect.program_flavor = 2.;
				break;
			}

			rect.z = current_args.z_value;

			++i;
		}

		draw_batches.emplace_back(DrawBatch{int(start), int(i - start),
		                                    template_args.texture.texture_id,
		                                    template_args.mask.texture_id, template_args.blend_mode});
	}

	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, gl_rects_buffer_.object(),
					  rects.unmap(), i * sizeof(PerRectData));

	setup_index_buffer(i);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_index_buffer_.object());

	// Now do the draw calls.
	for (const auto& draw_arg : draw_batches) {
		gl_state.bind(GL_TEXTURE0, draw_arg.texture);
		gl_state.bind(GL_TEXTURE1, draw_arg.mask);

		if (draw_arg.blend_mode == BlendMode::Copy) {
			glBlendFunc(GL_ONE, GL_ZERO);
		}

		glDrawElements(GL_TRIANGLES, 6 * draw_arg.count, GL_UNSIGNED_SHORT,
		               static_cast<uint16_t*>(nullptr) + 6 * draw_arg.offset);

		if (draw_arg.blend_mode == BlendMode::Copy) {
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
	}

	// TODO(nha): bind via state
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void BlitProgramGl4::setup_index_buffer(unsigned num_rects)
{
	if (num_rects <= num_index_rects_)
		return;

	if (num_rects > 65536 / 4)
		throw wexception("Too many rectangles for 16-bit indices");

	std::vector<uint16_t> indices;
	indices.reserve(num_rects * 6);

	for (unsigned i = 0; i < num_rects; ++i) {
		indices.push_back(4 * i);
		indices.push_back(4 * i + 1);
		indices.push_back(4 * i + 2);

		indices.push_back(4 * i + 2);
		indices.push_back(4 * i + 1);
		indices.push_back(4 * i + 3);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_index_buffer_.object());
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * indices.size(),
	             indices.data(), GL_STATIC_DRAW);
	num_index_rects_ = num_rects;
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

namespace {

class BlitProgramHolder {
public:
	BlitProgramHolder() {
		if (BlitProgramGl4::supported())
			program_.reset(new BlitProgramGl4);
		else
			program_.reset(new BlitProgramGl2);
	}

	BlitProgram& program() {
		return *program_;
	}

private:
	std::unique_ptr<BlitProgram> program_;
};

} // namespace

// static
BlitProgram& BlitProgram::instance() {
	static BlitProgramHolder holder;
	return holder.program();
}
