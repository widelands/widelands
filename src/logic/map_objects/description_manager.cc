/*
 * Copyright (C) 2020 by the Widelands Development Team
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

#include "logic/map_objects/description_manager.h"

#include <cassert>
#include <memory>

#include "io/filesystem/layered_filesystem.h"
#include "logic/game_data_error.h"
#include "scripting/lua_table.h"

namespace Widelands {
DescriptionManager::DescriptionManager(LuaInterface* lua) : lua_(lua) {

	map_objecttype_subscriber_ = Notifications::subscribe<NoteMapObjectDescription>(
	   [this](const NoteMapObjectDescription& note) {
		   assert(!registered_descriptions_.empty());
		   switch (note.type) {
		   case NoteMapObjectDescription::LoadType::kObject:
			   load_description_on_demand(note.name);
			   break;
		   case NoteMapObjectDescription::LoadType::kAttribute:
			   auto it = registered_attributes_.find(note.name);
			   if (it != registered_attributes_.end()) {
				   for (const std::string& objectname : it->second) {
					   load_description_on_demand(objectname);
				   }
				   registered_attributes_.erase(it);
			   }
			   break;
		   }
	   });
}

DescriptionManager::~DescriptionManager() {
}

/// Walk given directory and register descriptions
void DescriptionManager::register_directory(const std::string& dirname,
                                            FileSystem* filesystem,
                                            bool is_scenario) {
	FilenameSet files = filesystem->list_directory(dirname);
	for (const std::string& file : files) {
		if (filesystem->is_directory(file)) {
			register_directory(file, filesystem, is_scenario);
		} else {
			if (strcmp(FileSystem::fs_filename(file.c_str()), "register.lua") == 0) {
				if (is_scenario) {
					std::unique_ptr<LuaTable> names_table = lua_->run_script("map:" + file);
					for (const std::string& object_name : names_table->keys<std::string>()) {
						const std::vector<std::string> attributes =
						   names_table->get_table(object_name)->array_entries<std::string>();
						register_scenario_description(filesystem, object_name,
						                              FileSystem::fs_dirname(file) + "init.lua",
						                              attributes);
						register_attributes(attributes, object_name);
					}
				} else {
					std::unique_ptr<LuaTable> names_table = lua_->run_script(file);
					for (const std::string& object_name : names_table->keys<std::string>()) {
						const std::vector<std::string> attributes =
						   names_table->get_table(object_name)->array_entries<std::string>();
						register_description(
						   object_name, FileSystem::fs_dirname(file) + "init.lua", attributes);
						register_attributes(attributes, object_name);
					}
				}
			}
		}
	}
}

void DescriptionManager::register_description(const std::string& description_name,
                                              const std::string& script_path,
                                              const std::vector<std::string>& attributes) {
	if (registered_descriptions_.count(description_name) == 1) {
		throw GameDataError(
		   "DescriptionManager::register_description: Attempt to register description\n"
		   "   name: '%s'\n"
		   "   script: '%s'\n"
		   "but the object has already been registered to\n"
		   "   script: '%s'\n",
		   description_name.c_str(), script_path.c_str(),
		   registered_descriptions_.at(description_name).script_path.c_str());
	}
	if (!g_fs->file_exists(script_path)) {
		throw GameDataError(
		   "DescriptionManager::register_description: Attempt to register description\n"
		   "   name: '%s'\n"
		   "   script: '%s'\n"
		   "but the script file does not exist",
		   description_name.c_str(), script_path.c_str());
	}

	registered_descriptions_.insert(
	   std::make_pair(description_name, RegisteredObject(script_path, attributes)));
}

void DescriptionManager::register_scenario_description(FileSystem* filesystem,
                                                       const std::string& description_name,
                                                       const std::string& script_path,
                                                       const std::vector<std::string>& attributes) {
	if (registered_scenario_descriptions_.count(description_name) == 1) {
		throw GameDataError(
		   "DescriptionManager::register_scenario_description: Attempt to register scenario object\n"
		   "   name: '%s'\n"
		   "   script: '%s'\n"
		   "but the object has already been registered to\n"
		   "   script: '%s'\n",
		   description_name.c_str(), script_path.c_str(),
		   registered_scenario_descriptions_.at(description_name).script_path.c_str());
	}
	if (!filesystem->file_exists(script_path)) {
		throw GameDataError(
		   "DescriptionManager::register_scenario_description: Attempt to register scenario object\n"
		   "   name: '%s'\n"
		   "   script: '%s'\n"
		   "but the script file does not exist",
		   description_name.c_str(), script_path.c_str());
	}

	registered_scenario_descriptions_.insert(
	   std::make_pair(description_name, RegisteredObject("map:" + script_path, attributes)));
}

void DescriptionManager::load_description(const std::string& description_name) {
	// Nothing to do if it's already loaded
	if (loaded_descriptions_.count(description_name) == 1) {
		return;
	}

	// Protect against circular dependencies
	if (descriptions_being_loaded_.count(description_name) == 1) {
		throw GameDataError(
		   "DescriptionManager::load_description: Object '%s' is already being loaded",
		   description_name.c_str());
	}

	std::string object_script;

	// Load it - scenario descriptions take precedence
	if (registered_scenario_descriptions_.count(description_name) == 1) {
		object_script = registered_scenario_descriptions_.at(description_name).script_path;
	} else if (registered_descriptions_.count(description_name) == 1) {
		object_script = registered_descriptions_.at(description_name).script_path;
	} else {
		throw GameDataError("DescriptionManager::load_description: Object '%s' was not registered",
		                    description_name.c_str());
	}

	// Protect against circular dependencies when 1 script file has multiple descriptions in it
	if (descriptions_being_loaded_.count(object_script) == 1) {
		return;
	}
	descriptions_being_loaded_.insert(object_script);
	lua_->run_script(object_script);
	descriptions_being_loaded_.erase(descriptions_being_loaded_.find(object_script));
}

const std::vector<std::string>&
DescriptionManager::get_attributes(const std::string& description_name) const {
	assert(registered_scenario_descriptions_.count(description_name) == 1 ||
	       registered_descriptions_.count(description_name) == 1);

	return registered_scenario_descriptions_.count(description_name) == 1 ?
	          registered_scenario_descriptions_.at(description_name).attributes :
	          registered_descriptions_.at(description_name).attributes;
}

void DescriptionManager::clear_scenario_descriptions() {
	registered_scenario_descriptions_.clear();
}

void DescriptionManager::mark_loading_in_progress(const std::string& description_name) {
	assert(registered_scenario_descriptions_.count(description_name) == 1 ||
	       registered_descriptions_.count(description_name) == 1);
	descriptions_being_loaded_.insert(description_name);
}

void DescriptionManager::mark_loading_done(const std::string& description_name) {
	// Update status
	loaded_descriptions_.insert(description_name);

	// Mark as done
	descriptions_being_loaded_.erase(descriptions_being_loaded_.find(description_name));
}

void DescriptionManager::load_description_on_demand(const std::string& description_name) {
	if (registered_scenario_descriptions_.count(description_name) == 1 ||
	    registered_descriptions_.count(description_name) == 1) {
		if (descriptions_being_loaded_.count(description_name) == 0) {
			load_description(description_name);
		}
	} else {
		throw GameDataError("Unknown map object type '%s'", description_name.c_str());
	}
}

void DescriptionManager::register_attributes(const std::vector<std::string>& attributes,
                                             const std::string& description_name) {
	for (const std::string& attribute : attributes) {
		if (registered_attributes_.count(attribute) == 1) {
			registered_attributes_.at(attribute).insert(description_name);
		} else {
			registered_attributes_.insert(
			   std::make_pair(attribute, std::set<std::string>{description_name}));
		}
	}
}

}  // namespace Widelands
