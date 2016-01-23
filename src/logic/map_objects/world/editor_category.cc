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

#include "logic/map_objects/world/editor_category.h"

#include "graphic/graphic.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game_data_error.h"
#include "scripting/lua_table.h"

namespace Widelands {

EditorCategory::EditorCategory(const LuaTable& table)
   : name_(table.get_string("name")),
     descname_(table.get_string("descname")),
     image_file_(table.get_string("picture")) {
	if (!g_fs->file_exists(image_file_)) {
		throw GameDataError("EditorCategory %s has non-existing \"picture\".", name_.c_str());
	}
}

const std::string& EditorCategory::name() const {
	return name_;
}

const std::string& EditorCategory::descname() const {
	return descname_;
}

const Image* EditorCategory::picture() const {
	const Image* image = g_gr->images().get(image_file_);
	assert(image);
	return image;
}

}  // namespace Widelands
