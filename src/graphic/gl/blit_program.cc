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

namespace  {

const char kBlitVertexShader[] = R"(
#version 120

// Attributes.
attribute vec2 attr_position;

// Uniforms.
uniform vec4 u_dst_rect;
uniform vec4 u_src_rect;

varying vec2 out_texture_coordinate;

void main() {
	out_texture_coordinate = u_src_rect.xy + attr_position.xy * u_src_rect.zw;
	gl_Position = vec4(u_dst_rect.xy + attr_position.xy * u_dst_rect.zw, 0., 1.);
}
)";

const char kBlitFragmentShader[] = R"(
#version 120

uniform sampler2D u_texture;

varying vec2 out_texture_coordinate;

void main() {
	gl_FragColor = texture2D(u_texture, out_texture_coordinate);
}
)";

}  // namespace

// static
BlitProgram& BlitProgram::instance() {
	static BlitProgram blit_program;
	return blit_program;
}

BlitProgram::BlitProgram() {
	gl_program_.build(kBlitVertexShader, kBlitFragmentShader);

	attr_position_ = glGetAttribLocation(gl_program_.object(), "attr_position");

	u_texture_ = glGetUniformLocation(gl_program_.object(), "u_texture");
	u_dst_rect_ = glGetUniformLocation(gl_program_.object(), "u_dst_rect");
	u_src_rect_ = glGetUniformLocation(gl_program_.object(), "u_src_rect");

	std::vector<PerVertexData> vertices;
	vertices.push_back(PerVertexData
			{0., 1.});
	vertices.push_back(PerVertexData
			{1., 1.});
	vertices.push_back(PerVertexData
			{0., 0.});
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

void BlitProgram::draw(const FloatRect& gl_dest_rect,
                       const FloatRect& gl_src_rect,
                       const GLuint gl_texture,
                       const Composite composite) {
	glUseProgram(gl_program_.object());
	glEnableVertexAttribArray(attr_position_);
	glBindBuffer(GL_ARRAY_BUFFER, gl_array_buffer_.object());

	glVertexAttribPointer(attr_position_,
								 2,
								 GL_FLOAT,
								 GL_FALSE,
								 sizeof(PerVertexData),
								 reinterpret_cast<void*>(0));


	glUniform1i(u_texture_, 0);
	glUniform4f(u_dst_rect_, gl_dest_rect.x, gl_dest_rect.y, gl_dest_rect.w, gl_dest_rect.h);
	glUniform4f(u_src_rect_, gl_src_rect.x, gl_src_rect.y, gl_src_rect.w, gl_src_rect.h);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gl_texture);

	if (composite == CM_Copy) {
		glBlendFunc(GL_ONE, GL_ZERO);
	}

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	if (composite == CM_Copy) {
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glDisableVertexAttribArray(attr_position_);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
