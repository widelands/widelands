/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "logic/map_objects/world/terrain_description.h"

#include <memory>

#include <SDL_surface.h>

#include "base/i18n.h"
#include "base/log.h"
#include "graphic/animation/animation.h"
#include "graphic/image_cache.h"
#include "graphic/image_io.h"
#include "graphic/texture.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/descriptions.h"
#include "scripting/lua_table.h"

namespace Widelands {

namespace {

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
	throw LuaError(format("Invalid terrain \"is\" value '%s'", type));
}

}  // namespace

TerrainDescription::Type::Type(TerrainDescription::Is init_is) : is(init_is) {
	switch (is) {
	case Is::kArable:
		/** TRANSLATORS: This is a terrain type tooltip in the editor */
		descname = _("arable");
		icon = g_image_cache->get("images/wui/editor/terrain_arable.png");
		break;
	case Is::kWalkable:
		/** TRANSLATORS: This is a terrain type tooltip in the editor */
		descname = _("walkable");
		icon = g_image_cache->get("images/wui/editor/terrain_walkable.png");
		break;
	case Is::kWater:
		/** TRANSLATORS: This is a terrain type tooltip in the editor */
		descname = _("navigable");
		icon = g_image_cache->get("images/wui/editor/terrain_water.png");
		break;
	case Is::kUnreachable:
		/** TRANSLATORS: This is a terrain type tooltip in the editor */
		descname = _("unreachable");
		icon = g_image_cache->get("images/wui/editor/terrain_unreachable.png");
		break;
	case Is::kMineable:
		/** TRANSLATORS: This is a terrain type tooltip in the editor */
		descname = _("mineable");
		icon = g_image_cache->get("images/wui/editor/terrain_mineable.png");
		break;
	case Is::kUnwalkable:
		/** TRANSLATORS: This is a terrain type tooltip in the editor */
		descname = _("unwalkable");
		icon = g_image_cache->get("images/wui/editor/terrain_unwalkable.png");
		break;
	}
}

TerrainDescription::TerrainDescription(const LuaTable& table,
                                       Descriptions& descriptions,
                                       const uint16_t dither_layer_disambiguator)
   : name_(table.get_string("name")),
     descname_(table.get_string("descname")),
     is_(terrain_type_from_string(table.get_string("is"))),
     default_resource_amount_(table.get_int("default_resource_amount")),
     dither_layer_(table.get_int("dither_layer") * kMaxDitherLayerDisambiguator +
                   dither_layer_disambiguator),
     temperature_(table.get_int("temperature")),
     fertility_(table.get_int("fertility")),
     humidity_(table.get_int("humidity")) {
	if (dither_layer_disambiguator >= kMaxDitherLayerDisambiguator) {
		throw wexception("Terrain %s: dither layer disambiguator %u exceeds maximum of %u",
		                 name_.c_str(), dither_layer_disambiguator, kMaxDitherLayerDisambiguator);
	}

	if (table.has_key("tooltips")) {
		// TODO(GunChleoc): Compatibility, remove after v1.0
		log_warn("Terrain '%s' contains obsolete 'tooltips' table", name().c_str());
	}

	for (DescriptionIndex di = descriptions.nr_terrains(); di; --di) {
		const TerrainDescription* t = descriptions.get_terrain_descr(di - 1);
		if (t->dither_layer_ == dither_layer_) {
			throw GameDataError("Terrain %s has the same dither layer %i as %s", name_.c_str(),
			                    dither_layer_, t->name_.c_str());
		}
	}

	if (table.has_key("enhancement")) {
		std::unique_ptr<LuaTable> t = table.get_table("enhancement");
		for (const std::string& key : t->keys<std::string>()) {
			set_enhancement(key, t->get_string(key));
		}
	}

	if (!(0 < fertility_ && fertility_ < 1000)) {
		throw GameDataError("%s: fertility is not in (0, 1000).", name_.c_str());
	}
	if (!(0 < humidity_ && humidity_ < 1000)) {
		throw GameDataError("%s: humidity is not in (0, 1000).", name_.c_str());
	}
	if (temperature_ < 0) {
		throw GameDataError("%s: temperature is not possible.", name_.c_str());
	}

	for (const std::string& resource :
	     table.get_table("valid_resources")->array_entries<std::string>()) {
		valid_resources_.push_back(descriptions.load_resource(resource));
	}

	const std::string default_resource(table.get_string("default_resource"));
	default_resource_index_ = !default_resource.empty() ?
                                descriptions.load_resource(default_resource) :
                                Widelands::INVALID_INDEX;

	if (default_resource_amount_ > 0 && !is_resource_valid(default_resource_index_)) {
		throw GameDataError("Default resource is not in valid resources.\n");
	}

	replace_textures(table);
}

