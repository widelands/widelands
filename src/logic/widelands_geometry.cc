/*
 * Copyright (C) 2006-2024 by the Widelands Development Team
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

#include "logic/widelands_geometry.h"

namespace Widelands {

void Coords::reorigin(Coords new_origin, const Extent& extent) {
	if (valid()) {
		if (y < new_origin.y) {
			y += extent.h;
		}
		y -= new_origin.y;
		if (((y & 1) != 0) && ((new_origin.y & 1) != 0) && ++new_origin.x == extent.w) {
			new_origin.x = 0;
		}
		if (x < new_origin.x) {
			x += extent.w;
		}
		x -= new_origin.x;
	}
}

// Hash coordinates to use them as keys in a container
uint32_t Coords::hash() const {
	return x << 16 | y;
}

// Unhash coordinates so they can be gotten from a container
Coords Coords::unhash(uint32_t hash) {
	Coords coords;
	coords.x = hash >> 16;  // is cast needed here???
	coords.y = hash;
	return coords;
}

}  // namespace Widelands
