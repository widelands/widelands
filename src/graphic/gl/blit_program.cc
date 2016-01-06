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

#include <vector>

#include "base/log.h"
#include "graphic/gl/blit_data.h"
#include "graphic/gl/coordinate_conversion.h"
#include "graphic/gl/utils.h"

namespace  {

const char kBlitVertexShader[] = R"(
#version 120

// Attributes.
attribute vec2 attr_mask_texture_position;
attribute vec2 attr_texture_position;
attribute vec3 attr_position;
attribute vec4 attr_blend;

varying vec2 out_mask_texture_coordinate;
varying vec2 out_texture_coordinate;
varying vec4 out_blend;

void main() {
	out_mask_texture_coordinate = attr_mask_texture_position;
	out_texture_coordinate = attr_texture_position;
	out_blend = attr_blend;
	gl_Position = vec4(attr_position, 1.);
}
)";

const char kVanillaBlitFragmentShader[] = R"(
#version 120

uniform sampler2D u_texture;

varying vec2 out_texture_coordinate;
varying vec4 out_blend;

void main() {
	vec4 color = texture2D(u_texture, out_texture_coordinate);
	gl_FragColor = color * out_blend;
}
)";

const char kMonochromeBlitFragmentShader[] = R"(
#version 120

uniform sampler2D u_texture;

varying vec2 out_texture_coordinate;
varying vec4 out_blend;

void main() {
	vec4 texture_color = texture2D(u_texture, out_texture_coordinate);

	// See http://en.wikipedia.org/wiki/YUV.
	float luminance = dot(vec3(0.299, 0.587, 0.114), texture_color.rgb);

	gl_FragColor = vec4(vec3(luminance) * out_blend.rgb, out_blend.a * texture_color.a);
}
)";

const char kBlendedBlitFragmentShader[] = R"(
#version 120

uniform sampler2D u_texture;
uniform sampler2D u_mask;

varying vec2 out_mask_texture_coordinate;
varying vec2 out_texture_coordinate;
varying vec4 out_blend;

void main() {
	vec4 texture_color = texture2D(u_texture, out_texture_coordinate);
	vec4 mask_color = texture2D(u_mask, out_mask_texture_coordinate);

	// See http://en.wikipedia.org/wiki/YUV.
	float luminance = dot(vec3(0.299, 0.587, 0.114), texture_color.rgb);
	float blend_influence = mask_color.r * mask_color.a;
	gl_FragColor = vec4(
		mix(texture_color.rgb, out_blend.rgb * luminance, blend_influence), out_blend.a * texture_color.a);
}
)";

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

class BlitProgram {
public:
	struct Arguments {
		FloatRect destination_rect;
		float z_value;
		BlitData texture;
		BlitData mask;
		RGBAColor blend;
		BlendMode blend_mode;
	};
	BlitProgram(const std::string& fragment_shader);

	void activate();

	void draw_and_deactivate(const std::vector<Arguments>& arguments);

	int program_object() const {
		return gl_program_.object();
	}

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
		              float init_blend_a)
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
		     blend_a(init_blend_a) {
		}

		float gl_x, gl_y, gl_z;
		float texture_x, texture_y;
		float mask_texture_x, mask_texture_y;
		float blend_r, blend_g, blend_b, blend_a;
	};
	static_assert(sizeof(PerVertexData) == 44, "Wrong padding.");

	// The buffer that will contain the quad for rendering.
	Gl::Buffer<PerVertexData> gl_array_buffer_;

	// The program.
	Gl::Program gl_program_;

	// Attributes.
	GLint attr_blend_;
	GLint attr_mask_texture_position_;
	GLint attr_position_;
	GLint attr_texture_position_;

	// Uniforms.
	GLint u_texture_;
	GLint u_mask_;

	// Cached for efficiency.
	std::vector<PerVertexData> vertices_;

	DISALLOW_COPY_AND_ASSIGN(BlitProgram);
};

BlitProgram::BlitProgram(const std::string& fragment_shader) {
	gl_program_.build(kBlitVertexShader, fragment_shader.c_str());

	attr_blend_ = glGetAttribLocation(gl_program_.object(), "attr_blend");
	attr_mask_texture_position_ = glGetAttribLocation(gl_program_.object(), "attr_mask_texture_position");
	attr_position_ = glGetAttribLocation(gl_program_.object(), "attr_position");
	attr_texture_position_ = glGetAttribLocation(gl_program_.object(), "attr_texture_position");

	u_texture_ = glGetUniformLocation(gl_program_.object(), "u_texture");
	u_mask_ = glGetUniformLocation(gl_program_.object(), "u_mask");
}

