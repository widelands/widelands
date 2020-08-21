/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_WORLD_WORLD_H
#define WL_LOGIC_MAP_OBJECTS_WORLD_WORLD_H

#include <memory>

#include "base/macros.h"
#include "logic/map_objects/description_maintainer.h"
#include "logic/map_objects/description_manager.h"

class LuaTable;

namespace Widelands {

class CritterDescr;
class ImmovableDescr;
class ResourceDescription;
class TerrainDescription;

/// This is the in memory descriptions of the world and provides access to
/// terrains, immovables and resources.
class World {
public:
	World(DescriptionManager* description_manager);
	~World() = default;

	/// Add a world object type to the world.
	void add_world_object_type(const LuaTable& table, MapObjectType type);

	// TODO(sirver): Refactor these to only return the description_maintainer so that world
	// becomes a pure container.
	const DescriptionMaintainer<TerrainDescription>& terrains() const;
	TerrainDescription& terrain_descr(DescriptionIndex i) const;
	const TerrainDescription* terrain_descr(const std::string& name) const;
	DescriptionIndex get_terrain_index(const std::string& name) const;
	DescriptionIndex safe_terrain_index(const std::string& name) const;
	DescriptionIndex get_nr_terrains() const;

	const DescriptionMaintainer<CritterDescr>& critters() const;
	DescriptionIndex critter_index(const std::string& name) const;
	DescriptionIndex safe_critter_index(const std::string& name) const;
	CritterDescr const* get_critter_descr(DescriptionIndex index) const;
	CritterDescr const* get_critter_descr(const std::string& name) const;
	DescriptionIndex get_nr_critters() const;

	const DescriptionMaintainer<ImmovableDescr>& immovables() const;
	DescriptionIndex get_immovable_index(const std::string& name) const;
	DescriptionIndex safe_immovable_index(const std::string& name) const;
	DescriptionIndex get_nr_immovables() const;
	ImmovableDescr const* get_immovable_descr(DescriptionIndex index) const;


	DescriptionIndex resource_index(const std::string& name) const;
	DescriptionIndex safe_resource_index(const std::string& warename) const;
	ResourceDescription const* get_resource(DescriptionIndex res) const;
	DescriptionIndex get_nr_resources() const;

	/// Load a critter that has been registered previously with 'register_description'
	DescriptionIndex load_critter(const std::string& crittername);
	/// Load an immovable that has been registered previously with 'register_description'
	DescriptionIndex load_immovable(const std::string& immovablename);
	/// Load a resource that has been registered previously with 'register_description'
	DescriptionIndex load_resource(const std::string& resourcename);
	/// Load a terrain that has been registered previously with 'register_description'
	DescriptionIndex load_terrain(const std::string& terrainname);

private:
	std::unique_ptr<DescriptionMaintainer<CritterDescr>> critters_;
	std::unique_ptr<DescriptionMaintainer<ImmovableDescr>> immovables_;
	std::unique_ptr<DescriptionMaintainer<TerrainDescription>> terrains_;
	std::unique_ptr<DescriptionMaintainer<ResourceDescription>> resources_;


	DescriptionManager* description_manager_; // Not owned
	DISALLOW_COPY_AND_ASSIGN(World);
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_WORLD_WORLD_H
