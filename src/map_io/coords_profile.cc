/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

#include "map_io/coords_profile.h"

#include <cstdlib>

#include "base/wexception.h"
#include "io/profile.h"
#include "logic/widelands_geometry.h"

namespace Widelands {

namespace {

Coords parse_coords(const std::string& name, const char* const coords, const Extent& extent) {
	char* endp = const_cast<char*>(coords);
	const int64_t x = strtol(endp, &endp, 0);
	const int64_t y = strtol(endp, &endp, 0);

	//  Check of consistence should NOT be at x, y < 0 as (-1, -1) == Coords::null() is used for
	//  not set starting positions in the editor. So check whether x, y < -1 so
	//  the editor can load incomplete maps. For games the starting positions
	//  will be checked in player initalisation anyway.
	if (((x < 0 || extent.w <= x || y < 0 || extent.h <= y) && (x != -1 || y != -1)) ||
	    (*endp != 0)) {
		throw wexception("%s: \"%s\" is not a Coords on a map with size (%u, %u)", name.c_str(),
		                 coords, extent.w, extent.h);
	}
	return Coords(x, y);
}

}  // namespace

void set_coords(const std::string& name, const Coords& value, Section* section) {
	char buffer[sizeof("-32769 -32769")];
	sprintf(buffer, "%i %i", value.x, value.y);
	section->set_string(name.c_str(), buffer);
}

Coords
get_coords(const std::string& name, const Extent& extent, const Coords& def, Section* section) {
	const char* const v = section->get_string(name.c_str());
	if (v == nullptr) {
		return def;
	}

	return parse_coords(name, v, extent);
}

Coords get_safe_coords(const std::string& name, const Extent& extent, Section* section) {
	return parse_coords(name, section->get_safe_string(name), extent);
}

}  // namespace Widelands
