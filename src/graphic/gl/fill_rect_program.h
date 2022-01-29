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

#ifndef WL_GRAPHIC_GL_FILL_RECT_PROGRAM_H
#define WL_GRAPHIC_GL_FILL_RECT_PROGRAM_H

#include "base/rect.h"
#include "graphic/blend_mode.h"
#include "graphic/color.h"
#include "graphic/gl/utils.h"

class FillRectProgram {
public:
	struct Arguments {
		Rectf destination_rect;
		float z_value;
		RGBAColor color;
		BlendMode blend_mode;
	};

	// Returns the (singleton) instance of this class.
	static FillRectProgram& instance();

	// Fills a solid rect in 'color'. If blend_mode is BlendMode::UseAlpha, this
	// will brighten the rect, if it is BlendMode::Subtract it darkens it.
	void
	draw(const Rectf& destination_rect, float z_value, const RGBAColor& color, BlendMode blend_mode);

	void draw(const std::vector<Arguments>& arguments);

private:
	FillRectProgram();

	struct PerVertexData {
		PerVertexData(float init_gl_x,
		              float init_gl_y,
		              float init_gl_z,
		              float init_r,
		              float init_g,
		              float init_b,
		              float init_a)
		   : gl_x(init_gl_x),
		     gl_y(init_gl_y),
		     gl_z(init_gl_z),
		     r(init_r),
		     g(init_g),
		     b(init_b),
		     a(init_a) {
		}

		float gl_x, gl_y, gl_z;
		float r, g, b, a;
	};
	static_assert(sizeof(PerVertexData) == 28, "Wrong padding.");

	// This is only kept around so that we do not constantly allocate memory for
	// it.
	std::vector<PerVertexData> vertices_;

	// The buffer that will contain the quad for rendering.
	Gl::Buffer<PerVertexData> gl_array_buffer_;

	// The program.
	Gl::Program gl_program_;

	// Attributes.
	GLint attr_position_;
	GLint attr_color_;

	DISALLOW_COPY_AND_ASSIGN(FillRectProgram);
};

#endif  // end of include guard: WL_GRAPHIC_GL_FILL_RECT_PROGRAM_H
