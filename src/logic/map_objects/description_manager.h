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

#ifndef WL_LOGIC_MAP_OBJECTS_DESCRIPTION_MANAGER_H
#define WL_LOGIC_MAP_OBJECTS_DESCRIPTION_MANAGER_H

#include <map>
#include <memory>
#include <set>
#include <vector>

#include "base/macros.h"
#include "io/filesystem/filesystem.h"
#include "logic/map_objects/map_object_type.h"
#include "scripting/lua_interface.h"

namespace Widelands {

/// Keeps a registry of names, attributes and init script directory locations for map object
/// descriptions. Names must be globally unique, but we allow overriding a name via
/// scenario-specific map object descriptions.
class DescriptionManager {
public:
	explicit DescriptionManager(LuaInterface* lua);
	~DescriptionManager() noexcept(false);

	enum class RegistryCallerType { kDefault, kScenario, kTribeAddon, kWorldAddon };
	using RegistryCallerInfo = std::pair<RegistryCallerType, std::string /* add-on name */>;

	/// Search a directory for 'register.lua' files and register their 'init.lua' scripts
	void register_directory(const std::string& dirname,
	                        FileSystem* filesystem,
	                        const RegistryCallerInfo&);
	/// Map a map object description's name to its init script so that we can load it when we want
	/// it.
	void register_description(const std::string& description_name,
	                          const std::string& script_path,
	                          const std::vector<std::string>& attributes,
	                          const RegistryCallerInfo&);
	/// Map a scenario's map object description's name to its init script so that we can load it when
	/// we want it.
	void register_scenario_description(FileSystem* filesystem,
	                                   const std::string& description_name,
	                                   const std::string& script_path,
	                                   const std::vector<std::string>& attributes);
	/// Load the map object description for the given 'description_name' that has been registered
	/// previously with 'register_description'
	void load_description(const std::string& description_name);

	/// Return the attributes registered to the given description name.
	const std::vector<std::string>& get_attributes(const std::string& description_name) const;

	const RegistryCallerInfo& get_registry_caller_info(const std::string& description_name) const;

	/// Deregister all scenario object descrptions
	void clear_scenario_descriptions();

	/// Mark a description as being loaded to prevent concurrent loading
	void mark_loading_in_progress(const std::string& description_name);
	/// Mark a description as having been loaded
	void mark_loading_done(const std::string& description_name);

	/**
	 * The exact order in which all units have been loaded, e.g.
	 * {"barbarians", "barbarians_ship", "barbarians_well", ...}.
	 */
	const std::vector<std::string>& load_order() const {
		return load_order_;
	}

	struct NoteMapObjectDescriptionTypeCheck {
		CAN_BE_SENT_AS_NOTE(NoteId::MapObjectDescriptionTypeCheck)

		const std::string description_name;
		const RegistryCallerInfo caller;

		NoteMapObjectDescriptionTypeCheck(const std::string& n, const RegistryCallerInfo& c)
		   : description_name(n), caller(c) {
		}
	};

private:
	/// Load a description on demand via notification
	void load_description_on_demand(const std::string& description_name, bool allow_failure);
	/// For loading all map object descriptions with a given attribute on demand
	void register_attributes(const std::vector<std::string>& attributes,
	                         const std::string& description_name);

	struct RegisteredObject {
		explicit RegisteredObject(const std::string& init_script_path,
		                          const std::vector<std::string>& init_attributes,
		                          const RegistryCallerInfo& c)
		   : script_path(init_script_path), attributes(init_attributes), caller(c) {
		}
		const std::string script_path;
		const std::vector<std::string> attributes;
		const RegistryCallerInfo caller;
	};

	// The order in which all units have been loaded.
	std::vector<std::string> load_order_;

	/// A list of all available map object descriptions as <name, init script path>
	std::map<std::string, RegisteredObject> registered_descriptions_;
	/// A list of all extra or replacement map object descriptions used by a scenario as <name, init
	/// script path>
	std::map<std::string, RegisteredObject> registered_scenario_descriptions_;
	/// Maps attributes to map object description names, in case we need them dynamically
	std::map<std::string, std::set<std::string>> registered_attributes_;
	/// A list of map object descriptions currently being loaded, to avoid circular dependencies
	std::set<std::string> descriptions_being_loaded_;
	/// List of the map object descriptions that have already been loaded
	std::set<std::string> loaded_descriptions_;

	LuaInterface* lua_;  // Not owned

	/// For loading any registered map object description via Lua in scenarios.
	/// Do not use this for normal loading in tribes, since this will circumvent the check for
	/// circular dependencies. If the object is not known, loading is skipped quietly.
	std::unique_ptr<Notifications::Subscriber<Widelands::NoteMapObjectDescription>>
	   map_objecttype_subscriber_;

	DISALLOW_COPY_AND_ASSIGN(DescriptionManager);
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_DESCRIPTION_MANAGER_H
