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

#ifndef WL_GRAPHIC_GL_BLIT_PROGRAM_H
#define WL_GRAPHIC_GL_BLIT_PROGRAM_H

#include "base/rect.h"
#include "graphic/color.h"
#include "graphic/gl/utils.h"
// NOCOM(#sirver): rename to composite
#include "graphic/compositemode.h"

class BlitProgram {
public:
	// Returns the (singleton) instance of this class.
	static BlitProgram& instance();

	// Draws the rectangle 'gl_src_rect' from the texture with the name
	// 'gl_texture' to 'gl_dest_rect' in the currently bound framebuffer. All
	// coordinates are in the OpenGL frame. The 'composite' defines if the
	// values are copied or if alpha values are used.
	void draw(const FloatRect& gl_dest_rect,
	          const FloatRect& gl_src_rect,
	          const GLuint gl_texture,
	          const Composite composite);

private:
	BlitProgram();

	struct PerVertexData {
		float gl_x, gl_y;
	};
	static_assert(sizeof(PerVertexData) == 8, "Wrong padding.");

	// The buffer that will contain the quad for rendering.
	Gl::Buffer gl_array_buffer_;

	// The program.
	Gl::Program gl_program_;

	// Attributes.
	GLint attr_position_;

	// Uniforms.
	GLint u_texture_;
	GLint u_dst_rect_;
	GLint u_src_rect_;

	DISALLOW_COPY_AND_ASSIGN(BlitProgram);
};


#endif  // end of include guard: WL_GRAPHIC_GL_DRAW_RECT_PROGRAM_H
