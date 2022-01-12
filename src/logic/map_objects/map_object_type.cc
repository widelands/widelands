/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "logic/map_objects/map_object_type.h"

#include "base/wexception.h"

namespace Widelands {

std::string to_string(const MapObjectType type) {
	// The types are documented in scripting/lua_map.cc -> LuaMapObjectDescription::get_type_name for
	// the Lua interface, so make sure to change the documentation there when changing anything in
	// this function.
	switch (type) {
	case MapObjectType::RESOURCE:
		return "resource";
	case MapObjectType::TERRAIN:
		return "terrain";
	case MapObjectType::BOB:
		return "bob";
	case MapObjectType::CRITTER:
		return "critter";
	case MapObjectType::SHIP:
		return "ship";
	case MapObjectType::WORKER:
		return "worker";
	case MapObjectType::CARRIER:
		return "carrier";
	case MapObjectType::FERRY:
		return "ferry";
	case MapObjectType::SOLDIER:
		return "soldier";
	case MapObjectType::WARE:
		return "ware";
	case MapObjectType::BATTLE:
		return "battle";
	case MapObjectType::SHIP_FLEET:
		return "ship_fleet";
	case MapObjectType::FERRY_FLEET:
		return "ferry_fleet";
	case MapObjectType::IMMOVABLE:
		return "immovable";
	case MapObjectType::FLAG:
		return "flag";
	case MapObjectType::ROAD:
		return "road";
	case MapObjectType::WATERWAY:
		return "waterway";
	case MapObjectType::ROADBASE:
		return "roadbase";
	case MapObjectType::PORTDOCK:
		return "portdock";
	case MapObjectType::BUILDING:
		return "building";
	case MapObjectType::CONSTRUCTIONSITE:
		return "constructionsite";
	case MapObjectType::DISMANTLESITE:
		return "dismantlesite";
	case MapObjectType::WAREHOUSE:
		return "warehouse";
	case MapObjectType::MARKET:
		return "market";
	case MapObjectType::PRODUCTIONSITE:
		return "productionsite";
	case MapObjectType::MILITARYSITE:
		return "militarysite";
	case MapObjectType::TRAININGSITE:
		return "trainingsite";
	case MapObjectType::MAPOBJECT:
		throw wexception("Unknown MapObjectType %d.", static_cast<int>(type));
	}
	NEVER_HERE();
}
}  // namespace Widelands
