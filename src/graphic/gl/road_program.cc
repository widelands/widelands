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

#include "graphic/gl/road_program.h"

#include <cassert>
#include <cmath>

#include "base/log.h"
#include "graphic/gl/coordinate_conversion.h"
#include "graphic/gl/fields_to_draw.h"
#include "graphic/graphic.h"
#include "graphic/image_io.h"
#include "graphic/texture.h"
#include "logic/roadtype.h"

namespace  {

// We target OpenGL 2.1 for the desktop here.
const char kRoadVertexShader[] = R"(
#version 120

// Attributes.
attribute vec2 attr_position;
attribute vec2 attr_texture_position;
attribute float attr_brightness;

uniform float u_z_value;

// Outputs.
varying vec2 out_texture_position;
varying float out_brightness;

void main() {
	out_texture_position = attr_texture_position;
	out_brightness = attr_brightness;
	gl_Position = vec4(attr_position, u_z_value, 1.);
}
)";

const char kRoadFragmentShader[] = R"(
#version 120

// Inputs.
varying vec2 out_texture_position;
varying float out_brightness;

uniform sampler2D u_texture;

void main() {
	vec4 color = texture2D(u_texture, out_texture_position);
	color.rgb *= out_brightness;
	gl_FragColor = color;
}
)";

}  // namespace

RoadProgram::RoadProgram() {
	gl_program_.build(kRoadVertexShader, kRoadFragmentShader);

	attr_position_ = glGetAttribLocation(gl_program_.object(), "attr_position");
	attr_texture_position_ = glGetAttribLocation(gl_program_.object(), "attr_texture_position");
	attr_brightness_ = glGetAttribLocation(gl_program_.object(), "attr_brightness");

	u_z_value_ = glGetUniformLocation(gl_program_.object(), "u_z_value");
	u_texture_ = glGetUniformLocation(gl_program_.object(), "u_texture");
}

RoadProgram::~RoadProgram() {
}

void RoadProgram::add_road(const int renderbuffer_width,
                           const int renderbuffer_height,
                           const FieldsToDraw::Field& start,
                           const FieldsToDraw::Field& end,
                           const Widelands::RoadType road_type,
                           const Direction direction,
                           uint32_t* gl_texture) {
	// The thickness of the road in pixels on screen.
	static constexpr float kRoadThicknessInPixels = 5.;

	// The overshot of the road in either direction in percent.
	static constexpr float kRoadElongationInPercent = .1;

	const float delta_x = end.pixel_x - start.pixel_x;
	const float delta_y = end.pixel_y - start.pixel_y;
	const float vector_length = std::hypot(delta_x, delta_y);

	const float road_overshoot_x = delta_x * kRoadElongationInPercent;
	const float road_overshoot_y = delta_y * kRoadElongationInPercent;

	// Find the reciprocal unit vector, so that we can calculate start and end
	// points for the quad that will make the road.
	const float road_thickness_x = (-delta_y / vector_length) * kRoadThicknessInPixels;
	const float road_thickness_y = (delta_x / vector_length) * kRoadThicknessInPixels;

	const Texture& texture =
	   road_type == Widelands::RoadType::kNormal ?
	      start.road_textures->get_normal_texture(start.fx, start.fy, direction) :
	      start.road_textures->get_busy_texture(start.fx, start.fy, direction);
	if (*gl_texture == 0) {
		*gl_texture = texture.blit_data().texture_id;
	}
	// We assume that all road textures are in the same OpenGL texture, i.e. in
	// one texture atlas.
	assert(*gl_texture == texture.blit_data().texture_id);

	const FloatRect texture_rect = to_gl_texture(texture.blit_data());

	vertices_.emplace_back(PerVertexData{
	   start.pixel_x - road_overshoot_x + road_thickness_x,
	   start.pixel_y - road_overshoot_y + road_thickness_y,
	   texture_rect.x,
	   texture_rect.y,
	   start.brightness,
	});
	pixel_to_gl_renderbuffer(
	   renderbuffer_width, renderbuffer_height, &vertices_.back().gl_x, &vertices_.back().gl_y);

	vertices_.emplace_back(PerVertexData{
	   start.pixel_x - road_overshoot_x - road_thickness_x,
	   start.pixel_y - road_overshoot_y - road_thickness_y,
	   texture_rect.x,
	   texture_rect.y + texture_rect.h,
	   start.brightness,
	});
	pixel_to_gl_renderbuffer(
	   renderbuffer_width, renderbuffer_height, &vertices_.back().gl_x, &vertices_.back().gl_y);

	vertices_.emplace_back(PerVertexData{
	   end.pixel_x + road_overshoot_x + road_thickness_x,
	   end.pixel_y + road_overshoot_y + road_thickness_y,
	   texture_rect.x + texture_rect.w,
	   texture_rect.y,
	   end.brightness,
	});
	pixel_to_gl_renderbuffer(
	   renderbuffer_width, renderbuffer_height, &vertices_.back().gl_x, &vertices_.back().gl_y);

	// As OpenGl does not support drawing quads in modern days and we have a
	// bunch of roads that might not be neighbored, we need to add two triangles
	// for each road. :(. Another alternative would be to use primitive restart,
	// but that is a fairly recent OpenGL feature.
	vertices_.emplace_back(vertices_.at(vertices_.size() - 2));
	vertices_.emplace_back(vertices_.at(vertices_.size() - 2));

	vertices_.emplace_back(PerVertexData{
	   end.pixel_x + road_overshoot_x - road_thickness_x,
	   end.pixel_y + road_overshoot_y - road_thickness_y,
	   texture_rect.x + texture_rect.w,
	   texture_rect.y + texture_rect.h,
	   end.brightness,
	});
	pixel_to_gl_renderbuffer(
	   renderbuffer_width, renderbuffer_height, &vertices_.back().gl_x, &vertices_.back().gl_y);
}

