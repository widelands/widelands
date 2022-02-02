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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "graphic/gl/initialize.h"

#include <csignal>
#include <regex>

#include <SDL_messagebox.h>

#include "base/i18n.h"
#include "base/log.h"
#include "base/macros.h"
#include "graphic/gl/utils.h"
#include "graphic/text/bidi.h"

namespace Gl {

SDL_GLContext initialize(
#ifdef USE_GLBINDING
   const Trace& trace,
#else
   const Trace&,
#endif
   SDL_Window* sdl_window,
   GLint* max_texture_size) {
	// Request an OpenGL 2 context with double buffering.
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_GLContext gl_context = SDL_GL_CreateContext(sdl_window);
	SDL_GL_MakeCurrent(sdl_window, gl_context);

	SDL_GL_SetSwapInterval(0);

#ifdef USE_GLBINDING
#ifndef GLBINDING3
	glbinding::Binding::initialize();

	// The undocumented command line argument --debug_gl_trace will set
	// Trace::kYes. This will log every OpenGL call that is made, together with
	// arguments, return values and glError status. This requires that Widelands
	// is built using -DOPTION_USE_GLBINDING:BOOL=ON. It is a NoOp for GLEW.
	if (trace == Trace::kYes) {
		setCallbackMaskExcept(
		   glbinding::CallbackMask::After | glbinding::CallbackMask::ParametersAndReturnValue,
		   {"glGetError"});
		glbinding::setAfterCallback([](const glbinding::FunctionCall& call) {
			log_dbg("%s(", call.function->name());
			for (size_t i = 0; i < call.parameters.size(); ++i) {
				log_dbg("%s", call.parameters[i]->asString().c_str());
				if (i < call.parameters.size() - 1)
					log_dbg(", ");
			}
			log_dbg(")");
			if (call.returnValue) {
				log_dbg(" -> %s", call.returnValue->asString().c_str());
			}
			const auto error = glGetError();
			log_dbg(" [%s]\n", gl_error_to_string(error));
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
	const glbinding::GetProcAddress get_proc_address = [](const char* name) {
		return reinterpret_cast<glbinding::ProcAddress>(SDL_GL_GetProcAddress(name));
	};
	glbinding::Binding::initialize(get_proc_address, true);

	// The undocumented command line argument --debug_gl_trace will set
	// Trace::kYes. This will log every OpenGL call that is made, together with
	// arguments, return values and glError status. This requires that Widelands
	// is built using -DOPTION_USE_GLBINDING:BOOL=ON. It is a NoOp for GLEW.
	if (trace == Trace::kYes) {
		glbinding::setCallbackMaskExcept(
		   glbinding::CallbackMask::After | glbinding::CallbackMask::ParametersAndReturnValue,
		   {"glGetError"});
		glbinding::setAfterCallback([](const glbinding::FunctionCall& call) {
			log_dbg("%s(", call.function->name());
			for (size_t i = 0; i < call.parameters.size(); ++i) {
				FORMAT_WARNINGS_OFF
				log_dbg("%p", call.parameters[i].get());
				FORMAT_WARNINGS_ON
				if (i < call.parameters.size() - 1)
					log_dbg(", ");
			}
			log_dbg(")");
			if (call.returnValue) {
				FORMAT_WARNINGS_OFF
				log_dbg(" -> %p", call.returnValue.get());
				FORMAT_WARNINGS_ON
			}
			const auto error = glGetError();
			log_dbg(" [%s]\n", gl_error_to_string(error));
			// The next few lines will terminate Widelands if there was any OpenGL
			// error. This is useful for super aggressive debugging, but probably
			// not for regular builds. Comment it in if you need to understand
			// OpenGL problems.
			// if (error != GL_NO_ERROR) {
			// std::raise(SIGINT);
			// }
		});
	}
#endif
#else
	// See graphic/gl/system_headers.h for an explanation of the next line.
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	// LeakSanitizer reports a memory leak which is triggered somewhere above this line, probably
	// coming from the gaphics drivers

	if (err != GLEW_OK) {
		log_err("glewInit returns %i\nYour OpenGL installation must be __very__ broken. %s\n", err,
		        glewGetErrorString(err));
		throw wexception("glewInit returns %i: Broken OpenGL installation.", err);
	}
#endif

	// Show a basic SDL window with an error message, and log it too, then exit 1. Since font support
	// does not exist for all languages, we show both the original and a localized text.
	auto show_opengl_error_and_exit = [](const std::string& message,
	                                     const std::string& localized_message) {
		std::string display_message = message;
		if (message != localized_message) {
			display_message += "\n\n";
			display_message +=
			   (i18n::has_rtl_character(localized_message.c_str()) ?
                i18n::line2bidi(i18n::make_ligatures(localized_message.c_str()).c_str()) :
                localized_message);
		}

		log_err("%s\n", display_message.c_str());
		SDL_ShowSimpleMessageBox(
		   SDL_MESSAGEBOX_ERROR, "OpenGL Error", display_message.c_str(), nullptr);
		exit(1);
	};

	// Exit because we couldn't detect the shading language version, so there must be a problem
	// communicating with the graphics adapter.
	auto handle_unreadable_opengl_shading_language = [show_opengl_error_and_exit]() {
		show_opengl_error_and_exit(
		   "Widelands won't work because we were unable to detect the shading language version.\n"
		   "There is an unknown problem with reading the information from the graphics driver.",
		   format("%s\n%s",
		          /** TRANSLATORS: Basic error message when we can't handle the graphics driver. Font
		             support is limited here, so do not use advanced typography **/
		          _("Widelands won't work because we were unable to detect the shading language "
		            "version."),
		          /** TRANSLATORS: Basic error message when we can't handle the graphics driver. Font
		             support is limited here, so do not use advanced typography **/
		          _("There is an unknown problem with reading the information from the graphics "
		            "driver.")));
	};
	auto handle_unreadable_opengl_version = [show_opengl_error_and_exit]() {
		show_opengl_error_and_exit(
		   "Widelands won't work because we were unable to detect the OpenGL version.\n"
		   "There is an unknown problem with reading the information from the graphics driver.",
		   format("%s\n%s",
		          /** TRANSLATORS: Basic error message when we can't handle the graphics driver. Font
		             support is limited here, so do not use advanced typography **/
		          _("Widelands won't work because we were unable to detect the OpenGL version."),
		          /** TRANSLATORS: Basic error message when we can't handle the graphics driver. Font
		             support is limited here, so do not use advanced typography **/
		          _("There is an unknown problem with reading the information from the graphics "
		            "driver.")));
	};
	auto check_version = [show_opengl_error_and_exit](
	                        const std::string& version_string, const std::string& name,
	                        const std::string& descname, const int required_major_version,
	                        const int required_minor_version, const std::function<void()>& error) {
		std::vector<std::string> version_vector;
		split(version_vector, version_string, {'.', ' '});
		if (version_vector.size() >= 2) {
			int major_version = 0, minor_version = 0;
			try {
				major_version = std::stoi(version_vector[0]);
				minor_version = std::stoi(version_vector[1]);
			} catch (...) {
				error();
			}
			// The version has been detected properly. Exit if the version is too old.
			if (major_version < required_major_version ||
			    (major_version == required_major_version && minor_version < required_minor_version)) {
				show_opengl_error_and_exit(
				   format("Widelands won’t work because your graphics driver is too old.\n"
				          "The %u version needs to be version %u.%u or newer.",
				          name, required_major_version, required_minor_version),
				   format("%s\n%s",
				          /** TRANSLATORS: Basic error message when we can't handle the graphics driver.
				             Font support is limited here, so do not use advanced typography **/
				          _("Widelands won’t work because your graphics driver is too old."),
				          /** TRANSLATORS: Basic error message when we can't handle the graphics driver.
				             Font support is limited here, so do not use advanced typography **/
				          format(_("The %1$u version needs to be version %2$u.%3$u or newer."),
				                 descname, required_major_version, required_minor_version)));
			}
		} else {
			// We don't have a minor version. Ensure that the string to compare is a valid integer
			// before conversion
			std::regex re("\\d+");
			if (std::regex_match(version_string, re)) {
				if (std::stol(version_string) < required_major_version + 1) {
					show_opengl_error_and_exit(
					   format("Widelands won’t work because your graphics driver is too old.\n"
					          "The %s needs to be version %u.%u or newer.",
					          name, required_major_version, required_minor_version),
					   format(
					      "%s\n%s",
					      /** TRANSLATORS: Basic error message when we can't handle the graphics driver.
					         Font support is limited here, so do not use advanced typography **/
					      _("Widelands won’t work because your graphics driver is too old."),
					      /** TRANSLATORS: Basic error message when we can't handle the graphics driver.
					         Font support is limited here, so do not use advanced typography **/
					      format(_("The %1$s needs to be version %2$u.%3$u or newer."), descname,
					             required_major_version, required_minor_version)));
				}
			} else {
				// We don't know how to interpret the version info
				error();
			}
		}
	};

	const char* const opengl_version_string = reinterpret_cast<const char*>(glGetString(GL_VERSION));
	if (opengl_version_string == nullptr) {
		handle_unreadable_opengl_version();
	}
	log_info("Graphics: OpenGL: Version \"%s\"\n", opengl_version_string);
	check_version(
	   opengl_version_string, "OpenGL", _("OpenGL"), 2, 1, handle_unreadable_opengl_version);

#define LOG_SDL_GL_ATTRIBUTE(x)                                                                    \
	{                                                                                               \
		int value;                                                                                   \
		SDL_GL_GetAttribute(x, &value);                                                              \
		log_info("Graphics: %s is %d\n", #x, value);                                                 \
	}

	LOG_SDL_GL_ATTRIBUTE(SDL_GL_RED_SIZE)
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_GREEN_SIZE)
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_BLUE_SIZE)
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_ALPHA_SIZE)
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_BUFFER_SIZE)
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_DOUBLEBUFFER)
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_DEPTH_SIZE)
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_STENCIL_SIZE)
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_ACCUM_RED_SIZE)
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_ACCUM_GREEN_SIZE)
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_ACCUM_BLUE_SIZE)
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_ACCUM_ALPHA_SIZE)
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_STEREO)
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_MULTISAMPLEBUFFERS)
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_MULTISAMPLESAMPLES)
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_ACCELERATED_VISUAL)
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_CONTEXT_MAJOR_VERSION)
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_CONTEXT_MINOR_VERSION)
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_CONTEXT_FLAGS)
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_CONTEXT_PROFILE_MASK)
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_SHARE_WITH_CURRENT_CONTEXT)
	LOG_SDL_GL_ATTRIBUTE(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE)
#undef LOG_SDL_GL_ATTRIBUTE

	GLboolean glBool;
	glGetBooleanv(GL_DOUBLEBUFFER, &glBool);
	log_info(
	   "Graphics: OpenGL: Double buffering %s\n", (glBool == GL_TRUE) ? "enabled" : "disabled");

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, max_texture_size);
	log_info("Graphics: OpenGL: Max texture size: %u\n", *max_texture_size);

	const char* const shading_language_version_string =
	   reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
	if (shading_language_version_string == nullptr) {
		handle_unreadable_opengl_shading_language();
	}
	log_info("Graphics: OpenGL: ShadingLanguage: \"%s\"\n", shading_language_version_string);
	check_version(shading_language_version_string, "Shading Language", _("Shading Language"), 1, 20,
	              handle_unreadable_opengl_shading_language);

	glDrawBuffer(GL_BACK);

	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClear(GL_COLOR_BUFFER_BIT);

	return gl_context;
}

}  // namespace Gl
