/*
 * Copyright (C) 2002, 2003 by the Widelands Development Team
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

#include <string>
#include <SDL_endian.h>
#include "types.h"

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define Little16(x)		(x)
#define Little32(x)		(x)
#define LittleFloat(x)		(x)
#define Big16(x)		Swap16((x))
#define Big32(x)		Swap32((x))
#define BigFloat(x)		SwapFloat((x))
#else
#define Little16(x)		Swap16((x))
#define Little32(x)		Swap32((x))
#define LittleFloat(x)		SwapFloat((x))
#define Big16(x)		(x)
#define Big32(x)		(x)
#define BigFloat(x)		(x)
#endif

inline short Swap16(short x) {
	short s;
	((uchar *)&s)[0] = ((uchar *)&x)[1];
	((uchar *)&s)[1] = ((uchar *)&x)[0];
	return s;
}

inline int Swap32(int x) {
	int s;
	((uchar *)&s)[0] = ((uchar *)&x)[3];
	((uchar *)&s)[1] = ((uchar *)&x)[2];
	((uchar *)&s)[2] = ((uchar *)&x)[1];
	((uchar *)&s)[3] = ((uchar *)&x)[0];
	return s;
}

inline float SwapFloat(float x)
{
	float s;
	((uchar *)&s)[0] = ((uchar *)&x)[3];
	((uchar *)&s)[1] = ((uchar *)&x)[2];
	((uchar *)&s)[2] = ((uchar *)&x)[1];
	((uchar *)&s)[3] = ((uchar *)&x)[0];
	return s;
}

inline char Deref8(const void* ptr)
{
	return *reinterpret_cast<const char*>(ptr);
}

inline short Deref16(const void* ptr)
{
	short r;
	memcpy(&r, ptr, sizeof(r));
	return r;
}

inline int Deref32(const void* ptr)
{
	int r;
	memcpy(&r, ptr, sizeof(r));
	return r;
}

inline float DerefFloat(const void* ptr)
{
	float r;
	memcpy(&r, ptr, sizeof(r));
	return r;
}

#endif
