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

#ifndef WL_GRAPHIC_GL_ROAD_PROGRAM_H
#define WL_GRAPHIC_GL_ROAD_PROGRAM_H

#include <vector>

#include "base/macros.h"
#include "graphic/gl/fields_to_draw.h"
#include "graphic/gl/utils.h"
#include "logic/roadtype.h"

class RoadProgram {
public:
	// Compiles the program. Throws on error.
	RoadProgram();

	// Draws the roads.
	void draw(const FieldsToDraw& fields_to_draw);

private:
	struct PerVertexData {
		float x;
		float y;
		float texture_x;
		float texture_y;
		float brightness;

		// This is a hack: we want to draw busy and normal roads in the same
		// run, but since samplers (apparently?) cannot be passed through
		// attribute arrays, we instead sample twice (busy and normal) and mix
		// them together with 'texture_mix' which is either 1 or 0.
		float texture_mix;
	};
	// NOCOM(#sirver): add this in.
	// static_assert(sizeof(PerVertexData) == 20, "Wrong padding.");

	// Adds a road from 'start' to 'end' to be rendered in this frame using the
	// correct texture for 'road_type'.
	void add_road(const FieldsToDraw::Field& start,
	              const FieldsToDraw::Field& end,
	              const Widelands::RoadType road_type);

	// The buffer that will contain 'vertices_' for rendering.
	Gl::Buffer gl_array_buffer_;

	// The program used for drawing the roads.
	Gl::Program gl_program_;

	// Attributes.
	GLint attr_position_;
	GLint attr_texture_position_;
	GLint attr_brightness_;
	GLint attr_texture_mix_;

	// Uniforms.
	GLint u_normal_road_texture_;
	GLint u_busy_road_texture_;

	// All vertices that get rendered this frame.
	std::vector<PerVertexData> vertices_;

	DISALLOW_COPY_AND_ASSIGN(RoadProgram);
};

#endif  // end of include guard: WL_GRAPHIC_GL_ROAD_PROGRAM_H
