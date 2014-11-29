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

#include "graphic/gl/terrain_program.h"

#include "graphic/gl/fields_to_draw.h"
#include "graphic/graphic.h"
#include "graphic/terrain_texture.h"
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
attribute vec2 attr_position;
attribute float attr_brightness;
attribute vec2 attr_texture_position;

// Output of vertex shader.
varying float var_brightness;
varying vec2 var_texture_position;

void main() {
	var_texture_position = attr_texture_position;
	var_brightness = attr_brightness;
	gl_Position = vec4(attr_position, 0., 1.);
}
)";

const char kTerrainFragmentShader[] = R"(
#version 120

uniform sampler2D u_terrain_texture;

varying float var_brightness;
varying vec2 var_texture_position;

void main() {
	vec4 clr = texture2D(u_terrain_texture, var_texture_position);
	clr.rgb *= var_brightness;
	gl_FragColor = clr;
}
)";

}  // namespace

TerrainProgram::TerrainProgram() {
	gl_program_.build(kTerrainVertexShader, kTerrainFragmentShader);

	attr_position_ = glGetAttribLocation(gl_program_.object(), "attr_position");
	attr_texture_position_ =
	   glGetAttribLocation(gl_program_.object(), "attr_texture_position");
	attr_brightness_ = glGetAttribLocation(gl_program_.object(), "attr_brightness");

	u_terrain_texture_ = glGetUniformLocation(gl_program_.object(), "u_terrain_texture");
}

void TerrainProgram::gl_draw(int num_vertices,
                             const DescriptionMaintainer<TerrainDescription>& terrains) {
	glUseProgram(gl_program_.object());

	glEnableVertexAttribArray(attr_position_);
	glEnableVertexAttribArray(attr_texture_position_);
	glEnableVertexAttribArray(attr_brightness_);

	glBindBuffer(GL_ARRAY_BUFFER, gl_array_buffer_.object());
	glBufferData(GL_ARRAY_BUFFER,
	             sizeof(TerrainProgram::PerVertexData) * num_vertices,
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
	set_attrib_pointer(attr_position_, 2, offsetof(PerVertexData, gl_x));
	set_attrib_pointer(attr_brightness_, 1, offsetof(PerVertexData, brightness));
	set_attrib_pointer(attr_texture_position_, 2, offsetof(PerVertexData, texture_x));

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Set the sampler texture unit to 0
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(u_terrain_texture_, 0);

	// Which triangles to draw?
	for (size_t i = 0; i < terrains_to_indices_.size(); ++i) {
		const auto& indices = terrains_to_indices_[i];
		if (indices.empty()) {
			continue;
		}
		glBindTexture(GL_TEXTURE_2D,
		              g_gr->get_maptexture_data(terrains.get_unmutable(i).get_texture())
		                 ->texture()
		                 .get_gl_texture());
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, indices.data());
	}

	glDisableVertexAttribArray(attr_position_);
	glDisableVertexAttribArray(attr_texture_position_);
	glDisableVertexAttribArray(attr_brightness_);
}

void TerrainProgram::draw(const DescriptionMaintainer<TerrainDescription>& terrains,
                          const FieldsToDraw& fields_to_draw) {
	if (vertices_.size() < fields_to_draw.size()) {
		vertices_.resize(fields_to_draw.size());
		terrains_to_indices_.resize(terrains.size());
	}
	for (auto& container : terrains_to_indices_) {
		container.clear();
		container.reserve(fields_to_draw.size());
	}

	for (size_t current_index = 0; current_index < fields_to_draw.size(); ++current_index) {
		const FieldsToDraw::Field& field = fields_to_draw.at(current_index);

		PerVertexData& vertex = vertices_[current_index];
		vertex.texture_x = field.texture_x;
		vertex.texture_y = field.texture_y;
		vertex.gl_x = field.gl_x;
		vertex.gl_y = field.gl_y;
		vertex.brightness = field.brightness;

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
			terrains_to_indices_[field.ter_d].push_back(current_index);
			terrains_to_indices_[field.ter_d].push_back(bln_index);
			terrains_to_indices_[field.ter_d].push_back(brn_index);
		}

		// Right triangle.
		const int rn_index = fields_to_draw.calculate_index(field.fx + 1, field.fy);
		if (rn_index != -1) {
			terrains_to_indices_[field.ter_r].push_back(current_index);
			terrains_to_indices_[field.ter_r].push_back(brn_index);
			terrains_to_indices_[field.ter_r].push_back(rn_index);
		}
	}

	gl_draw(fields_to_draw.size(), terrains);
}
