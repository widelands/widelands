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

#ifndef WL_GRAPHIC_GL_ROAD_PROGRAM_H
#define WL_GRAPHIC_GL_ROAD_PROGRAM_H

#include "base/macros.h"
#include "graphic/gl/fields_to_draw.h"
#include "graphic/gl/utils.h"
#include "graphic/road_segments.h"

class RoadProgram {
public:
	// Compiles the program. Throws on error.
	RoadProgram();
	~RoadProgram() = default;

	// Draws the roads. The dimensions of the renderbuffer are needed to convert from pixel to GL
	// space.
	void draw(int renderbuffer_width,
	          int renderbuffer_height,
	          const FieldsToDraw& fields_to_draw,
	          float scale,
	          float z_value);

private:
	struct PerVertexData {
		float gl_x;
		float gl_y;
		float texture_x;
		float texture_y;
		float brightness;
	};
	static_assert(sizeof(PerVertexData) == 20, "Wrong padding.");

	// Adds a road from 'start' to 'end' to be rendered in this frame using the
	// correct texture for 'road_type'.
	enum Direction { kEast, kSouthEast, kSouthWest };
	void add_road(int renderbuffer_width,
	              int renderbuffer_height,
	              const FieldsToDraw::Field& start,
	              const FieldsToDraw::Field& end,
	              float scale,
	              const Widelands::RoadSegment road_type,
	              const Direction direction,
	              uint32_t* gl_texture);

	// The buffer that will contain 'vertices_' for rendering.
	Gl::Buffer<PerVertexData> gl_array_buffer_;

	// The program used for drawing the roads.
	Gl::Program gl_program_;

	// Attributes.
	GLint attr_position_;
	GLint attr_texture_position_;
	GLint attr_brightness_;

	// Uniforms.
	GLint u_texture_;
	GLint u_z_value_;

	// All vertices that get rendered this frame.
	std::vector<PerVertexData> vertices_;

	DISALLOW_COPY_AND_ASSIGN(RoadProgram);
};

#endif  // end of include guard: WL_GRAPHIC_GL_ROAD_PROGRAM_H
