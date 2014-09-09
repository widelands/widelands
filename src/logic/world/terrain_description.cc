/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#include <boost/format.hpp>

#include "graphic/graphic.h"
#include "logic/game_data_error.h"
#include "logic/world/editor_category.h"
#include "logic/world/world.h"
#include "profile/profile.h"
#include "scripting/lua_table.h"

namespace Widelands {

namespace  {

// Parse a terrain type from the giving string.
TerrainDescription::Type TerrainTypeFromString(const std::string& type) {
	if (type == "green") {
		return TerrainDescription::GREEN;
	}
	if (type == "dry") {
		return TerrainDescription::DRY;
	}
	if (type == "water") {
		return static_cast<TerrainDescription::Type>(
		   TerrainDescription::WATER | TerrainDescription::DRY | TerrainDescription::UNPASSABLE);
	}
	if (type == "acid") {
		return static_cast<TerrainDescription::Type>(
		   TerrainDescription::ACID | TerrainDescription::DRY | TerrainDescription::UNPASSABLE);
	}
	if (type == "mountain") {
		return static_cast<TerrainDescription::Type>(TerrainDescription::DRY |
		                                             TerrainDescription::MOUNTAIN);
	}
	if (type == "dead") {
		return static_cast<TerrainDescription::Type>(
		   TerrainDescription::DRY | TerrainDescription::UNPASSABLE | TerrainDescription::ACID);
	}
	if (type == "unpassable") {
		return static_cast<TerrainDescription::Type>(TerrainDescription::DRY |
		                                             TerrainDescription::UNPASSABLE);
	}
	throw LuaError((boost::format("invalid terrain type '%s'") % type).str());
}

}  // namespace

TerrainDescription::TerrainDescription(const LuaTable& table, const Widelands::World& world)
   : name_(table.get_string("name")),
     descname_(table.get_string("descname")),
     is_(TerrainTypeFromString(table.get_string("is"))),
     default_resource_index_(world.get_resource(table.get_string("default_resource").c_str())),
     default_resource_amount_(table.get_int("default_resource_amount")),
     dither_layer_(table.get_int("dither_layer")),
     temperature_(table.get_double("temperature")),
     fertility_(table.get_double("fertility")),
     humidity_(table.get_double("humidity")) {

	if (!(0 <= fertility_ && fertility_ <= 1.)) {
		throw GameDataError("%s: fertility is not in [0, 1].", name_.c_str());
	}
	if (!(0 <= humidity_ && humidity_ <= 1.)) {
		throw GameDataError("%s: humidity is not in [0, 1].", name_.c_str());
	}
	if (temperature_ < 0) {
		throw GameDataError("%s: temperature is not in Kelvin.", name_.c_str());
	}

	const std::vector<std::string> textures =
	   table.get_table("textures")->array_entries<std::string>();
	int frame_length = FRAME_LENGTH;
	if (textures.empty()) {
		throw GameDataError("Terrain %s has no images.", name_.c_str());
	} else if (textures.size() == 1) {
		if (table.has_key("fps")) {
			throw GameDataError("Terrain %s with one images must not have fps.", name_.c_str());
		}
	} else {
		frame_length = 1000 / get_positive_int(table, "fps");
	}
	texture_ = g_gr->new_maptexture(textures, frame_length);

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

uint32_t TerrainDescription::get_texture() const {
	return texture_;
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

Resource_Index TerrainDescription::get_valid_resource(uint8_t index) const {
	return valid_resources_[index];
}

int TerrainDescription::get_num_valid_resources() const {
	return valid_resources_.size();
}

bool TerrainDescription::is_resource_valid(const int32_t res) const {
	for (const uint8_t resource_index : valid_resources_) {
		if (resource_index == res) {
			return true;
		}
	}
	return false;
}

int8_t TerrainDescription::get_default_resource() const {
	return default_resource_index_;
}

int32_t TerrainDescription::get_default_resource_amount() const {
	return default_resource_amount_;
}

int32_t TerrainDescription::dither_layer() const {
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

}  // namespace Widelands
