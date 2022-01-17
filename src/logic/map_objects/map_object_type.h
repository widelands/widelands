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

#ifndef WL_LOGIC_MAP_OBJECTS_MAP_OBJECT_TYPE_H
#define WL_LOGIC_MAP_OBJECTS_MAP_OBJECT_TYPE_H

#include <string>

#include "notifications/note_ids.h"
#include "notifications/notifications.h"

namespace Widelands {

// This enum lists the available classes of Map Objects.
enum class MapObjectType : uint8_t {
	MAPOBJECT = 0,  // Root superclass

	RESOURCE,
	TERRAIN,

	WARE,  //  class WareInstance
	BATTLE,
	SHIP_FLEET,
	FERRY_FLEET,

	BOB = 10,  // Bob
	CRITTER,   // Bob -- Critter
	SHIP,      // Bob -- Ship
	WORKER,    // Bob -- Worker
	SOLDIER,   // Bob -- Worker -- Soldier
	CARRIER,   // Bob -- Worker -- Carrier
	FERRY,     // Bob -- Worker -- Carrier -- Ferry

	// everything below is at least a BaseImmovable
	IMMOVABLE = 30,

	// everything below is at least a PlayerImmovable
	FLAG = 40,  // Flag
	PORTDOCK,   // Portdock
	ROADBASE,   // Roadbase
	ROAD,       // Roadbase -- Road
	WATERWAY,   // Roadbase -- Waterway

	// everything below is at least a Building
	BUILDING = 100,    // Building
	CONSTRUCTIONSITE,  // Building -- PartiallyFinishedBuilding -- Constructionsite
	DISMANTLESITE,     // Building -- PartiallyFinishedBuilding -- Dismantlesite
	WAREHOUSE,         // Building -- Warehouse
	MARKET,            // Building -- Market
	MILITARYSITE,      // Building -- Militarysite
	PRODUCTIONSITE,    // Building -- Productionsite
	TRAININGSITE       // Building -- Productionsite -- Trainingsite
};

// Returns a string representation for 'type'.
std::string to_string(MapObjectType type);

struct NoteMapObjectDescription {
	CAN_BE_SENT_AS_NOTE(NoteId::MapObjectDescription)

	enum class LoadType {
		// Load a registered map object if it's not being loaded yet
		kObject,
		// Load all registered map objects that have this attribute and that aren't being loaded yet
		kAttribute
	};

	const std::string name;
	const LoadType type;
	const bool allow_failure;

	NoteMapObjectDescription(const std::string& init_name,
	                         LoadType init_type,
	                         bool init_allow_failure = false)
	   : name(init_name), type(init_type), allow_failure(init_allow_failure) {
	}
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_MAP_OBJECT_TYPE_H
