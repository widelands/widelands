/*
 * Copyright 2010 by the Widelands Development Team
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

#include "graphic/render/gl_utils.h"

#include <SDL_video.h>

#include "log.h"

/**
 * Return the smallest power of two greater than or equal to \p x.
 */
uint32_t next_power_of_two(uint32_t x)
{
	uint32_t pot = 1;

	while (pot < x)
		pot *= 2;

	return pot;
}

/**
 * \return the standard 32-bit RGBA format that we use in OpenGL
 */
const SDL_PixelFormat & gl_rgba_format()
{
	static SDL_PixelFormat format;
	static bool init = false;
	if (init)
		return format;

	init = true;
	memset(&format, 0, sizeof(format));
	format.BitsPerPixel = 32;
	format.BytesPerPixel = 4;
	format.Rmask = 0x000000ff;
	format.Gmask = 0x0000ff00;
	format.Bmask = 0x00ff0000;
	format.Amask = 0xff000000;
	format.Rshift = 0;
	format.Gshift = 8;
	format.Bshift = 16;
	format.Ashift = 24;
	return format;
}

GLenum _handle_glerror(const char * file, unsigned int line)
{
	GLenum err = glGetError();
#ifndef NDEBUG
	if (err == GL_NO_ERROR)
		return err;

	log("%s:%d: OpenGL ERROR: ", file, line);

	switch (err)
	{
	case GL_INVALID_VALUE:
		log("invalid value\n");
		break;
	case GL_INVALID_ENUM:
		log("invalid enum\n");
		break;
	case GL_INVALID_OPERATION:
		log("invalid operation\n");
		break;
	case GL_STACK_OVERFLOW:
		log("stack overflow\n");
		break;
	case GL_STACK_UNDERFLOW:
		log("stack undeflow\n");
		break;
	case GL_OUT_OF_MEMORY:
		log("out of memory\n");
		break;
	case GL_TABLE_TOO_LARGE:
		log("table too large\n");
		break;
	default:
		log("unknown\n");
	}
#endif
	return err;
}
