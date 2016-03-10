/*
 * Copyright (C) 2002, 2006-2015 by the Widelands Development Team
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

#include "logic/map_objects/tribes/tribe_basic_info.h"

#include <memory>

#include "base/i18n.h"
#include "logic/game_data_error.h"
#include "scripting/lua_interface.h"

TribeBasicInfo::TribeBasicInfo(std::unique_ptr<LuaTable> table) {
	try {
		i18n::Textdomain td("tribes");
		name = table->get_string("name");
		author = _(table->get_string("author"));
		descname = _(table->get_string("descname"));
		tooltip = _(table->get_string("tooltip"));
		icon = table->get_string("icon");
		std::unique_ptr<LuaTable> starting_conditions = table->get_table("starting_conditions");
		LuaInterface lua;

		for (const std::string& script_path : starting_conditions->array_entries<std::string>())
		{
			std::unique_ptr<LuaTable> script_table = lua.run_script(script_path);
			script_table->do_not_warn_about_unaccessed_keys();
			initializations.push_back(Initialization(script_path,
																  script_table->get_string("descname"),
																  script_table->get_string("tooltip")));
		}
	} catch (const WException & e) {
		throw Widelands::GameDataError
			("reading basic info for tribe \"%s\": %s",
			 name.c_str(), e.what());
	}
}
