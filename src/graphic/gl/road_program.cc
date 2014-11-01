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

#include "graphic/gl/road_program.h"

#include <cmath>

#include "base/log.h"
#include "graphic/gl/fields_to_draw.h"
#include "graphic/graphic.h"
#include "graphic/texture.h"
#include "logic/roadtype.h"

namespace  {

// We target OpenGL 2.1 for the desktop here.
// TODO(sirver): In the end we need to replace gl_ProjectionMatrix. It is not
// supported in ES and more modern Open GL version.
const char kRoadVertexShader[] = R"(
#version 120

// Attributes.
attribute vec2 attr_position;

// Outputs.
// NOCOM(#sirver): fix this
// varying vec2 var_texture_position;

void main() {
	gl_Position = gl_ProjectionMatrix * vec4(attr_position, 0., 1.);
}
)";

const char kRoadFragmentShader[] = R"(
#version 120

// uniform sampler2D u_terrain_texture;

void main() {
	gl_FragColor = vec4(1., 0., 0., 1.);
}
)";

}  // namespace

RoadProgram::RoadProgram() {
	gl_program_.build(kRoadVertexShader, kRoadFragmentShader);

	attr_position_ = glGetAttribLocation(gl_program_.object(), "attr_position");
	// attr_texture_position_ =
		// glGetAttribLocation(gl_program_.object(), "attr_texture_position");
	// attr_brightness_ = glGetAttribLocation(gl_program_.object(), "attr_brightness");

	// u_terrain_texture_ = glGetUniformLocation(gl_program_.object(), "u_terrain_texture");
}

void RoadProgram::add_road(const FieldsToDraw::Field& start, const FieldsToDraw::Field& end) {
	static constexpr float kRoadThicknessInPixels = 3.;
	const float delta_x = end.x - start.x;
	const float delta_y = end.y - start.y;
	const float vector_length = std::hypot(delta_x, delta_y);

	// Find the reciprocal unit vector, so that we can calculate start and end
	// points for the quad that will make the road.
	const float dx = -delta_y / vector_length;
	const float dy = delta_x / vector_length;

	const PerVertexData p1 = {
	   start.x + kRoadThicknessInPixels * dx, start.y + kRoadThicknessInPixels * dy
	};
	const PerVertexData p2 = {
	   start.x - kRoadThicknessInPixels * dx, start.y - kRoadThicknessInPixels * dy
	};
	const PerVertexData p3 = {
	   end.x + kRoadThicknessInPixels * dx, end.y + kRoadThicknessInPixels * dy
	};
	const PerVertexData p4 = {
	   end.x - kRoadThicknessInPixels * dx, end.y - kRoadThicknessInPixels * dy
	};

	// As OpenGl does not support drawing quads in modern days and we have a
	// bunch of roads that might not be neighbored, we need to add two triangles
	// for each road. :(. Another alternative would be to use primitive restart,
	// but that is a fairly recent OpenGL feature.
	vertices_.emplace_back(p1);
	vertices_.emplace_back(p2);
	vertices_.emplace_back(p3);
	vertices_.emplace_back(p2);
	vertices_.emplace_back(p3);
	vertices_.emplace_back(p4);
}

void RoadProgram::draw(const FieldsToDraw& fields_to_draw) {
	vertices_.clear();

	for (size_t current_index = 0; current_index < fields_to_draw.size(); ++current_index) {
		const FieldsToDraw::Field& field = fields_to_draw.at(current_index);

		// Road to right neighbor.
		const int rn_index = fields_to_draw.calculate_index(field.fx + 1, field.fy);
		if (rn_index != -1) {
			const Widelands::RoadType road =
			   static_cast<Widelands::RoadType>(field.roads & Widelands::Road_Mask);
			if (road != Widelands::Road_None) {
				add_road(field, fields_to_draw.at(rn_index));
			}
		}

		// Road to bottom right neighbor.
		const int brn_index = fields_to_draw.calculate_index(field.fx + (field.fy & 1), field.fy + 1);
		if (brn_index != -1) {
			const Widelands::RoadType road =
			   static_cast<Widelands::RoadType>((field.roads >> 2) & Widelands::Road_Mask);
			if (road != Widelands::Road_None) {
				add_road(field, fields_to_draw.at(brn_index));
			}
		}

		// Road to bottom right neighbor.
		const int bln_index =
		   fields_to_draw.calculate_index(field.fx + (field.fy & 1) - 1, field.fy + 1);
		if (brn_index != -1) {
			const Widelands::RoadType road =
			   static_cast<Widelands::RoadType>((field.roads >> 4) & Widelands::Road_Mask);
			if (road != Widelands::Road_None) {
				add_road(field, fields_to_draw.at(bln_index));
			}
		}
	}

	glUseProgram(gl_program_.object());

	glEnableVertexAttribArray(attr_position_);

	glBindBuffer(GL_ARRAY_BUFFER, gl_array_buffer_.object());
	glBufferData(
	   GL_ARRAY_BUFFER, sizeof(PerVertexData) * vertices_.size(), vertices_.data(), GL_STREAM_DRAW);

	const auto set_attrib_pointer = [](const int vertex_index, int num_items, int offset) {
		glVertexAttribPointer(vertex_index,
		                      num_items,
		                      GL_FLOAT,
		                      GL_FALSE,
		                      sizeof(PerVertexData),
		                      reinterpret_cast<void*>(offset));
	};
	set_attrib_pointer(attr_position_, 2, offsetof(PerVertexData, x));

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// NOCOM(#sirver): what
	// Set the sampler texture unit to 0
	// glActiveTexture(GL_TEXTURE0);
	// glUniform1i(u_terrain_texture_, 0);

	// Which triangles to draw?
	// NOCOM(#sirver): what
		// glBindTexture(
			// GL_TEXTURE_2D,
			// g_gr->get_maptexture_data(terrains.get_unmutable(i).get_texture())->get_texture());
	glDrawArrays(GL_TRIANGLES, 0, vertices_.size());

	glDisableVertexAttribArray(attr_position_);

	glUseProgram(0);

}
