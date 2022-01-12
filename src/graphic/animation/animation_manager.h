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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_GRAPHIC_ANIMATION_ANIMATION_MANAGER_H
#define WL_GRAPHIC_ANIMATION_ANIMATION_MANAGER_H

#include <memory>

#include "graphic/animation/animation.h"
#include "graphic/color.h"
#include "graphic/image.h"
#include "scripting/lua_table.h"

/**
 * The animation manager manages a list of all active animations.
 */
class AnimationManager {
public:
	/**
	 * Loads an animation, graphics sound and everything from a Lua table.
	 * For the contents of the Lua table, cf. doc/sphinx/source/animations.rst or
	 * https://www.widelands.org/documentation/animations/.
	 *
	 * The 'basename' is the filename prefix for loading the images, e.g. "idle" or "walk_ne".
	 */
	uint32_t load(const LuaTable& table,
	              const std::string& basename,
	              const std::string& animation_directory,
	              Animation::Type type);
	/// Same as above, but this animation will be used for getting a representative image by map
	/// object name
	uint32_t load(const std::string& map_object_name,
	              const LuaTable& table,
	              const std::string& basename,
	              const std::string& animation_directory,
	              Animation::Type type);

	/// Returns the animation with the given ID or throws an exception if it is
	/// unknown.
	const Animation& get_animation(uint32_t id) const;

	/// Returns the representative image for the animation with the given 'id', using the given
	/// player color. If this image has already been generated, it is pulled from the cache using the
	/// clr argument that was used previously.
	const Image* get_representative_image(uint32_t id, const RGBColor* clr = nullptr);

	/// Returns the representative image for the given map object, using the given player color.
	/// If this image has already been generated, it is pulled from the cache using
	/// the clr argument that was used previously.
	const Image* get_representative_image(const std::string& map_object_name,
	                                      const RGBColor* clr = nullptr);

private:
	/// A list of all known animations
	std::vector<std::unique_ptr<Animation>> animations_;
	/// Maps the animations' vector indices + player colors to their representative images
	std::map<std::pair<uint32_t, const RGBColor*>, std::unique_ptr<const Image>>
	   representative_images_;
	/// Maps map object names to the ID of the animations that contain their representative images
	std::map<std::string, uint32_t> representative_animations_by_map_object_name_;
};

extern AnimationManager* g_animation_manager;

#endif  // end of include guard: WL_GRAPHIC_ANIMATION_ANIMATION_MANAGER_H
