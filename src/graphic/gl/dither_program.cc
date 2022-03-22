/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "graphic/gl/dither_program.h"

#include "base/wexception.h"
#include "graphic/gl/coordinate_conversion.h"
#include "graphic/gl/fields_to_draw.h"
#include "graphic/gl/utils.h"
#include "graphic/image_io.h"
#include "graphic/texture.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/player.h"

DitherProgram::DitherProgram() {
	gl_program_.build("dither");

	attr_brightness_ = glGetAttribLocation(gl_program_.object(), "attr_brightness");
	attr_dither_texture_position_ =
	   glGetAttribLocation(gl_program_.object(), "attr_dither_texture_position");
	attr_position_ = glGetAttribLocation(gl_program_.object(), "attr_position");
	attr_texture_offset_ = glGetAttribLocation(gl_program_.object(), "attr_texture_offset");
	attr_texture_position_ = glGetAttribLocation(gl_program_.object(), "attr_texture_position");

	u_dither_texture_ = glGetUniformLocation(gl_program_.object(), "u_dither_texture");
	u_terrain_texture_ = glGetUniformLocation(gl_program_.object(), "u_terrain_texture");
	u_texture_dimensions_ = glGetUniformLocation(gl_program_.object(), "u_texture_dimensions");
	u_z_value_ = glGetUniformLocation(gl_program_.object(), "u_z_value");

	dither_mask_.reset(new Texture(load_image_as_sdl_surface("world/pics/edge.png", g_fs), true));

	Gl::State::instance().bind(GL_TEXTURE0, dither_mask_->blit_data().texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(GL_CLAMP_TO_EDGE));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(GL_CLAMP_TO_EDGE));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(GL_LINEAR));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(GL_LINEAR));
}

void DitherProgram::add_vertex(const FieldsToDraw::Field& field,
                               const TrianglePoint triangle_point,
                               const Vector2f& texture_offset) {
	vertices_.emplace_back();
	PerVertexData& back = vertices_.back();

	back.gl_x = field.gl_position.x;
	back.gl_y = field.gl_position.y;
	back.texture_x = field.texture_coords.x;
	back.texture_y = field.texture_coords.y;
	back.brightness = field.brightness;
	back.texture_offset_x = texture_offset.x;
	back.texture_offset_y = texture_offset.y;

	switch (triangle_point) {
	case TrianglePoint::kTopRight:
		back.dither_texture_x = 1.;
		back.dither_texture_y = 1.;
		break;
	case TrianglePoint::kTopLeft:
		back.dither_texture_x = 0.;
		back.dither_texture_y = 1.;
		break;
	case TrianglePoint::kBottomMiddle:
		back.dither_texture_x = 0.5;
		back.dither_texture_y = 0.;
		break;
	}
}

void DitherProgram::maybe_add_dithering_triangle(
   const uint32_t gametime,
   const Widelands::DescriptionMaintainer<Widelands::TerrainDescription>& terrains,
   const FieldsToDraw& fields_to_draw,
   const int idx1,
   const int idx2,
   const int idx3,
   const int my_terrain,
   const int other_terrain) {
	if (my_terrain == other_terrain) {
		return;
	}
	const Widelands::TerrainDescription& other_terrain_description = terrains.get(other_terrain);
	if (terrains.get(my_terrain).dither_layer() < other_terrain_description.dither_layer()) {
		const Vector2f texture_offset =
		   to_gl_texture(other_terrain_description.get_texture(gametime).blit_data()).origin();
		add_vertex(fields_to_draw.at(idx1), TrianglePoint::kTopRight, texture_offset);
		add_vertex(fields_to_draw.at(idx2), TrianglePoint::kTopLeft, texture_offset);
		add_vertex(fields_to_draw.at(idx3), TrianglePoint::kBottomMiddle, texture_offset);
	}
}

void DitherProgram::gl_draw(int gl_texture, float texture_w, float texture_h, const float z_value) {
	glUseProgram(gl_program_.object());

	auto& gl_state = Gl::State::instance();
	gl_state.enable_vertex_attrib_array({attr_brightness_, attr_dither_texture_position_,
	                                     attr_position_, attr_texture_offset_,
	                                     attr_texture_position_});

	gl_array_buffer_.bind();
	gl_array_buffer_.update(vertices_);

	Gl::vertex_attrib_pointer(
	   attr_brightness_, 1, sizeof(PerVertexData), offsetof(PerVertexData, brightness));
	Gl::vertex_attrib_pointer(attr_dither_texture_position_, 2, sizeof(PerVertexData),
	                          offsetof(PerVertexData, dither_texture_x));
	Gl::vertex_attrib_pointer(
	   attr_position_, 2, sizeof(PerVertexData), offsetof(PerVertexData, gl_x));
	Gl::vertex_attrib_pointer(
	   attr_texture_offset_, 2, sizeof(PerVertexData), offsetof(PerVertexData, texture_offset_x));
	Gl::vertex_attrib_pointer(
	   attr_texture_position_, 2, sizeof(PerVertexData), offsetof(PerVertexData, texture_x));

	gl_state.bind(GL_TEXTURE0, dither_mask_->blit_data().texture_id);
	gl_state.bind(GL_TEXTURE1, gl_texture);

	glUniform1f(u_z_value_, z_value);
	glUniform1i(u_dither_texture_, 0);
	glUniform1i(u_terrain_texture_, 1);
	glUniform2f(u_texture_dimensions_, texture_w, texture_h);

	glDrawArrays(GL_TRIANGLES, 0, vertices_.size());
}

