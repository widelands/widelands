/*
 * Copyright (C) 2006-2019 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_GL_TERRAIN_PROGRAM_H
#define WL_GRAPHIC_GL_TERRAIN_PROGRAM_H

#include "base/vector.h"
#include "graphic/gl/fields_to_draw.h"
#include "graphic/gl/utils.h"
#include "logic/map_objects/description_maintainer.h"
#include "logic/map_objects/world/terrain_description.h"

class TerrainProgram {
public:
	// Compiles the program. Throws on errors.
	TerrainProgram();

	// Draws the terrain.
	void draw(uint32_t gametime,
	          const Widelands::DescriptionMaintainer<Widelands::TerrainDescription>& terrains,
	          const FieldsToDraw& fields_to_draw,
	          float z_value);

private:
	struct PerVertexData {
		float gl_x;
		float gl_y;
		float brightness;
		float texture_x;
		float texture_y;
		float texture_offset_x;
		float texture_offset_y;
	};
	static_assert(sizeof(PerVertexData) == 28, "Wrong padding.");

	void gl_draw(int gl_texture, float texture_w, float texture_h, float z_value);

	// Adds a vertex to the end of vertices with data from 'field' and 'texture_coordinates'.
	void add_vertex(const FieldsToDraw::Field& field, const Vector2f& texture_coordinates);

	// The program used for drawing the terrain.
	Gl::Program gl_program_;

	// The buffer that will contain 'vertices_' for rendering.
	Gl::Buffer<PerVertexData> gl_array_buffer_;

	// Attributes.
	GLint attr_brightness_;
	GLint attr_position_;
	GLint attr_texture_offset_;
	GLint attr_texture_position_;

	// Uniforms.
	GLint u_terrain_texture_;
	GLint u_texture_dimensions_;
	GLint u_z_value_;

	// Objects below are kept around to avoid memory allocations on each frame.
	// They could theoretically also be recreated.
	std::vector<PerVertexData> vertices_;

	DISALLOW_COPY_AND_ASSIGN(TerrainProgram);
};

#endif  // end of include guard: WL_GRAPHIC_GL_TERRAIN_PROGRAM_H
