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
		// The line is drawn from the top left to the bottom right of
		// this rectangle.
		FloatRect destination_rect;
		float z_value;
		RGBAColor color;
		uint8_t line_width;
		BlendMode blend_mode;
	};

	// Returns the (singleton) instance of this class.
	static DrawLineProgram& instance();

	// Draws a line from (x1, y1) to (x2, y2) which are in gl
	// coordinates in 'color' with a 'line_width' in pixels.
	void draw(const FloatPoint& start,
	          const FloatPoint& end,
	          const float z_value,
	          const RGBColor& color,
	          const int line_width);

	void draw(std::vector<Arguments> arguments);


private:
	DrawLineProgram();

	struct PerVertexData {
		PerVertexData(float init_gl_x,
		              float init_gl_y,
		              float init_gl_z,
		              float init_color_r,
		              float init_color_g,
		              float init_color_b)
		   : gl_x(init_gl_x),
		     gl_y(init_gl_y),
		     gl_z(init_gl_z),
		     color_r(init_color_r),
		     color_g(init_color_g),
		     color_b(init_color_b) {
		}

		float gl_x, gl_y, gl_z;
		float color_r, color_g, color_b;
	};
	static_assert(sizeof(PerVertexData) == 24, "Wrong padding.");

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

	DISALLOW_COPY_AND_ASSIGN(DrawLineProgram);
};

#endif  // end of include guard: WL_GRAPHIC_GL_DRAW_LINE_PROGRAM_H
