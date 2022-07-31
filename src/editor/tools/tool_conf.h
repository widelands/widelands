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

#ifndef WL_EDITOR_TOOLS_TOOL_CONF_H
#define WL_EDITOR_TOOLS_TOOL_CONF_H

#include <set>
#include <string>

#include "logic/widelands.h"
#include "logic/widelands_geometry.h"

class EditorTool;

struct ToolConf {
	ToolConf();
	ToolConf(const ToolConf&) = default;
	ToolConf& operator=(const ToolConf&) = default;

	/// Returns a description of the conf's content in non-locale-dependent form.
	std::string to_key() const;

	EditorTool* primary;

	int32_t change_by;
	Widelands::ResourceAmount set_to;
	Widelands::HeightInterval interval;
	std::set<Widelands::DescriptionIndex> map_obj_types;
	Widelands::DescriptionIndex resource;
};

#endif  // end of include guard: WL_EDITOR_TOOLS_TOOL_CONF_H
