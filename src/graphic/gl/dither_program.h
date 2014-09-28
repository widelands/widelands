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

#ifndef WL_GRAPHIC_GL_DITHER_PROGRAM_H
#define WL_GRAPHIC_GL_DITHER_PROGRAM_H

#include "graphic/gl/fields_to_draw.h"
#include "graphic/gl/utils.h"
#include "logic/description_maintainer.h"
#include "logic/world/terrain_description.h"

class DitherProgram {
public:
	DitherProgram();

	// Draws the terrain.
	void draw(const DescriptionMaintainer<Widelands::TerrainDescription>& terrains,
	          const FieldsToDraw& fields_to_draw);

private:
	// Adds the triangle between the indexes (which index 'fields_to_draw' to
	// vertices_ if the my_terrain != other_terrain and the dither_layer()
	// agree.
	void maybe_add_dithering_triangle(
	   const DescriptionMaintainer<Widelands::TerrainDescription>& terrains,
	   const FieldsToDraw& fields_to_draw,
	   int idx1,
	   int idx2,
	   int idx3,
	   int my_terrain,
	   int other_terrain);

	// Adds the 'field' as an vertex to the 'vertices_' entry for 'terrain'. The
	// 'order_index' defines which texture position will be used for this
	// vertcx.
	void add_vertex(const FieldsToDraw::Field& field, int order_index, int terrain);

	struct PerVertexData {
		float x;
		float y;
		float texture_x;
		float texture_y;
		float brightness;
		float dither_texture_x;
		float dither_texture_y;
	};

	// The program used for drawing the terrain.
	Gl::Program gl_program_;

	// The buffer that contains the data to be rendered.
	Gl::Buffer gl_array_buffer_;

	// Attributes.
	GLint attr_position_;
	GLint attr_texture_position_;
	GLint attr_dither_texture_position_;
	GLint attr_brightness_;

	// Uniforms.
	GLint u_terrain_texture_;
	GLint u_dither_texture_;

	// Objects below are here to avoid memory allocations on each frame, they
	// could theoretically also always be recreated.
	std::vector<std::vector<PerVertexData>> vertices_;
};

#endif  // end of include guard: WL_GRAPHIC_GL_DITHER_PROGRAM_H
