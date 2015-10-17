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

#ifndef WL_LOGIC_WORKER_PROGRAM_H
#define WL_LOGIC_WORKER_PROGRAM_H

#include <memory>

#include "base/macros.h"
#include "logic/bob.h"
#include "logic/workarea_info.h"
#include "logic/worker.h"

namespace Widelands {

// TODO(Antonio Trueba#1#): Get rid of forward class
// declaration (Chicken-and-egg problem)
class WorkerDescr;

struct WorkerProgram : public BobProgramBase {
	struct Parser {
		WorkerDescr* descr;
		std::string  directory;
		std::unique_ptr<LuaTable> table;
	};

	using ParseWorkerProgramFn = void (WorkerProgram::*)
		(WorkerDescr*,
		 Worker::Action*,
		 Parser*,
		 const Tribes& tribes,
		 const std::vector<std::string>&);

	WorkerProgram(const std::string & name) : m_name(name) {}
	virtual ~WorkerProgram() {}

	std::string get_name() const override {return m_name;}
	using Actions = std::vector<Worker::Action>;
	Actions::size_type get_size() const {return m_actions.size();}
	const Actions & actions() const {return m_actions;}
	Worker::Action const * get_action(int32_t idx) const {
		assert(idx >= 0);
		assert(static_cast<uint32_t>(idx) < m_actions.size());
		return &m_actions[idx];
	}

	void parse(WorkerDescr *, Parser *, char const * name, const Tribes& tribes);
	const WorkareaInfo & get_workarea_info() const {return m_workarea_info;}

private:
	WorkareaInfo m_workarea_info;
	struct ParseMap {
		const char * name;
		ParseWorkerProgramFn function;
	};

	void parse_mine
		(WorkerDescr*,
		 Worker::Action*,
		 Parser*, const Tribes&,
		 const std::vector<std::string>& cmd);
	void parse_breed
		(WorkerDescr*,
		 Worker::Action*,
		 Parser*, const Tribes&,
		 const std::vector<std::string>& cmd);
	void parse_createware
		(WorkerDescr*,
		 Worker::Action*,
		 Parser*,
		 const Tribes& tribes,
		 const std::vector<std::string>& cmd);
	void parse_setbobdescription
		(WorkerDescr*,
		 Worker::Action*,
		 Parser*, const Tribes&,
		 const std::vector<std::string>& cmd);
	void parse_findobject
		(WorkerDescr*,
		 Worker::Action*,
		 Parser*, const Tribes&,
		 const std::vector<std::string>& cmd);
	void parse_findspace
		(WorkerDescr*,
		 Worker::Action*,
		 Parser*, const Tribes&,
		 const std::vector<std::string>& cmd);
	void parse_walk
		(WorkerDescr*,
		 Worker::Action*,
		 Parser*, const Tribes&,
		 const std::vector<std::string>& cmd);
	void parse_animation
		(WorkerDescr* descr,
		 Worker::Action*, Parser*, const Tribes&,
		 const std::vector<std::string>& cmd);
	void parse_return
		(WorkerDescr*,
		 Worker::Action*,
		 Parser*, const Tribes&, const std::vector<std::string>&);
	void parse_object
		(WorkerDescr*,
		 Worker::Action*,
		 Parser*, const Tribes&,
		 const std::vector<std::string>& cmd);
	void parse_plant
		(WorkerDescr*,
		 Worker::Action*,
		 Parser*, const Tribes& tribes,
		 const std::vector<std::string>& cmd);
	void parse_create_bob
		(WorkerDescr*,
		 Worker::Action*,
		 Parser*, const Tribes&, const std::vector<std::string>&);
	void parse_removeobject
		(WorkerDescr*,
		 Worker::Action*,
		 Parser*, const Tribes&, const std::vector<std::string>&);
	void parse_geologist
		(WorkerDescr*,
		 Worker::Action*,
		 Parser*, const Tribes&,
		 const std::vector<std::string>& cmd);
	void parse_geologist_find
		(WorkerDescr*,
		 Worker::Action*,
		 Parser*, const Tribes&,
		 const std::vector<std::string>& cmd);
	void parse_scout
		(WorkerDescr*,
		 Worker::Action*,
		 Parser*, const Tribes&,
		 const std::vector<std::string>& cmd);
	void parse_play_fx
		(WorkerDescr*,
		 Worker::Action*,
		 Parser* parser, const Tribes&,
		 const std::vector<std::string>& cmd);
	void parse_construct
		(WorkerDescr*,
		 Worker::Action*,
		 Parser*, const Tribes&,
		 const std::vector<std::string>& cmd);

	const std::string                 m_name;
	Actions           m_actions;
	static ParseMap       const s_parsemap[];
	DISALLOW_COPY_AND_ASSIGN(WorkerProgram);
};

}

#endif  // end of include guard: WL_LOGIC_WORKER_PROGRAM_H
