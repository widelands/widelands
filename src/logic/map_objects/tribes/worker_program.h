/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_WORKER_PROGRAM_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_WORKER_PROGRAM_H

#include <memory>

#include "base/macros.h"
#include "logic/map_objects/bob.h"
#include "logic/map_objects/tribes/tribes.h"
#include "logic/map_objects/tribes/workarea_info.h"
#include "logic/map_objects/tribes/worker.h"
#include "scripting/lua_table.h"

namespace Widelands {

// TODO(Antonio Trueba#1#): Get rid of forward class
// declaration (Chicken-and-egg problem)
class WorkerDescr;

struct WorkerProgram : public BobProgramBase {

	using ParseWorkerProgramFn = void (WorkerProgram::*)(Worker::Action*,
	                                                     const std::vector<std::string>&);

	WorkerProgram(const std::string& name, const WorkerDescr& worker, const Tribes& tribes)
	   : name_(name), worker_(worker), tribes_(tribes) {
	}
	virtual ~WorkerProgram() {
	}

	std::string get_name() const override {
		return name_;
	}
	using Actions = std::vector<Worker::Action>;
	Actions::size_type get_size() const {
		return actions_.size();
	}
	const Actions& actions() const {
		return actions_;
	}
	Worker::Action const* get_action(int32_t idx) const {
		assert(idx >= 0);
		assert(static_cast<uint32_t>(idx) < actions_.size());
		return &actions_[idx];
	}

	void parse(const LuaTable& table);
	const WorkareaInfo& get_workarea_info() const {
		return workarea_info_;
	}

private:
	WorkareaInfo workarea_info_;
	struct ParseMap {
		const char* name;
		ParseWorkerProgramFn function;
	};

	void parse_mine(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_breed(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_createware(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_setbobdescription(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_findobject(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_findspace(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_walk(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_animation(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_return(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_object(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_plant(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_create_bob(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_removeobject(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_geologist(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_geologist_find(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_scout(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_play_sound(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_construct(Worker::Action* act, const std::vector<std::string>& cmd);

	const std::string name_;
	const WorkerDescr& worker_;
	const Tribes& tribes_;
	Actions actions_;
	static ParseMap const parsemap_[];
	DISALLOW_COPY_AND_ASSIGN(WorkerProgram);
};
}

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_WORKER_PROGRAM_H
