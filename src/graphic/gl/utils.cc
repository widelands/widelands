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

#include "graphic/gl/utils.h"

#include <memory>
#include <string>

#include <SDL_video.h>

#include "base/log.h"
#include "base/wexception.h"

namespace Gl {

namespace {

// Returns a readable string for a GL_*_SHADER 'type' for debug output.
std::string shader_to_string(GLenum type) {
	if (type == GL_VERTEX_SHADER) {
		return "vertex";
	}
	if (type == GL_FRAGMENT_SHADER) {
		return "fragment";
	}
	return "unknown";
}

// Creates one OpenGL buffer.
GLuint create_buffer() {
	GLuint buffer = 0;
	glGenBuffers(1, &buffer);
	return buffer;
}

}  // namespace

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
	return err;
}

// Thin wrapper around a Shader object to ensure proper cleanup.
class Shader {
public:
	Shader(GLenum type);
	~Shader();

	GLuint object() const {
		return shader_object_;
	}

	// Compiles 'source'. Throws an exception on error.
	void compile(const char* source);

private:
	const GLenum type_;
	const GLuint shader_object_;

	DISALLOW_COPY_AND_ASSIGN(Shader);
};

Shader::Shader(GLenum type) : type_(type), shader_object_(glCreateShader(type)) {
	if (!shader_object_) {
		throw wexception("Could not create %s shader.", shader_to_string(type).c_str());
	}
}

Shader::~Shader() {
	if (shader_object_) {
		glDeleteShader(shader_object_);
	}
}

void Shader::compile(const char* source) {
	glShaderSource(shader_object_, 1, &source, nullptr);

	glCompileShader(shader_object_);
	GLint compiled;
	glGetShaderiv(shader_object_, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		GLint infoLen = 0;
		glGetShaderiv(shader_object_, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen > 1) {
			std::unique_ptr<char[]> infoLog(new char[infoLen]);
			glGetShaderInfoLog(shader_object_, infoLen, NULL, infoLog.get());
			throw wexception(
			   "Error compiling %s shader:\n%s", shader_to_string(type_).c_str(), infoLog.get());
		}
	}
}

Buffer::Buffer() : buffer_object_(create_buffer()) {
	if (!buffer_object_) {
		throw wexception("Could not create GL program.");
	}
}

Buffer::~Buffer() {
	if (buffer_object_) {
		glDeleteBuffers(1, &buffer_object_);
	}
}

Program::Program() : program_object_(glCreateProgram()) {
	if (!program_object_) {
		throw wexception("Could not create GL program.");
	}
}

Program::~Program() {
	if (program_object_) {
		glDeleteProgram(program_object_);
	}
}

void Program::build(const char* vertex_shader_source, const char* fragment_shader_source) {
	vertex_shader_.reset(new Shader(GL_VERTEX_SHADER));
	vertex_shader_->compile(vertex_shader_source);
	glAttachShader(program_object_, vertex_shader_->object());

	fragment_shader_.reset(new Shader(GL_FRAGMENT_SHADER));
	fragment_shader_->compile(fragment_shader_source);
	glAttachShader(program_object_, fragment_shader_->object());

	glLinkProgram(program_object_);

	// Check the link status
	GLint linked;
	glGetProgramiv(program_object_, GL_LINK_STATUS, &linked);
	if (!linked) {
		GLint infoLen = 0;
		glGetProgramiv(program_object_, GL_INFO_LOG_LENGTH, &infoLen);

		if (infoLen > 1) {
			std::unique_ptr<char[]> infoLog(new char[infoLen]);
			glGetProgramInfoLog(program_object_, infoLen, NULL, infoLog.get());
			throw wexception("Error linking:\n%s", infoLog.get());
		}
	}
}

}  // namespace Gl
