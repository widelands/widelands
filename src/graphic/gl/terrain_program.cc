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

#include "graphic/gl/terrain_program.h"

#include "graphic/gl/fields_to_draw.h"
#include "graphic/texture.h"

namespace  {

using namespace Widelands;

// QuickRef:
// http://www.cs.unh.edu/~cs770/docs/glsl-1.20-quickref.pdf
// Full specification:
// http://www.opengl.org/registry/doc/GLSLangSpec.Full.1.20.8.pdf
// We target OpenGL 2.1 for the desktop here.
const char kTerrainVertexShader[] = R"(
#version 120

// Attributes.
attribute float attr_brightness;
attribute vec2 attr_position;
attribute vec2 attr_texture_offset;
attribute vec2 attr_texture_position;

// Output of vertex shader.
varying float var_brightness;
varying vec2 var_texture_offset;
varying vec2 var_texture_position;

void main() {
	var_texture_position = attr_texture_position;
	var_brightness = attr_brightness;
	var_texture_offset = attr_texture_offset;
	gl_Position = vec4(attr_position, 0., 1.);
}
)";

const char kTerrainFragmentShader[] = R"(
#version 120

uniform sampler2D u_terrain_texture;
uniform vec2 u_texture_dimensions;

varying float var_brightness;
varying vec2 var_texture_position;
varying vec2 var_texture_offset;

void main() {
	vec4 clr = texture2D(u_terrain_texture,
			var_texture_offset + u_texture_dimensions * fract(var_texture_position));
	clr.rgb *= var_brightness;
	gl_FragColor = clr;
}
)";

}  // namespace

TerrainProgram::TerrainProgram() {
	gl_program_.build(kTerrainVertexShader, kTerrainFragmentShader);

	attr_brightness_ = glGetAttribLocation(gl_program_.object(), "attr_brightness");
	attr_position_ = glGetAttribLocation(gl_program_.object(), "attr_position");
	attr_texture_offset_ = glGetAttribLocation(gl_program_.object(), "attr_texture_offset");
	attr_texture_position_ = glGetAttribLocation(gl_program_.object(), "attr_texture_position");

	u_terrain_texture_ = glGetUniformLocation(gl_program_.object(), "u_terrain_texture");
	u_texture_dimensions_ = glGetUniformLocation(gl_program_.object(), "u_texture_dimensions");
}

void TerrainProgram::gl_draw(int gl_texture, float texture_w, float texture_h) {
	glUseProgram(gl_program_.object());

	glEnableVertexAttribArray(attr_brightness_);
	glEnableVertexAttribArray(attr_position_);
	glEnableVertexAttribArray(attr_texture_offset_);
	glEnableVertexAttribArray(attr_texture_position_);

	glBindBuffer(GL_ARRAY_BUFFER, gl_array_buffer_.object());
	glBufferData(GL_ARRAY_BUFFER,
	             sizeof(TerrainProgram::PerVertexData) * vertices_.size(),
	             vertices_.data(),
	             GL_STREAM_DRAW);

	const auto set_attrib_pointer = [](const int vertex_index, int num_items, int offset) {
		glVertexAttribPointer(vertex_index,
		                      num_items,
		                      GL_FLOAT,
		                      GL_FALSE,
		                      sizeof(TerrainProgram::PerVertexData),
		                      reinterpret_cast<void*>(offset));
	};
	set_attrib_pointer(attr_brightness_, 1, offsetof(PerVertexData, brightness));
	set_attrib_pointer(attr_position_, 2, offsetof(PerVertexData, gl_x));
	set_attrib_pointer(attr_texture_offset_, 2, offsetof(PerVertexData, texture_offset_x));
	set_attrib_pointer(attr_texture_position_, 2, offsetof(PerVertexData, texture_x));

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gl_texture);

	glUniform1i(u_terrain_texture_, 0);
	glUniform2f(u_texture_dimensions_, texture_w, texture_h);

	glDrawArrays(GL_TRIANGLES, 0, vertices_.size());

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisableVertexAttribArray(attr_brightness_);
	glDisableVertexAttribArray(attr_position_);
	glDisableVertexAttribArray(attr_texture_offset_);
	glDisableVertexAttribArray(attr_texture_position_);
}

void TerrainProgram::add_vertex(const FieldsToDraw::Field& field,
                                const FloatPoint& texture_offset) {
	vertices_.emplace_back();
	PerVertexData& back = vertices_.back();

	back.gl_x = field.gl_x;
	back.gl_y = field.gl_y;
	back.brightness = field.brightness;
	back.texture_x = field.texture_x;
	back.texture_y = field.texture_y;
	back.texture_offset_x = texture_offset.x;
	back.texture_offset_y = texture_offset.y;
}

void TerrainProgram::draw(uint32_t gametime,
                          const DescriptionMaintainer<TerrainDescription>& terrains,
                          const FieldsToDraw& fields_to_draw) {
	// This method expects that all terrains have the same dimensions and that
	// all are packed into the same texture atlas, i.e. all are in the same GL
	// texture. It does not check for this invariance for speeds sake.

	vertices_.clear();
	vertices_.reserve(fields_to_draw.size() * 3);

	for (size_t current_index = 0; current_index < fields_to_draw.size(); ++current_index) {
		const FieldsToDraw::Field& field = fields_to_draw.at(current_index);

		// The bottom right neighbor fields_to_draw is needed for both triangles
		// associated with this field. If it is not in fields_to_draw, there is no need to
		// draw any triangles.
		const int brn_index = fields_to_draw.calculate_index(field.fx + (field.fy & 1), field.fy + 1);
		if (brn_index == -1) {
			continue;
		}

		// Down triangle.
		const int bln_index =
		   fields_to_draw.calculate_index(field.fx + (field.fy & 1) - 1, field.fy + 1);
		if (bln_index != -1) {
			const FloatPoint texture_offset =
			   terrains.get(field.ter_d).get_texture(gametime).texture_coordinates().top_left();
			add_vertex(fields_to_draw.at(current_index), texture_offset);
			add_vertex(fields_to_draw.at(bln_index), texture_offset);
			add_vertex(fields_to_draw.at(brn_index), texture_offset);
		}

		// Right triangle.
		const int rn_index = fields_to_draw.calculate_index(field.fx + 1, field.fy);
		if (rn_index != -1) {
			const FloatPoint texture_offset =
			   terrains.get(field.ter_r).get_texture(gametime).texture_coordinates().top_left();
			add_vertex(fields_to_draw.at(current_index), texture_offset);
			add_vertex(fields_to_draw.at(brn_index), texture_offset);
			add_vertex(fields_to_draw.at(rn_index), texture_offset);
		}
	}

	const Texture& texture = terrains.get(0).get_texture(0);
	gl_draw(texture.get_gl_texture(), texture.texture_coordinates().w, texture.texture_coordinates().h);
}
