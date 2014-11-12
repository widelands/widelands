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

#ifndef WL_GRAPHIC_GL_UTILS_H
#define WL_GRAPHIC_GL_UTILS_H

#include <memory>

#include <stdint.h>

#include "base/macros.h"
#include "graphic/gl/system_headers.h"

struct SDL_PixelFormat;

namespace Gl {

class Shader;

const SDL_PixelFormat & gl_rgba_format();
GLenum _handle_glerror(const char * file, unsigned int line);

// Thin wrapper around a OpenGL program object to ensure proper cleanup. Throws
// on all errors.
class Program {
public:
	Program();
	~Program();

	GLuint object() const {
		return program_object_;
	}

	// Creates and compiles 'vertex_shader_source' and 'fragment_shader_source'
	// into shader objects. Then links them into the program.
	void build(const char* vertex_shader_source, const char* fragment_shader_source);

private:
	const GLuint program_object_;
	std::unique_ptr<Shader> vertex_shader_;
	std::unique_ptr<Shader> fragment_shader_;

	DISALLOW_COPY_AND_ASSIGN(Program);
};

// Thin wrapper around a OpenGL buffer object to ensure proper cleanup. Throws
// on all errors.
class Buffer {
public:
	Buffer();
	~Buffer();

	GLuint object() const {
		return buffer_object_;
	}

private:
	const GLuint buffer_object_;

	DISALLOW_COPY_AND_ASSIGN(Buffer);
};

}  // namespace Gl

/**
 * handle_glerror() is intended to make debugging of OpenGL easier. It logs the
 * error code returned by glGetError and returns the error code.
 */
#define handle_glerror() Gl::_handle_glerror(__FILE__, __LINE__)

#endif  // end of include guard: WL_GRAPHIC_GL_UTILS_H
