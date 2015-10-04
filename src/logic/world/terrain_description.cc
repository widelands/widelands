/*
 * Copyright (C) 2006-2015 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "logic/world/terrain_description.h"

#include <memory>

#include <boost/format.hpp>

#include "graphic/animation.h"
#include "graphic/graphic.h"
#include "graphic/texture.h"
#include "logic/game_data_error.h"
#include "logic/world/editor_category.h"
#include "logic/world/world.h"
#include "profile/profile.h"
#include "scripting/lua_table.h"

namespace Widelands {

namespace  {

// Parse a terrain type from the giving string.
TerrainDescription::Type terrain_type_from_string(const std::string& type) {
	if (type == "green") {
		return TerrainDescription::Type::kGreen;
	}
	if (type == "dry") {
		return TerrainDescription::Type::kDry;
	}
	if (type == "water") {
		return static_cast<TerrainDescription::Type>(TerrainDescription::Type::kWater |
																	TerrainDescription::Type::kDry |
																	TerrainDescription::Type::kImpassable);
	}
	if (type == "dead") {
		return static_cast<TerrainDescription::Type>(TerrainDescription::Type::kDead |
																	TerrainDescription::Type::kDry |
																	TerrainDescription::Type::kImpassable);
	}
	if (type == "mountain") {
		return static_cast<TerrainDescription::Type>(TerrainDescription::Type::kDry |
																	TerrainDescription::Type::kMountain);
	}
	if (type == "impassable") {
		return static_cast<TerrainDescription::Type>(TerrainDescription::Type::kDry |
																	TerrainDescription::Type::kImpassable);
	}
	throw LuaError((boost::format("invalid terrain type '%s'") % type).str());
}

}  // namespace

TerrainDescription::TerrainDescription(const LuaTable& table, const Widelands::World& world)
   : name_(table.get_string("name")),
     descname_(table.get_string("descname")),
	  is_(terrain_type_from_string(table.get_string("is"))),
     default_resource_index_(world.get_resource(table.get_string("default_resource").c_str())),
     default_resource_amount_(table.get_int("default_resource_amount")),
     dither_layer_(table.get_int("dither_layer")),
     temperature_(table.get_double("temperature")),
     fertility_(table.get_double("fertility")),
     humidity_(table.get_double("humidity")) {

	if (!(0 < fertility_ && fertility_ < 1.)) {
		throw GameDataError("%s: fertility is not in (0, 1).", name_.c_str());
	}
	if (!(0 < humidity_ && humidity_ < 1.)) {
		throw GameDataError("%s: humidity is not in (0, 1).", name_.c_str());
	}
	if (temperature_ < 0) {
		throw GameDataError("%s: temperature is not possible.", name_.c_str());
	}

	 texture_paths_ =
	   table.get_table("textures")->array_entries<std::string>();
	frame_length_ = FRAME_LENGTH;
	if (texture_paths_.empty()) {
		throw GameDataError("Terrain %s has no images.", name_.c_str());
	} else if (texture_paths_.size() == 1) {
		if (table.has_key("fps")) {
			throw GameDataError("Terrain %s with one images must not have fps.", name_.c_str());
		}
	} else {
		frame_length_ = 1000 / get_positive_int(table, "fps");
	}

	for (const std::string& resource :
	     table.get_table("valid_resources")->array_entries<std::string>()) {
		valid_resources_.push_back(world.safe_resource_index(resource.c_str()));
	}

	if (default_resource_amount_ > 0 && !is_resource_valid(default_resource_index_)) {
		throw GameDataError("Default resource is not in valid resources.\n");
	}

	int editor_category_index =
	   world.editor_terrain_categories().get_index(table.get_string("editor_category"));
	if (editor_category_index < 0) {
		throw GameDataError(
		   "Unknown editor_category: %s\n", table.get_string("editor_category").c_str());
	}
	editor_category_ = world.editor_terrain_categories().get(editor_category_index);
}

TerrainDescription::~TerrainDescription() {
}

const Texture& TerrainDescription::get_texture(uint32_t gametime) const {
	return *textures_.at((gametime / frame_length_) % textures_.size());
}

void TerrainDescription::add_texture(std::unique_ptr<Texture> texture) {
	if (texture->width() != kTextureSideLength || texture->height() != kTextureSideLength) {
		throw wexception("Tried to add a texture with wrong size.");
	}
	textures_.emplace_back(std::move(texture));
}

const std::vector<std::string>& TerrainDescription::texture_paths() const {
	return texture_paths_;
}

TerrainDescription::Type TerrainDescription::get_is() const {
	return is_;
}

const std::string& TerrainDescription::name() const {
	return name_;
}

const std::string& TerrainDescription::descname() const {
	return descname_;
}

const EditorCategory& TerrainDescription::editor_category() const {
	return *editor_category_;
}

ResourceIndex TerrainDescription::get_valid_resource(uint8_t index) const {
	return valid_resources_[index];
}

int TerrainDescription::get_num_valid_resources() const {
	return valid_resources_.size();
}

bool TerrainDescription::is_resource_valid(const int res) const {
	for (const uint8_t resource_index : valid_resources_) {
		if (resource_index == res) {
			return true;
		}
	}
	return false;
}

int TerrainDescription::get_default_resource() const {
	return default_resource_index_;
}

int TerrainDescription::get_default_resource_amount() const {
	return default_resource_amount_;
}

int TerrainDescription::dither_layer() const {
	return dither_layer_;
}

double TerrainDescription::temperature() const {
	return temperature_;
}

double TerrainDescription::humidity() const {
	return humidity_;
}

double TerrainDescription::fertility() const {
	return fertility_;
}

void TerrainDescription::set_minimap_color(const RGBColor& color) {
	for (int i = -128; i < 128; i++) {
		const int shade = 128 + i;
		int new_r = std::min<int>((color.r * shade) >> 7, 255);
		int new_g = std::min<int>((color.g * shade) >> 7, 255);
		int new_b = std::min<int>((color.b * shade) >> 7, 255);
		minimap_colors_[shade] = RGBColor(new_r, new_g, new_b);
	}
}

const RGBColor& TerrainDescription::get_minimap_color(int shade) {
	assert(-128 <= shade && shade <= 127);
	return minimap_colors_[128 + shade];
}

}  // namespace Widelands
