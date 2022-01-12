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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef WL_GRAPHIC_GL_DRAW_LINE_PROGRAM_H
#define WL_GRAPHIC_GL_DRAW_LINE_PROGRAM_H

#include "base/rect.h"
#include "graphic/blend_mode.h"
#include "graphic/gl/utils.h"

// This program actually only draws Triangles, which are tesselations of the
// lines to draw. It uses the alpha value of each point to fade out the lines
// and achieve good looking anti-aliasing.
// Though it is conceptually quite similar to the 'FillRectProgram', the
// differences in the fragment shader make this require a separate OpenGl
// program, though probably some code could be shared.
class DrawLineProgram {
public:
	struct PerVertexData {
		float gl_x, gl_y, gl_z;
		float color_r, color_g, color_b, float_a;
	};
	static_assert(sizeof(PerVertexData) == 28, "Wrong padding.");

	struct Arguments {
		// Vertices of the triangles to draw. We expect everything but 'gl_z' to
		// be filled in by the caller. We fill up the z value with 'z_value'
		// before drawing. We directly expose the PerVertexData structure to
		// avoid a copy.
		std::vector<PerVertexData> vertices;
		float z_value;
		BlendMode blend_mode;  // Always BlendMode::kUseAlpha.
	};

	// Returns the (singleton) instance of this class.
	static DrawLineProgram& instance();

	// Takes the 'arguments' by value on purpose.
	void draw(std::vector<Arguments> arguments);

private:
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

	DISALLOW_COPY_AND_ASSIGN(DrawLineProgram);
};

#endif  // end of include guard: WL_GRAPHIC_GL_DRAW_LINE_PROGRAM_H
