/*
 * Copyright (C) 2006-2013 by the Widelands Development Team
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

#include "graphic/surface.h"

#include <SDL.h>

#include "graphic/render/gl_surface_texture.h"
#include "graphic/render/sdl_helper.h"
#include "graphic/render/sdl_surface.h"

extern bool g_opengl;

namespace {

bool s_is_diplay_format_defined = false;

SDL_Surface* maybe_convert_to_diplay_format(SDL_Surface* surface) {
	if (!s_is_diplay_format_defined) {
		return surface;
	}
	SDL_Surface * converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ARGB8888, 0);
	SDL_FreeSurface(surface);
	return converted;
}

} // namespace

void Surface::display_format_is_now_defined() {
	s_is_diplay_format_defined = true;
}

Surface* Surface::create(SDL_Surface* surf) {
	if (g_opengl) {
		return new GLSurfaceTexture(surf);
	}
	return new SDLSurface(maybe_convert_to_diplay_format(surf));
}

Surface* Surface::create(uint16_t w, uint16_t h) {
	if (g_opengl) {
		return new GLSurfaceTexture(w, h);
	}
	return new SDLSurface(empty_sdl_surface(w, h));
}
