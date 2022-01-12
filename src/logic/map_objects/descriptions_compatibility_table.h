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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef WL_LOGIC_MAP_OBJECTS_DESCRIPTIONS_COMPATIBILITY_TABLE_H
#define WL_LOGIC_MAP_OBJECTS_DESCRIPTIONS_COMPATIBILITY_TABLE_H

#include <map>
#include <string>

#include "base/macros.h"

/**
 * @brief The DescriptionsCompatibilityTable class contains mappings from old unit names to new unit
 * names for map and game compatibility. World units need to be mapped forever, Tribe units can be
 * removed from the list whenever we break savegame compatibility.
 */
class DescriptionsCompatibilityTable {
public:
	DescriptionsCompatibilityTable();
	virtual ~DescriptionsCompatibilityTable() = default;

	/// Looks up the new name for the 'resource'.
	virtual std::string lookup_resource(const std::string& resource) const = 0;

	/// Looks up the new name for the 'terrain'.
	virtual std::string lookup_terrain(const std::string& terrain) const = 0;

	/// Looks up the new name for the 'critter'.
	virtual std::string lookup_critter(const std::string& critter) const = 0;

	/// Looks up the new name for the 'immovable'.
	virtual std::string lookup_immovable(const std::string& immovable) const = 0;

	/// Looks up the new name for the 'worker'.
	const std::string& lookup_worker(const std::string& worker) const;

	/// Looks up the new name for the 'ware'.
	const std::string& lookup_ware(const std::string& ware) const;

	/// Looks up the new name for the 'building'.
	const std::string& lookup_building(const std::string& building) const;

	/// Looks up the new name for the 'ship'.
	const std::string& lookup_ship(const std::string& ship) const;

	/// Looks up the new name for the 'program'.
	const std::string& lookup_program(const std::string& program) const;

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

	// Implements DescriptionsCompatibilityTable.
	std::string lookup_resource(const std::string& resource) const override;
	std::string lookup_terrain(const std::string& terrain) const override;
	std::string lookup_critter(const std::string& critter) const override;
	std::string lookup_immovable(const std::string& immovable) const override;

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

	// Implements DescriptionsCompatibilityTable.
	std::string lookup_resource(const std::string& resource) const override;
	std::string lookup_terrain(const std::string& terrain) const override;
	std::string lookup_critter(const std::string& critter) const override;
	std::string lookup_immovable(const std::string& immovable) const override;

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

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_DESCRIPTIONS_COMPATIBILITY_TABLE_H
