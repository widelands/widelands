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
attribute float attr_distance_from_center;

varying vec3 var_color;
varying float var_distance_from_center;

void main() {
	var_color = attr_color;
	var_distance_from_center = attr_distance_from_center;
	gl_Position = vec4(attr_position, 1.);
}
)";

const char kDrawLineFragmentShader[] = R"(
#version 120

varying vec3 var_color;
varying float var_distance_from_center;

// The percentage of the line that should be solid, i.e. not feathered into
// alpha = 0.
#define SOLID 0.3

void main() {
	// This means till SOLID we want full alpha, then a (1 - t**2) with t [0, 1]
	// falloff.
	float d = abs(var_distance_from_center);
	float opaqueness =
		step(SOLID, d) * ((d - SOLID) / (1. - SOLID));
	gl_FragColor = vec4(var_color.rgb, 1. - pow(opaqueness, 2));
}
)";

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
	attr_distance_from_center_ = glGetAttribLocation(gl_program_.object(), "attr_distance_from_center");
}

void DrawLineProgram::draw(const std::vector<Arguments>& arguments) {
	glUseProgram(gl_program_.object());

	auto& gl_state = Gl::State::instance();
	gl_state.enable_vertex_attrib_array({
	   attr_position_, attr_color_, attr_distance_from_center_
	});

	gl_array_buffer_.bind();

	Gl::vertex_attrib_pointer(attr_position_, 3, sizeof(PerVertexData), offsetof(PerVertexData, gl_x));
	Gl::vertex_attrib_pointer(
	   attr_color_, 3, sizeof(PerVertexData), offsetof(PerVertexData, color_r));
	Gl::vertex_attrib_pointer(attr_distance_from_center_, 1, sizeof(PerVertexData),
	                          offsetof(PerVertexData, distance_from_center));

	vertices_.clear();

	for (const Arguments& current_args : arguments) {
		// We do not support anything else for drawing lines, really.
		assert(current_args.blend_mode == BlendMode::UseAlpha);
		assert(current_args.points.size() % 4 == 0);

		const float r = current_args.color.r / 255.;
		const float g = current_args.color.g / 255.;
		const float b = current_args.color.b / 255.;
		const float z_value = current_args.z_value;

		const auto& p = current_args.points;
		for (size_t i = 0; i < p.size(); i += 4) {
			vertices_.emplace_back(
			   PerVertexData{p[i].x, p[i].y, z_value, r, g, b, 1.f});
			vertices_.emplace_back(
			   PerVertexData{p[i + 1].x, p[i + 1].y, z_value, r, g, b, 1.f});
			vertices_.emplace_back(
			   PerVertexData{p[i + 2].x, p[i + 2].y, z_value, r, g, b, -1.f});
			vertices_.emplace_back(vertices_[vertices_.size() - 2]);
			vertices_.emplace_back(vertices_[vertices_.size() - 2]);
			vertices_.emplace_back(
			   PerVertexData{p[i + 3].x, p[i + 3].y, z_value, r, g, b, -1.f});
		}
	}
	gl_array_buffer_.update(vertices_);

	glDrawArrays(GL_TRIANGLES, 0, vertices_.size());
}
