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

#include "logic/map_objects/world/terrain_description.h"

#include <memory>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "graphic/animation.h"
#include "graphic/graphic.h"
#include "graphic/texture.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/world/editor_category.h"
#include "logic/map_objects/world/world.h"
#include "scripting/lua_table.h"

namespace Widelands {

namespace  {

// Parse a terrain type from the giving string.
TerrainDescription::Is terrain_type_from_string(const std::string& type) {
	if (type == "arable") {
		return TerrainDescription::Is::kArable;
	}
	if (type == "walkable") {
		return TerrainDescription::Is::kWalkable;
	}
	if (type == "water") {
		return static_cast<TerrainDescription::Is>(TerrainDescription::Is::kWater |
																	TerrainDescription::Is::kUnwalkable);
	}
	if (type == "unreachable") {
		return static_cast<TerrainDescription::Is>(TerrainDescription::Is::kUnreachable |
																	TerrainDescription::Is::kUnwalkable);
	}
	if (type == "mineable") {
		return TerrainDescription::Is::kMineable;
	}
	if (type == "unwalkable") {
		return TerrainDescription::Is::kUnwalkable;
	}
	throw LuaError((boost::format("Invalid terrain \"is\" value '%s'") % type).str());
}

}  // namespace


TerrainDescription::Type::Type(TerrainDescription::Is init_is) : is(init_is) {
	switch (is) {
	case Is::kArable:
		/** TRANSLATORS: This is a terrain type tooltip in the editor */
		descname = _("arable");
		icon = g_gr->images().get("images/wui/editor/terrain_arable.png");
		break;
	case Is::kWalkable:
		/** TRANSLATORS: This is a terrain type tooltip in the editor */
		descname = _("walkable");
		icon = g_gr->images().get("images/wui/editor/terrain_walkable.png");
		break;
	case Is::kWater:
		/** TRANSLATORS: This is a terrain type tooltip in the editor */
		descname = _("navigable");
		icon = g_gr->images().get("images/wui/editor/terrain_water.png");
		break;
	case Is::kUnreachable:
		/** TRANSLATORS: This is a terrain type tooltip in the editor */
		descname = _("unreachable");
		icon = g_gr->images().get("images/wui/editor/terrain_unreachable.png");
		break;
	case Is::kMineable:
		/** TRANSLATORS: This is a terrain type tooltip in the editor */
		descname = _("mineable");
		icon = g_gr->images().get("images/wui/editor/terrain_mineable.png");
		break;
	case Is::kUnwalkable:
		/** TRANSLATORS: This is a terrain type tooltip in the editor */
		descname = _("unwalkable");
		icon = g_gr->images().get("images/wui/editor/terrain_unwalkable.png");
		break;
	}
}

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

	if (table.has_key("tooltips")) {
		custom_tooltips_ = table.get_table("tooltips")->array_entries<std::string>();
	}

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
	if (editor_category_index == Widelands::INVALID_INDEX) {
		throw GameDataError(
		   "Unknown editor_category: %s\n", table.get_string("editor_category").c_str());
	}
	editor_category_ = world.editor_terrain_categories().get_mutable(editor_category_index);
}

TerrainDescription::~TerrainDescription() {
}

const Image& TerrainDescription::get_texture(uint32_t gametime) const {
	return *textures_.at((gametime / frame_length_) % textures_.size());
}

void TerrainDescription::add_texture(const Image* texture) {
	if (texture->width() != kTextureSideLength || texture->height() != kTextureSideLength) {
		throw wexception("Tried to add a texture with wrong size.");
	}
	textures_.emplace_back(texture);
}

const std::vector<std::string>& TerrainDescription::texture_paths() const {
	return texture_paths_;
}

TerrainDescription::Is TerrainDescription::get_is() const {
	return is_;
}


const std::vector<TerrainDescription::Type> TerrainDescription::get_types() const {
	std::vector<TerrainDescription::Type> terrain_types;

	if (is_ == Widelands::TerrainDescription::Is::kArable) {
		terrain_types.push_back(TerrainDescription::Type(TerrainDescription::Is::kArable));
	}
	if (is_ & Widelands::TerrainDescription::Is::kWalkable) {
		terrain_types.push_back(TerrainDescription::Type(TerrainDescription::Is::kWalkable));
	}
	if (is_ & Widelands::TerrainDescription::Is::kWater) {
		terrain_types.push_back(TerrainDescription::Type(TerrainDescription::Is::kWater));
	}
	if (is_ & Widelands::TerrainDescription::Is::kUnreachable) {
		terrain_types.push_back(TerrainDescription::Type(TerrainDescription::Is::kUnreachable));
	}
	if (is_ & Widelands::TerrainDescription::Is::kMineable) {
		terrain_types.push_back(TerrainDescription::Type(TerrainDescription::Is::kMineable));
	}
	if (is_ & Widelands::TerrainDescription::Is::kUnwalkable) {
		terrain_types.push_back(TerrainDescription::Type(TerrainDescription::Is::kUnwalkable));
	}
	return terrain_types;
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

DescriptionIndex TerrainDescription::get_valid_resource(uint8_t index) const {
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
