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

#ifndef WL_LOGIC_MAP_OBJECTS_DESCRIPTIONS_COMPATIBILITY_TABLE_H
#define WL_LOGIC_MAP_OBJECTS_DESCRIPTIONS_COMPATIBILITY_TABLE_H

#include <map>
#include <string>

#include "base/macros.h"
#include "logic/map_objects/map_object_type.h"

namespace Widelands {

/**
 * @brief The DescriptionsCompatibilityTable class contains mappings from old unit names to new unit
 * names for map and game compatibility. World units need to be mapped forever, Tribe units can be
 * removed from the list whenever we break savegame compatibility.
 */
class DescriptionsCompatibilityTable {
public:
	DescriptionsCompatibilityTable();
	virtual ~DescriptionsCompatibilityTable() = default;

	/// Looks up the new name for the description 'type' with the old name 'name'.
	virtual const std::string& lookup(const std::string& name, MapObjectType type) const;

protected:
	const std::string& lookup_entry(const std::string& entry,
	                                const std::map<std::string, std::string>& table) const;

	// <old name, new name>
	const std::map<std::string, std::string> workers_;
	const std::map<std::string, std::string> wares_;
	const std::map<std::string, std::string> buildings_;
	const std::map<std::string, std::string> ships_;

private:
	DISALLOW_COPY_AND_ASSIGN(DescriptionsCompatibilityTable);
};

/// If the map is newish and there is no old world to convert names from, we use
/// this one that simply returns the looked up values, except for some renaming
/// introduced through the merging of the tribes, which are handled here.
class PostOneWorldLegacyLookupTable : public DescriptionsCompatibilityTable {
public:
	PostOneWorldLegacyLookupTable();

	const std::string& lookup(const std::string& name, MapObjectType type) const override;

private:
	// <old name, new name>
	const std::map<std::string, std::string> critters_;
	const std::map<std::string, std::string> immovables_;
	const std::map<std::string, std::string> resources_;
	const std::map<std::string, std::string> terrains_;
};

class OneWorldLegacyLookupTable : public DescriptionsCompatibilityTable {
public:
	explicit OneWorldLegacyLookupTable(const std::string& old_world_name);

	const std::string& lookup(const std::string& name, MapObjectType type) const override;

private:
	const std::string&
	lookup_world_entry(const std::string& entry,
	                   const std::map<std::string, std::map<std::string, std::string>>& table) const;

	const std::string old_world_name_;
	// <old name, new name>
	const std::map<std::string, std::string> resources_;
	// <world_name, <old name, new name>>
	const std::map<std::string, std::map<std::string, std::string>> terrains_;
	const std::map<std::string, std::map<std::string, std::string>> critters_;
	const std::map<std::string, std::map<std::string, std::string>> immovables_;
};
} // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_DESCRIPTIONS_COMPATIBILITY_TABLE_H