void TerrainDescription::replace_textures(const LuaTable& table) {
	texture_paths_.clear();
	textures_.clear();
	// Note: Terrain texures are loaded in "graphic/build_texture_atlas.h"

	texture_paths_ = table.get_table("textures")->array_entries<std::string>();
	frame_length_ = kFrameLength;
	if (texture_paths_.empty()) {
		throw GameDataError("Terrain %s has no images.", name_.c_str());
	}
	if (texture_paths_.size() == 1) {
		if (table.has_key("fps")) {
			throw GameDataError("Terrain %s with one images must not have fps.", name_.c_str());
		}
	} else {
		frame_length_ = 1000 / get_positive_int(table, "fps");
	}

	for (size_t j = 0; j < texture_paths().size(); ++j) {
		// Set the minimap color on the first loaded image.
		if (j == 0) {
			SDL_Surface* sdl_surface = load_image_as_sdl_surface(texture_paths()[j]);
			uint8_t top_left_pixel = static_cast<uint8_t*>(sdl_surface->pixels)[0];
			const SDL_Color top_left_pixel_color =
			   sdl_surface->format->palette->colors[top_left_pixel];
			set_minimap_color(
			   RGBColor(top_left_pixel_color.r, top_left_pixel_color.g, top_left_pixel_color.b));
			SDL_FreeSurface(sdl_surface);
		}
		add_texture(g_image_cache->get(texture_paths()[j]));
	}
}

void TerrainDescription::set_enhancement(const std::string& cat, const std::string& e) {
	if (e == name_) {
		throw GameDataError("%s: a terrain cannot be enhanced to itself", name_.c_str());
	}

	if (e.empty()) {
		auto it = enhancement_.find(cat);
		if (it != enhancement_.end()) {
			enhancement_.erase(it);
		}
		return;
	}

	// Ensure terrain exists and is loaded
	enhancement_[cat] = e;
	if (!e.empty()) {
		Notifications::publish(
		   NoteMapObjectDescription(e, NoteMapObjectDescription::LoadType::kObject));
	}
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

DescriptionIndex TerrainDescription::get_valid_resource(DescriptionIndex index) const {
	return valid_resources_[index];
}

size_t TerrainDescription::get_num_valid_resources() const {
	return valid_resources_.size();
}

std::vector<DescriptionIndex> TerrainDescription::valid_resources() const {
	return valid_resources_;
}

bool TerrainDescription::is_resource_valid(const DescriptionIndex res) const {
	for (const DescriptionIndex resource_index : valid_resources_) {
		if (resource_index == res) {
			return true;
		}
	}
	return false;
}

DescriptionIndex TerrainDescription::get_default_resource() const {
	return default_resource_index_;
}

ResourceAmount TerrainDescription::get_default_resource_amount() const {
	return default_resource_amount_;
}

int TerrainDescription::dither_layer() const {
	return dither_layer_;
}

int TerrainDescription::temperature() const {
	return temperature_;
}

int TerrainDescription::humidity() const {
	return humidity_;
}

int TerrainDescription::fertility() const {
	return fertility_;
}

std::string TerrainDescription::enhancement(const std::string& key) const {
	const auto it = enhancement_.find(key);
	return it == enhancement_.end() ? "" : it->second;
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

const RGBColor& TerrainDescription::get_minimap_color(int shade) const {
	assert(-128 <= shade && shade <= 127);
	return minimap_colors_[128 + shade];
}

}  // namespace Widelands
