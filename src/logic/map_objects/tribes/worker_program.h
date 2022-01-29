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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_WORKER_PROGRAM_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_WORKER_PROGRAM_H

#include "base/macros.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/map_object_program.h"
#include "logic/map_objects/tribes/workarea_info.h"
#include "logic/map_objects/tribes/worker.h"
#include "scripting/lua_table.h"

namespace Widelands {

struct WorkerProgram : public MapObjectProgram {

	using ParseWorkerProgramFn = void (WorkerProgram::*)(Worker::Action*,
	                                                     const std::vector<std::string>&);

	WorkerProgram(const std::string& init_name,
	              const LuaTable& actions_table,
	              const WorkerDescr& worker,
	              Descriptions& descriptions);

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

	const WorkareaInfo& get_workarea_info() const {
		return workarea_info_;
	}
	const std::set<std::pair<MapObjectType, MapObjectDescr::AttributeIndex>>&
	needed_attributes() const {
		return needed_attributes_;
	}
	const std::set<std::pair<MapObjectType, MapObjectDescr::AttributeIndex>>&
	collected_attributes() const {
		return collected_attributes_;
	}
	const std::set<std::pair<MapObjectType, MapObjectDescr::AttributeIndex>>&
	created_attributes() const {
		return created_attributes_;
	}
	const std::set<std::string>& collected_resources() const {
		return collected_resources_;
	}
	const std::set<std::string>& created_resources() const {
		return created_resources_;
	}
	const std::set<std::string>& created_bobs() const {
		return created_bobs_;
	}

	/// Set of ware types produced by this program
	const std::set<DescriptionIndex>& produced_ware_types() const {
		return produced_ware_types_;
	}

private:
	WorkareaInfo workarea_info_;
	std::set<std::pair<MapObjectType, MapObjectDescr::AttributeIndex>> needed_attributes_;
	std::set<std::pair<MapObjectType, MapObjectDescr::AttributeIndex>> collected_attributes_;
	std::set<std::pair<MapObjectType, MapObjectDescr::AttributeIndex>> created_attributes_;
	std::set<std::string> collected_resources_;
	std::set<std::string> created_resources_;
	std::set<std::string> created_bobs_;

	struct ParseMap {
		const char* name;
		ParseWorkerProgramFn function;
	};

	void parse_mine(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_breed(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_createware(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_findobject(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_findspace(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_walk(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_animate(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_return(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_callobject(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_plant(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_createbob(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_buildferry(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_removeobject(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_repeatsearch(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_findresources(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_scout(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_playsound(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_construct(Worker::Action* act, const std::vector<std::string>& cmd);
	void parse_terraform(Worker::Action* act, const std::vector<std::string>& cmd);

	const WorkerDescr& worker_;
	Descriptions& descriptions_;
	Actions actions_;
	static ParseMap const parsemap_[];
	std::set<DescriptionIndex> produced_ware_types_;
	DISALLOW_COPY_AND_ASSIGN(WorkerProgram);
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_WORKER_PROGRAM_H
