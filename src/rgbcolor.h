/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#ifndef RGBCOLOR_H
#define RGBCOLOR_H

#include <SDL.h>

struct RGBColor : protected SDL_Color {
	RGBColor() {}
	RGBColor(const Uint8 R, const Uint8 G, const Uint8 B) throw ()
	{SDL_Color::r = R, SDL_Color::g = G, SDL_Color::b = B;}

	Uint8 r() const throw () {return SDL_Color::r;}
	Uint8 g() const throw () {return SDL_Color::g;}
	Uint8 b() const throw () {return SDL_Color::b;}

	Uint32 map(const SDL_PixelFormat & fmt) const throw ()
	{return SDL_MapRGB(&const_cast<SDL_PixelFormat &>(fmt), r(), g(), b());}
	void set(SDL_PixelFormat * const fmt, Uint32 clr) throw ()
	{SDL_GetRGB(clr, fmt, &(SDL_Color::r), &(SDL_Color::g), &(SDL_Color::b));}

	bool operator== (RGBColor const & other) const throw () {
		return r() == other.r() and g() == other.g() and b() == other.b();
	}
};

struct RGBAColor {
	// Colors are very straightforward; there's no need to encapsulate them further.
	Uint8 r;
	Uint8 g;
	Uint8 b;
	Uint8 a;

	RGBAColor() {}
	RGBAColor(Uint8 _r, Uint8 _g, Uint8 _b, Uint8 _a) {
		r = _r;
		g = _g;
		b = _b;
		a = _a;
	}

	Uint32 map(const SDL_PixelFormat & fmt) const {
		return SDL_MapRGBA(&const_cast<SDL_PixelFormat &>(fmt), r, g, b, a);
	}
	void set(SDL_PixelFormat const & fmt, Uint32 const clr) {
		SDL_GetRGBA(clr, const_cast<SDL_PixelFormat *>(&fmt), &r, &g, &b, &a);
	}
};

#endif
