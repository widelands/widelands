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

#ifndef WL_LOGIC_MAP_OBJECTS_WORLD_RESOURCE_DESCRIPTION_H
#define WL_LOGIC_MAP_OBJECTS_WORLD_RESOURCE_DESCRIPTION_H

#include <string>

#include "base/macros.h"
#include "logic/widelands.h"

class LuaTable;

namespace Widelands {

class ResourceDescription {
public:
	struct EditorPicture {
		std::string picname;
		int upper_limit;
	};

	explicit ResourceDescription(const LuaTable& table);

	/// Returns the in engine name of this resource.
	const std::string& name() const;

	/// Returns the name of this resource for users. Usually translated.
	const std::string& descname() const;

	/// Returns if this resource is detectable by a geologist.
	bool detectable() const;

	/// Returns the time for which nearby geologist messages for this resource are muted
	uint32_t timeout_ms() const;

	/// Returns the radius within which geologist messages for this resource are temporarily muted
	uint32_t timeout_radius() const;

	/// Returns the maximum amount that can be in a field for this resource.
	ResourceAmount max_amount() const;

	/// Returns the path to the image that should be used in the editor to
	/// represent an 'amount' of this resource.
	const std::string& editor_image(uint32_t amount) const;

	/// Returns the path to the image that should be used in menus to represent this resource
	const std::string& representative_image() const {
		return representative_image_;
	}

	void set_max_amount(ResourceAmount r) {
		max_amount_ = r;
	}

private:
	const std::string name_;
	const std::string descname_;
	const bool detectable_;
	const uint32_t timeout_ms_;
	const uint32_t timeout_radius_;
	ResourceAmount max_amount_;
	const std::string representative_image_;
	std::vector<EditorPicture> editor_pictures_;

	DISALLOW_COPY_AND_ASSIGN(ResourceDescription);
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_WORLD_RESOURCE_DESCRIPTION_H
