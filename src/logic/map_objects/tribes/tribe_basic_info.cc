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
#include "io/filesystem/layered_filesystem.h"
#include "logic/addons.h"
#include "logic/filesystem_constants.h"
#include "logic/game_data_error.h"
#include "scripting/lua_interface.h"

namespace Widelands {

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
				const std::string script_path = kAddOnDir + g_fs->file_separator() + pair.first.internal_name + g_fs->file_separator() + name + ".lua";
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
				initializations.push_back(Initialization(script_path, script_table->get_string("descname"),
					                                     script_table->get_string("tooltip"), tags));
			}
		}
	} catch (const WException& e) {
		throw Widelands::GameDataError(
		   "reading basic info for tribe \"%s\": %s", name.c_str(), e.what());
	}
}

static std::vector<std::string> get_preload_scripts() {
	std::vector<std::string> v = {"tribes/preload.lua"};
	for (const auto& pair : g_addons) {
		if (pair.first.category == AddOnCategory::kTribes && pair.second) {
			const std::string script_path = kAddOnDir + g_fs->file_separator() + pair.first.internal_name + g_fs->file_separator() + "preload.lua";
			if (g_fs->file_exists(script_path)) {
				v.push_back(script_path);
			}
		}
	}
	return v;
}

std::vector<std::string> get_all_tribenames() {
	std::vector<std::string> tribenames;
	LuaInterface lua;
	for (std::string script : get_preload_scripts()) {
		std::unique_ptr<LuaTable> table(lua.run_script(script));
		for (const int key : table->keys<int>()) {
			std::unique_ptr<LuaTable> info = table->get_table(key);
			info->do_not_warn_about_unaccessed_keys();
			tribenames.push_back(info->get_string("name"));
		}
	}
	return tribenames;
}

std::vector<TribeBasicInfo> get_all_tribeinfos() {
	std::vector<TribeBasicInfo> tribeinfos;
	LuaInterface lua;
	for (std::string script : get_preload_scripts()) {
		std::unique_ptr<LuaTable> table(lua.run_script(script));
		for (const int key : table->keys<int>()) {
			tribeinfos.push_back(TribeBasicInfo(table->get_table(key)));
		}
	}
	return tribeinfos;
}

TribeBasicInfo get_tribeinfo(const std::string& tribename) {
	if (Widelands::tribe_exists(tribename)) {
		for (const TribeBasicInfo& info : Widelands::get_all_tribeinfos()) {
			if (info.name == tribename) {
				return info;
			}
		}
	}
	throw GameDataError("The tribe '%s'' does not exist.", tribename.c_str());
}

bool tribe_exists(const std::string& tribename) {
	for (const std::string& name : get_all_tribenames()) {
		if (name == tribename) {
			return true;
		}
	}
	return false;
}

}  // namespace Widelands
