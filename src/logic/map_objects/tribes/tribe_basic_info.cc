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

#include "logic/map_objects/tribes/tribe_basic_info.h"

#include <memory>

#include "base/i18n.h"
#include "base/log.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/filesystem_constants.h"
#include "logic/game_data_error.h"
#include "scripting/lua_interface.h"

namespace Widelands {

TribeBasicInfo::TribeBasicInfo(std::unique_ptr<LuaTable> table)
   : name(table->get_string("name")),
     icon(table->get_string("icon")),
     script(table->get_string("script")),
     suited_for_ai(!table->has_key("suited_for_ai") || table->get_bool("suited_for_ai")) {
	try {
		author = table->get_string("author");
		descname = table->get_string("descname");
		tooltip = table->get_string("tooltip");
		std::unique_ptr<LuaTable> starting_conditions = table->get_table("starting_conditions");
		LuaInterface lua;

		for (const std::string& script_path : starting_conditions->array_entries<std::string>()) {
			std::unique_ptr<LuaTable> script_table = lua.run_script(script_path);
			script_table->do_not_warn_about_unaccessed_keys();
			// TODO(hessenfarmer): This initialization code is duplicated in Addons below and in
			// gameclient. Should be pulled out to a common class.
			std::set<std::string> tags;
			std::set<std::string> incompatible_wc;
			if (script_table->has_key("map_tags")) {
				std::unique_ptr<LuaTable> t = script_table->get_table("map_tags");
				for (int key : t->keys<int>()) {
					tags.insert(t->get_string(key));
				}
			}
			if (script_table->has_key("incompatible_wc")) {
				std::unique_ptr<LuaTable> w = script_table->get_table("incompatible_wc");
				for (int key : w->keys<int>()) {
					incompatible_wc.insert(w->get_string(key));
				}
			}
			initializations.emplace_back(script_path, script_table->get_string("descname"),
			                             script_table->get_string("tooltip"), tags, incompatible_wc,
			                             !script_table->has_key("uses_map_starting_position") ||
			                                script_table->get_bool("uses_map_starting_position"));
		}
		for (const auto& pair : AddOns::g_addons) {
			if (pair.first->category == AddOns::AddOnCategory::kStartingCondition && pair.second) {
				const std::string script_path = kAddOnDir + FileSystem::file_separator() +
				                                pair.first->internal_name +
				                                FileSystem::file_separator() + name + ".lua";
				if (!g_fs->file_exists(script_path)) {
					continue;
				}
				std::unique_ptr<LuaTable> script_table = lua.run_script(script_path);
				script_table->do_not_warn_about_unaccessed_keys();
				// TODO(hessenfarmer): Needs to be pulled out as it is duplicated
				std::set<std::string> tags;
				std::set<std::string> incompatible_wc;
				if (script_table->has_key("map_tags")) {
					std::unique_ptr<LuaTable> t = script_table->get_table("map_tags");
					for (int key : t->keys<int>()) {
						tags.insert(t->get_string(key));
					}
				}
				if (script_table->has_key("incompatible_wc")) {
					std::unique_ptr<LuaTable> w = script_table->get_table("incompatible_wc");
					for (int key : w->keys<int>()) {
						incompatible_wc.insert(w->get_string(key));
					}
				}
				initializations.emplace_back(script_path, script_table->get_string("descname"),
				                             script_table->get_string("tooltip"), tags, incompatible_wc,
				                             !script_table->has_key("uses_map_starting_position") ||
				                                script_table->get_bool("uses_map_starting_position"));
			}
		}
	} catch (const WException& e) {
		throw Widelands::GameDataError(
		   "reading basic info for tribe \"%s\": %s", name.c_str(), e.what());
	}
}

AllTribes get_all_tribeinfos(const AddOns::AddOnsList* addons_to_consider) {
	AllTribes tribeinfos;
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

	const AddOns::AddOnsList* addons;
	AddOns::AddOnsList enabled_tribe_addons;
	if (addons_to_consider != nullptr) {
		addons = addons_to_consider;
	} else {
		for (auto& pair : AddOns::g_addons) {
			if (pair.first->category == AddOns::AddOnCategory::kTribes && pair.second) {
				enabled_tribe_addons.push_back(pair.first);
			}
		}
		addons = &enabled_tribe_addons;
	}

	for (const auto& a : *addons) {
		const std::string dirname = kAddOnDir + FileSystem::file_separator() + a->internal_name +
		                            FileSystem::file_separator() + "tribes";
		if (g_fs->is_directory(dirname)) {
			for (const std::string& tribe : g_fs->list_directory(dirname)) {
				const std::string script_path = tribe + FileSystem::file_separator() + "init.lua";
				if (g_fs->file_exists(script_path)) {
					tribeinfos.push_back(Widelands::TribeBasicInfo(lua.run_script(script_path)));
				}
			}
		}
	}

	return tribeinfos;
}

TribeBasicInfo get_tribeinfo(const std::string& tribename, const AllTribes& all) {
	for (const TribeBasicInfo& info : all) {
		if (info.name == tribename) {
			return info;
		}
	}
	throw GameDataError("The tribe '%s' does not exist.", tribename.c_str());
}

bool tribe_exists(const std::string& tribename, const AllTribes& tribeinfos) {
	for (const auto& tribeinfo : tribeinfos) {
		if (tribeinfo.name == tribename) {
			return true;
		}
	}
	return false;
}

}  // namespace Widelands
