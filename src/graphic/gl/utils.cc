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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include "graphic/gl/utils.h"

#include <cassert>
#include <memory>

#include "base/wexception.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"

namespace Gl {

namespace {

constexpr GLenum NONE = static_cast<GLenum>(0);

// Reads 'filename' from g_fs into a string.
std::string read_file(const std::string& filename) {
	std::string content;
	FileRead fr;
	fr.open(*g_fs, filename);
	content.assign(fr.data(0), fr.get_size());
	fr.close();
	return content;
}

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

}  // namespace

const char* gl_error_to_string(const GLenum err) {
	CLANG_DIAG_OFF("-Wswitch-enum")
#define LOG(a)                                                                                     \
	case a:                                                                                         \
		return #a
	switch (err) {
		LOG(GL_INVALID_ENUM);
		LOG(GL_INVALID_OPERATION);
		LOG(GL_INVALID_VALUE);
		LOG(GL_NO_ERROR);
		LOG(GL_OUT_OF_MEMORY);
		LOG(GL_STACK_OVERFLOW);
		LOG(GL_STACK_UNDERFLOW);
		LOG(GL_TABLE_TOO_LARGE);
	default:
		break;
	}
#undef LOG
	CLANG_DIAG_ON("-Wswitch-enum")
	return "unknown";
}

// Thin wrapper around a Shader object to ensure proper cleanup.
class Shader {
public:
	explicit Shader(GLenum type);
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
			CLANG_DIAG_OFF("-Wunknown-pragmas")
			CLANG_DIAG_OFF("-Wzero-as-null-pointer-constant")
			glGetShaderInfoLog(shader_object_, infoLen, nullptr, infoLog.get());
			CLANG_DIAG_ON("-Wzero-as-null-pointer-constant")
			CLANG_DIAG_ON("-Wunknown-pragmas")
			throw wexception(
			   "Error compiling %s shader:\n%s", shader_to_string(type_).c_str(), infoLog.get());
		}
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

void Program::build(const std::string& program_name) {
	std::string fragment_shader_source = read_file("shaders/" + program_name + ".fp");
	std::string vertex_shader_source = read_file("shaders/" + program_name + ".vp");

	vertex_shader_.reset(new Shader(GL_VERTEX_SHADER));
	vertex_shader_->compile(vertex_shader_source.c_str());
	glAttachShader(program_object_, vertex_shader_->object());

	fragment_shader_.reset(new Shader(GL_FRAGMENT_SHADER));
	fragment_shader_->compile(fragment_shader_source.c_str());
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
			CLANG_DIAG_OFF("-Wunknown-pragmas")
			CLANG_DIAG_OFF("-Wzero-as-null-pointer-constant")
			glGetProgramInfoLog(program_object_, infoLen, nullptr, infoLog.get());
			CLANG_DIAG_ON("-Wzero-as-null-pointer-constant")
			CLANG_DIAG_ON("-Wunknown-pragmas")
			throw wexception("Error linking:\n%s", infoLog.get());
		}
	}
}

State::State()
   : last_active_texture_(NONE), current_framebuffer_(0), current_framebuffer_texture_(0) {
}

void State::bind(const GLenum target, const GLuint texture) {
	if (texture == 0) {
		return;
	}
	do_bind(target, texture);
}

void State::do_bind(const GLenum target, const GLuint texture) {
	const auto currently_bound_texture = target_to_texture_[target];
	if (currently_bound_texture == texture) {
		return;
	}
	if (last_active_texture_ != target) {
		glActiveTexture(target);
		last_active_texture_ = target;
	}
	glBindTexture(GL_TEXTURE_2D, texture);

	target_to_texture_[target] = texture;
	texture_to_target_[currently_bound_texture] = NONE;
	texture_to_target_[texture] = target;
}

void State::unbind_texture_if_bound(const GLuint texture) {
	if (texture == 0) {
		return;
	}
	const auto target = texture_to_target_[texture];
	if (target != 0) {
		do_bind(target, 0);
	}
}

void State::delete_texture(const GLuint texture) {
	unbind_texture_if_bound(texture);
	glDeleteTextures(1, &texture);

	if (current_framebuffer_texture_ == texture) {
		current_framebuffer_texture_ = 0;
	}
}

void State::bind_framebuffer(const GLuint framebuffer, const GLuint texture) {
	if (current_framebuffer_ == framebuffer && current_framebuffer_texture_ == texture) {
		return;
	}

	// Some graphic drivers inaccurately do not flush their pipeline when
	// switching the framebuffer - and happily do draw calls into the wrong
	// framebuffers. I AM LOOKING AT YOU, INTEL!!!
	glFlush();

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	if (framebuffer != 0) {
		unbind_texture_if_bound(texture);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	}
	current_framebuffer_ = framebuffer;
	current_framebuffer_texture_ = texture;
}

void State::enable_vertex_attrib_array(std::unordered_set<GLint> entries) {
	for (const auto e : entries) {
		if (!enabled_attrib_arrays_.count(e)) {
			glEnableVertexAttribArray(e);
		}
	}
	for (const auto e : enabled_attrib_arrays_) {
		if (!entries.count(e)) {
			glDisableVertexAttribArray(e);
		}
	}
	enabled_attrib_arrays_ = entries;
}

// static
State& State::instance() {
	static State binder;
	return binder;
}

void vertex_attrib_pointer(int vertex_index, int num_items, int stride, size_t offset) {
	glVertexAttribPointer(
	   vertex_index, num_items, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offset));
}

void swap_rows(const int width, const int height, const int pitch, const int bpp, uint8_t* pixels) {
	uint8_t* begin_row = pixels;
	uint8_t* end_row = pixels + pitch * (height - 1);
	while (begin_row < end_row) {
		for (int x = 0; x < width * bpp; ++x) {
			std::swap(begin_row[x], end_row[x]);
		}
		begin_row += pitch;
		end_row -= pitch;
	}
}

}  // namespace Gl
