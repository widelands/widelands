/*
 * Copyright (C) 2002-2003, 2006 by the Widelands Development Team
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

#ifndef MACHDEP_H
#define MACHDEP_H

#include <SDL_endian.h>

#include <stdint.h>
#include <string>

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

inline Uint16 Swap16(const Uint16 x) {
	Uint16 s;
	reinterpret_cast<Uint8 *>(&s)[0] =
		reinterpret_cast<const Uint8 *>(&x)[1];
	reinterpret_cast<Uint8 *>(&s)[1] =
		reinterpret_cast<const Uint8 *>(&x)[0];
	return s;
}

inline Uint32 Swap32(const Uint32 x) {
	Uint32 s;
	reinterpret_cast<Uint8 *>(&s)[0] =
		reinterpret_cast<const Uint8 *>(&x)[3];
	reinterpret_cast<Uint8 *>(&s)[1] =
		reinterpret_cast<const Uint8 *>(&x)[2];
	reinterpret_cast<Uint8 *>(&s)[2] =
		reinterpret_cast<const Uint8 *>(&x)[1];
	reinterpret_cast<Uint8 *>(&s)[3] =
		reinterpret_cast<const Uint8 *>(&x)[0];
	return s;
}

inline float SwapFloat(const float x)
{
	float s;
	reinterpret_cast<Uint8 *>(&s)[0] =
		reinterpret_cast<const Uint8 *>(&x)[3];
	reinterpret_cast<uint8_t *>(&s)[1] =
		reinterpret_cast<const Uint8 *>(&x)[2];
	reinterpret_cast<uint8_t *>(&s)[2] =
		reinterpret_cast<const Uint8 *>(&x)[1];
	reinterpret_cast<Uint8 *>(&s)[3] =
		reinterpret_cast<const Uint8 *>(&x)[0];
	return s;
}

inline Uint8 Deref8(const void * const ptr)
{return *reinterpret_cast<const Uint8 *>(ptr);}

inline Uint16 Deref16(const void * const ptr) {
	Uint16 r;
	memcpy(&r, ptr, sizeof(r));
	return r;
}

inline Uint32 Deref32(const void * const ptr) {
	Uint32 r;
	memcpy(&r, ptr, sizeof(r));
	return r;
}

inline float DerefFloat(const void * const ptr) {
	float r;
	memcpy(&r, ptr, sizeof(r));
	return r;
}

#endif
