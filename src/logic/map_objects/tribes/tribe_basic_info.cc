/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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
#include "base/log.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/addons.h"
#include "logic/filesystem_constants.h"
#include "logic/game_data_error.h"
#include "scripting/lua_interface.h"

namespace Widelands {

TribeBasicInfo::TribeBasicInfo(std::unique_ptr<LuaTable> table)
   : name(table->get_string("name")),
     icon(table->get_string("icon")),
     script(table->get_string("script")) {
	try {
		author = table->get_string("author");
		descname = table->get_string("descname");
		tooltip = table->get_string("tooltip");
		std::unique_ptr<LuaTable> starting_conditions = table->get_table("starting_conditions");
		LuaInterface lua;

		for (const std::string& script_path : starting_conditions->array_entries<std::string>()) {
			std::unique_ptr<LuaTable> script_table = lua.run_script(script_path);
			script_table->do_not_warn_about_unaccessed_keys();
			std::set<std::string> tags;
			if (script_table->has_key("map_tags")) {
				std::unique_ptr<LuaTable> t = script_table->get_table("map_tags");
				for (int key : t->keys<int>()) {
					tags.insert(t->get_string(key));
				}
			}
			initializations.push_back(Initialization(script_path, script_table->get_string("descname"),
			                                         script_table->get_string("tooltip"), tags));
		}
		for (const auto& pair : g_addons) {
			if (pair.first.category == AddOnCategory::kStartingCondition) {
				const std::string script_path = kAddOnDir + FileSystem::file_separator() +
				                                pair.first.internal_name +
				                                FileSystem::file_separator() + name + ".lua";
				if (!g_fs->file_exists(script_path)) {
					continue;
				}
				std::unique_ptr<LuaTable> script_table = lua.run_script(script_path);
				script_table->do_not_warn_about_unaccessed_keys();
				std::set<std::string> tags;
				if (script_table->has_key("map_tags")) {
					std::unique_ptr<LuaTable> t = script_table->get_table("map_tags");
					for (int key : t->keys<int>()) {
						tags.insert(t->get_string(key));
					}
				}
				initializations.push_back(Initialization(script_path,
				                                         script_table->get_string("descname"),
				                                         script_table->get_string("tooltip"), tags));
			}
		}
	} catch (const WException& e) {
		throw Widelands::GameDataError(
		   "reading basic info for tribe \"%s\": %s", name.c_str(), e.what());
	}
}

std::vector<TribeBasicInfo> get_all_tribeinfos() {
	std::vector<TribeBasicInfo> tribeinfos;
	LuaInterface lua;

	if (g_fs->is_directory("tribes/initialization")) {
		FilenameSet dirs = g_fs->list_directory("tribes/initialization");
		for (const std::string& dir : dirs) {
			for (const std::string& file : g_fs->list_directory(dir)) {
				if (strcmp(FileSystem::fs_filename(file.c_str()), "init.lua") == 0) {
					tribeinfos.push_back(Widelands::TribeBasicInfo(lua.run_script(file)));
				}
			}
		}
	}

	if (tribeinfos.empty()) {
		log_err("No tribe infos found at 'tribes/initialization/<tribename>/init.lua'");
	}

	for (const auto& pair : g_addons) {
		if (pair.first.category == AddOnCategory::kTribes && pair.second) {
			const std::string dirname = kAddOnDir + FileSystem::file_separator() +
			                            pair.first.internal_name + FileSystem::file_separator() +
			                            "tribes";
			if (g_fs->is_directory(dirname)) {
				for (const std::string& tribe : g_fs->list_directory(dirname)) {
					const std::string script_path = tribe + FileSystem::file_separator() + "init.lua";
					if (g_fs->file_exists(script_path)) {
						tribeinfos.push_back(Widelands::TribeBasicInfo(lua.run_script(script_path)));
					}
				}
			}
		}
	}

	return tribeinfos;
}

TribeBasicInfo get_tribeinfo(const std::string& tribename) {
	for (const TribeBasicInfo& info : Widelands::get_all_tribeinfos()) {
		if (info.name == tribename) {
			return info;
		}
	}
	throw GameDataError("The tribe '%s'' does not exist.", tribename.c_str());
}

bool tribe_exists(const std::string& tribename, std::vector<TribeBasicInfo> tribeinfos) {
	for (const auto& tribeinfo : tribeinfos) {
		if (tribeinfo.name == tribename) {
			return true;
		}
	}
	return false;
}

}  // namespace Widelands
