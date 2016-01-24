/*
 * Copyright (C) 2006-2016 by the Widelands Development Team
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

#include "graphic/gl/initialize.h"

#include <csignal>

#include <SDL.h>

#include "base/macros.h"
#include "graphic/gl/utils.h"

namespace Gl {

SDL_GLContext
initialize(const Trace& trace, SDL_Window* sdl_window, GLint* max_texture_size) {
	// Request an OpenGL 2 context with double buffering.
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_GLContext gl_context = SDL_GL_CreateContext(sdl_window);
	SDL_GL_MakeCurrent(sdl_window, gl_context);

#ifdef USE_GLBINDING
	glbinding::Binding::initialize();

	if (trace == Trace::kYes) {
		setCallbackMaskExcept(
				glbinding::CallbackMask::After | glbinding::CallbackMask::ParametersAndReturnValue,
				{"glGetError"});
		glbinding::setAfterCallback([](const glbinding::FunctionCall& call) {
			log("%s(", call.function.name());
			for (size_t i = 0; i < call.parameters.size(); ++i) {
				log("%s", call.parameters[i]->asString().c_str());
				if (i < call.parameters.size() - 1)
					log(", ");
			}
			log(")");
			if (call.returnValue) {
				log(" -> %s", call.returnValue->asString().c_str());
			}
			const auto error = glGetError();
			log(" [%s]\n", gl_error_to_string(error));
			// The next few lines will terminate Widelands if there was any OpenGL
			// error. This is useful for super aggressive debugging, but probably
			// not for regular builds. Comment it in if you need to understand
			// OpenGL problems.
			// if (error != GL_NO_ERROR) {
			// std::raise(SIGINT);
			// }
		});
	}
#else
	// See graphic/gl/system_headers.h for an explanation of the next line.
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		log("glewInit returns %i\nYour OpenGL installation must be __very__ broken. %s\n", err,
		    glewGetErrorString(err));
		throw wexception("glewInit returns %i: Broken OpenGL installation.", err);
	}
#endif

	log(
	   "Graphics: OpenGL: Version \"%s\"\n", reinterpret_cast<const char*>(glGetString(GL_VERSION)));

#define LOG_SDL_GL_ATTRIBUTE(x)                                                                        \
	{                                                                                               \
		int value;                                                                                   \
		SDL_GL_GetAttribute(x, &value);                                                              \
		log("Graphics: %s is %d\n", #x, value);                                                      \
	}

	LOG_SDL_GL_ATTRIBUTE(SDL_GL_RED_SIZE)
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_GREEN_SIZE)
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_BLUE_SIZE);
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_ALPHA_SIZE);
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_BUFFER_SIZE);
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_DOUBLEBUFFER);
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_DEPTH_SIZE);
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_STENCIL_SIZE);
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_ACCUM_RED_SIZE);
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_ACCUM_GREEN_SIZE);
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_ACCUM_BLUE_SIZE);
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_ACCUM_ALPHA_SIZE);
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_STEREO);
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_MULTISAMPLEBUFFERS);
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_MULTISAMPLESAMPLES);
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_ACCELERATED_VISUAL);
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_CONTEXT_MAJOR_VERSION);
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_CONTEXT_MINOR_VERSION);
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_CONTEXT_FLAGS);
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_CONTEXT_PROFILE_MASK);
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_SHARE_WITH_CURRENT_CONTEXT);
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE);
#undef LOG_SDL_GL_ATTRIBUTE

	GLboolean glBool;
	glGetBooleanv(GL_DOUBLEBUFFER, &glBool);
	log("Graphics: OpenGL: Double buffering %s\n", (glBool == GL_TRUE) ? "enabled" : "disabled");

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, max_texture_size);
	log("Graphics: OpenGL: Max texture size: %u\n", *max_texture_size);

	log("Graphics: OpenGL: ShadingLanguage: \"%s\"\n",
	    reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));

	glDrawBuffer(GL_BACK);

	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClear(GL_COLOR_BUFFER_BIT);

	return gl_context;
}

}  // namespace Gl
