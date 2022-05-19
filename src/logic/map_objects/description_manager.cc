/*
 * Copyright (C) 2020-2022 by the Widelands Development Team
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

#include "logic/map_objects/description_manager.h"

#include <cassert>
#include <cstring>
#include <list>
#include <memory>

#include "base/log.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game_data_error.h"
#include "scripting/lua_table.h"

namespace Widelands {

static std::list<DescriptionManager*> description_managers_stack_;

DescriptionManager::DescriptionManager(LuaInterface* lua) : lua_(lua) {
	description_managers_stack_.push_back(this);

	map_objecttype_subscriber_ = Notifications::subscribe<NoteMapObjectDescription>(
	   [this](const NoteMapObjectDescription& note) {
		   if (description_managers_stack_.back() != this) {
			   // Not meant for us
			   return;
		   }
		   assert(!registered_descriptions_.empty());
		   switch (note.type) {
		   case NoteMapObjectDescription::LoadType::kObject:
			   load_description_on_demand(note.name, note.allow_failure);
			   break;
		   case NoteMapObjectDescription::LoadType::kAttribute:
			   auto it = registered_attributes_.find(note.name);
			   if (it != registered_attributes_.end()) {
				   for (const std::string& objectname : it->second) {
					   load_description_on_demand(objectname, false);
				   }
				   registered_attributes_.erase(it);
			   }
			   break;
		   }
	   });
}

DescriptionManager::~DescriptionManager() {
	auto it =
	   std::find(description_managers_stack_.begin(), description_managers_stack_.end(), this);
	assert(it != description_managers_stack_.end());
	description_managers_stack_.erase(it);
}

/// Walk given directory and register descriptions
void DescriptionManager::register_directory(const std::string& dirname,
                                            FileSystem* filesystem,
                                            const RegistryCallerInfo& caller) {
	FilenameSet files = filesystem->list_directory(dirname);
	for (const std::string& file : files) {
		if (filesystem->is_directory(file)) {
			register_directory(file, filesystem, caller);
		} else {
			if (strcmp(FileSystem::fs_filename(file.c_str()), "register.lua") == 0) {
				unsigned nr_registered_items = 0;
				std::set<std::string> all_registered_items;
				if (caller.first == RegistryCallerType::kScenario) {
					std::unique_ptr<LuaTable> names_table = lua_->run_script("map:" + file);
					for (const std::string& object_name : names_table->keys<std::string>()) {
						const std::vector<std::string> attributes =
						   names_table->get_table(object_name)->array_entries<std::string>();
						register_scenario_description(filesystem, object_name,
						                              FileSystem::fs_dirname(file) + "init.lua",
						                              attributes);
						register_attributes(attributes, object_name);
						all_registered_items.insert(object_name);
						++nr_registered_items;
					}
				} else {
					std::unique_ptr<LuaTable> names_table = lua_->run_script(file);
					for (const std::string& object_name : names_table->keys<std::string>()) {
						const std::vector<std::string> attributes =
						   names_table->get_table(object_name)->array_entries<std::string>();
						register_description(
						   object_name, FileSystem::fs_dirname(file) + "init.lua", attributes, caller);
						register_attributes(attributes, object_name);
						all_registered_items.insert(object_name);
						++nr_registered_items;
					}
				}
				if (nr_registered_items == 0) {
					log_warn("Registry file %s does not define any units", file.c_str());
				} else if ((g_verbose || caller.first != RegistryCallerType::kDefault) &&
				           nr_registered_items > 1) {
					std::string all;
					/* Arbitrary estimate to reduce number of memory reallocations */
					all.reserve(nr_registered_items * 20);
					for (const std::string& str : all_registered_items) {
						all += ' ';
						all += str;
					}
					log_warn("The registry file %s\n"
					         "    defines multiple (%u) units:%s\n"
					         "    Defining multiple units in one file is error-prone. "
					         "Every unit should be defined in a registry directory of its own.",
					         file.c_str(), nr_registered_items, all.c_str());
				}
			}
		}
	}
}

