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
 *
 */

#ifndef WL_GRAPHIC_GL_INITIALIZE_H
#define WL_GRAPHIC_GL_INITIALIZE_H

#include <SDL_video.h>

#include "graphic/gl/system_headers.h"

namespace Gl {

// Initializes OpenGL. Creates a context for 'window' using SDL and loads the
// GL library. Fills in 'max_texture_size' and returns the created SDL_Context
// which must be closed by the caller.
// If we are built against glbinding, 'trace' will set up tracing for
// OpenGL and output every single opengl call ever made, together with it's
// arguments, return values and the result from glGetError.
enum class Trace {
	kYes,
	kNo,
};
SDL_GLContext initialize(const Trace& trace, SDL_Window* window, GLint* max_texture_size);

}  // namespace Gl

#endif  // end of include guard: WL_GRAPHIC_GL_INITIALIZE_H
