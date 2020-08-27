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

#include "logic/map_objects/world/world.h"

#include <memory>

#include "base/i18n.h"
#include "graphic/graphic.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/world/critter.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/map_objects/world/terrain_description.h"
#include "scripting/lua_table.h"

namespace Widelands {

World::World(DescriptionManager* description_manager)
   : critters_(new DescriptionMaintainer<CritterDescr>()),
     immovables_(new DescriptionMaintainer<ImmovableDescr>()),
     terrains_(new DescriptionMaintainer<TerrainDescription>()),
     resources_(new DescriptionMaintainer<ResourceDescription>()),

     description_manager_(description_manager) {

	// Walk world directory and register objects
	description_manager_->register_directory("world", g_fs, false);
}

void World::add_world_object_type(const LuaTable& table, MapObjectType type) {
	const std::string& type_name = table.get_string("name");

	// TODO(GunChleoc): Compatibility, remove after v1.0
	if (table.has_key<std::string>("msgctxt")) {
		log("WARNING: The 'msgctxt' entry is no longer needed in '%s', please remove it\n", type_name.c_str());
	}

	description_manager_->mark_loading_in_progress(type_name);

	// Add
	switch (type) {
	case MapObjectType::CRITTER:
		critters_->add(new CritterDescr(
		   table.get_string("descname"), table, description_manager_->get_attributes(type_name)));
		break;
	case MapObjectType::IMMOVABLE:
		immovables_->add(new ImmovableDescr(table.get_string("descname"), table, MapObjectDescr::OwnerType::kWorld,
		                                    description_manager_->get_attributes(type_name)));
		break;
	case MapObjectType::RESOURCE:
		resources_->add(new ResourceDescription(table));
		break;
	case MapObjectType::TERRAIN:
		terrains_->add(new TerrainDescription(table, *this));
		break;
	default:
		NEVER_HERE();
	}

	// Update status
	description_manager_->mark_loading_done(type_name);
}

const DescriptionMaintainer<TerrainDescription>& World::terrains() const {
	return *terrains_;
}

const DescriptionMaintainer<ImmovableDescr>& World::immovables() const {
	return *immovables_;
}

DescriptionIndex World::load_critter(const std::string& crittername) {
	try {
		description_manager_->load_description(crittername);
	} catch (WException& e) {
		throw GameDataError(
		   "Error while loading critter type '%s': %s", crittername.c_str(), e.what());
	}
	return safe_critter_index(crittername);
}
DescriptionIndex World::load_immovable(const std::string& immovablename) {
	try {
		description_manager_->load_description(immovablename);
	} catch (WException& e) {
		throw GameDataError(
		   "Error while loading immovable type '%s': %s", immovablename.c_str(), e.what());
	}
	return safe_immovable_index(immovablename);
}
DescriptionIndex World::load_resource(const std::string& resourcename) {
	try {
		description_manager_->load_description(resourcename);
	} catch (WException& e) {
		throw GameDataError(
		   "Error while loading resource type '%s': %s", resourcename.c_str(), e.what());
	}
	return safe_resource_index(resourcename);
}
DescriptionIndex World::load_terrain(const std::string& terrainname) {
	try {
		description_manager_->load_description(terrainname);
	} catch (WException& e) {
		throw GameDataError(
		   "Error while loading terrain type '%s': %s", terrainname.c_str(), e.what());
	}
	return safe_terrain_index(terrainname);
}

TerrainDescription& World::terrain_descr(DescriptionIndex const i) const {
	return *terrains_->get_mutable(i);
}

const TerrainDescription* World::terrain_descr(const std::string& name) const {
	DescriptionIndex const i = terrains_->get_index(name);
	return i != INVALID_INDEX ? terrains_->get_mutable(i) : nullptr;
}

DescriptionIndex World::get_terrain_index(const std::string& name) const {
	return terrains_->get_index(name);
}
DescriptionIndex World::safe_terrain_index(const std::string& name) const {
	DescriptionIndex const result = get_terrain_index(name);

	if (result == INVALID_INDEX) {
		throw GameDataError("world does not define terrain type \"%s\"", name.c_str());
	}
	return result;
}

DescriptionIndex World::get_nr_terrains() const {
	return terrains_->size();
}

DescriptionIndex World::get_nr_critters() const {
	return critters_->size();
}

DescriptionIndex World::critter_index(const std::string& name) const {
	return critters_->get_index(name);
}
DescriptionIndex World::safe_critter_index(const std::string& name) const {
	DescriptionIndex const result = critter_index(name);

	if (result == INVALID_INDEX) {
		throw GameDataError("world does not define critter type \"%s\"", name.c_str());
	}
	return result;
}

const DescriptionMaintainer<CritterDescr>& World::critters() const {
	return *critters_;
}

CritterDescr const* World::get_critter_descr(DescriptionIndex index) const {
	return critters_->get_mutable(index);
}

CritterDescr const* World::get_critter_descr(const std::string& name) const {
	return critters_->exists(name.c_str());
}

DescriptionIndex World::get_immovable_index(const std::string& name) const {
	return immovables_->get_index(name);
}
DescriptionIndex World::safe_immovable_index(const std::string& name) const {
	DescriptionIndex const result = get_immovable_index(name);

	if (result == INVALID_INDEX) {
		throw GameDataError("world does not define immovable type \"%s\"", name.c_str());
	}
	return result;
}

DescriptionIndex World::get_nr_immovables() const {
	return immovables_->size();
}

ImmovableDescr const* World::get_immovable_descr(DescriptionIndex const index) const {
	return immovables_->get_mutable(index);
}

DescriptionIndex World::resource_index(const std::string& name) const {
	return name != "none" ? resources_->get_index(name) : Widelands::kNoResource;
}
DescriptionIndex World::safe_resource_index(const std::string& resourcename) const {
	DescriptionIndex const result = resource_index(resourcename);

	if (result == INVALID_INDEX) {
		throw GameDataError("world does not define resource type \"%s\"", resourcename.c_str());
	}
	return result;
}

/***
 * @return The ResourceDescription for the given index. Returns Nullptr for kNoResource.
 */
ResourceDescription const* World::get_resource(DescriptionIndex const res) const {
	assert(res < resources_->size() || res == Widelands::kNoResource);
	return resources_->get_mutable(res);
}

DescriptionIndex World::get_nr_resources() const {
	return resources_->size();
}

}  // namespace Widelands
