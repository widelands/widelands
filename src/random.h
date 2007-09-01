/*
 * Copyright (C) 2004 by the Wide Lands Development Team
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

#ifndef __RANDOM_H__
#define __RANDOM_H__

#include "types.h"

extern const uint rng_sbox[256];

class StreamRead;
class StreamWrite;

struct RNG {
	RNG ();

	void seed (uint);

	uint rand ();

	void ReadState(StreamRead&);
	void WriteState(StreamWrite&);

private:
	uint state0;
	uint state1;
};

#define SIMPLE_RAND(x) (((x) >> 8) ^ rng_sbox[(x) & 0xff])

#endif
