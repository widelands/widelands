/*
 * Copyright (C) 2002-2003, 2006, 2009 by the Widelands Development Team
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

#ifndef MACHDEP_H
#define MACHDEP_H

#include <cstring>
#include <string>

#include <SDL_endian.h>
#include <stdint.h>

#include "compile_diagnostics.h"

// Disable this warning for files where we might use these macros.
CLANG_DIAG_OFF("-Wself-assign");

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define Little16(x)    (x)
#define Little32(x)    (x)
#define LittleFloat(x) (x)
#define Big16(x)       Swap16(x)
#define Big32(x)       Swap32(x)
#define BigFloat(x)    SwapFloat(x)
#else
#define Little16(x)    Swap16(x)
#define Little32(x)    Swap32(x)
#define LittleFloat(x) SwapFloat(x)
#define Big16(x)       (x)
#define Big32(x)       (x)
#define BigFloat(x)    (x)
#endif

inline uint16_t Swap16(const uint16_t x) {
	uint16_t s;
	reinterpret_cast<uint8_t *>(&s)[0] =
		reinterpret_cast<const uint8_t *>(&x)[1];
	reinterpret_cast<uint8_t *>(&s)[1] =
		reinterpret_cast<const uint8_t *>(&x)[0];
	return s;
}

inline uint32_t Swap32(const uint32_t x) {
	uint32_t s;
	reinterpret_cast<uint8_t *>(&s)[0] =
		reinterpret_cast<const uint8_t *>(&x)[3];
	reinterpret_cast<uint8_t *>(&s)[1] =
		reinterpret_cast<const uint8_t *>(&x)[2];
	reinterpret_cast<uint8_t *>(&s)[2] =
		reinterpret_cast<const uint8_t *>(&x)[1];
	reinterpret_cast<uint8_t *>(&s)[3] =
		reinterpret_cast<const uint8_t *>(&x)[0];
	return s;
}

inline float SwapFloat(const float x)
{
	float s;
	reinterpret_cast<uint8_t *>(&s)[0] =
		reinterpret_cast<const uint8_t *>(&x)[3];
	reinterpret_cast<uint8_t *>(&s)[1] =
		reinterpret_cast<const uint8_t *>(&x)[2];
	reinterpret_cast<uint8_t *>(&s)[2] =
		reinterpret_cast<const uint8_t *>(&x)[1];
	reinterpret_cast<uint8_t *>(&s)[3] =
		reinterpret_cast<const uint8_t *>(&x)[0];
	return s;
}

inline uint8_t Deref8(void const * const ptr) {
	return *reinterpret_cast<uint8_t const *>(ptr);
}

inline uint16_t Deref16(const void * const ptr) {
	uint16_t r;
	memcpy(&r, ptr, sizeof(r));
	return r;
}

inline uint32_t Deref32(const void * const ptr) {
	uint32_t r;
	memcpy(&r, ptr, sizeof(r));
	return r;
}

inline float DerefFloat(const void * const ptr) {
	float r;
	memcpy(&r, ptr, sizeof(r));
	return r;
}

#endif
