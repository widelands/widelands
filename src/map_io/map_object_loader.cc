/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#include "map_io/map_object_loader.h"

#include "base/wexception.h"
#include "logic/editor_game_base.h"
#include "logic/map_objects/map_object.h"

namespace Widelands {

/*
 * Returns true if this object has already been inserted
 */
bool MapObjectLoader::is_object_known(Serial const n) const {
	return objects_.count(n) == 1;
}

/*
 * mark this object as saved
 */
void MapObjectLoader::mark_object_as_loaded(MapObject& obj) {
	loaded_objects_[&obj] = true;
}

/*
 * Return the number of unsaved objects
 */
int32_t MapObjectLoader::get_nr_unloaded_objects() {
	int32_t result = 0;
	std::map<MapObject*, bool>::const_iterator const loaded_obj_end = loaded_objects_.end();
	for (std::map<MapObject*, bool>::const_iterator it = loaded_objects_.begin();
	     it != loaded_obj_end; ++it)
		if (!it->second)
			++result;
	return result;
}

/**
 * Mark this object to be scheduled for destruction after loading has finished.
 *
 * \note Only use this for compatibility hacks!
 */
void MapObjectLoader::schedule_destroy(MapObject& obj) {
	schedule_destroy_.push_back(&obj);
}

/**
 * Mark this bob to have a forced act scheduled, so that it can refresh itself
 * and update states properly.
 *
 * \note Only use this for compatibility hacks!
 */
void MapObjectLoader::schedule_act(Bob& bob) {
	schedule_act_.push_back(&bob);
}

/**
 * Part of compatibility hacks that need to be run after the load has finished.
 *
 * \note Only use this for compatibility hacks!
 */
void MapObjectLoader::load_finish_game(Game& g) {
	while (!schedule_destroy_.empty()) {
		schedule_destroy_.back()->schedule_destroy(g);
		schedule_destroy_.pop_back();
	}

	while (!schedule_act_.empty()) {
		schedule_act_.back()->schedule_act(g, 1);
		schedule_act_.pop_back();
	}
}

Serial MapObjectLoader::get_economy_savegame_compatibility(Serial ware_economy) {
	const auto it = economy_savegame_compatibility_.find(ware_economy);
	if (it == economy_savegame_compatibility_.end()) {
		// Since we don't now know how many economies there are left to load,
		// we choose the highest possible serial for the new economy.
		// That way it is highly unlikely that the running serial for new economies
		// will collide with this savegame hack.
		const Serial worker_economy = kInvalidSerial - 1 - economy_savegame_compatibility_.size();
		log("Savegame compatibility: Splitting economy %u into ware economy %u and worker economy "
		    "%u\n",
		    ware_economy, ware_economy, worker_economy);
		economy_savegame_compatibility_[ware_economy] = worker_economy;
		return worker_economy;
	}
	return it->second;
}

Serial MapObjectLoader::get_existing_economy_savegame_compatibility(Serial ware_economy) const {
	const auto it = economy_savegame_compatibility_.find(ware_economy);
	if (it == economy_savegame_compatibility_.end()) {
		throw GameDataError("MapObjectLoader::get_existing_economy_savegame_compatibility: "
		                    "no matching worker economy found for ware economy %u",
		                    ware_economy);
	}
	return it->second;
}

}  // namespace Widelands
