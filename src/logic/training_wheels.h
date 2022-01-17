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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
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
	 * @brief A training wheel's script, display name, solving status and dependencies
	 */
	struct TrainingWheel {
		explicit TrainingWheel(bool init_solved,
		                       const std::string& key,
		                       const std::string& init_descname,
		                       const std::vector<std::string>& init_dependencies)
		   : solved(init_solved), script(key + ".lua"), descname(init_descname) {
			for (const std::string& dependency : init_dependencies) {
				dependencies.insert(dependency);
			}
		}

		bool solved;
		const std::string script;
		const std::string descname;
		std::set<std::string> dependencies;
	};

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
	 * @brief release_lock Releases the current training wheel lock without marking it as solved.
	 */
	void release_lock();

	/**
	 * @brief mark_as_solved Mark the given training wheel objective as solved and release the lock
	 * @param objective The training wheel objective to be marked as solved
	 * @param run_some_more Whether to trigger more training wheel scripts when available
	 */
	void mark_as_solved(const std::string& objective, bool run_some_more);

	/**
	 * The same as mark_as_solved without recording it as solved in the config file, so that it will
	 * run again in a new game but stop blocking other training wheels that depend on it.
	 */
	void skip(const std::string& objective, bool run_some_more);

	/**
	 * @brief mark_as_unsolved Mark the given training wheel objective as no longer solved. Does not
	 * trigger anything else and does not refresh the information about currently running training
	 * wheels or training wheels to run.
	 * @param objective The training wheel objective to be marked as unsolved
	 */
	void mark_as_unsolved(const std::string& objective);
	/**
	 * @brief run_objectives Trigger all queued training wheel objectives
	 */
	void run_objectives();

	/**
	 * @brief run Ignore dependencies and run this objective if it wasn't solved yet.
	 * @param objective The objective to run
	 * @param force Run anyway if it was solved previously
	 */
	void run(const std::string& objective, bool force);

	/**
	 * @brief has_objectives See if there are still unsolved objectives
	 * @return Whether any more training wheels want to run
	 */
	bool has_objectives() const;

	/**
	 * @brief current_objective The training wheel that's currently active
	 * @return A string with the training wheel's name, or empty if none are active
	 */
	const std::string& current_objective() const {
		return current_objective_;
	}

	std::map<std::string, TrainingWheel> all_objectives() const;

private:
	/**
	 * @brief load_objectives Check for training wheel objectives that can be run
	 */
	void load_objectives();
	/**
	 * @brief write Write configuration to file
	 */
	void write();

	void solve(const std::string& objective, bool run_some_more, bool write_to_config);

	// Objective name and its scripting information
	std::map<std::string, TrainingWheel> idle_objectives_;
	// Prevent concurrency issues while loading objectives, and remember descname for options. Name,
	// descname
	std::map<std::string, TrainingWheel> running_objectives_;
	// Remember solved objectives for dependency check and options. Name, descname
	std::map<std::string, TrainingWheel> solved_objectives_;
	// The scripts that had their dependencies met and are waiting to run
	std::set<std::string> scripts_to_run_;
	// Mutex Lock for the currently running objective
	std::string current_objective_;
	// For reading/writing progress to disk
	Profile profile_;

	LuaInterface& lua_;  // Not owned
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_TRAINING_WHEELS_H
#endif
