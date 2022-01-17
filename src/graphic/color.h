/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 *
 */

#ifndef WL_GRAPHIC_COLOR_H
#define WL_GRAPHIC_COLOR_H

#include <string>

#include <SDL_pixels.h>

struct RGBColor {
	RGBColor(uint8_t R, uint8_t G, uint8_t B);
	RGBColor(const RGBColor& other) = default;
	explicit RGBColor(uint32_t);

	// Initializes the color to black.
	RGBColor();

	// Returns this color in hex format.
	std::string hex_value() const;

	// Map this color to the given 'fmt'
	uint32_t map(const SDL_PixelFormat& fmt) const;

	// Set it to the given 'clr' which is interpretes through 'fmt'.
	void set(SDL_PixelFormat* fmt, uint32_t clr);

	RGBColor& operator=(const RGBColor& other) = default;
	bool operator!=(const RGBColor& other) const;
	bool operator==(const RGBColor& other) const;

	uint8_t r, g, b;
};

struct RGBAColor {
	RGBAColor(uint8_t R, uint8_t G, uint8_t B, uint8_t A);
	explicit RGBAColor(uint32_t);
	RGBAColor(const RGBAColor& other) = default;

	// Initializes the color to black.
	RGBAColor();

	// Initializes to opaque color.
	RGBAColor(const RGBColor& c);

	// Returns this color in hex format.
	std::string hex_value() const;

	// Map this color to the given 'fmt'
	uint32_t map(const SDL_PixelFormat& fmt) const;

	// Set it to the given 'clr' which is interpretes through 'fmt'.
	void set(const SDL_PixelFormat& fmt, uint32_t clr);

	RGBAColor& operator=(const RGBAColor& other) = default;
	bool operator!=(const RGBAColor& other) const;
	bool operator==(const RGBAColor& other) const;

	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

#endif  // end of include guard: WL_GRAPHIC_COLOR_H
