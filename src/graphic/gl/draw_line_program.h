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

#ifndef WL_GRAPHIC_GL_DRAW_LINE_PROGRAM_H
#define WL_GRAPHIC_GL_DRAW_LINE_PROGRAM_H

#include <vector>

#include "base/point.h"
#include "base/rect.h"
#include "graphic/blend_mode.h"
#include "graphic/color.h"
#include "graphic/gl/utils.h"

class DrawLineProgram {
public:
	struct Arguments {
		// Vertices of the quads that make up the lines plus. That means
		// points.size() == <number of lines> * 4.
		std::vector<FloatPoint> points;
		RGBColor color;
		float z_value;
		BlendMode blend_mode;  // Always BlendMode::kUseAlpha.
	};

	// Returns the (singleton) instance of this class.
	static DrawLineProgram& instance();

	void draw(const std::vector<Arguments>& arguments);

private:
	struct PerVertexData {
		float gl_x, gl_y, gl_z;
		float color_r, color_g, color_b;
		// This value is changing from -1 to 1 and is used for alpha blending.
		float distance_from_center;
	};
	static_assert(sizeof(PerVertexData) == 28, "Wrong padding.");

	DrawLineProgram();

	// This is only kept around so that we do not constantly
	// allocate memory for it.
	std::vector<PerVertexData> vertices_;

	// The buffer that contains the vertices for rendering.
	Gl::Buffer<PerVertexData> gl_array_buffer_;

	// The program.
	Gl::Program gl_program_;

	// Attributes.
	GLint attr_position_;
	GLint attr_color_;
	GLint attr_distance_from_center_;

	DISALLOW_COPY_AND_ASSIGN(DrawLineProgram);
};

#endif  // end of include guard: WL_GRAPHIC_GL_DRAW_LINE_PROGRAM_H
