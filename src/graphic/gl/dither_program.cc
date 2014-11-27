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

#include "graphic/gl/dither_program.h"

#include "base/wexception.h"
#include "graphic/gl/fields_to_draw.h"
#include "graphic/graphic.h"
#include "graphic/image_io.h"
#include "graphic/terrain_texture.h"
#include "graphic/texture.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"

namespace  {

using namespace Widelands;

const char kDitherVertexShader[] = R"(
#version 120

// Attributes.
attribute float attr_brightness;
attribute vec2 attr_position;
attribute vec2 attr_texture_position;
attribute vec2 attr_dither_texture_position;

// Output of vertex shader.
varying vec2 var_texture_position;
varying vec2 var_dither_texture_position;
varying float var_brightness;

void main() {
	var_texture_position = attr_texture_position;
	var_dither_texture_position = attr_dither_texture_position;
	var_brightness = attr_brightness;
	gl_Position = vec4(attr_position, 0., 1.);
}
)";

const char kDitherFragmentShader[] = R"(
#version 120

uniform sampler2D u_dither_texture;
uniform sampler2D u_terrain_texture;

varying float var_brightness;
varying vec2 var_texture_position;
varying vec2 var_dither_texture_position;

void main() {
	vec4 clr = texture2D(u_terrain_texture, var_texture_position);
	clr.rgb *= var_brightness;
	clr.a = 1. - texture2D(u_dither_texture, var_dither_texture_position).a;
	gl_FragColor = clr;
}
)";


}  // namespace

DitherProgram::DitherProgram() {
	gl_program_.build(kDitherVertexShader, kDitherFragmentShader);

	attr_brightness_ = glGetAttribLocation(gl_program_.object(), "attr_brightness");
	attr_dither_texture_position_ =
	   glGetAttribLocation(gl_program_.object(), "attr_dither_texture_position");
	attr_position_ = glGetAttribLocation(gl_program_.object(), "attr_position");
	attr_texture_position_ =
	   glGetAttribLocation(gl_program_.object(), "attr_texture_position");

	u_dither_texture_ = glGetUniformLocation(gl_program_.object(), "u_dither_texture");
	u_terrain_texture_ = glGetUniformLocation(gl_program_.object(), "u_terrain_texture");

	SDL_Surface* sdlsurf = load_image_as_sdl_surface("world/pics/edge.png", g_fs);
	dither_mask_.reset(new Texture(sdlsurf, true));
}

DitherProgram::~DitherProgram() {}

void DitherProgram::add_vertex(const FieldsToDraw::Field& field,
                               const int order_index,
                               const int terrain) {
	vertices_[terrain].emplace_back();
	PerVertexData& back = vertices_[terrain].back();

	back.gl_x = field.gl_x;
	back.gl_y = field.gl_y;
	back.texture_x = field.texture_x;
	back.texture_y = field.texture_y;
	back.brightness = field.brightness;

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
	if (terrains.get_unmutable(my_terrain).dither_layer() <
	    terrains.get_unmutable(other_terrain).dither_layer()) {
		add_vertex(fields_to_draw.at(idx1), 0, other_terrain);
		add_vertex(fields_to_draw.at(idx2), 1, other_terrain);
		add_vertex(fields_to_draw.at(idx3), 2, other_terrain);
	}
}

void DitherProgram::draw(const DescriptionMaintainer<TerrainDescription>& terrains,
                         const FieldsToDraw& fields_to_draw) {
	glUseProgram(gl_program_.object());

	glEnableVertexAttribArray(attr_brightness_);
	glEnableVertexAttribArray(attr_dither_texture_position_);
	glEnableVertexAttribArray(attr_position_);
	glEnableVertexAttribArray(attr_texture_position_);

	if (vertices_.size() != terrains.size()) {
		vertices_.resize(terrains.size());
	}
	for (auto& container : vertices_) {
		container.clear();
	}

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
			maybe_add_dithering_triangle(terrains, fields_to_draw,
			   brn_index, current_index, bln_index, field.ter_d, field.ter_r);

			const int terrain_dd = fields_to_draw.at(bln_index).ter_r;
			maybe_add_dithering_triangle(terrains, fields_to_draw,
			   bln_index, brn_index, current_index, field.ter_d, terrain_dd);

			const int ln_index = fields_to_draw.calculate_index(field.fx - 1, field.fy);
			if (ln_index != -1) {
				const int terrain_l = fields_to_draw.at(ln_index).ter_r;
				maybe_add_dithering_triangle(terrains, fields_to_draw,
				   current_index, bln_index, brn_index, field.ter_d, terrain_l);
			}
		}

		// Dithering for right triangle.
		const int rn_index = fields_to_draw.calculate_index(field.fx + 1, field.fy);
		if (rn_index != -1) {
			maybe_add_dithering_triangle(terrains, fields_to_draw,
			   current_index, brn_index, rn_index, field.ter_r, field.ter_d);
			int terrain_rr = fields_to_draw.at(rn_index).ter_d;
			maybe_add_dithering_triangle(terrains, fields_to_draw,
			   brn_index, rn_index, current_index, field.ter_r, terrain_rr);

			const int trn_index =
			   fields_to_draw.calculate_index(field.fx + (field.fy & 1), field.fy - 1);
			if (trn_index != -1) {
				const int terrain_u = fields_to_draw.at(trn_index).ter_d;
				maybe_add_dithering_triangle(terrains, fields_to_draw,
				   rn_index, current_index, brn_index, field.ter_r, terrain_u);
			}
		}
	}

	// Set the sampler texture unit to 0
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(u_dither_texture_, 0);
	glBindTexture(GL_TEXTURE_2D, dither_mask_->get_gl_texture());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glActiveTexture(GL_TEXTURE1);
	glUniform1i(u_terrain_texture_, 1);

	// Which triangles to draw?
	glBindBuffer(GL_ARRAY_BUFFER, gl_array_buffer_.object());
	for (size_t i = 0; i < vertices_.size(); ++i) {
		const auto& current_data = vertices_[i];
		if (current_data.empty()) {
			continue;
		}
		glBindTexture(GL_TEXTURE_2D, terrains.get_unmutable(i).get_texture().get_gl_texture());
		glBufferData(GL_ARRAY_BUFFER,
		             sizeof(PerVertexData) * current_data.size(),
		             current_data.data(),
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
		set_attrib_pointer(attr_texture_position_, 2, offsetof(PerVertexData, texture_x));

		glDrawArrays(GL_TRIANGLES, 0, current_data.size());
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(attr_brightness_);
	glDisableVertexAttribArray(attr_dither_texture_position_);
	glDisableVertexAttribArray(attr_position_);
	glDisableVertexAttribArray(attr_texture_position_);

	glActiveTexture(GL_TEXTURE0);
}
