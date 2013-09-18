/*
 * Copyright 2010-2011 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef GL_UTILS_H
#define GL_UTILS_H

#define NO_SDL_GLEXT

#include <GL/glew.h>
#include <SDL_opengl.h>
#include <stdint.h>

struct SDL_PixelFormat;

uint32_t next_power_of_two(uint32_t x);
const SDL_PixelFormat & gl_rgba_format();
GLenum _handle_glerror(const char * file, unsigned int line);

/**
 * handle_glerror() is intended to make debugging of OpenGL easier. It logs the
 * error code returned by glGetError and returns the error code.
 */
#define handle_glerror() _handle_glerror(__FILE__, __LINE__)

#endif // GL_UTILS_H
