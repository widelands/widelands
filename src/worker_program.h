/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

/*
 * Implementation is in worker.cc
 */

#include "bob.h"

class Game;

struct WorkerAction {
	typedef bool (Worker::*execute_t)(Game* g, Bob::State* state, const WorkerAction* act);

	enum {
		walkObject,			// walk to objvar1
		walkCoords,			// walk to coords
	};

	execute_t		function;
	int				iparam1;
	int				iparam2;
	std::string		sparam1;

	std::vector<std::string>	sparamv;
};

class WorkerProgram : public BobProgramBase {
public:
	struct Parser {
		Worker_Descr*		descr;
		std::string			directory;
		Profile*				prof;
      const EncodeData* encdata;
	};

	typedef void (WorkerProgram::*parse_t)(Worker_Descr*, WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);

public:
	WorkerProgram(std::string name);
   virtual ~WorkerProgram(void) { }

	std::string get_name() const { return m_name; }
	int get_size() const { return m_actions.size(); }
	const WorkerAction* get_action(int idx) const {
		assert(idx >= 0 && (uint)idx < m_actions.size());
		return &m_actions[idx];
	}

	void parse(Worker_Descr*, Parser* parser, std::string name);

private:
	struct ParseMap {
		const char*		name;
		parse_t			function;
	};

private:
	void parse_mine(Worker_Descr*, WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_createitem(Worker_Descr*, WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_setdescription(Worker_Descr*, WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_setbobdescription(Worker_Descr*, WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_findobject(Worker_Descr*, WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_findspace(Worker_Descr*, WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_walk(Worker_Descr*, WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_animation(Worker_Descr*, WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_return(Worker_Descr*, WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_object(Worker_Descr*, WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_plant(Worker_Descr*, WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_create_bob(Worker_Descr*, WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_removeobject(Worker_Descr*, WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_geologist(Worker_Descr*, WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_geologist_find(Worker_Descr*, WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);

private:
	std::string						m_name;
	std::vector<WorkerAction>	m_actions;

private:
	static const ParseMap		s_parsemap[];
};


#endif // __S__WORKER_PROGRAM_H
