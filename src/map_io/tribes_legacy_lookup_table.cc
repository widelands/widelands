/*
 * Copyright (C) 2006-2019 by the Widelands Development Team
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

#include "map_io/tribes_legacy_lookup_table.h"
// Whenever we break savegame compatibility, we can empty these maps
// NOCOM(GunChleoc): Since we now have a unified tribe, no need to have the tribe as key for fetching
TribesLegacyLookupTable::TribesLegacyLookupTable()
   :  // Workers
     workers_{
        std::make_pair("atlanteans",
                       std::map<std::string, std::string>{
                       }),
        std::make_pair("barbarians",
                       std::map<std::string, std::string>{
                       }),
        std::make_pair("empire",
                       std::map<std::string, std::string>{
                       }),
     },
     // Wares
     wares_{
        std::make_pair("atlanteans",
                       std::map<std::string, std::string>{
                       }),
        std::make_pair("barbarians",
                       std::map<std::string, std::string>{
                       }),
        std::make_pair("empire",
                       std::map<std::string, std::string>{
                       }),
     },
     // Immovables
     immovables_{
        std::make_pair("atlanteans",
                       std::map<std::string, std::string>{
                       }),
        std::make_pair("barbarians",
                       std::map<std::string, std::string>{
                          {"field_medium", "wheatfield_medium"},
                          {"field_small", "wheatfield_small"},
                          {"field_tiny", "wheatfield_tiny"},
                          {"field_ripe", "wheatfield_ripe"},
                          {"field_harvested", "wheatfield_harvested"},
                          {"reed_medium", "reed_medium"},
                          {"reed_small", "reedfield_small"},
                          {"reed_tiny", "reedfield_tiny"},
                          {"reed_tiny", "reedfield_ripe"},
                       }),
        std::make_pair("empire",
                       std::map<std::string, std::string>{
                          {"field_medium", "wheatfield_medium"},
                          {"field_small", "wheatfield_small"},
                          {"field_tiny", "wheatfield_tiny"},
                          {"field_ripe", "wheatfield_ripe"},
                          {"field_harvested", "wheatfield_harvested"},
                       }),
        std::make_pair("frisians",
                       std::map<std::string, std::string>{
                       }),
     } {
}

const std::string& TribesLegacyLookupTable::lookup_worker(const std::string& tribe,
                                                          const std::string& worker) const {
	if (workers_.count(tribe)) {
		const std::map<std::string, std::string>& tribe_workers = workers_.at(tribe);
		const auto& i = tribe_workers.find(worker);
		if (i != tribe_workers.end()) {
			return i->second;
		}
	}
	return worker;
}

const std::string& TribesLegacyLookupTable::lookup_ware(const std::string& tribe,
                                                        const std::string& ware) const {
	if (wares_.count(tribe)) {
		const std::map<std::string, std::string>& tribe_wares = wares_.at(tribe);
		const auto& i = tribe_wares.find(ware);
		if (i != tribe_wares.end()) {
			return i->second;
		}
	}
	return ware;
}

const std::string& TribesLegacyLookupTable::lookup_immovable(const std::string& tribe,
                                                             const std::string& immovable) const {
	if (immovables_.count(tribe)) {
		const std::map<std::string, std::string>& tribe_immovables = immovables_.at(tribe);
		const auto& i = tribe_immovables.find(immovable);
		if (i != tribe_immovables.end()) {
			return i->second;
		}
	}
	return immovable;
}
