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

#include "graphic/gl/blit_program.h"

#include <vector>

#include "base/log.h"
#include "graphic/gl/utils.h"

namespace  {

const char kBlitVertexShader[] = R"(
#version 120

// Attributes.
attribute vec3 attr_position;
attribute vec2 attr_texture_position;

varying vec2 out_texture_coordinate;

void main() {
	out_texture_coordinate = attr_texture_position;
	gl_Position = vec4(attr_position, 1.);
}
)";

const char kVanillaBlitFragmentShader[] = R"(
#version 120

uniform float u_opacity;
uniform sampler2D u_texture;

varying vec2 out_texture_coordinate;

void main() {
	vec4 color = texture2D(u_texture, out_texture_coordinate);
	gl_FragColor = vec4(color.rgb, u_opacity * color.a);
}
)";

const char kMonochromeBlitFragmentShader[] = R"(
#version 120

uniform float u_opacity;
uniform sampler2D u_texture;
uniform vec3 u_blend;

varying vec2 out_texture_coordinate;

void main() {
	vec4 texture_color = texture2D(u_texture, out_texture_coordinate);

	// See http://en.wikipedia.org/wiki/YUV.
	float luminance = dot(vec3(0.299, 0.587, 0.114), texture_color.rgb);

	gl_FragColor = vec4(vec3(luminance) * u_blend, u_opacity * texture_color.a);
}
)";

const char kBlendedBlitFragmentShader[] = R"(
#version 120

uniform float u_opacity;
uniform sampler2D u_texture;
uniform sampler2D u_mask;
uniform vec3 u_blend;

varying vec2 out_texture_coordinate;

void main() {
	vec4 texture_color = texture2D(u_texture, out_texture_coordinate);
	vec4 mask_color = texture2D(u_mask, out_texture_coordinate);

	// See http://en.wikipedia.org/wiki/YUV.
	float luminance = dot(vec3(0.299, 0.587, 0.114), texture_color.rgb);
	float blend_influence = mask_color.r * mask_color.a;
	gl_FragColor = vec4(
	   mix(texture_color.rgb, u_blend * luminance, blend_influence), u_opacity * texture_color.a);
}
)";

}  // namespace

class BlitProgram {
public:
	BlitProgram(const std::string& fragment_shader);

	void activate(const FloatRect& gl_dest_rect,
	              const FloatRect& gl_src_rect,
	              const float z_value,
	              const GLuint gl_texture,
	              const float opacity,
	              const BlendMode blend_mode);

	void draw_and_deactivate(BlendMode blend_mode);

	GLuint program_object() const {
		return gl_program_.object();
	}

private:
	struct PerVertexData {
		PerVertexData(float init_gl_x,
		              float init_gl_y,
		              float init_gl_z,
		              float init_texture_x,
		              float init_texture_y)
		   : gl_x(init_gl_x),
		     gl_y(init_gl_y),
		     gl_z(init_gl_z),
		     texture_x(init_texture_x),
		     texture_y(init_texture_y) {
		}

		float gl_x, gl_y, gl_z;
		float texture_x, texture_y;
	};
	static_assert(sizeof(PerVertexData) == 20, "Wrong padding.");

	// The buffer that will contain the quad for rendering.
	Gl::Buffer gl_array_buffer_;

	// The program.
	Gl::Program gl_program_;

	// Attributes.
	GLint attr_position_;
	GLint attr_texture_position_;

	// Uniforms.
	// NOCOM(#sirver): opacity should be an attribute too - might just make color one.
	GLint u_opacity_;
	GLint u_texture_;

	DISALLOW_COPY_AND_ASSIGN(BlitProgram);
};

BlitProgram::BlitProgram(const std::string& fragment_shader) {
	gl_program_.build(kBlitVertexShader, fragment_shader.c_str());

	attr_position_ = glGetAttribLocation(gl_program_.object(), "attr_position");
	attr_texture_position_ = glGetAttribLocation(gl_program_.object(), "attr_texture_position");

	u_texture_ = glGetUniformLocation(gl_program_.object(), "u_texture");
	u_opacity_ = glGetUniformLocation(gl_program_.object(), "u_opacity");

}