void DescriptionManager::register_description(const std::string& description_name,
                                              const std::string& script_path,
                                              const std::vector<std::string>& attributes,
                                              const RegistryCallerInfo& caller) {
	bool skip =
	   std::find(attributes.begin(), attributes.end(), "__skip_if_exists") != attributes.end();
	bool replace =
	   std::find(attributes.begin(), attributes.end(), "__replace_if_exists") != attributes.end();
	if (skip && replace) {
		throw GameDataError("DescriptionManager::register_description %s: '__skip_if_exists' and "
		                    "'__replace_if_exists' are mutually exclusive",
		                    description_name.c_str());
	}
	switch (caller.first) {
	case RegistryCallerType::kDefault:
	case RegistryCallerType::kScenario:
		if (skip || replace) {
			throw GameDataError("DescriptionManager::register_description %s: '__skip_if_exists' and "
			                    "'__replace_if_exists' may be used only by add-ons",
			                    description_name.c_str());
		}
		break;
	case RegistryCallerType::kTribeAddon:
	case RegistryCallerType::kWorldAddon:
		if (!skip && !replace) {
			log_warn("DescriptionManager::register_description %s: add-on entities "
			         "should define '__skip_if_exists' or '__replace_if_exists'",
			         description_name.c_str());
			replace = true;
		}
		break;
	}

	if (registered_descriptions_.count(description_name) == 1) {
		// World and tribe addons are loaded first actually
		if (caller.first == RegistryCallerType::kDefault ||
		    caller.first == RegistryCallerType::kScenario) {
			const std::vector<std::string>& registered_attributes =
			   registered_descriptions_.at(description_name).attributes;
			// Reverse logic: replace if should have been skipped,
			//                skip if would replace this
			replace = std::find(registered_attributes.begin(), registered_attributes.end(),
			                    "__skip_if_exists") != registered_attributes.end();
			skip = std::find(registered_attributes.begin(), registered_attributes.end(),
			                 "__replace_if_exists") != registered_attributes.end();
			assert(!(replace && skip));
			replace |= !skip;  // Default to replace if policy is not set.
		}
		if (skip) {
			verb_log_info("%s: using '%s' instead of '%s'", description_name.c_str(),
			              registered_descriptions_.at(description_name).script_path.c_str(),
			              script_path.c_str());
			return;
		}
		if (replace) {
			verb_log_info("%s: using '%s' instead of '%s'", description_name.c_str(),
			              script_path.c_str(),
			              registered_descriptions_.at(description_name).script_path.c_str());
			registered_descriptions_.erase(registered_descriptions_.find(description_name));
		} else {
			throw GameDataError(
			   "DescriptionManager::register_description: Attempt to register description\n"
			   "   name: '%s'\n"
			   "   script: '%s'\n"
			   "but the object has already been registered to\n"
			   "   script: '%s'\n",
			   description_name.c_str(), script_path.c_str(),
			   registered_descriptions_.at(description_name).script_path.c_str());
		}
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
	   std::make_pair(description_name, RegisteredObject(script_path, attributes, caller)));
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

	registered_scenario_descriptions_.insert(std::make_pair(
	   description_name,
	   RegisteredObject("map:" + script_path, attributes,
	                    RegistryCallerInfo(RegistryCallerType::kScenario, std::string()))));
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

	// Load it - scenario descriptions take precedence
	load_order_.push_back(description_name);
	const RegisteredObject* object = nullptr;

	if (registered_scenario_descriptions_.count(description_name) != 0u) {
		object = &registered_scenario_descriptions_.at(description_name);
	} else if (registered_descriptions_.count(description_name) != 0u) {
		object = &registered_descriptions_.at(description_name);
	} else {
		throw GameDataError("DescriptionManager::load_description: Object '%s' was not registered",
		                    description_name.c_str());
	}
	assert(object);

	// Protect against circular dependencies when 1 script file has multiple descriptions in it
	if (descriptions_being_loaded_.count(object->script_path) == 1) {
		return;
	}
	descriptions_being_loaded_.insert(object->script_path);
	lua_->run_script(object->script_path);
	descriptions_being_loaded_.erase(descriptions_being_loaded_.find(object->script_path));

	// Ensure that resources are not created by tribes add-ons, wares not by world add-ons, etc
	Notifications::publish(NoteMapObjectDescriptionTypeCheck(description_name, object->caller));
}

const std::vector<std::string>&
DescriptionManager::get_attributes(const std::string& description_name) const {
	assert(registered_scenario_descriptions_.count(description_name) == 1 ||
	       registered_descriptions_.count(description_name) == 1);

	return registered_scenario_descriptions_.count(description_name) == 1 ?
             registered_scenario_descriptions_.at(description_name).attributes :
             registered_descriptions_.at(description_name).attributes;
}

const DescriptionManager::RegistryCallerInfo&
DescriptionManager::get_registry_caller_info(const std::string& description_name) const {
	assert(registered_scenario_descriptions_.count(description_name) == 1 ||
	       registered_descriptions_.count(description_name) == 1);
	return registered_scenario_descriptions_.count(description_name) == 1 ?
             registered_scenario_descriptions_.at(description_name).caller :
             registered_descriptions_.at(description_name).caller;
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

void DescriptionManager::load_description_on_demand(const std::string& description_name,
                                                    bool allow_failure) {
	if (registered_scenario_descriptions_.count(description_name) == 1 ||
	    registered_descriptions_.count(description_name) == 1) {
		if (descriptions_being_loaded_.count(description_name) == 0) {
			load_description(description_name);
		}
	} else if (!allow_failure) {
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
