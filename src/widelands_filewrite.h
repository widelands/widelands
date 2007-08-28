/*
 * Copyright (C) 2007 by the Widelands Development Team
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

#ifndef WIDELANDS_FILEWRITE_H
#define WIDELANDS_FILEWRITE_H

#include "filewrite.h"
#include "geometry.h"

#include <cassert>

/// A FileWrite that can write Widelands-specific types.
struct WidelandsFileWrite : public FileWrite {

	void  Coords32(const  Coords);

};

inline void WidelandsFileWrite::Coords32(const Coords c) {
	assert(static_cast<Uint16>(c.x) < 0x8000 or c.x == -1);
	assert(static_cast<Uint16>(c.y) < 0x8000 or c.y == -1);
	{const Uint16 x = Little16(c.x); Data(&x, 2);}
	{const Uint16 y = Little16(c.y); Data(&y, 2);}
}

#endif
