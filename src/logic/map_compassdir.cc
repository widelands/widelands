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

#include "logic/map_compassdir.h"

#include <cassert>
#include <cstdlib>

namespace Widelands {

CompassDir
get_compass_dir(const Coords& from, const Coords& to, const int map_width, const int map_height) {
	assert(to.x >= 0 && to.x < map_width);
	assert(to.y >= 0 && to.y < map_height);
	assert(from.x >= 0 && from.x < map_width);
	assert(from.y >= 0 && from.y < map_height);

	Coords rel(to.x - from.x, to.y - from.y);

	/*** Normalize relative coordinates ***/
	// Handle wrap-around
	if (rel.x < -map_width / 2) {
		rel.x += map_width;
	} else if (rel.x > map_width / 2) {
		rel.x -= map_width;
	}
	if (rel.y < -map_height / 2) {
		rel.y += map_height;
	} else if (rel.y > map_height / 2) {
		rel.y -= map_height;
	}

	// Handle zig-zagging of even/odd lines (see Map class)
	if ((rel.y & 2) != 0) {
		rel.y *= 2;
		rel.x = rel.x * 2 + ((from.y & 2) != 0 ? -1 : 1);
	}

	/*** Check direction ***/
	if (rel.x == 0 && rel.y == 0) {
		return CompassDir::kInvalid;
	}

	const int abs_x = std::abs(rel.x);
	const int abs_y = std::abs(rel.y);

	// Cardinal directions
	// Limits are actually 3:1 and 1:3 screen length, because 1 triangle width (x) on screen
	// is 2 triangle heights (y)
	if (abs_y >= 6 * abs_x) {
		return rel.y > 0 ? CompassDir::kSouth : CompassDir::kNorth;
	}
	if (3 * abs_y <= 2 * abs_x) {
		return rel.x > 0 ? CompassDir::kEast : CompassDir::kWest;
	}

	// Ordinal directions
	if (rel.y > 0) {
		return rel.x > 0 ? CompassDir::kSouthEast : CompassDir::kSouthWest;
	}  // else
	return rel.x > 0 ? CompassDir::kNorthEast : CompassDir::kNorthWest;
}

}  // namespace Widelands
