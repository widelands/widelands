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
#include "container_iterate.h"
#include "graphic/graphic.h"
#include "helper.h"
#include "i18n.h"
#include "io/filesystem/layered_filesystem.h"
#include "log.h"
#include "logic/critter_bob.h"
#include "logic/game_data_error.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"
#include "logic/world/data.h"
#include "logic/world/map_gen.h"
#include "parse_map_object_types.h"
#include "profile/profile.h"
#include "wexception.h"

namespace Widelands {

World::World() :
	basedir_("world/"),
	mapGenInfo_(new MapGenInfo())
{
	try {
		i18n::Textdomain textdomain("world_" + name);

		std::unique_ptr<FileSystem> fs(g_fs->MakeSubFileSystem(basedir_));
		FileSystemLayer filesystemlayer(*fs);

		{
			Profile root_conf((basedir_ + "conf").c_str());
			parse_root_conf(name, root_conf);
			parse_resources();
			parse_terrains();
			log("Parsing world bobs...\n");
			parse_bobs(basedir_, root_conf);
			root_conf.check_used();
		}

		{ //  General bobs mainly for scenarios
			Profile global_root_conf("global/conf");
			std::string global_dir = "global/bobs/";
			log("Parsing global bobs in world...\n");
			parse_bobs(global_dir, global_root_conf);
			global_root_conf.check_used();
		}

		{
			log("Parsing map gen info...\n");
			parse_mapgen();
		}
	} catch (const std::exception & e) {
		throw game_data_error("world %s: %s", name.c_str(), e.what());
	}
}

World::~World() {}

void World::load_graphics()
{
	int32_t i;

	g_gr->flush_maptextures();

	// Load terrain graphics
	for (i = 0; i < terrain_descriptions_.get_nitems(); ++i)
		terrain_descriptions_.get(i)->load_graphics();

	// TODO: load more graphics
}

const std::string& World::get_name() const {
	return name_;
}

const std::string& World::get_author() const {
	return author_;
}

const std::string& World::get_description() const {
	return description_;
}

//
// down here: Private functions for loading
//

/**
 * Read the <world-directory>/conf
 */
void World::parse_root_conf(const std::string & name, Profile & root_conf)
{
	Section& s = root_conf.get_safe_section("world");
	name_ = s.get_string("name", name.c_str());
	author_ = s.get_string("author", s.get_safe_string("author"));
	description_ = s.get_safe_string("descr");
}

void World::parse_resources()
{
	char fname[256];

	snprintf(fname, sizeof(fname), "%s/resconf", basedir_.c_str());

	try {
		Profile prof(fname);
		while (Section * const section = prof.get_next_section(nullptr)) {
			ResourceDescription & descr = *new ResourceDescription();
			descr.parse(*section, basedir_);
			resources_.add(&descr);
		}
	} catch (const std::exception & e) {
		throw game_data_error("%s: %s", fname, e.what());
	}
}

void World::parse_terrains()
{
	char fname[256];

	snprintf(fname, sizeof(fname), "%s/terrainconf", basedir_.c_str());

	try {
		Profile prof(fname);

		for (Terrain_Index i = 0;; ++i) {
			Section * const s = prof.get_next_section(nullptr);
			if (not s)
				break;
			if (i == 0x10)
				throw game_data_error
					(_("%s: too many terrain types, can not be more than 16"),
					 fname);
			terrain_descriptions_.add(new TerrainDescription(basedir_.c_str(), s, &resources_));
		}

		prof.check_used();
	} catch (const game_data_error & e) {
		throw game_data_error("%s: %s", fname, e.what());
	}
}

void World::parse_bobs(const std::string& given_path, Profile & root_conf) {
	std::string path(given_path);
	std::string::size_type const base_path_size = path.size();
	std::set<std::string> names; //  To enforce name uniqueness.

	PARSE_MAP_OBJECT_TYPES_BEGIN("immovable")
		immovables.add
			(new Immovable_Descr
			 	(_name, _descname, path, prof, global_s, *this, nullptr));
	PARSE_MAP_OBJECT_TYPES_END;

	PARSE_MAP_OBJECT_TYPES_BEGIN("critter bob")
		bobs      .add
			(new Critter_Bob_Descr
			 	(_name, _descname, path, prof, global_s, nullptr));
	PARSE_MAP_OBJECT_TYPES_END;
}

void World::parse_mapgen   ()
{
	char fname[256];

	snprintf(fname, sizeof(fname), "%s/mapgenconf", basedir_.c_str());

	try {
		Profile prof(fname);

		mapGenInfo_->parseProfile(this, prof);

		if (mapGenInfo_->getNumAreas(MapGenAreaInfo::atWater) < 1)
			throw game_data_error("missing a water area");

		if (mapGenInfo_->getNumAreas(MapGenAreaInfo::atWater) < 1)
			throw game_data_error("too many water areas (>3)");

		if (mapGenInfo_->getNumAreas(MapGenAreaInfo::atLand) < 1)
			throw game_data_error("missing a land area");

		if (mapGenInfo_->getNumAreas(MapGenAreaInfo::atLand) > 3)
			throw game_data_error("too many land areas (>3)");

		if (mapGenInfo_->getNumAreas(MapGenAreaInfo::atWasteland) < 1)
			throw game_data_error("missing a wasteland area");

		if (mapGenInfo_->getNumAreas(MapGenAreaInfo::atWasteland) > 2)
			throw game_data_error("too many wasteland areas (>2)");

		if (mapGenInfo_->getNumAreas(MapGenAreaInfo::atMountains) < 1)
			throw game_data_error("missing a mountain area");

		if (mapGenInfo_->getNumAreas(MapGenAreaInfo::atMountains) < 1)
			throw game_data_error("too many mountain areas (>1)");

		if
			(mapGenInfo_->getArea(MapGenAreaInfo::atWater, 0).getNumTerrains
			 	(MapGenAreaInfo::ttWaterOcean) < 1)
			throw game_data_error("missing a water/ocean terrain type");

		if
			(mapGenInfo_->getArea(MapGenAreaInfo::atWater, 0).getNumTerrains
			 	(MapGenAreaInfo::ttWaterShelf)
			 <
			 1)
			throw game_data_error("missing a water/shelf terrain type");

		if
			(mapGenInfo_->getArea(MapGenAreaInfo::atWater, 0).getNumTerrains
			 	(MapGenAreaInfo::ttWaterShallow)
			 <
			 1)
			throw game_data_error("is missing a water/shallow terrain type");

		if
			(mapGenInfo_->getArea(MapGenAreaInfo::atLand, 0).getNumTerrains
			 	(MapGenAreaInfo::ttLandCoast)
			 <
			 1)
			throw game_data_error("missing a land/coast terrain type");

		if
			(mapGenInfo_->getArea(MapGenAreaInfo::atLand, 0).getNumTerrains
			 	(MapGenAreaInfo::ttLandLand)
			 <
			 1)
			throw game_data_error("missing a land/land terrain type");

		if
			(mapGenInfo_->getArea(MapGenAreaInfo::atMountains, 0).getNumTerrains
			 	(MapGenAreaInfo::ttMountainsFoot)
			 <
			 1)
			throw game_data_error("missing a mountain/foot terrain type");

		if
			(mapGenInfo_->getArea(MapGenAreaInfo::atMountains, 0).getNumTerrains
			 	(MapGenAreaInfo::ttMountainsMountain)
			 <
			 1)
			throw game_data_error("missing a monutain/mountain terrain type");

		if
			(mapGenInfo_->getArea(MapGenAreaInfo::atMountains, 0).getNumTerrains
			 	(MapGenAreaInfo::ttMountainsSnow)
			 <
			 1)
			throw game_data_error("missing a mountain/snow terrain type");

		if
			(mapGenInfo_->getArea(MapGenAreaInfo::atWasteland, 0).getNumTerrains
			 	(MapGenAreaInfo::ttWastelandInner)
			 <
			 1)
			throw game_data_error("missing a land/coast terrain type");

		if
			(mapGenInfo_->getArea(MapGenAreaInfo::atWasteland, 0).getNumTerrains
			 	(MapGenAreaInfo::ttWastelandOuter)
			 <
			 1)
			throw game_data_error("missing a land/land terrain type");

		prof.check_used();
	} catch (const _wexception & e) {
		throw game_data_error("%s: %s", fname, e.what());
	}
}

/**
 * Check if the world data can actually be read
 */
bool World::exists_world(std::string worldname)
{
	FileRead f;
	return
		f.TryOpen
			(*
			 std::unique_ptr<FileSystem>
				(g_fs->MakeSubFileSystem("worlds/" + worldname)),
			 "conf");
}

int32_t World::safe_resource_index(const char * const resourcename) const {
	int32_t const result = get_resource(resourcename);

	if (result == -1)
		throw game_data_error
			("world %s does not define resource type \"%s\"",
			 get_name().c_str(), resourcename);
	return result;
}


MapGenInfo & World::getMapGenInfo()
{
	return *mapGenInfo_;
}



}
