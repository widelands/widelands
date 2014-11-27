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

#ifndef WL_GRAPHIC_GL_TERRAIN_PROGRAM_H
#define WL_GRAPHIC_GL_TERRAIN_PROGRAM_H

#include <vector>

#include "graphic/gl/utils.h"
#include "logic/description_maintainer.h"
#include "logic/world/terrain_description.h"

class FieldsToDraw;

class TerrainProgram {
public:
	// Compiles the program. Throws on errors.
	TerrainProgram();

	// Draws the terrain.
	void draw(const DescriptionMaintainer<Widelands::TerrainDescription>& terrains,
	          const FieldsToDraw& fields_to_draw);

private:
	struct PerVertexData {
		PerVertexData(float init_gl_x,
		              float init_gl_y,
		              float init_brightness,
		              float init_texture_x,
		              float init_texture_y,
		              float init_texture_offset_x,
		              float init_texture_offset_y)
		   : gl_x(init_gl_x),
		     gl_y(init_gl_y),
		     brightness(init_brightness),
		     texture_x(init_texture_x),
		     texture_y(init_texture_y),
		     texture_offset_x(init_texture_offset_x),
		     texture_offset_y(init_texture_offset_y) {
		}

		float gl_x;
		float gl_y;
		float brightness;
		float texture_x;
		float texture_y;
		float texture_offset_x;
		float texture_offset_y;
	};
	static_assert(sizeof(PerVertexData) == 28, "Wrong padding.");

	void gl_draw(int gl_texture, float texture_w, float texture_h);

	// The buffer that will contain 'vertices_' for rendering.
	Gl::Buffer gl_array_buffer_;

	// The program used for drawing the terrain.
	Gl::Program gl_program_;

	// Attributes.
	GLint attr_brightness_;
	GLint attr_position_;
	GLint attr_texture_offset_;
	GLint attr_texture_position_;

	// Uniforms.
	GLint u_terrain_texture_;
	GLint u_texture_dimensions_;

	// Objects below are kept around to avoid memory allocations on each frame.
	// They could theoretically also be recreated.

	// A map from terrain index in world.terrains() to vertices_
	// that have this terrain type.
	// NOCOM(#sirver): fix comments
	std::vector<PerVertexData> vertices_;

	DISALLOW_COPY_AND_ASSIGN(TerrainProgram);
};

#endif  // end of include guard: WL_GRAPHIC_GL_TERRAIN_PROGRAM_H
