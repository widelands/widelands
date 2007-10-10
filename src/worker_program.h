/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#ifndef __S__WORKER_PROGRAM_H
#define __S__WORKER_PROGRAM_H


#include "bob.h"
#include <stdint.h>
#include "workarea_info.h"
#include "worker.h"

/// \todo (Antonio Trueba#1#): Get rid of forward class declaration (Chicken-and-egg problem)
class Worker_Descr;

struct WorkerProgram : public BobProgramBase {
	struct Parser {
		Worker_Descr     * descr;
		std::string        directory;
		Profile          * prof;
		const EncodeData * encdata;
	};

	typedef void (WorkerProgram::*parse_t)
		(Worker_Descr                   *,
		 Worker::Action                 *,
		 Parser                         *,
		 const std::vector<std::string> &);

	WorkerProgram(const std::string & name) : m_name(name) {}
	virtual ~WorkerProgram() {}

		std::string get_name() const {return m_name;}
		int32_t get_size() const {return m_actions.size();}
		const Worker::Action* get_action(int32_t idx) const {
			assert(idx >= 0);
			assert(static_cast<uint32_t>(idx) < m_actions.size());
			return &m_actions[idx];
		}

	void parse(Worker_Descr *, Parser *, std::string name);
	const Workarea_Info & get_workarea_info() const {return m_workarea_info;}

private:
	Workarea_Info m_workarea_info;
	struct ParseMap {
		const char * name;
		parse_t      function;
	};

	void parse_mine
		(Worker_Descr                   *,
		 Worker::Action                 *,
		 Parser                         *,
		 const std::vector<std::string> & cmd);
	void parse_createitem
		(Worker_Descr                   *,
		 Worker::Action                 *,
		 Parser                         *,
		 const std::vector<std::string> & cmd);
	void parse_setdescription
		(Worker_Descr                   *,
		 Worker::Action                 *,
		 Parser                         *,
		 const std::vector<std::string> & cmd);
	void parse_setbobdescription
		(Worker_Descr                   *,
		 Worker::Action                 *,
		 Parser                         *,
		 const std::vector<std::string> & cmd);
	void parse_findobject
		(Worker_Descr                   *,
		 Worker::Action                 *,
		 Parser                         *,
		 const std::vector<std::string> & cmd);
	void parse_findspace
		(Worker_Descr                   *,
		 Worker::Action                 *,
		 Parser                         *,
		 const std::vector<std::string> & cmd);
	void parse_walk
		(Worker_Descr                   *,
		 Worker::Action                 *,
		 Parser                         *,
		 const std::vector<std::string> & cmd);
	void parse_animation
		(Worker_Descr                   *,
		 Worker::Action                 *,
		 Parser                         *,
		 const std::vector<std::string> & cmd);
	void parse_return
		(Worker_Descr                   *,
		 Worker::Action                 *,
		 Parser                         *,
		 const std::vector<std::string> & cmd);
	void parse_object
		(Worker_Descr                   *,
		 Worker::Action                 *,
		 Parser                         *,
		 const std::vector<std::string> & cmd);
	void parse_plant
		(Worker_Descr                   *,
		 Worker::Action                 *,
		 Parser                         *,
		 const std::vector<std::string> & cmd);
	void parse_create_bob
		(Worker_Descr                   *,
		 Worker::Action                 *,
		 Parser                         *,
		 const std::vector<std::string> & cmd);
	void parse_removeobject
		(Worker_Descr                   *,
		 Worker::Action                 *,
		 Parser                         *,
		 const std::vector<std::string> & cmd);
	void parse_geologist
		(Worker_Descr                   *,
		 Worker::Action                 *,
		 Parser                         *,
		 const std::vector<std::string> & cmd);
	void parse_geologist_find
		(Worker_Descr                   *,
		 Worker::Action                 *,
		 Parser                         *,
		 const std::vector<std::string> & cmd);
	void parse_playFX
		(Worker_Descr                   *,
		 Worker::Action                 *,
		 Parser                         *,
		 const std::vector<std::string> & cmd);

	const std::string                 m_name;
		std::vector<Worker::Action> m_actions;
		static const ParseMap       s_parsemap[];
};


#endif // __S__WORKER_PROGRAM_H
