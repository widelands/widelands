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

#ifndef WL_LOGIC_TRAINING_WHEELS_H
#define WL_LOGIC_TRAINING_WHEELS_H

#include <map>
#include <set>

#include "io/profile.h"
#include "scripting/lua_interface.h"

namespace Widelands {

// Teach new players about Widelands, integrated into any singleplayer non-scenario game
// Check the scripting documentation for more information.
class TrainingWheels {
public:
	/**
	 * @brief Parses the training wheel scripts defined in data/scripting/training_wheels
	 * @param lua the game's Lua interface
	 */
	explicit TrainingWheels(LuaInterface& lua);
	~TrainingWheels() = default;

	/**
	 * @brief acquire_lock Try to set the lock to the given training wheel objective
	 * @param objective The training wheel objective wanting the lock
	 * @return Whether the training wheel objective acquired the lock
	 */
	bool acquire_lock(const std::string& objective);
	/**
	 * @brief mark_as_solved Mark the given training wheel objective as solved and release the lock
	 * @param objective The training wheel objective to be marked as solved
	 * @param run_some_more Whether to trigger more training wheel scripts when available
	 */
	void mark_as_solved(const std::string& objective, bool run_some_more);
	/**
	 * @brief mark_as_unsolved Mark the given training wheel objective as no longer solved. Does not trigger anything else and does not refresh the information about currently running training wheels or training wheels to run.
	 * @param objective The training wheel objective to be marked as unsolved
	 */
	void mark_as_unsolved(const std::string& objective);
	/**
	 * @brief run_objectives Trigger all queued training wheel objectives
	 */
	void run_objectives();
	/**
	 * @brief has_objectives See if there are still unsolved objectives
	 * @return Whether any more training wheels want to run
	 */
	bool has_objectives() const;

	const std::set<std::string>& solved_objectives() const {
		return solved_objectives_;
	}

private:
	/**
	 * @brief load_objectives Check for training wheel objectives that can be run
	 */
	void load_objectives();
	/**
	 * @brief write Write configuration to file
	 */
	void write();

	/**
	 * @brief A training wheel's script and filename
	 */
	struct TrainingWheel {
		explicit TrainingWheel(const std::string& key,
		                       const std::vector<std::string>& init_dependencies)
		   : script(key + ".lua") {
			for (const std::string& dependency : init_dependencies) {
				dependencies.insert(dependency);
			}
		}

		const std::string script;
		std::set<std::string> dependencies;
	};

	// Objective name and its scripting information
	std::map<std::string, TrainingWheel> idle_objectives_;
	// Prevent concurrency issues while loading objectives
	std::set<std::string> running_objectives_;
	// Remember solved objectives for dependency check
	std::set<std::string> solved_objectives_;
	// The scripts that had their dependencies met and are waiting to run
	std::set<std::string> scripts_to_run_;
	// Mutex Lock for the currently rinnung objective
	std::string current_objective_;
	// For reading/writing progress to disk
	Profile profile_;

	LuaInterface& lua_;  // Not owned
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_TRAINING_WHEELS_H
