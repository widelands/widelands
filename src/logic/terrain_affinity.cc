/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#include "logic/terrain_affinity.h"

#include "scripting/lua_table.h"

namespace Widelands {

TerrainAffinity::TerrainAffinity(const LuaTable& table, const std::string& immovable_name)
   : preferred_temperature_(table.get_double("preferred_temperature")),
     preferred_humidity_(table.get_double("preferred_humidity")),
     preferred_fertility_(table.get_double("preferred_fertility")),
     pickiness_(table.get_double("pickiness")) {
	if (!(0 <= preferred_fertility_ && preferred_fertility_ <= 1.)) {
		throw game_data_error("%s: preferred_fertility is not in [0, 1].", immovable_name.c_str());
	}
	if (!(0 <= preferred_humidity_ && preferred_humidity_ <= 1.)) {
		throw game_data_error("%s: preferred_humidity is not in [0, 1].", immovable_name.c_str());
	}
	if (preferred_temperature_ < 0) {
		throw game_data_error("%s: preferred_temperature is not in Kelvin.", immovable_name.c_str());
	}
	// NOCOM(#sirver): bound check pickiness
}

double TerrainAffinity::preferred_temperature() const {
	return preferred_temperature_;
}

double TerrainAffinity::preferred_fertility() const {
	return preferred_fertility_;
}

double TerrainAffinity::preferred_humidity() const {
	return preferred_humidity_;
}

double TerrainAffinity::pickiness() const {
	return pickiness_;
}

}  // namespace Widelands
