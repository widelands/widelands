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

#ifndef __S__IMMOVABLE_PROGRAM_H
#define __S__IMMOVABLE_PROGRAM_H

/*
 * Implementation is in immovable.cc
 */

#include "immovable.h"

#include <string>


class Profile;

// Additional parameters for op parsing routines
struct ProgramParser {
	Immovable_Descr * descr;
	std::string       directory;
	Profile         * prof;
};

// One action of a program
struct ImmovableAction {
	typedef bool (Immovable::*execute_t)(Game* g, bool killable, const ImmovableAction& action);

	execute_t   function;
	int32_t         iparam1;
	int32_t         iparam2;
	std::string sparam1;
   std::string sparam2;
};

// The ImmovableProgram
class ImmovableProgram {
	typedef void (ImmovableProgram::*parse_t)
		(ImmovableAction                *,
		 const ProgramParser            *,
		 const std::vector<std::string> & cmd);

public:
	ImmovableProgram(std::string name);

	std::string get_name() const {return m_name;}
	uint32_t get_size() const {return m_actions.size();}
	const ImmovableAction& get_action(uint32_t idx) const {assert(idx < m_actions.size()); return m_actions[idx];}

	void add_action(const ImmovableAction& act);
	void parse(Immovable_Descr* descr, std::string directory, Profile* prof);

private:
	void parse_animation(ImmovableAction* act, const ProgramParser* parser, const std::vector<std::string>& cmd);
	void parse_transform(ImmovableAction* act, const ProgramParser* parser, const std::vector<std::string>& cmd);
	void parse_remove(ImmovableAction* act, const ProgramParser* parser, const std::vector<std::string>& cmd);
	void parse_playFX(ImmovableAction* act, const ProgramParser* parser, const std::vector<std::string>& cmd);

private:
	struct ParseMap {
		const char * name;
		parse_t      function;
	};

private:
	std::string                  m_name;
	std::vector<ImmovableAction> m_actions;

private:
	static const ParseMap        s_parsemap[];
};

#endif // __S__IMMOVABLE_PROGRAM_H
