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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_IO_MACHDEP_H
#define WL_IO_MACHDEP_H

#include <SDL_endian.h>

#include "base/macros.h"

// Disable this warning for files where we might use these macros.
CLANG_DIAG_OFF("-Wself-assign")

#if !defined(SDL_BYTEORDER) || SDL_BYTEORDER == SDL_LIL_ENDIAN
#define little_16(x) (x)
#define little_32(x) (x)
#define little_float(x) (x)
#else
#define little_16(x) swap_16(x)
#define little_32(x) swap_32(x)
#define little_float(x) swap_float(x)
#endif

inline uint16_t swap_16(const uint16_t x) {
	uint16_t s;
	reinterpret_cast<uint8_t*>(&s)[0] = reinterpret_cast<const uint8_t*>(&x)[1];
	reinterpret_cast<uint8_t*>(&s)[1] = reinterpret_cast<const uint8_t*>(&x)[0];
	return s;
}

inline uint32_t swap_32(const uint32_t x) {
	uint32_t s;
	reinterpret_cast<uint8_t*>(&s)[0] = reinterpret_cast<const uint8_t*>(&x)[3];
	reinterpret_cast<uint8_t*>(&s)[1] = reinterpret_cast<const uint8_t*>(&x)[2];
	reinterpret_cast<uint8_t*>(&s)[2] = reinterpret_cast<const uint8_t*>(&x)[1];
	reinterpret_cast<uint8_t*>(&s)[3] = reinterpret_cast<const uint8_t*>(&x)[0];
	return s;
}

inline float swap_float(const float x) {
	float s;
	reinterpret_cast<uint8_t*>(&s)[0] = reinterpret_cast<const uint8_t*>(&x)[3];
	reinterpret_cast<uint8_t*>(&s)[1] = reinterpret_cast<const uint8_t*>(&x)[2];
	reinterpret_cast<uint8_t*>(&s)[2] = reinterpret_cast<const uint8_t*>(&x)[1];
	reinterpret_cast<uint8_t*>(&s)[3] = reinterpret_cast<const uint8_t*>(&x)[0];
	return s;
}

inline uint8_t deref_8(void const* const ptr) {
	return *reinterpret_cast<uint8_t const*>(ptr);
}

inline uint16_t deref_16(const void* const ptr) {
	uint16_t r;
	memcpy(&r, ptr, sizeof(r));
	return r;
}

inline uint32_t deref_32(const void* const ptr) {
	uint32_t r;
	memcpy(&r, ptr, sizeof(r));
	return r;
}

inline float deref_float(const void* const ptr) {
	float r;
	memcpy(&r, ptr, sizeof(r));
	return r;
}

#endif  // end of include guard:
