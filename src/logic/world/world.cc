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
     editor_categories_(new DescriptionMaintainer<EditorCategory>()),
     mapGenInfo_(new MapGenInfo()) {
}

// World::ParseConfiguaration(LuaInterface* lua) {
// NOCOM(#sirver): bring all of this back
// try {
// i18n::Textdomain textdomain("world");

// std::unique_ptr<FileSystem> fs(g_fs->MakeSubFileSystem(basedir_));
// FileSystemLayer filesystemlayer(*fs);

// {
// Profile root_conf((basedir_ + "conf").c_str());
// parse_root_conf(name, root_conf);
// log("Parsing world bobs...\n");
// parse_bobs(basedir_, root_conf);
// root_conf.check_used();
// }

// { //  General bobs mainly for scenarios
// Profile global_root_conf("global/conf");
// std::string global_dir = "global/bobs/";
// log("Parsing global bobs in world...\n");
// parse_bobs(global_dir, global_root_conf);
// global_root_conf.check_used();
// }

// {
// log("Parsing map gen info...\n");
// parse_mapgen();
// }
// } catch (const std::exception & e) {
// throw game_data_error("world %s: %s", name.c_str(), e.what());
// }
// }

World::~World() {
}

void World::load_graphics() {
	// NOCOM(#sirver): this is not really needed anymore I thinks.
	g_gr->flush_maptextures();
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

void World::add_editor_category(const LuaTable& table) {
	editor_categories_->add(new EditorCategory(table));
}

const DescriptionMaintainer<EditorCategory>& World::editor_categories() const {
	return *editor_categories_;
}

// NOCOM(#sirver): remove these
// void World::parse_bobs(const std::string& given_path, Profile & root_conf) {
// std::string path(given_path);
// std::string::size_type const base_path_size = path.size();
// std::set<std::string> names; //  To enforce name uniqueness.

// PARSE_MAP_OBJECT_TYPES_BEGIN("immovable")
// immovables_.add
// (new Immovable_Descr
// (_name, _descname, path, prof, global_s, *this, nullptr));
// PARSE_MAP_OBJECT_TYPES_END;

// PARSE_MAP_OBJECT_TYPES_BEGIN("critter bob")
// bobs      .add
// (new Critter_Bob_Descr
// (_name, _descname, path, prof, global_s, nullptr));
// PARSE_MAP_OBJECT_TYPES_END;
// }

// void World::parse_mapgen   ()
// {
// char fname[256];

// snprintf(fname, sizeof(fname), "%s/mapgenconf", basedir_.c_str());

// try {
// Profile prof(fname);

// mapGenInfo_->parseProfile(this, prof);

// if (mapGenInfo_->getNumAreas(MapGenAreaInfo::atWater) < 1)
// throw game_data_error("missing a water area");

// if (mapGenInfo_->getNumAreas(MapGenAreaInfo::atWater) < 1)
// throw game_data_error("too many water areas (>3)");

// if (mapGenInfo_->getNumAreas(MapGenAreaInfo::atLand) < 1)
// throw game_data_error("missing a land area");

// if (mapGenInfo_->getNumAreas(MapGenAreaInfo::atLand) > 3)
// throw game_data_error("too many land areas (>3)");

// if (mapGenInfo_->getNumAreas(MapGenAreaInfo::atWasteland) < 1)
// throw game_data_error("missing a wasteland area");

// if (mapGenInfo_->getNumAreas(MapGenAreaInfo::atWasteland) > 2)
// throw game_data_error("too many wasteland areas (>2)");

// if (mapGenInfo_->getNumAreas(MapGenAreaInfo::atMountains) < 1)
// throw game_data_error("missing a mountain area");

// if (mapGenInfo_->getNumAreas(MapGenAreaInfo::atMountains) < 1)
// throw game_data_error("too many mountain areas (>1)");

// if
// (mapGenInfo_->getArea(MapGenAreaInfo::atWater, 0).getNumTerrains
// (MapGenAreaInfo::ttWaterOcean) < 1)
// throw game_data_error("missing a water/ocean terrain type");

// if
// (mapGenInfo_->getArea(MapGenAreaInfo::atWater, 0).getNumTerrains
// (MapGenAreaInfo::ttWaterShelf)
// <
// 1)
// throw game_data_error("missing a water/shelf terrain type");

// if
// (mapGenInfo_->getArea(MapGenAreaInfo::atWater, 0).getNumTerrains
// (MapGenAreaInfo::ttWaterShallow)
// <
// 1)
// throw game_data_error("is missing a water/shallow terrain type");

// if
// (mapGenInfo_->getArea(MapGenAreaInfo::atLand, 0).getNumTerrains
// (MapGenAreaInfo::ttLandCoast)
// <
// 1)
// throw game_data_error("missing a land/coast terrain type");

// if
// (mapGenInfo_->getArea(MapGenAreaInfo::atLand, 0).getNumTerrains
// (MapGenAreaInfo::ttLandLand)
// <
// 1)
// throw game_data_error("missing a land/land terrain type");

// if
// (mapGenInfo_->getArea(MapGenAreaInfo::atMountains, 0).getNumTerrains
// (MapGenAreaInfo::ttMountainsFoot)
// <
// 1)
// throw game_data_error("missing a mountain/foot terrain type");

// if
// (mapGenInfo_->getArea(MapGenAreaInfo::atMountains, 0).getNumTerrains
// (MapGenAreaInfo::ttMountainsMountain)
// <
// 1)
// throw game_data_error("missing a monutain/mountain terrain type");

// if
// (mapGenInfo_->getArea(MapGenAreaInfo::atMountains, 0).getNumTerrains
// (MapGenAreaInfo::ttMountainsSnow)
// <
// 1)
// throw game_data_error("missing a mountain/snow terrain type");

// if
// (mapGenInfo_->getArea(MapGenAreaInfo::atWasteland, 0).getNumTerrains
// (MapGenAreaInfo::ttWastelandInner)
// <
// 1)
// throw game_data_error("missing a land/coast terrain type");

// if
// (mapGenInfo_->getArea(MapGenAreaInfo::atWasteland, 0).getNumTerrains
// (MapGenAreaInfo::ttWastelandOuter)
// <
// 1)
// throw game_data_error("missing a land/land terrain type");

// prof.check_used();
// } catch (const _wexception & e) {
// throw game_data_error("%s: %s", fname, e.what());
// }
// }

int32_t World::safe_resource_index(const char* const resourcename) const {
	int32_t const result = get_resource(resourcename);

	if (result == -1)
		throw game_data_error("world does not define resource type \"%s\"", resourcename);
	return result;
}

const MapGenInfo& World::getMapGenInfo() const {
	return *mapGenInfo_;
}

Terrain_Index World::index_of_terrain(char const* const name) const {
	return terrains_->get_index(name);
}

TerrainDescription& World::terrain_descr(Terrain_Index const i) const {
	return *terrains_->get(i);
}

TerrainDescription const* World::get_ter(char const* const name) const {
	int32_t const i = terrains_->get_index(name);
	return i != -1 ? terrains_->get(i) : nullptr;
}

int32_t World::get_nr_terrains() const {
	return terrains_->get_nitems();
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
