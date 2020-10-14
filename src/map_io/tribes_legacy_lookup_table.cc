/*
 * Copyright (C) 2006-2020 by the Widelands Development Team
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
TribesLegacyLookupTable::TribesLegacyLookupTable()
   :  // Workers
     workers_{},
     // Wares
     wares_{
        // {"old_name", "new_name"},
     },
     // Immovables
     immovables_{},
     // Buildings
     buildings_{},
     // Ships
     ships_{},
     // Working Programs
     programs_{} {
}

const std::string& TribesLegacyLookupTable::lookup_worker(const std::string& worker) const {
	return lookup_entry(worker, workers_);
}

const std::string& TribesLegacyLookupTable::lookup_ware(const std::string& ware) const {
	return lookup_entry(ware, wares_);
}

const std::string& TribesLegacyLookupTable::lookup_immovable(const std::string& immovable) const {
	return lookup_entry(immovable, immovables_);
}

const std::string& TribesLegacyLookupTable::lookup_building(const std::string& building) const {
	return lookup_entry(building, buildings_);
}

const std::string& TribesLegacyLookupTable::lookup_ship(const std::string& ship) const {
	return lookup_entry(ship, ships_);
}

const std::string& TribesLegacyLookupTable::lookup_program(const std::string& program) const {
	return lookup_entry(program, programs_);
}

const std::string&
TribesLegacyLookupTable::lookup_entry(const std::string& entry,
                                      const std::map<std::string, std::string>& table) const {
	const auto& i = table.find(entry);
	if (i != table.end()) {
		return i->second;
	}
	return entry;
}
