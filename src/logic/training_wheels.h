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
class TrainingWheels {
public:
	/*
	 * Runs the training wheel scripts defined in data/scripting/training_wheels
	 */
	explicit TrainingWheels(LuaInterface& lua);
	~TrainingWheels() = default;

	void mark_as_solved(const std::string& objective);

private:
	void run_objectives();
	void write();

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

	// Objective name, whether it was completed
	std::map<std::string, TrainingWheel> idle_objectives_;
	std::set<std::string> running_objectives_;
	std::set<std::string> solved_objectives_;
	Profile profile_;

	LuaInterface& lua_;  // Not owned
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_TRAINING_WHEELS_H
