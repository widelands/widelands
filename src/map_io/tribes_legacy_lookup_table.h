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

#ifndef WL_MAP_IO_TRIBES_LEGACY_LOOKUP_TABLE_H
#define WL_MAP_IO_TRIBES_LEGACY_LOOKUP_TABLE_H

#include <map>
#include <string>

#include "base/macros.h"

class TribesLegacyLookupTable {
public:
	TribesLegacyLookupTable();

	/// Looks up the new name for the 'worker'.
	const std::string& lookup_worker(const std::string& worker) const;

	/// Looks up the new name for the 'ware'.
	const std::string& lookup_ware(const std::string& ware) const;

	/// Looks up the new name for the 'immovable'.
	const std::string& lookup_immovable(const std::string& immovable) const;

	/// Looks up the new name for the 'building'.
	const std::string& lookup_building(const std::string& building) const;

	/// Looks up the new name for the 'ship'.
	const std::string& lookup_ship(const std::string& ship) const;

	/// Looks up the new name for the 'program'.
	const std::string& lookup_program(const std::string& program) const;

private:
	const std::string& lookup_entry(const std::string& entry,
	                                const std::map<std::string, std::string>& table) const;

	/// {tribe name, {old name, new name}}
	const std::map<std::string, std::string> workers_;
	const std::map<std::string, std::string> wares_;
	const std::map<std::string, std::string> immovables_;
	const std::map<std::string, std::string> buildings_;
	const std::map<std::string, std::string> ships_;
	const std::map<std::string, std::string> programs_;

	DISALLOW_COPY_AND_ASSIGN(TribesLegacyLookupTable);
};

#endif  // end of include guard: WL_MAP_IO_TRIBES_LEGACY_LOOKUP_TABLE_H
