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

#ifndef WL_EDITOR_EDITOR_CATEGORY_H
#define WL_EDITOR_EDITOR_CATEGORY_H

#include <string>
#include <vector>

#include "base/macros.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/map_object_type.h"
#include "logic/widelands.h"

class Image;

/// Represents a category for grouping items in the Editor, so purely a UI
/// distinction and not a logical one.
class EditorCategory {
public:
	explicit EditorCategory(const LuaTable& table,
	                        Widelands::MapObjectType type,
	                        Widelands::Descriptions& descriptions);

	/// Internal name.
	const std::string& name() const;

	/// User facing (translated) name.
	const std::string& descname() const;

	/// The menu image for the category.
	const Image* picture() const;

	/// The number of items displayed in each row.
	int items_per_row() const;

	const std::vector<Widelands::DescriptionIndex>& items() const;

private:
	const std::string name_;
	const std::string descname_;
	const std::string image_file_;
	const int items_per_row_;
	std::vector<Widelands::DescriptionIndex> items_;
	DISALLOW_COPY_AND_ASSIGN(EditorCategory);
};

#endif  // end of include guard: WL_EDITOR_EDITOR_CATEGORY_H
