/*
 * Copyright (C) 2004, 2008-2009 by the Widelands Development Team
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

#ifndef RANDOM_H
#define RANDOM_H

#include <stdint.h>

extern const uint32_t rng_sbox[256];

class StreamRead;
class StreamWrite;

struct RNG {
	RNG ();

	void seed (uint32_t);

	uint32_t rand ();

	void ReadState(StreamRead &);
	void WriteState(StreamWrite &);

private:
	uint32_t state0;
	uint32_t state1;
};

#define SIMPLE_RAND(x) (((x) >> 8) ^ rng_sbox[(x) & 0xff])

#endif
