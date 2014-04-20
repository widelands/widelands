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

namespace Widelands {

class EditorCategory;
class Editor_Game_Base;
class ResourceDescription;
class TerrainDescription;
struct Critter_Bob_Descr;
struct Immovable_Descr;
struct MapGenInfo;

// This is the in memory descriptions of the world and provides access to
// terrains, immovables and resources.
class World : boost::noncopyable {
public:
	World();
	~World();  // Defined in .cc because all forward declarations are known then.

	// NOCOM(#sirver): refactor to return containers instead.
	const DescriptionMaintainer<TerrainDescription>& terrains() const;
	TerrainDescription& terrain_descr(Terrain_Index i) const;
	TerrainDescription const* get_ter(char const* const name) const;

	int32_t get_bob(char const* const l) const;
	BobDescr const* get_bob_descr(uint16_t index) const;
	BobDescr const* get_bob_descr(const std::string& name) const;
	int32_t get_nr_bobs() const;

	int32_t get_immovable_index(char const* const l) const;
	int32_t get_nr_immovables() const;
	Immovable_Descr const* get_immovable_descr(int32_t index) const;

	int32_t get_resource(const char* const name) const;
	ResourceDescription const* get_resource(Resource_Index res) const;
	int32_t get_nr_resources() const;
	int32_t safe_resource_index(const char* const warename) const;


	// Add this new resource to the world description.
	void add_resource_type(const LuaTable& table);

	// Add this new terrain to the world description.
	void add_terrain_type(const LuaTable& table);

	// Add a new critter to the world description.
	void add_critter_type(const LuaTable& table);

	// Add a new immovable to the world description.
	void add_immovable_type(const LuaTable& table);

	// Add an editor category for grouping items in the editor.
	void add_editor_category(const LuaTable& table);

	// Access to the editor categories.
	const DescriptionMaintainer<EditorCategory>& editor_categories() const;

	const MapGenInfo& getMapGenInfo() const;

private:
	std::unique_ptr<DescriptionMaintainer<BobDescr>> bobs_;
	std::unique_ptr<DescriptionMaintainer<Immovable_Descr>> immovables_;
	std::unique_ptr<DescriptionMaintainer<TerrainDescription>> terrains_;
	std::unique_ptr<DescriptionMaintainer<ResourceDescription>> resources_;
	std::unique_ptr<DescriptionMaintainer<EditorCategory>> editor_categories_;
	std::unique_ptr<MapGenInfo> mapGenInfo_;
};
}  // namespace Widelands

#endif
