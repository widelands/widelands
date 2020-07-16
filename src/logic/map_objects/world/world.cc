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
#include "graphic/image_io.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/world/critter.h"
#include "logic/map_objects/world/editor_category.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/map_objects/world/terrain_description.h"
#include "scripting/lua_table.h"

namespace Widelands {

World::World()
   : critters_(new DescriptionMaintainer<CritterDescr>()),
     immovables_(new DescriptionMaintainer<ImmovableDescr>()),
     terrains_(new DescriptionMaintainer<TerrainDescription>()),
     resources_(new DescriptionMaintainer<ResourceDescription>()),
     editor_terrain_categories_(new DescriptionMaintainer<EditorCategory>()),
     editor_critter_categories_(new DescriptionMaintainer<EditorCategory>()),
     editor_immovable_categories_(new DescriptionMaintainer<EditorCategory>()) {
}

World::~World() {
}

void World::load_graphics() {
	for (size_t i = 0; i < terrains_->size(); ++i) {
		TerrainDescription* terrain = terrains_->get_mutable(i);
		for (size_t j = 0; j < terrain->texture_paths().size(); ++j) {
			// Set the minimap color on the first loaded image.
			if (j == 0) {
				SDL_Surface* sdl_surface = load_image_as_sdl_surface(terrain->texture_paths()[j]);
				uint8_t top_left_pixel = static_cast<uint8_t*>(sdl_surface->pixels)[0];
				const SDL_Color top_left_pixel_color =
				   sdl_surface->format->palette->colors[top_left_pixel];
				terrain->set_minimap_color(
				   RGBColor(top_left_pixel_color.r, top_left_pixel_color.g, top_left_pixel_color.b));
				SDL_FreeSurface(sdl_surface);
			}
			terrain->add_texture(g_gr->images().get(terrain->texture_paths()[j]));
		}
	}
}

void World::postload() {
	const DescriptionIndex nr_t = get_nr_terrains();
	for (size_t i = 0; i < nr_t; ++i) {
		const TerrainDescription& t = terrain_descr(i);
		if (!t.enhancement().empty()) {
			if (!terrain_descr(t.enhancement())) {
				throw GameDataError(
				   "Terrain %s: Unknown enhancement %s", t.name().c_str(), t.enhancement().c_str());
			}
		}
	}
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
	i18n::Textdomain td("world");
	critters_->add(new CritterDescr(_(table.get_string("descname")), table, *this));
}

const DescriptionMaintainer<ImmovableDescr>& World::immovables() const {
	return *immovables_;
}

void World::add_immovable_type(const LuaTable& table) {
	i18n::Textdomain td("world");
	immovables_->add(new ImmovableDescr(_(table.get_string("descname")), table, *this));
}

void World::add_editor_terrain_category(const LuaTable& table) {
	editor_terrain_categories_->add(new EditorCategory(table));
}

const DescriptionMaintainer<EditorCategory>& World::editor_terrain_categories() const {
	return *editor_terrain_categories_;
}

void World::add_editor_critter_category(const LuaTable& table) {
	editor_critter_categories_->add(new EditorCategory(table));
}

const DescriptionMaintainer<EditorCategory>& World::editor_critter_categories() const {
	return *editor_critter_categories_;
}

void World::add_editor_immovable_category(const LuaTable& table) {
	editor_immovable_categories_->add(new EditorCategory(table));
}

const DescriptionMaintainer<EditorCategory>& World::editor_immovable_categories() const {
	return *editor_immovable_categories_;
}

DescriptionIndex World::safe_resource_index(const char* const resourcename) const {
	DescriptionIndex const result = resource_index(resourcename);

	if (result == INVALID_INDEX)
		throw GameDataError("world does not define resource type \"%s\"", resourcename);
	return result;
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

DescriptionIndex World::get_nr_terrains() const {
	return terrains_->size();
}

DescriptionIndex World::get_nr_critters() const {
	return critters_->size();
}

DescriptionIndex World::get_critter(char const* const l) const {
	return critters_->get_index(l);
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

DescriptionIndex World::get_nr_immovables() const {
	return immovables_->size();
}

ImmovableDescr const* World::get_immovable_descr(DescriptionIndex const index) const {
	return immovables_->get_mutable(index);
}

DescriptionIndex World::resource_index(const char* const name) const {
	return strcmp(name, "none") ? resources_->get_index(name) : Widelands::kNoResource;
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
