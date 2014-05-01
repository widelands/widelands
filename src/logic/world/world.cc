/*
 * Copyright (C) 2002, 2004, 2006-2013 by the Widelands Development Team
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

#include "logic/world/world.h"

#include <iostream>
#include <sstream>

#include "constants.h"
#include "graphic/graphic.h"
#include "io/filesystem/layered_filesystem.h"
#include "log.h"
#include "logic/bob.h"
#include "logic/critter_bob.h"
#include "logic/game_data_error.h"
#include "logic/immovable.h"
#include "logic/widelands.h"
#include "logic/world/editor_category.h"
#include "logic/world/map_gen.h"
#include "logic/world/resource_description.h"
#include "logic/world/terrain_description.h"
#include "wexception.h"

namespace Widelands {

World::World()
   : bobs_(new DescriptionMaintainer<BobDescr>()),
     immovables_(new DescriptionMaintainer<Immovable_Descr>()),
     terrains_(new DescriptionMaintainer<TerrainDescription>()),
     resources_(new DescriptionMaintainer<ResourceDescription>()),
     editor_terrain_categories_(new DescriptionMaintainer<EditorCategory>()),
     mapGenInfo_(new MapGenInfo()) {
}

World::~World() {
}

const DescriptionMaintainer<TerrainDescription>& World::terrains() const {
	return *terrains_;
}

void World::add_resource_type(const LuaTable& table) {
	resources_->add(new ResourceDescription(table));
}

void World::add_terrain_type(const LuaTable& table) {
	terrains_->add(new TerrainDescription(table, *this));
}

void World::add_critter_type(const LuaTable& table) {
	bobs_->add(new Critter_Bob_Descr(table));
}

void World::add_immovable_type(const LuaTable& table) {
	immovables_->add(new Immovable_Descr(table));
}

void World::add_editor_terrain_category(const LuaTable& table) {
	editor_terrain_categories_->add(new EditorCategory(table));
}

const DescriptionMaintainer<EditorCategory>& World::editor_terrain_categories() const {
	return *editor_terrain_categories_;
}

int32_t World::safe_resource_index(const char* const resourcename) const {
	int32_t const result = get_resource(resourcename);

	if (result == INVALID_INDEX)
		throw game_data_error("world does not define resource type \"%s\"", resourcename);
	return result;
}

const MapGenInfo& World::getMapGenInfo() const {
	return *mapGenInfo_;
}

TerrainDescription& World::terrain_descr(Terrain_Index const i) const {
	return *terrains_->get(i);
}

TerrainDescription const* World::get_ter(char const* const name) const {
	int32_t const i = terrains_->get_index(name);
	return i != INVALID_INDEX ? terrains_->get(i) : nullptr;
}

int32_t World::get_bob(char const* const l) const {
	return bobs_->get_index(l);
}

BobDescr const* World::get_bob_descr(uint16_t const index) const {
	return bobs_->get(index);
}

BobDescr const* World::get_bob_descr(const std::string& name) const {
	return bobs_->exists(name.c_str());
}

int32_t World::get_nr_bobs() const {
	return bobs_->get_nitems();
}

int32_t World::get_immovable_index(char const* const l) const {
	return immovables_->get_index(l);
}

int32_t World::get_nr_immovables() const {
	return immovables_->get_nitems();
}

Immovable_Descr const* World::get_immovable_descr(int32_t const index) const {
	return immovables_->get(index);
}

int32_t World::get_resource(const char* const name) const {
	return resources_->get_index(name);
}

ResourceDescription const* World::get_resource(Resource_Index const res) const {
	assert(res < resources_->get_nitems());
	return resources_->get(res);
}

int32_t World::get_nr_resources() const {
	return resources_->get_nitems();
}

}  // namespace Widelands
