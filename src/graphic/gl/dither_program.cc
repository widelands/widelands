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

#include "graphic/gl/dither_program.h"

#include "base/wexception.h"
#include "graphic/gl/fields_to_draw.h"
#include "graphic/image_io.h"
#include "graphic/texture.h"
#include "io/filesystem/layered_filesystem.h"

namespace  {

const char kDitherVertexShader[] = R"(
#version 120

// Attributes.
attribute float attr_brightness;
attribute vec2 attr_dither_texture_position;
attribute vec2 attr_position;
attribute vec2 attr_texture_offset;
attribute vec2 attr_texture_position;

// Output of vertex shader.
varying float var_brightness;
varying vec2 var_dither_texture_position;
varying vec2 var_texture_offset;
varying vec2 var_texture_position;

void main() {
	var_brightness = attr_brightness;
	var_dither_texture_position = attr_dither_texture_position;
	var_texture_offset = attr_texture_offset;
	var_texture_position = attr_texture_position;
	gl_Position = vec4(attr_position, 0., 1.);
}
)";

const char kDitherFragmentShader[] = R"(
#version 120

uniform sampler2D u_dither_texture;
uniform sampler2D u_terrain_texture;
uniform vec2 u_texture_dimensions;

varying float var_brightness;
varying vec2 var_dither_texture_position;
varying vec2 var_texture_position;
varying vec2 var_texture_offset;

void main() {
	vec4 clr = texture2D(u_terrain_texture,
			var_texture_offset + u_texture_dimensions * fract(var_texture_position));
	clr.rgb *= var_brightness;
	clr.a = 1. - texture2D(u_dither_texture, var_dither_texture_position).a;
	gl_FragColor = clr;
}
)";

}  // namespace

DitherProgram::DitherProgram() {
	gl_program_.build(kDitherVertexShader, kDitherFragmentShader);

	attr_brightness_ = glGetAttribLocation(gl_program_.object(), "attr_brightness");
	attr_dither_texture_position_ = glGetAttribLocation(gl_program_.object(), "attr_dither_texture_position");
	attr_position_ = glGetAttribLocation(gl_program_.object(), "attr_position");
	attr_texture_offset_ = glGetAttribLocation(gl_program_.object(), "attr_texture_offset");
	attr_texture_position_ = glGetAttribLocation(gl_program_.object(), "attr_texture_position");

	u_dither_texture_ = glGetUniformLocation(gl_program_.object(), "u_dither_texture");
	u_terrain_texture_ = glGetUniformLocation(gl_program_.object(), "u_terrain_texture");
	u_texture_dimensions_ = glGetUniformLocation(gl_program_.object(), "u_texture_dimensions");

	dither_mask_.reset(new Texture(load_image_as_sdl_surface("world/pics/edge.png", g_fs), true));

	glBindTexture(GL_TEXTURE_2D, dither_mask_->get_gl_texture());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(GL_CLAMP));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(GL_CLAMP));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(GL_LINEAR));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(GL_LINEAR));
	glBindTexture(GL_TEXTURE_2D, 0);
}

DitherProgram::~DitherProgram() {}

void DitherProgram::add_vertex(const FieldsToDraw::Field& field,
                               const int order_index,
                               const FloatPoint& texture_offset) {
	vertices_.emplace_back();
	PerVertexData& back = vertices_.back();

	back.gl_x = field.gl_x;
	back.gl_y = field.gl_y;
	back.texture_x = field.texture_x;
	back.texture_y = field.texture_y;
	back.brightness = field.brightness;
	back.texture_offset_x = texture_offset.x;
	back.texture_offset_y = texture_offset.y;

	switch (order_index) {
	case 0:
		back.dither_texture_x = 0.;
		back.dither_texture_y = 0.;
		break;
	case 1:
		back.dither_texture_x = 1.;
		back.dither_texture_y = 0.;
		break;
	case 2:
		back.dither_texture_x = 0.5;
		back.dither_texture_y = 1.;
		break;
	default:
		throw wexception("Never here.");
	}
}

void DitherProgram::maybe_add_dithering_triangle(
   const uint32_t gametime,
   const DescriptionMaintainer<Widelands::TerrainDescription>& terrains,
   const FieldsToDraw& fields_to_draw,
   const int idx1,
   const int idx2,
   const int idx3,
   const int my_terrain,
   const int other_terrain) {
	if (my_terrain == other_terrain) {
		return;
	}
	const Widelands::TerrainDescription& other_terrain_description =
	   terrains.get(other_terrain);
	if (terrains.get(my_terrain).dither_layer() <
	    other_terrain_description.dither_layer()) {
		const FloatPoint texture_offset =
		   other_terrain_description.get_texture(gametime).texture_coordinates().top_left();
		add_vertex(fields_to_draw.at(idx1), 0, texture_offset);
		add_vertex(fields_to_draw.at(idx2), 1, texture_offset);
		add_vertex(fields_to_draw.at(idx3), 2, texture_offset);
	}
}

