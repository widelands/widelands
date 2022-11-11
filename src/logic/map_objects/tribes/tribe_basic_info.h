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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_TRIBE_BASIC_INFO_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_TRIBE_BASIC_INFO_H

#include <memory>

#include "logic/addons.h"
#include "scripting/lua_table.h"

namespace Widelands {

/// Basic information about the tribe that is determined only from the conf
/// file and needed before the actual game.
struct TribeBasicInfo {

	/// Script path and localized name for a starting condition
	struct Initialization {
		Initialization(const std::string& init_script,
		               const std::string& init_descname,
		               const std::string& init_tooltip,
		               const std::set<std::string>& tags,
		               const std::set<std::string>& incompatible_wc,
		               bool s)
		   : script(init_script),
		     descname(init_descname),
		     tooltip(init_tooltip),
		     required_map_tags(tags),
		     incompatible_win_conditions(incompatible_wc),
		     uses_map_starting_position(s) {
		}
		std::string script;
		std::string descname;
		std::string tooltip;
		std::set<std::string> required_map_tags;
		std::set<std::string> incompatible_win_conditions;
		bool uses_map_starting_position;
	};

	explicit TribeBasicInfo(std::unique_ptr<LuaTable> table);

	/// The add-on that defines this tribe (empty for non-add-on tribes).
	const std::string addon;
	/// Internal name to reference this tribe
	const std::string name;
	/// Filepath of the tribe's icon
	const std::string icon;
	/// Filepath of the tribe's loading script
	const std::string script;
	/// Who designed this tribe
	std::string author;
	/// Name to present to the user
	std::string descname;
	/// Basic information about this tribe
	std::string tooltip;
	/// Whether the AI should be allowed to use this tribe.
	const bool suited_for_ai;

	std::vector<Initialization> initializations;
};

using AllTribes = std::vector<TribeBasicInfo>;

/// Returns a vector with the basic info for all tribes.
AllTribes get_all_tribeinfos(const AddOns::AddOnsList*);

/// Returns the basic preload info for a tribe.
TribeBasicInfo get_tribeinfo(const std::string& tribename, const AllTribes&);

/// Returns whether this tribe is listed in tribes/initialization/<tribe>/init.lua.
bool tribe_exists(const std::string& tribename, const AllTribes&);

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_TRIBE_BASIC_INFO_H
