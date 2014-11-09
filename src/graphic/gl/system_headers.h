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

#ifndef WL_GRAPHIC_GL_SYSTEM_HEADERS_H
#define WL_GRAPHIC_GL_SYSTEM_HEADERS_H

// This includes the correct OpenGL headers for us. Use this instead of
// including any system OpenGL headers yourself.

// We do not want to include SDL_opengl.h because it defines the
// fixed pipeline only which we do not want to use.
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#else
#ifdef _WIN32
#include <GL/glew.h>
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#endif  // end of include guard: WL_GRAPHIC_GL_SYSTEM_HEADERS_H