void BlitProgram::activate() {
	glUseProgram(gl_program_.object());

	glEnableVertexAttribArray(attr_blend_);
	glEnableVertexAttribArray(attr_mask_texture_position_);
	glEnableVertexAttribArray(attr_position_);
	glEnableVertexAttribArray(attr_texture_position_);
}

void BlitProgram::draw_and_deactivate(const std::vector<Arguments>& arguments) {
	size_t i = 0;

	gl_array_buffer_.bind();

	Gl::vertex_attrib_pointer(attr_blend_, 4, sizeof(PerVertexData), offsetof(PerVertexData, blend_r));
	Gl::vertex_attrib_pointer(attr_mask_texture_position_,
	                       2,
	                       sizeof(PerVertexData),
	                       offsetof(PerVertexData, mask_texture_x));
	Gl::vertex_attrib_pointer(attr_position_, 3, sizeof(PerVertexData), offsetof(PerVertexData, gl_x));
	Gl::vertex_attrib_pointer(
	   attr_texture_position_, 2, sizeof(PerVertexData), offsetof(PerVertexData, texture_x));

	glUniform1i(u_texture_, 0);
	glUniform1i(u_mask_, 1);

	// Prepare the buffer for many draw calls.
	std::vector<DrawBatch> draw_batches;
	int offset = 0;
	vertices_.clear();
	while (i < arguments.size()) {
		const Arguments& template_args = arguments[i];

		// Batch common blit operations up.
		while (i < arguments.size()) {
			const Arguments& current_args = arguments[i];
			if (current_args.blend_mode != template_args.blend_mode ||
					current_args.texture.texture_id != template_args.texture.texture_id ||
					current_args.mask.texture_id != template_args.mask.texture_id) {
				break;
			}

			const float blend_r = current_args.blend.r / 255.;
			const float blend_g = current_args.blend.g / 255.;
			const float blend_b = current_args.blend.b / 255.;
			const float blend_a = current_args.blend.a / 255.;

			const FloatRect texture_rect = to_gl_texture(current_args.texture);
			const FloatRect mask_rect = to_gl_texture(current_args.mask);
			vertices_.emplace_back(current_args.destination_rect.x,
					current_args.destination_rect.y,
					current_args.z_value,
					texture_rect.x,
					texture_rect.y,
					mask_rect.x,
					mask_rect.y,
					blend_r,
					blend_g,
					blend_b,
					blend_a);

			vertices_.emplace_back(current_args.destination_rect.x + current_args.destination_rect.w,
					current_args.destination_rect.y,
					current_args.z_value,
					texture_rect.x + texture_rect.w,
					texture_rect.y,
					mask_rect.x + mask_rect.w,
					mask_rect.y,
					blend_r,
					blend_g,
					blend_b,
					blend_a);

			vertices_.emplace_back(current_args.destination_rect.x,
					current_args.destination_rect.y + current_args.destination_rect.h,
					current_args.z_value,
					texture_rect.x,
					texture_rect.y + texture_rect.h,
					mask_rect.x,
					mask_rect.y + mask_rect.h,
					blend_r,
					blend_g,
					blend_b,
					blend_a);

			vertices_.emplace_back(vertices_.at(vertices_.size() - 2));
			vertices_.emplace_back(vertices_.at(vertices_.size() - 2));

			vertices_.emplace_back(current_args.destination_rect.x + current_args.destination_rect.w,
					current_args.destination_rect.y + current_args.destination_rect.h,
					current_args.z_value,
					texture_rect.x + texture_rect.w,
					texture_rect.y + texture_rect.h,
					mask_rect.x + mask_rect.w,
					mask_rect.y + mask_rect.h,
					blend_r,
					blend_g,
					blend_b,
					blend_a);
			++i;
		}

		draw_batches.emplace_back(DrawBatch{offset,
		                                    static_cast<int>(vertices_.size() - offset),
		                                    template_args.texture.texture_id,
		                                    template_args.mask.texture_id,
		                                    template_args.blend_mode});
		offset = vertices_.size();
	}
	gl_array_buffer_.update(vertices_);

	// Now do the draw calls.
	for (const auto& draw_arg : draw_batches) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, draw_arg.texture);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, draw_arg.mask);

		if (draw_arg.blend_mode == BlendMode::Copy) {
			glBlendFunc(GL_ONE, GL_ZERO);
		}
		glDrawArrays(GL_TRIANGLES, draw_arg.offset, draw_arg.count);

		if (draw_arg.blend_mode == BlendMode::Copy) {
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
	}

	glDisableVertexAttribArray(attr_blend_);
	glDisableVertexAttribArray(attr_mask_texture_position_);
	glDisableVertexAttribArray(attr_position_);
	glDisableVertexAttribArray(attr_texture_position_);

	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// static
