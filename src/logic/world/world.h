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

#ifndef WL_LOGIC_WORLD_WORLD_H
#define WL_LOGIC_WORLD_WORLD_H

#include <memory>

#include "base/macros.h"
#include "logic/description_maintainer.h"
#include "logic/widelands.h"

class LuaInterface;
class LuaTable;
class Texture;

namespace Widelands {

class BobDescr;
class EditorCategory;
class EditorGameBase;
class ImmovableDescr;
class ResourceDescription;
class TerrainDescription;
struct CritterDescr;

/// This is the in memory descriptions of the world and provides access to
/// terrains, immovables and resources.
class World {
public:
	World();
	~World();

	// TODO(sirver): Refactor these to only return the description_maintainer so that world
	// becomes a pure container.
	const DescriptionMaintainer<TerrainDescription>& terrains() const;
	TerrainDescription& terrain_descr(DescriptionIndex i) const;
	TerrainDescription const* get_ter(char const* const name) const;

	DescriptionIndex get_bob(char const* const l) const;
	BobDescr const* get_bob_descr(DescriptionIndex index) const;
	BobDescr const* get_bob_descr(const std::string& name) const;
	int32_t get_nr_bobs() const;

	const DescriptionMaintainer<ImmovableDescr>& immovables() const;
	DescriptionIndex get_immovable_index(const std::string& name) const;
	DescriptionIndex get_nr_immovables() const;
	ImmovableDescr const* get_immovable_descr(DescriptionIndex index) const;

	DescriptionIndex get_resource(const char* const name) const;
	ResourceDescription const* get_resource(DescriptionIndex res) const;
	DescriptionIndex get_nr_resources() const;
	DescriptionIndex safe_resource_index(const char* const warename) const;

	/// Add this new resource to the world description.
	void add_resource_type(const LuaTable& table);

	/// Add this new terrain to the world description.
	void add_terrain_type(const LuaTable& table);

	/// Add a new critter to the world description.
	void add_critter_type(const LuaTable& table);

	/// Add a new immovable to the world description.
	void add_immovable_type(const LuaTable& table);

	/// Add an editor categories for grouping items in the editor.
	void add_editor_terrain_category(const LuaTable& table);
	void add_editor_immovable_category(const LuaTable& table);

	/// Access to the editor categories.
	const DescriptionMaintainer<EditorCategory>& editor_terrain_categories() const;
	const DescriptionMaintainer<EditorCategory>& editor_immovable_categories() const;

	// Load the graphics for the world. Animations are loaded on
	// demand.
	void load_graphics();

private:
	std::unique_ptr<DescriptionMaintainer<BobDescr>> bobs_;
	std::unique_ptr<DescriptionMaintainer<ImmovableDescr>> immovables_;
	std::unique_ptr<DescriptionMaintainer<TerrainDescription>> terrains_;
	std::unique_ptr<DescriptionMaintainer<ResourceDescription>> resources_;
	std::unique_ptr<DescriptionMaintainer<EditorCategory>> editor_terrain_categories_;
	std::unique_ptr<DescriptionMaintainer<EditorCategory>> editor_immovable_categories_;
	std::unique_ptr<Texture> terrain_texture_;

	DISALLOW_COPY_AND_ASSIGN(World);
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_WORLD_WORLD_H
