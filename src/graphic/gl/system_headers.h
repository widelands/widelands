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

#ifndef WL_GRAPHIC_GL_SYSTEM_HEADERS_H
#define WL_GRAPHIC_GL_SYSTEM_HEADERS_H

#include "base/macros.h"

// This includes the correct OpenGL headers for us. Use this
// instead of including any system OpenGL headers yourself.

// So, GLEW is really a crappy piece of software, but for now we
// are stuck with it. Before making any changes here, see:
// https://www.opengl.org/wiki/OpenGL_Loading_Library
// and
// https://stackoverflow.com/questions/13558073/program-crash-on-glgenvertexarrays-call.
//
// TODO(sirver): glbinding seems to be a sane solution to the GL
// loading problem. Switch to it everywhere. (https://github.com/hpicgs/glbinding).

#ifdef USE_GLBINDING
#include <glbinding/Binding.h>
#include <glbinding/gl/gl.h>

// testing for the presence of glbinding.h to determine whether we have a glbinding version newer
// then 2.1.4
#ifdef __has_include
#if __has_include("glbinding/glbinding.h")
#include <glbinding/ProcAddress.h>
#include <glbinding/glbinding.h>
#define GLBINDING3
#endif
#endif

// This fakes that most other gl bindings define gl functions in the public namespace.
CLANG_DIAG_OFF("-Wheader-hygiene")
using namespace gl;
CLANG_DIAG_ON("-Wheader-hygiene")
#else
// GLEW must be first. Do not include any other GL headers, it
// should define all functions.
#include <GL/glew.h>
#endif

#endif  // end of include guard: WL_GRAPHIC_GL_SYSTEM_HEADERS_H
