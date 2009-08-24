/*
 * Copyright (C) 2007-2009 by the Widelands Development Team
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

#ifndef WIDELANDS_STREAMWRITE_H
#define WIDELANDS_STREAMWRITE_H

#include "io/streamwrite.h"

#include "widelands.h"
#include "widelands_geometry.h"

#include <cassert>

namespace Widelands {

struct Building_Descr;
struct Immovable_Descr;
struct Tribe_Descr;

/// A StreamWrite that can write Widelands-specific types. It has no extra data
/// members. Therefore it is binary compatible with StreamWrite, so any
/// ::StreamWrite can be used as a Widelands::StreamWrite to read
/// Widelands-specific types.
struct StreamWrite : public ::StreamWrite {
	void Direction8           (Direction);
	void Direction8_allow_null(Direction);
	void Map_Index32(Map_Index const i) {Unsigned32(i);}
	void Coords32      (Coords);
	void Area48        (Area<Coords, uint16_t>);
	void Player_Number8(const Player_Number pn) {Unsigned8(pn);}
	void Tribe         (Tribe_Descr     const &);
	void Tribe         (Tribe_Descr     const *);
	void Immovable_Type(Immovable_Descr const &);
	void Building_Type (Building_Descr  const &);
};

inline void StreamWrite::Direction8           (Direction const d) {
	assert(0 < d);
	assert    (d <= 6);
	Data(&d, 1);
}

inline void StreamWrite::Direction8_allow_null(Direction const d) {
	assert    (d <= 6);
	Data(&d, 1);
}

inline void StreamWrite::Coords32(const Coords c) {
	assert(static_cast<uint16_t>(c.x) < 0x8000 or c.x == -1);
	assert(static_cast<uint16_t>(c.y) < 0x8000 or c.y == -1);
	{uint16_t const x = Little16(c.x); Data(&x, 2);}
	{uint16_t const y = Little16(c.y); Data(&y, 2);}
}

inline void StreamWrite::Area48(Area<Coords, uint16_t> const area) {
	Coords32  (area);
	Unsigned16(area.radius);
}

};

#endif