void RoadProgram::draw(const int renderbuffer_width,
                       const int renderbuffer_height,
                       const FieldsToDraw& fields_to_draw,
                       float z_value) {
	vertices_.clear();

	uint32_t gl_texture = 0;
	for (size_t current_index = 0; current_index < fields_to_draw.size(); ++current_index) {
		const FieldsToDraw::Field& field = fields_to_draw.at(current_index);

		// Road to right neighbor.
		const int rn_index = fields_to_draw.calculate_index(field.fx + 1, field.fy);
		if (rn_index != -1) {
			const Widelands::RoadType road =
			   static_cast<Widelands::RoadType>(field.roads & Widelands::RoadType::kMask);
			if (road != Widelands::RoadType::kNone) {
				add_road(renderbuffer_width,
				         renderbuffer_height,
				         field,
				         fields_to_draw.at(rn_index),
				         road,
				         kEast,
				         &gl_texture);
			}
		}

		// Road to bottom right neighbor.
		const int brn_index = fields_to_draw.calculate_index(field.fx + (field.fy & 1), field.fy + 1);
		if (brn_index != -1) {
			const Widelands::RoadType road =
			   static_cast<Widelands::RoadType>((field.roads >> 2) & Widelands::RoadType::kMask);
			if (road != Widelands::RoadType::kNone) {
				add_road(renderbuffer_width,
				         renderbuffer_height,
				         field,
				         fields_to_draw.at(brn_index),
				         road,
				         kSouthEast,
				         &gl_texture);
			}
		}

		// Road to bottom right neighbor.
		const int bln_index =
		   fields_to_draw.calculate_index(field.fx + (field.fy & 1) - 1, field.fy + 1);
		if (bln_index != -1) {
			const Widelands::RoadType road =
			   static_cast<Widelands::RoadType>((field.roads >> 4) & Widelands::RoadType::kMask);
			if (road != Widelands::RoadType::kNone) {
				add_road(renderbuffer_width,
				         renderbuffer_height,
				         field,
				         fields_to_draw.at(bln_index),
				         road,
				         kSouthWest,
				         &gl_texture);
			}
		}
	}

	glUseProgram(gl_program_.object());

	glEnableVertexAttribArray(attr_position_);
	glEnableVertexAttribArray(attr_texture_position_);
	glEnableVertexAttribArray(attr_brightness_);

	gl_array_buffer_.bind();
	gl_array_buffer_.update(vertices_);

	Gl::vertex_attrib_pointer(
	   attr_position_, 2, sizeof(PerVertexData), offsetof(PerVertexData, gl_x));
	Gl::vertex_attrib_pointer(
	   attr_texture_position_, 2, sizeof(PerVertexData), offsetof(PerVertexData, texture_x));
	Gl::vertex_attrib_pointer(
	   attr_brightness_, 1, sizeof(PerVertexData), offsetof(PerVertexData, brightness));

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Bind the textures.
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gl_texture);

	glUniform1i(u_texture_, 0);

	glUniform1f(u_z_value_, z_value);

	glDrawArrays(GL_TRIANGLES, 0, vertices_.size());

	glDisableVertexAttribArray(attr_position_);
	glDisableVertexAttribArray(attr_texture_position_);
	glDisableVertexAttribArray(attr_brightness_);
}
