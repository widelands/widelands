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

#include "graphic/gl/terrain_program.h"

#include <atomic>

#include "graphic/gl/coordinate_conversion.h"
#include "graphic/gl/fields_to_draw.h"
#include "graphic/gl/utils.h"
#include "graphic/texture.h"
#include "logic/player.h"

// Full specification:
// https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.1.20.pdf
// We target OpenGL 2.1 for the desktop here.
TerrainProgram::TerrainProgram() {
	gl_program_.build("terrain");

	attr_brightness_ = glGetAttribLocation(gl_program_.object(), "attr_brightness");
	attr_position_ = glGetAttribLocation(gl_program_.object(), "attr_position");
	attr_texture_offset_ = glGetAttribLocation(gl_program_.object(), "attr_texture_offset");
	attr_texture_position_ = glGetAttribLocation(gl_program_.object(), "attr_texture_position");

	u_terrain_texture_ = glGetUniformLocation(gl_program_.object(), "u_terrain_texture");
	u_texture_dimensions_ = glGetUniformLocation(gl_program_.object(), "u_texture_dimensions");
	u_z_value_ = glGetUniformLocation(gl_program_.object(), "u_z_value");
}

void TerrainProgram::gl_draw(int gl_texture, float texture_w, float texture_h, float z_value) {
	glUseProgram(gl_program_.object());

	auto& gl_state = Gl::State::instance();
	gl_state.enable_vertex_attrib_array(
	   {attr_brightness_, attr_position_, attr_texture_offset_, attr_texture_position_});

	gl_array_buffer_.bind();
	gl_array_buffer_.update(vertices_);

	Gl::vertex_attrib_pointer(
	   attr_brightness_, 1, sizeof(PerVertexData), offsetof(PerVertexData, brightness));
	Gl::vertex_attrib_pointer(
	   attr_position_, 2, sizeof(PerVertexData), offsetof(PerVertexData, gl_x));
	Gl::vertex_attrib_pointer(
	   attr_texture_offset_, 2, sizeof(PerVertexData), offsetof(PerVertexData, texture_offset_x));
	Gl::vertex_attrib_pointer(
	   attr_texture_position_, 2, sizeof(PerVertexData), offsetof(PerVertexData, texture_x));

	gl_state.bind(GL_TEXTURE0, gl_texture);

	glUniform1f(u_z_value_, z_value);
	glUniform1i(u_terrain_texture_, 0);
	glUniform2f(u_texture_dimensions_, texture_w, texture_h);

	glDrawArrays(GL_TRIANGLES, 0, vertices_.size());
}

void TerrainProgram::add_vertex(const FieldsToDraw::Field& field, const Vector2f& texture_offset) {
	vertices_.emplace_back();
	PerVertexData& back = vertices_.back();

	back.gl_x = field.gl_position.x;
	back.gl_y = field.gl_position.y;
	back.brightness = field.brightness;
	back.texture_x = field.texture_coords.x;
	back.texture_y = field.texture_coords.y;
	back.texture_offset_x = texture_offset.x;
	back.texture_offset_y = texture_offset.y;
}

void TerrainProgram::draw(
   uint32_t gametime,
   const Widelands::DescriptionMaintainer<Widelands::TerrainDescription>& terrains,
   const FieldsToDraw& fields_to_draw,
   float z_value,
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

		// Down triangle.
		if (field.bln_index != FieldsToDraw::kInvalidIndex) {
			const Widelands::DescriptionIndex terrain =
			   (player != nullptr) && !player->see_all() ?
               player->fields()[player->egbase().map().get_index(field.fcoords)].terrains.load().d :
               field.fcoords.field->terrain_d();
			const Vector2f texture_offset =
			   to_gl_texture(terrains.get(terrain).get_texture(gametime).blit_data()).origin();
			add_vertex(fields_to_draw.at(current_index), texture_offset);
			add_vertex(fields_to_draw.at(field.bln_index), texture_offset);
			add_vertex(fields_to_draw.at(field.brn_index), texture_offset);
		}

		// Right triangle.
		if (field.rn_index != FieldsToDraw::kInvalidIndex) {
			const Widelands::DescriptionIndex terrain =
			   (player != nullptr) && !player->see_all() ?
               player->fields()[player->egbase().map().get_index(field.fcoords)].terrains.load().r :
               field.fcoords.field->terrain_r();
			const Vector2f texture_offset =
			   to_gl_texture(terrains.get(terrain).get_texture(gametime).blit_data()).origin();
			add_vertex(fields_to_draw.at(current_index), texture_offset);
			add_vertex(fields_to_draw.at(field.brn_index), texture_offset);
			add_vertex(fields_to_draw.at(field.rn_index), texture_offset);
		}
	}

	const BlitData& blit_data = terrains.get(0).get_texture(0).blit_data();
	const Rectf texture_coordinates = to_gl_texture(blit_data);
	gl_draw(blit_data.texture_id, texture_coordinates.w, texture_coordinates.h, z_value);
}
