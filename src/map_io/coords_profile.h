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

#ifndef WL_MAP_IO_COORDS_PROFILE_H
#define WL_MAP_IO_COORDS_PROFILE_H

#include "logic/widelands_geometry.h"

#include <string>

class Section;

namespace Widelands {

// Returns the key 'name' from 'section' parsed as Coords. Will throw if the
// value cannot be parsed. If 'name' is not in 'section' will return 'def'.
Widelands::Coords get_coords(const std::string& name,
                             const Widelands::Extent& extent,
                             const Widelands::Coords& def,
                             Section* section);

// Like get_coords, but will also throw when 'name' is not in 'section'.
Widelands::Coords
get_safe_coords(const std::string& name, const Widelands::Extent& extent, Section* section);

// Sets the key 'name' in 'section' to a string representation of 'value'.
void set_coords(const std::string& name, const Widelands::Coords& value, Section* section);

}  // namespace Widelands

#endif  // end of include guard: WL_MAP_IO_COORDS_PROFILE_H
