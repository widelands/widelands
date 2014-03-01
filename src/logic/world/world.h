/*
 * Copyright (C) 2002, 2006-2013 by the Widelands Development Team
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

#ifndef WORLD_H
#define WORLD_H

#include <memory>

#include "descr_maintainer.h"
#include "logic/bob.h"
#include "logic/immovable.h"
#include "logic/widelands.h"
#include "logic/world/data.h"
#include "logic/world/resource_description.h"
#include "logic/world/terrain_description.h"

class Section;

namespace Widelands {

struct MapGenInfo;
class Editor_Game_Base;

// This is the in memory descriptions of the world and provides access to
// terrains, immovables and resources.
class World : boost::noncopyable {
public:
	World(const std::string& name);
	~World();

	static bool exists_world(std::string);  ///  check if a world really exists

	// Load the graphics data for this world.
	void load_graphics();

	// Returns various metadata.
	const std::string& get_name() const;
	const std::string& get_author() const;
	const std::string& get_description() const;

	Terrain_Index index_of_terrain(char const* const name) const {
		return terrain_descriptions_.get_index(name);
	}
	TerrainDescription& terrain_descr(Terrain_Index const i) const {
		return *terrain_descriptions_.get(i);
	}
	const TerrainDescription& get_ter(Terrain_Index const i) const {
		assert(i < terrain_descriptions_.get_nitems());
		return *terrain_descriptions_.get(i);
	}
	TerrainDescription const* get_ter(char const* const name) const {
		int32_t const i = terrain_descriptions_.get_index(name);
		return i != -1 ? terrain_descriptions_.get(i) : nullptr;
	}
	int32_t get_nr_terrains() const {
		return terrain_descriptions_.get_nitems();
	}
	int32_t get_bob(char const* const l) const {
		return bobs.get_index(l);
	}
	Bob::Descr const* get_bob_descr(uint16_t const index) const {
		return bobs.get(index);
	}
	Bob::Descr const* get_bob_descr(const std::string& name) const {
		return bobs.exists(name.c_str());
	}
	int32_t get_nr_bobs() const {
		return bobs.get_nitems();
	}
	int32_t get_immovable_index(char const* const l) const {
		return immovables.get_index(l);
	}
	int32_t get_nr_immovables() const {
		return immovables.get_nitems();
	}
	Immovable_Descr const* get_immovable_descr(int32_t const index) const {
		return immovables.get(index);
	}

	int32_t get_resource(const char* const name) const {
		return resources_.get_index(name);
	}
	ResourceDescription const* get_resource(Resource_Index const res) const {
		assert(res < resources_.get_nitems());
		return resources_.get(res);
	}
	int32_t get_nr_resources() const {
		return resources_.get_nitems();
	}
	int32_t safe_resource_index(const char* const warename) const;
	const std::string& basedir() const {
		return basedir_;
	}

	MapGenInfo& getMapGenInfo();

private:
	const std::string basedir_;  //  base directory, where the main conf file resides
	std::string name_; // The name of this world.
	std::string author_;  // The author of this world.
	std::string description_; // The description of this world.

	Descr_Maintainer<Bob::Descr> bobs;
	Descr_Maintainer<Immovable_Descr> immovables;
	Descr_Maintainer<TerrainDescription> terrain_descriptions_;
	Descr_Maintainer<ResourceDescription> resources_;

	std::unique_ptr<MapGenInfo> mapGenInfo_;

	void parse_root_conf(const std::string& name, Profile& root_conf);
	void parse_resources();
	void parse_terrains();
	void parse_bobs(const std::string& directory, Profile& root_conf);
	void parse_mapgen();
};
}  // namespace Widelands

#endif
