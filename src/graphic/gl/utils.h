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
#include <vector>

#include <stdint.h>

#include "base/log.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "graphic/gl/system_headers.h"

namespace Gl {

class Shader;

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
// on all errors. Also grows the server memory only when needed.
template<typename T>
class Buffer {
public:
	Buffer() : buffer_size_(0) {
		glGenBuffers(1, &object_);
		if (!object_) {
			throw wexception("Could not create GL program.");
		}
	}

	~Buffer() {
		if (object_) {
			glDeleteBuffers(1, &object_);
		}
	}

	// Calls glBindBuffer on the underlying buffer data.
	void bind() const {
		glBindBuffer(GL_ARRAY_BUFFER, object_);
	}


	// Copies 'elements' into the buffer. If the buffer is too small to hold the
	// data, it is reallocated. Does not check if the buffer is already bound.
	void update(const std::vector<T>& items) {
		if (buffer_size_ < items.size()) {
			glBufferData(GL_ARRAY_BUFFER, items.size() * sizeof(T), items.data(), GL_DYNAMIC_DRAW);
			buffer_size_ = items.size();
		} else {
			glBufferSubData(GL_ARRAY_BUFFER, 0, items.size() * sizeof(T), items.data());
		}
	}

private:
	GLuint object_;
	size_t buffer_size_;  // In number of elements.

	DISALLOW_COPY_AND_ASSIGN(Buffer);
};

// Calls glVertexAttribPointer.
void vertex_attrib_pointer(int vertex_index, int num_items, int stride, int offset);

// Swap order of rows in m_pixels, to compensate for the upside-down nature of the
// OpenGL coordinate system.
void swap_rows(int width, int height, int pitch, int bpp, uint8_t* pixels);

}  // namespace Gl

/**
 * handle_glerror() is intended to make debugging of OpenGL easier. It logs the
 * error code returned by glGetError and returns the error code.
 */
#define handle_glerror() Gl::_handle_glerror(__FILE__, __LINE__)

#endif  // end of include guard: WL_GRAPHIC_GL_UTILS_H
