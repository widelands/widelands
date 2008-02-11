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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef CRITTER_BOB_PROGRAM_H
#define CRITTER_BOB_PROGRAM_H

#include "bob.h"

namespace Widelands {

struct Critter_BobAction {
	typedef bool (Critter_Bob::*execute_t)(Game* g, Bob::State* state, const Critter_BobAction* act);

	enum {
		walkObject, //  walk to objvar1
		walkCoords, //  walk to coords
	};

	execute_t                function;
	int32_t                      iparam1;
	int32_t                      iparam2;
	std::string              sparam1;

	std::vector<std::string> sparamv;
};

struct Critter_BobProgram : public BobProgramBase {
	struct Parser {
		Critter_Bob_Descr * descr;
		std::string         directory;
		Profile           * prof;
		EncodeData  const * encdata;
	};

	typedef void (Critter_BobProgram::*parse_t)(Critter_BobAction* act, Parser* parser, const std::vector<std::string>& cmd);

	Critter_BobProgram(const std::string & name) : m_name(name) {}
	virtual ~Critter_BobProgram() {}

	std::string get_name() const {return m_name;}
	int32_t get_size() const {return m_actions.size();}
	const Critter_BobAction* get_action(int32_t idx) const {
		assert(idx >= 0);
		assert(static_cast<uint32_t>(idx) < m_actions.size());
		return &m_actions[idx];
	}

	void parse(Parser* parser, std::string name);

private:
	struct ParseMap {
		const char * name;
		parse_t      function;
	};

private:
	void parse_remove(Critter_BobAction* act, Parser* parser, const std::vector<std::string>& cmd);

private:
	std::string                    m_name;
	std::vector<Critter_BobAction> m_actions;

private:
	static const ParseMap          s_parsemap[];
};

};

#endif