void DitherProgram::draw(
   const uint32_t gametime,
   const Widelands::DescriptionMaintainer<Widelands::TerrainDescription>& terrains,
   const FieldsToDraw& fields_to_draw,
   const float z_value,
   const Widelands::Player* player) {
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
		if (field.brn_index == FieldsToDraw::kInvalidIndex) {
			continue;
		}

		const Widelands::Map* map =
		   (player != nullptr) && !player->see_all() ? &player->egbase().map() : nullptr;
		// Dithering triangles for Down triangle.
		if (field.bln_index != FieldsToDraw::kInvalidIndex) {
			const Widelands::DescriptionIndex terrain_d =
			   map != nullptr ? player->fields()[map->get_index(field.fcoords)].terrains.load().d :
                             field.fcoords.field->terrain_d();
			const Widelands::DescriptionIndex terrain_r =
			   map != nullptr ? player->fields()[map->get_index(field.fcoords)].terrains.load().r :
                             field.fcoords.field->terrain_r();
			maybe_add_dithering_triangle(gametime, terrains, fields_to_draw, field.brn_index,
			                             current_index, field.bln_index, terrain_d, terrain_r);

			const Widelands::DescriptionIndex terrain_dd =
			   map != nullptr ?
               player->fields()[map->get_index(map->bl_n(field.fcoords))].terrains.load().r :
               fields_to_draw.at(field.bln_index).fcoords.field->terrain_r();
			maybe_add_dithering_triangle(gametime, terrains, fields_to_draw, field.bln_index,
			                             field.brn_index, current_index, terrain_d, terrain_dd);

			if (field.ln_index != FieldsToDraw::kInvalidIndex) {
				const Widelands::DescriptionIndex terrain_l =
				   map != nullptr ?
                  player->fields()[map->get_index(map->l_n(field.fcoords))].terrains.load().r :
                  fields_to_draw.at(field.ln_index).fcoords.field->terrain_r();
				maybe_add_dithering_triangle(gametime, terrains, fields_to_draw, current_index,
				                             field.bln_index, field.brn_index, terrain_d, terrain_l);
			}
		}

		// Dithering for right triangle.
		if (field.rn_index != FieldsToDraw::kInvalidIndex) {
			const Widelands::DescriptionIndex terrain_r =
			   map != nullptr ? player->fields()[map->get_index(field.fcoords)].terrains.load().r :
                             field.fcoords.field->terrain_r();
			const Widelands::DescriptionIndex terrain_d =
			   map != nullptr ? player->fields()[map->get_index(field.fcoords)].terrains.load().d :
                             field.fcoords.field->terrain_d();

			maybe_add_dithering_triangle(gametime, terrains, fields_to_draw, current_index,
			                             field.brn_index, field.rn_index, terrain_r, terrain_d);
			const Widelands::DescriptionIndex terrain_rr =
			   map != nullptr ?
               player->fields()[map->get_index(map->r_n(field.fcoords))].terrains.load().d :
               fields_to_draw.at(field.rn_index).fcoords.field->terrain_d();
			maybe_add_dithering_triangle(gametime, terrains, fields_to_draw, field.brn_index,
			                             field.rn_index, current_index, terrain_r, terrain_rr);

			if (field.trn_index != FieldsToDraw::kInvalidIndex) {
				const Widelands::DescriptionIndex terrain_u =
				   map != nullptr ?
                  player->fields()[map->get_index(map->tr_n(field.fcoords))].terrains.load().d :
                  fields_to_draw.at(field.trn_index).fcoords.field->terrain_d();
				maybe_add_dithering_triangle(gametime, terrains, fields_to_draw, field.rn_index,
				                             current_index, field.brn_index, terrain_r, terrain_u);
			}
		}
	}

	const BlitData& blit_data = terrains.get(0).get_texture(0).blit_data();
	const Rectf texture_coordinates = to_gl_texture(blit_data);
	gl_draw(blit_data.texture_id, texture_coordinates.w, texture_coordinates.h, z_value);
}