VanillaBlitProgram& VanillaBlitProgram::instance() {
	static VanillaBlitProgram blit_program;
	return blit_program;
}

VanillaBlitProgram::~VanillaBlitProgram() {
}

VanillaBlitProgram::VanillaBlitProgram() {
	blit_program_.reset(new BlitProgram(kVanillaBlitFragmentShader));
}

void VanillaBlitProgram::draw(const FloatRect& gl_dest_rect,
                              const float z_value,
										const BlitData& texture,
                              const float opacity,
                              const BlendMode blend_mode) {
	draw({Arguments{gl_dest_rect, z_value, texture, opacity, blend_mode}});
}

void VanillaBlitProgram::draw(const std::vector<Arguments>& arguments) {
	std::vector<BlitProgram::Arguments> blit_arguments;
	for (const Arguments arg : arguments) {
		blit_arguments.emplace_back(BlitProgram::Arguments{
		   arg.destination_rect,
		   arg.z_value,
		   arg.texture,
		   BlitData{0, 0, 0, Rect()},
		   RGBAColor(255, 255, 255, arg.opacity * 255),
		   arg.blend_mode,
		});
	}

	blit_program_->activate();
	blit_program_->draw_and_deactivate(blit_arguments);
}


// static
MonochromeBlitProgram& MonochromeBlitProgram::instance() {
	static MonochromeBlitProgram blit_program;
	return blit_program;
}

MonochromeBlitProgram::~MonochromeBlitProgram() {
}

MonochromeBlitProgram::MonochromeBlitProgram() {
	blit_program_.reset(new BlitProgram(kMonochromeBlitFragmentShader));
}

void MonochromeBlitProgram::draw(const FloatRect& dest_rect,
                                 const float z_value,
											const BlitData& texture,
                                 const RGBAColor& blend) {
	draw({Arguments{dest_rect, z_value, texture, blend, BlendMode::UseAlpha}});
}

void MonochromeBlitProgram::draw(const std::vector<Arguments>& arguments) {
	std::vector<BlitProgram::Arguments> blit_arguments;
	for (const Arguments arg : arguments) {
		blit_arguments.emplace_back(BlitProgram::Arguments{
		   arg.destination_rect,
		   arg.z_value,
		   arg.texture,
		   BlitData{0, 0, 0, Rect()},
		   arg.blend,
		   arg.blend_mode,
		});
	}

	blit_program_->activate();
	blit_program_->draw_and_deactivate(blit_arguments);
}

// static
BlendedBlitProgram& BlendedBlitProgram::instance() {
	static BlendedBlitProgram blit_program;
	return blit_program;
}

BlendedBlitProgram::~BlendedBlitProgram() {
}

BlendedBlitProgram::BlendedBlitProgram() {
	blit_program_.reset(new BlitProgram(kBlendedBlitFragmentShader));
}

void BlendedBlitProgram::draw(const FloatRect& gl_dest_rect,
                              const float z_value,
										const BlitData& texture,
										const BlitData& mask,
                              const RGBAColor& blend) {
	draw({Arguments{gl_dest_rect, z_value, texture, mask, blend, BlendMode::UseAlpha}});
}

void BlendedBlitProgram::draw(const std::vector<Arguments>& arguments) {
	std::vector<BlitProgram::Arguments> blit_arguments;
	for (const Arguments arg : arguments) {
		blit_arguments.emplace_back(BlitProgram::Arguments{
		   arg.destination_rect,
		   arg.z_value,
		   arg.texture,
			arg.mask,
		   arg.blend,
		   arg.blend_mode,
		});
	}

	blit_program_->activate();
	blit_program_->draw_and_deactivate(blit_arguments);
}
