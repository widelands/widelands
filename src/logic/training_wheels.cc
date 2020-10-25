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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "logic/training_wheels.h"

#include <memory>

#include "base/log.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/filesystem_constants.h"
#include "scripting/lua_table.h"

const std::string kTrainingWheelsScriptingDir = std::string("scripting") + g_fs->file_separator() +
                                                std::string("training_wheels") +
                                                g_fs->file_separator();

namespace Widelands {

TrainingWheels::TrainingWheels(LuaInterface& lua)
   : current_objective_(""), profile_(Profile::err_log), lua_(lua) {
	g_fs->ensure_directory_exists(kSaveDir);
	if (g_fs->file_exists(kTrainingWheelsFile)) {
		profile_.read(kTrainingWheelsFile, "global");
	} else {
		write();
	}
	// Read init file and run
	Section& section = profile_.pull_section("global");
	std::unique_ptr<LuaTable> table(lua_.run_script("scripting/training_wheels/init.lua"));
	std::unique_ptr<LuaTable> wheel_table;
	for (const std::string& key : table->keys<std::string>()) {
		wheel_table = table->get_table(key);
		const bool is_solved = section.get_bool(key.c_str());
		section.set_bool(key.c_str(), is_solved);
		if (is_solved) {
			solved_objectives_.insert(std::make_pair(key, wheel_table->get_string("descname")));
			wheel_table->do_not_warn_about_unaccessed_keys();
		} else {
			std::unique_ptr<LuaTable> dependencies = wheel_table->get_table("dependencies");
			idle_objectives_.insert(
			   std::make_pair(key, TrainingWheel(key, wheel_table->get_string("descname"),
			                                     dependencies->array_entries<std::string>())));
		}
	}
	write();
	load_objectives();
}

void TrainingWheels::load_objectives() {
	// We collect the scripts before running them to prevent cocurrency issues
	scripts_to_run_.clear();
	// Now run
	for (auto it = idle_objectives_.begin(); it != idle_objectives_.end();) {
		if (running_objectives_.count(it->first) == 1) {
			continue;
		}
		// Any of the dependencies will do
		bool dependencies_met = it->second.dependencies.empty();
		for (const std::string& dependency : it->second.dependencies) {
			if (solved_objectives_.count(dependency) == 1) {
				dependencies_met = true;
				break;
			}
		}
		if (dependencies_met) {
			running_objectives_.insert(std::make_pair(it->first, it->second.descname));
			scripts_to_run_.insert(it->second.script);
			it = idle_objectives_.erase(it);
		} else {
			++it;
		}
	}
}

void TrainingWheels::run_objectives() {
	for (const std::string& runme : scripts_to_run_) {
		log_info("Running training wheel '%s'", runme.c_str());
		lua_.run_script(kTrainingWheelsScriptingDir + runme);
	}
	scripts_to_run_.clear();
}

bool TrainingWheels::has_objectives() const {
	return !scripts_to_run_.empty();
}

bool TrainingWheels::acquire_lock(const std::string& objective) {
	if (current_objective_.empty()) {
		current_objective_ = objective;
		return true;
	}
	return current_objective_ == objective;
}

void TrainingWheels::mark_as_solved(const std::string& objective, bool run_some_more) {
	log_info("Solved training wheel '%s'", objective.c_str());
	auto it = running_objectives_.find(objective);
	if (it != running_objectives_.end()) {
		solved_objectives_.insert(std::make_pair(objective, it->second));
		running_objectives_.erase(it);
	} else {
		solved_objectives_.insert(std::make_pair(objective, objective));
	}

	Section& section = profile_.pull_section("global");
	section.set_bool(objective.c_str(), true);
	write();
	current_objective_ = "";
	if (run_some_more) {
		load_objectives();
		run_objectives();
	}
}

void TrainingWheels::mark_as_unsolved(const std::string& objective) {
	log_info("Unsolved training wheel '%s'", objective.c_str());
	Section& section = profile_.pull_section("global");
	section.set_bool(objective.c_str(), false);
	write();
}

void TrainingWheels::write() {
	try {  //  overwrite the old config file
		profile_.write(kTrainingWheelsFile);
	} catch (const std::exception& e) {
		log_warn("could not save training wheels: %s\n", e.what());
	}
}

}  // namespace Widelands
