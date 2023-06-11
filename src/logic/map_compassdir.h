/*
 * Copyright (C) 2023 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_COMPASSDIR_H
#define WL_LOGIC_MAP_COMPASSDIR_H

#include "logic/widelands_geometry.h"

namespace Widelands {

// Constants for compass directions.
// Unlike walking directions, this has North and South as well.
enum class CompassDir {
	kInvalid = 0,
	kNorth = 1,
	kNorthEast = 2,
	kEast = 3,
	kSouthEast = 4,
	kSouth = 5,
	kSouthWest = 6,
	kWest = 7,
	kNorthWest = 8
};

// The apparent nearest compass direction
// (not mathematically correct, but what looks right on the game map)
CompassDir get_compass_dir(const Coords& from, const Coords& to, int map_width, int map_height);

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_COMPASSDIR_H
