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

#include "graphic/gl/utils.h"
#include "logic/description_maintainer.h"
#include "logic/world/terrain_description.h"

class FieldsToDraw;

class DitherProgram {
public:
	DitherProgram();

	// Draws the terrain.
	void draw(const DescriptionMaintainer<Widelands::TerrainDescription>& terrains,
	          const FieldsToDraw& fields_to_draw);

private:
	// NOCOM(#sirver): comment
	Gl::Buffer gl_array_buffer_;

	// The program used for drawing the terrain.
	Gl::Program dither_gl_program_;

	// Attributes.
	GLint attr_position_;
	GLint attr_texture_position_;
	GLint attr_dither_texture_position_;
	GLint attr_brightness_;

	// Uniforms.
	GLint u_terrain_texture_;
	GLint u_dither_texture_;
};

#endif  // end of include guard: WL_GRAPHIC_GL_DITHER_PROGRAM_H