void BlitProgram::activate(const FloatRect& gl_dest_rect,
                       const FloatRect& gl_src_rect,
							  const float z_value,
                       const GLuint gl_texture,
							  const float opacity,
                       const BlendMode blend_mode) {
	glUseProgram(gl_program_.object());

	glEnableVertexAttribArray(attr_position_);
	glEnableVertexAttribArray(attr_texture_position_);

	std::vector<PerVertexData> vertices;
	vertices.reserve(4);
	vertices.emplace_back(gl_dest_rect.x, gl_dest_rect.y, z_value, gl_src_rect.x, gl_src_rect.y);
	vertices.emplace_back(gl_dest_rect.x + gl_dest_rect.w,
	                      gl_dest_rect.y,
	                      z_value,
	                      gl_src_rect.x + gl_src_rect.w,
	                      gl_src_rect.y);

	vertices.emplace_back(gl_dest_rect.x,
	                      gl_dest_rect.y + gl_dest_rect.h,
	                      z_value,
	                      gl_src_rect.x,
	                      gl_src_rect.y + gl_src_rect.h);

	vertices.emplace_back(gl_dest_rect.x + gl_dest_rect.w,
	                      gl_dest_rect.y + gl_dest_rect.h,
	                      z_value,
	                      gl_src_rect.x + gl_src_rect.w,
	                      gl_src_rect.y + gl_src_rect.h);

	glBindBuffer(GL_ARRAY_BUFFER, gl_array_buffer_.object());
	glBufferData(
	   GL_ARRAY_BUFFER, sizeof(PerVertexData) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);

	const auto set_attrib_pointer = [](const int vertex_index, int num_items, int offset) {
		glVertexAttribPointer(vertex_index,
		                      num_items,
		                      GL_FLOAT,
		                      GL_FALSE,
		                      sizeof(PerVertexData),
		                      reinterpret_cast<void*>(offset));
	};
	set_attrib_pointer(attr_position_, 3, offsetof(PerVertexData, gl_x));
	set_attrib_pointer(attr_texture_position_, 2, offsetof(PerVertexData, texture_x));

	glUniform1i(u_texture_, 0);
	glUniform1f(u_opacity_, opacity);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gl_texture);

	if (blend_mode == BlendMode::Copy) {
		glBlendFunc(GL_ONE, GL_ZERO);
	}
}

void BlitProgram::draw_and_deactivate(BlendMode blend_mode) {
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	if (blend_mode == BlendMode::Copy) {
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glDisableVertexAttribArray(attr_position_);
	glDisableVertexAttribArray(attr_texture_position_);

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
                              const FloatRect& gl_src_rect,
                              const float z_value,
                              const GLuint gl_texture,
                              const float opacity,
                              const BlendMode blend_mode) {
	blit_program_->activate(gl_dest_rect, gl_src_rect, z_value, gl_texture, opacity, blend_mode);
	blit_program_->draw_and_deactivate(blend_mode);
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

	u_blend_ = glGetUniformLocation(blit_program_->program_object(), "u_blend");
}

void MonochromeBlitProgram::draw(const FloatRect& gl_dest_rect,
                                 const FloatRect& gl_src_rect,
                                 const float z_value,
                                 const GLuint gl_texture,
                                 const RGBAColor& blend) {
	blit_program_->activate(
	   gl_dest_rect, gl_src_rect, z_value, gl_texture, blend.a / 255., BlendMode::UseAlpha);

	glUniform3f(u_blend_, blend.r / 255., blend.g / 255., blend.b / 255.);

	blit_program_->draw_and_deactivate(BlendMode::UseAlpha);
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
	u_blend_ = glGetUniformLocation(blit_program_->program_object(), "u_blend");
	u_mask_ = glGetUniformLocation(blit_program_->program_object(), "u_mask");
}

void BlendedBlitProgram::draw(const FloatRect& gl_dest_rect,
                              const FloatRect& gl_src_rect,
                              const float z_value,
                              const GLuint gl_texture_image,
                              const GLuint gl_texture_mask,
                              const RGBAColor& blend) {
	blit_program_->activate(
	   gl_dest_rect, gl_src_rect, z_value, gl_texture_image, blend.a / 255., BlendMode::UseAlpha);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gl_texture_mask);
	glUniform1i(u_mask_, 1);

	glUniform3f(u_blend_, blend.r / 255., blend.g / 255., blend.b / 255.);

	blit_program_->draw_and_deactivate(BlendMode::UseAlpha);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
