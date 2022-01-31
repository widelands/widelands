/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "graphic/animation/animation_manager.h"

#include <memory>

#include "base/log.h"
#include "base/multithreading.h"
#include "graphic/animation/nonpacked_animation.h"
#include "graphic/animation/spritesheet_animation.h"
#include "graphic/texture.h"

AnimationManager* g_animation_manager;

uint32_t AnimationManager::load(const LuaTable& table,
                                const std::string& basename,
                                const std::string& animation_directory,
                                Animation::Type type) {
	switch (type) {
	case Animation::Type::kFiles:
		animations_.push_back(
		   std::unique_ptr<Animation>(new NonPackedAnimation(table, basename, animation_directory)));
		break;
	case Animation::Type::kSpritesheet:
		animations_.push_back(std::unique_ptr<Animation>(
		   new SpriteSheetAnimation(table, basename, animation_directory)));
	}
	return animations_.size();
}
uint32_t AnimationManager::load(const std::string& map_object_name,
                                const LuaTable& table,
                                const std::string& basename,
                                const std::string& animation_directory,
                                Animation::Type type) {
	const size_t result = load(table, basename, animation_directory, type);
	representative_animations_by_map_object_name_.insert(std::make_pair(map_object_name, result));
	return result;
}

const Animation& AnimationManager::get_animation(uint32_t id) const {
	if (!id || id > animations_.size()) {
		throw wexception("Requested unknown animation with id: %i", id);
	}
	return *animations_[id - 1];
}

const Image* AnimationManager::get_representative_image(uint32_t id, const RGBColor* clr) {
	const auto hash = std::make_pair(id, clr);
	if (representative_images_.count(hash) != 1) {
		NoteThreadSafeFunction::instantiate(
		   [this, id, hash, clr]() {
			   representative_images_.insert(std::make_pair(
			      hash, std::unique_ptr<const Image>(
			               g_animation_manager->get_animation(id).representative_image(clr))));
		   },
		   true);
	}
	return representative_images_.at(hash).get();
}

const Image* AnimationManager::get_representative_image(const std::string& map_object_name,
                                                        const RGBColor* clr) {
	if (representative_animations_by_map_object_name_.count(map_object_name) != 1) {
		log_warn("%s has no animation assigned for its representative image, or it's not a known "
		         "map object\n",
		         map_object_name.c_str());
		return new Texture(0, 0);
	}
	return get_representative_image(
	   representative_animations_by_map_object_name_.at(map_object_name), clr);
}
