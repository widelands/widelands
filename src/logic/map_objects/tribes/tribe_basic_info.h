/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_TRIBE_BASIC_INFO_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_TRIBE_BASIC_INFO_H

#include <memory>

#include "scripting/lua_table.h"

namespace Widelands {

/// Basic information about the tribe that is determined only from the conf
/// file and needed before the actual game.
struct TribeBasicInfo {

	/// Script path and localized name for a starting condition
	struct Initialization {
		Initialization(const std::string& init_script,
		               const std::string& init_descname,
		               const std::string& init_tooltip)
		   : script(init_script), descname(init_descname), tooltip(init_tooltip) {
		}
		std::string script;
		std::string descname;
		std::string tooltip;
	};

	explicit TribeBasicInfo(std::unique_ptr<LuaTable> table);

	/// Internal name to reference this tribe
	std::string name;
	/// Who designed this tribe
	std::string author;
	/// Name to present to the user
	std::string descname;
	/// Basic information about this tribe
	std::string tooltip;
	/// Filepath of the tribe's icon
	std::string icon;

	std::vector<Initialization> initializations;
};

/// Returns a string vector with the names of all tribes.
std::vector<std::string> get_all_tribenames();

/// Returns a vector with the basic info for all tribes.
std::vector<TribeBasicInfo> get_all_tribeinfos();

/// Returns the basic preload info for a tribe.
TribeBasicInfo get_tribeinfo(const std::string& tribename);

/// Returns whether this tribe is listed in tribes/preload.lua.
bool tribe_exists(const std::string& tribename);

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_TRIBE_BASIC_INFO_H
