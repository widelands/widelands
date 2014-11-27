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
#include <memory>
#include <sstream>

#include "base/i18n.h"
#include "base/log.h"
#include "base/wexception.h"
#include "graphic/graphic.h"
#include "graphic/image_io.h"
#include "graphic/texture.h"
#include "graphic/texture_atlas.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/filewrite.h"
#include "logic/critter.h"
#include "logic/game_data_error.h"
#include "logic/immovable.h"
#include "logic/parse_map_object_types.h"
#include "logic/widelands.h"
#include "logic/world/editor_category.h"
#include "logic/world/resource_description.h"
#include "logic/world/terrain_description.h"
#include "profile/profile.h"

namespace Widelands {

World::World()
   : bobs_(new DescriptionMaintainer<BobDescr>()),
     immovables_(new DescriptionMaintainer<ImmovableDescr>()),
     terrains_(new DescriptionMaintainer<TerrainDescription>()),
     resources_(new DescriptionMaintainer<ResourceDescription>()),
     editor_terrain_categories_(new DescriptionMaintainer<EditorCategory>()),
     editor_immovable_categories_(new DescriptionMaintainer<EditorCategory>()) {
}

World::~World() {
}

// NOCOM(#sirver): it feels wrong that this is here.
void World::postload() {
	// NOCOM(#sirver): figure this out, maybe just grow organically.
	TextureAtlas ta(1024, 1024);

	// These will be deleted at the end of the method.
	std::vector<std::unique_ptr<Texture>> individual_textures_;

	for (size_t i = 0; i < terrains_->size(); ++i) {
		const TerrainDescription& terrain = terrains_->get_unmutable(i);
		for (const std::string& texture_name : terrain.texture_paths()) {
			individual_textures_.emplace_back(new Texture(load_image_as_sdl_surface(texture_name)));
			ta.add(*individual_textures_.back());
		}
	}

	std::vector<std::unique_ptr<Texture>> textures;

	// NOCOM(#sirver): should be some member
	terrain_texture_ = ta.pack(&textures);
	assert(textures.size() == 198); // NOCOM(#sirver): reomve

	// NOCOM(#sirver): remove
	FileWrite fw;
	save_surface_to_png(terrain_texture_.get(), &fw);
	fw.write(*g_fs, "texture_atlas.png");

	// NOCOM(#sirver): check that loaded terrain has the correct size

	int next_texture_to_move = 0;
	for (size_t i = 0; i < terrains_->size(); ++i) {
		TerrainDescription* terrain = terrains_->get(i);
		for (size_t j = 0; j < terrain->texture_paths().size(); ++j) {
			terrain->add_texture(std::move(textures.at(next_texture_to_move++)));
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
	bobs_->add(new CritterDescr(table));
}

const DescriptionMaintainer<ImmovableDescr>& World::immovables() const {
	return *immovables_;
}

void World::add_immovable_type(const LuaTable& table) {
	immovables_->add(new ImmovableDescr(table, *this));
}

void World::add_editor_terrain_category(const LuaTable& table) {
	editor_terrain_categories_->add(new EditorCategory(table));
}

const DescriptionMaintainer<EditorCategory>& World::editor_terrain_categories() const {
	return *editor_terrain_categories_;
}

void World::add_editor_immovable_category(const LuaTable& table) {
	editor_immovable_categories_->add(new EditorCategory(table));
}

const DescriptionMaintainer<EditorCategory>& World::editor_immovable_categories() const {
	return *editor_immovable_categories_;
}

int32_t World::safe_resource_index(const char* const resourcename) const {
	int32_t const result = get_resource(resourcename);

	if (result == INVALID_INDEX)
		throw GameDataError("world does not define resource type \"%s\"", resourcename);
	return result;
}

TerrainDescription& World::terrain_descr(TerrainIndex const i) const {
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

ImmovableDescr const* World::get_immovable_descr(int32_t const index) const {
	return immovables_->get(index);
}

int32_t World::get_resource(const char* const name) const {
	return resources_->get_index(name);
}

ResourceDescription const* World::get_resource(ResourceIndex const res) const {
	assert(res < resources_->get_nitems());
	return resources_->get(res);
}

int32_t World::get_nr_resources() const {
	return resources_->get_nitems();
}

}  // namespace Widelands
