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
const char kRoadVertexShader[] = R"(
#version 120

// Attributes.
attribute vec2 attr_position;
attribute vec2 attr_texture_position;
attribute float attr_texture_mix;
attribute float attr_brightness;

// Outputs.
varying vec2 out_texture_position;
varying float out_texture_mix;
varying float out_brightness;

void main() {
	out_texture_position = attr_texture_position;
	out_texture_mix = attr_texture_mix;
	out_brightness = attr_brightness;
	gl_Position = vec4(attr_position, 0., 1.);
}
)";

const char kRoadFragmentShader[] = R"(
#version 120

// Inputs.
varying vec2 out_texture_position;
varying float out_texture_mix;
varying float out_brightness;

uniform sampler2D u_normal_road_texture;
uniform sampler2D u_busy_road_texture;

void main() {
	vec4 normal_road_color = texture2D(u_normal_road_texture, out_texture_position);
	vec4 busy_road_color = texture2D(u_busy_road_texture, out_texture_position);
	vec4 color = mix(normal_road_color, busy_road_color, out_texture_mix);
	color.rgb *= out_brightness;
	gl_FragColor = color;
}
)";

}  // namespace

RoadProgram::RoadProgram() {
	gl_program_.build(kRoadVertexShader, kRoadFragmentShader);

	attr_position_ = glGetAttribLocation(gl_program_.object(), "attr_position");
	attr_texture_position_ =
		glGetAttribLocation(gl_program_.object(), "attr_texture_position");
	attr_texture_mix_ = glGetAttribLocation(gl_program_.object(), "attr_texture_mix");
	attr_brightness_ = glGetAttribLocation(gl_program_.object(), "attr_brightness");

	u_normal_road_texture_ = glGetUniformLocation(gl_program_.object(), "u_normal_road_texture");
	u_busy_road_texture_ = glGetUniformLocation(gl_program_.object(), "u_busy_road_texture");
}

void RoadProgram::add_road(const GLSurface& surface,
                           const FieldsToDraw::Field& start,
                           const FieldsToDraw::Field& end,
                           const Widelands::RoadType road_type) {
	static constexpr float kRoadThicknessInPixels = 3.;
	const float delta_x = end.pixel_x - start.pixel_x;
	const float delta_y = end.pixel_y - start.pixel_y;
	const float vector_length = std::hypot(delta_x, delta_y);

	// Find the reciprocal unit vector, so that we can calculate start and end
	// points for the quad that will make the road.
	const float dx = -delta_y / vector_length;
	const float dy = delta_x / vector_length;

	const float texture_mix = road_type == Widelands::Road_Normal ? 0. : 1.;
	PerVertexData p1 = {
		start.pixel_x + kRoadThicknessInPixels * dx, start.pixel_y + kRoadThicknessInPixels * dy,
		0., 0.,
		start.brightness,
		texture_mix,
	};
	surface.pixel_to_gl(&p1.gl_x, &p1.gl_y);

	PerVertexData p2 = {
		start.pixel_x - kRoadThicknessInPixels * dx, start.pixel_y - kRoadThicknessInPixels * dy,
		0., 1.,
		start.brightness,
		texture_mix,
	};
	surface.pixel_to_gl(&p2.gl_x, &p2.gl_y);

	PerVertexData p3 = {
		end.pixel_x + kRoadThicknessInPixels * dx, end.pixel_y + kRoadThicknessInPixels * dy,
		1., 0.,
		end.brightness,
		texture_mix,
	};
	surface.pixel_to_gl(&p3.gl_x, &p3.gl_y);

	PerVertexData p4 = {
		end.pixel_x - kRoadThicknessInPixels * dx, end.pixel_y - kRoadThicknessInPixels * dy,
		1., 1.,
		end.brightness,
		texture_mix,
	};
	surface.pixel_to_gl(&p4.gl_x, &p4.gl_y);

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

void RoadProgram::draw(const GLSurface& surface, const FieldsToDraw& fields_to_draw) {
	vertices_.clear();

	for (size_t current_index = 0; current_index < fields_to_draw.size(); ++current_index) {
		const FieldsToDraw::Field& field = fields_to_draw.at(current_index);

		// Road to right neighbor.
		const int rn_index = fields_to_draw.calculate_index(field.fx + 1, field.fy);
		if (rn_index != -1) {
			const Widelands::RoadType road =
			   static_cast<Widelands::RoadType>(field.roads & Widelands::Road_Mask);
			if (road != Widelands::Road_None) {
				add_road(surface, field, fields_to_draw.at(rn_index), road);
			}
		}

		// Road to bottom right neighbor.
		const int brn_index = fields_to_draw.calculate_index(field.fx + (field.fy & 1), field.fy + 1);
		if (brn_index != -1) {
			const Widelands::RoadType road =
			   static_cast<Widelands::RoadType>((field.roads >> 2) & Widelands::Road_Mask);
			if (road != Widelands::Road_None) {
				add_road(surface, field, fields_to_draw.at(brn_index), road);
			}
		}

		// Road to bottom right neighbor.
		const int bln_index =
		   fields_to_draw.calculate_index(field.fx + (field.fy & 1) - 1, field.fy + 1);
		if (brn_index != -1) {
			const Widelands::RoadType road =
			   static_cast<Widelands::RoadType>((field.roads >> 4) & Widelands::Road_Mask);
			if (road != Widelands::Road_None) {
				add_road(surface, field, fields_to_draw.at(bln_index), road);
			}
		}
	}

	glUseProgram(gl_program_.object());

	glEnableVertexAttribArray(attr_position_);
	glEnableVertexAttribArray(attr_texture_position_);
	glEnableVertexAttribArray(attr_brightness_);
	glEnableVertexAttribArray(attr_texture_mix_);

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
	set_attrib_pointer(attr_position_, 2, offsetof(PerVertexData, gl_x));
	set_attrib_pointer(attr_texture_position_, 2, offsetof(PerVertexData, texture_x));
	set_attrib_pointer(attr_brightness_, 1, offsetof(PerVertexData, brightness));
	set_attrib_pointer(attr_texture_mix_, 1, offsetof(PerVertexData, texture_mix));

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Bind the textures.
	const GLuint rt_normal = dynamic_cast<const GLSurfaceTexture&>(
	                      g_gr->get_road_texture(Widelands::Road_Normal)).get_gl_texture();
	const GLuint rt_busy = dynamic_cast<const GLSurfaceTexture&>(
	                    g_gr->get_road_texture(Widelands::Road_Busy)).get_gl_texture();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, rt_normal);
	glUniform1i(u_normal_road_texture_, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, rt_busy);
	glUniform1i(u_busy_road_texture_, 1);

	glDrawArrays(GL_TRIANGLES, 0, vertices_.size());

	glDisableVertexAttribArray(attr_position_);
	glDisableVertexAttribArray(attr_texture_position_);
	glDisableVertexAttribArray(attr_brightness_);
	glDisableVertexAttribArray(attr_texture_mix_);

	glActiveTexture(GL_TEXTURE0);
}
