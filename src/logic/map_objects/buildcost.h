/*
 * Copyright (C) 2010-2022 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_BUILDCOST_H
#define WL_LOGIC_MAP_OBJECTS_BUILDCOST_H

#include <map>
#include <memory>

#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/widelands.h"
#include "scripting/lua_table.h"

namespace Widelands {

class TribeDescr;
class Descriptions;

struct Buildcost : std::map<DescriptionIndex, uint8_t> {

	Buildcost() = default;
	Buildcost(std::unique_ptr<LuaTable> table, Widelands::Descriptions& descriptions);

	Quantity total() const;

	void save(FileWrite& fw, const TribeDescr& tribe) const;
	void load(FileRead& fr, const TribeDescr& tribe);
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_BUILDCOST_H
