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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "graphic/color.h"

#include "base/string.h"

RGBColor::RGBColor() : RGBColor(0, 0, 0) {
}

RGBColor::RGBColor(uint8_t const R, uint8_t const G, uint8_t const B) : r(R), g(G), b(B) {
}

RGBColor::RGBColor(const uint32_t hex)
   : r((hex & 0xff0000) >> 16), g((hex & 0xff00) >> 8), b((hex & 0xff)) {
}

std::string RGBColor::hex_value() const {
	return format("%02x%02x%02x", int(r), int(g), int(b));
}

uint32_t RGBColor::map(const SDL_PixelFormat& fmt) const {
	return SDL_MapRGB(&const_cast<SDL_PixelFormat&>(fmt), r, g, b);
}

void RGBColor::set(SDL_PixelFormat* const fmt, uint32_t const clr) {
	SDL_GetRGB(clr, fmt, &r, &g, &b);
}

bool RGBColor::operator==(const RGBColor& other) const {
	return r == other.r && g == other.g && b == other.b;
}
bool RGBColor::operator!=(const RGBColor& other) const {
	return !(*this == other);
}

RGBAColor::RGBAColor(uint8_t R, uint8_t G, uint8_t B, uint8_t A) : r(R), g(G), b(B), a(A) {
}

RGBAColor::RGBAColor() : RGBAColor(0, 0, 0, 0) {
}

RGBAColor::RGBAColor(const RGBColor& c) {
	r = c.r;
	g = c.g;
	b = c.b;
	a = 255;
}

RGBAColor::RGBAColor(uint32_t hex)
   : r((hex & 0xff0000) >> 16),
     g((hex & 0xff00) >> 8),
     b((hex & 0xff)),
     a((hex & 0xff000000) >> 24) {
}

std::string RGBAColor::hex_value() const {
	return format("%02x%02x%02x%02x>", int(r), int(g), int(b), int(a));
}

uint32_t RGBAColor::map(const SDL_PixelFormat& fmt) const {
	return SDL_MapRGBA(&const_cast<SDL_PixelFormat&>(fmt), r, g, b, a);
}

void RGBAColor::set(const SDL_PixelFormat& fmt, const uint32_t clr) {
	SDL_GetRGBA(clr, const_cast<SDL_PixelFormat*>(&fmt), &r, &g, &b, &a);
}

bool RGBAColor::operator==(const RGBAColor& other) const {
	return (r == other.r && g == other.g && b == other.b && a == other.a);
}
bool RGBAColor::operator!=(const RGBAColor& other) const {
	return !(*this == other);
}