void DitherProgram::gl_draw(int gl_texture, float texture_w, float texture_h) {
	glUseProgram(gl_program_.object());

	glEnableVertexAttribArray(attr_brightness_);
	glEnableVertexAttribArray(attr_dither_texture_position_);
	glEnableVertexAttribArray(attr_position_);
	glEnableVertexAttribArray(attr_texture_offset_);
	glEnableVertexAttribArray(attr_texture_position_);

	glBindBuffer(GL_ARRAY_BUFFER, gl_array_buffer_.object());
	glBufferData(GL_ARRAY_BUFFER,
	             sizeof(PerVertexData) * vertices_.size(),
	             vertices_.data(),
	             GL_STREAM_DRAW);

	const auto set_attrib_pointer = [](const int vertex_index, int num_items, int offset) {
		glVertexAttribPointer(vertex_index,
		                      num_items,
		                      GL_FLOAT,
		                      GL_FALSE,
		                      sizeof(PerVertexData),
		                      reinterpret_cast<void*>(offset));
	};
	set_attrib_pointer(attr_brightness_, 1, offsetof(PerVertexData, brightness));
	set_attrib_pointer(attr_dither_texture_position_, 2, offsetof(PerVertexData, dither_texture_x));
	set_attrib_pointer(attr_position_, 2, offsetof(PerVertexData, gl_x));
	set_attrib_pointer(attr_texture_offset_, 2, offsetof(PerVertexData, texture_offset_x));
	set_attrib_pointer(attr_texture_position_, 2, offsetof(PerVertexData, texture_x));

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Set the sampler texture unit to 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, dither_mask_->get_gl_texture());

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gl_texture);

	glUniform1i(u_dither_texture_, 0);
	glUniform1i(u_terrain_texture_, 1);
	glUniform2f(u_texture_dimensions_, texture_w, texture_h);

	glDrawArrays(GL_TRIANGLES, 0, vertices_.size());

	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisableVertexAttribArray(attr_brightness_);
	glDisableVertexAttribArray(attr_dither_texture_position_);
	glDisableVertexAttribArray(attr_position_);
	glDisableVertexAttribArray(attr_texture_offset_);
	glDisableVertexAttribArray(attr_texture_position_);
}

void DitherProgram::draw(const uint32_t gametime,
                         const DescriptionMaintainer<Widelands::TerrainDescription>& terrains,
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

		// Dithering triangles for Down triangle.
		const int bln_index =
		   fields_to_draw.calculate_index(field.fx + (field.fy & 1) - 1, field.fy + 1);
		if (bln_index != -1) {
			maybe_add_dithering_triangle(gametime, terrains, fields_to_draw,
			   brn_index, current_index, bln_index, field.ter_d, field.ter_r);

			const int terrain_dd = fields_to_draw.at(bln_index).ter_r;
			maybe_add_dithering_triangle(gametime, terrains, fields_to_draw,
			   bln_index, brn_index, current_index, field.ter_d, terrain_dd);

			const int ln_index = fields_to_draw.calculate_index(field.fx - 1, field.fy);
			if (ln_index != -1) {
				const int terrain_l = fields_to_draw.at(ln_index).ter_r;
				maybe_add_dithering_triangle(gametime, terrains, fields_to_draw,
				   current_index, bln_index, brn_index, field.ter_d, terrain_l);
			}
		}

		// Dithering for right triangle.
		const int rn_index = fields_to_draw.calculate_index(field.fx + 1, field.fy);
		if (rn_index != -1) {
			maybe_add_dithering_triangle(gametime, terrains, fields_to_draw,
			   current_index, brn_index, rn_index, field.ter_r, field.ter_d);
			int terrain_rr = fields_to_draw.at(rn_index).ter_d;
			maybe_add_dithering_triangle(gametime, terrains, fields_to_draw,
					brn_index, rn_index, current_index, field.ter_r, terrain_rr);

			const int trn_index =
				fields_to_draw.calculate_index(field.fx + (field.fy & 1), field.fy - 1);
			if (trn_index != -1) {
				const int terrain_u = fields_to_draw.at(trn_index).ter_d;
				maybe_add_dithering_triangle(gametime, terrains, fields_to_draw,
				   rn_index, current_index, brn_index, field.ter_r, terrain_u);
			}
		}
	}

	const Texture& texture = terrains.get(0).get_texture(0);
	gl_draw(texture.get_gl_texture(), texture.texture_coordinates().w, texture.texture_coordinates().h);
}
