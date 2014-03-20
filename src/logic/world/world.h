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

#include "description_maintainer.h"
#include "logic/bob.h"
#include "logic/immovable.h"
#include "logic/widelands.h"
#include "logic/world/resource_description.h"
#include "logic/world/terrain_description.h"

class Section;
class LuaInterface;

namespace Widelands {

class Editor_Game_Base;
struct Critter_Bob_Descr;
struct MapGenInfo;

// This is the in memory descriptions of the world and provides access to
// terrains, immovables and resources.
class World : boost::noncopyable {
public:
	World();
	~World();  // Defined in .cc because all forward declarations are known then.

	// Load the graphics data for this world.
	void load_graphics();

	Terrain_Index index_of_terrain(char const* const name) const {
		return terrains_.get_index(name);
	}
	TerrainDescription& terrain_descr(Terrain_Index const i) const {
		return *terrains_.get(i);
	}
	const TerrainDescription& get_ter(Terrain_Index const i) const {
		assert(i < terrains_.get_nitems());
		return *terrains_.get(i);
	}
	TerrainDescription const* get_ter(char const* const name) const {
		int32_t const i = terrains_.get_index(name);
		return i != -1 ? terrains_.get(i) : nullptr;
	}
	int32_t get_nr_terrains() const {
		return terrains_.get_nitems();
	}
	int32_t get_bob(char const* const l) const {
		return bobs_.get_index(l);
	}
	Bob::Descr const* get_bob_descr(uint16_t const index) const {
		return bobs_.get(index);
	}
	Bob::Descr const* get_bob_descr(const std::string& name) const {
		return bobs_.exists(name.c_str());
	}
	int32_t get_nr_bobs() const {
		return bobs_.get_nitems();
	}
	int32_t get_immovable_index(char const* const l) const {
		return immovables_.get_index(l);
	}
	int32_t get_nr_immovables() const {
		return immovables_.get_nitems();
	}
	Immovable_Descr const* get_immovable_descr(int32_t const index) const {
		return immovables_.get(index);
	}

	// NOCOM(#sirver): all methods defined in .cc
	// Add this new resource to the world description. Transfers ownership.
	void add_new_resource_type(ResourceDescription* resource_description);

	// Add this new terrain to the world description. Transfers ownership.
	void add_new_terrain_type(TerrainDescription* terrain_description);

	// Add a new critter to the world description. Transfers ownership.
	void add_new_critter_type(Critter_Bob_Descr* resource_description);

	// Add a new immovable to the world description. Transfers ownership.
	void add_new_immovable_type(Immovable_Descr* resource_description);

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

	const MapGenInfo& getMapGenInfo() const;

private:
	DescriptionMaintainer<Bob::Descr> bobs_;
	DescriptionMaintainer<Immovable_Descr> immovables_;
	DescriptionMaintainer<TerrainDescription> terrains_;
	DescriptionMaintainer<ResourceDescription> resources_;

	std::unique_ptr<MapGenInfo> mapGenInfo_;
};
}  // namespace Widelands

#endif
