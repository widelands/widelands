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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "editor/editor_category.h"

#include "graphic/image_cache.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game_data_error.h"
#include "scripting/lua_table.h"

EditorCategory::EditorCategory(const LuaTable& table,
                               Widelands::MapObjectType type,
                               Widelands::Descriptions& descriptions)
   : name_(table.get_string("name")),
     descname_(table.get_string("descname")),
     image_file_(table.get_string("picture")),
     items_per_row_(table.get_int("items_per_row")) {
	if (!g_fs->file_exists(image_file_)) {
		throw Widelands::GameDataError(
		   "EditorCategory %s has non-existing \"picture\".", name_.c_str());
	}
	if (items_per_row_ <= 0) {
		throw Widelands::GameDataError(
		   "EditorCategory %s has less than 1 item per row.", name_.c_str());
	}

	for (const std::string& item : table.get_table("items")->array_entries<std::string>()) {
		switch (type) {
		case Widelands::MapObjectType::CRITTER:
			items_.push_back(descriptions.load_critter(item));
			break;
		case Widelands::MapObjectType::IMMOVABLE:
			items_.push_back(descriptions.load_immovable(item));
			break;
		case Widelands::MapObjectType::TERRAIN:
			items_.push_back(descriptions.load_terrain(item));
			break;
		default:
			NEVER_HERE();
		}
	}
}

const std::string& EditorCategory::name() const {
	return name_;
}

const std::string& EditorCategory::descname() const {
	return descname_;
}

const Image* EditorCategory::picture() const {
	const Image* image = g_image_cache->get(image_file_);
	assert(image);
	return image;
}

int EditorCategory::items_per_row() const {
	return items_per_row_;
}

const std::vector<Widelands::DescriptionIndex>& EditorCategory::items() const {
	return items_;
}
