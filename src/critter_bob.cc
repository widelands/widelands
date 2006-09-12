/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#include <stdio.h>
#include "bob.h"
#include "critter_bob.h"
#include "critter_bob_program.h"
#include "field.h"
#include "game.h"
#include "util.h"
#include "profile.h"
#include "wexception.h"

/*
==============================================================================

class WorkerProgram

==============================================================================
*/

const Critter_BobProgram::ParseMap Critter_BobProgram::s_parsemap[] = {
/*	{ "mine",		      &Critter_BobProgram::parse_mine },
	{ "createitem",		&Critter_BobProgram::parse_createitem },
	{ "setdescription",	&Critter_BobProgram::parse_setdescription },
	{ "setbobdescription", &Critter_BobProgram::parse_setbobdescription },
	{ "findobject",		&Critter_BobProgram::parse_findobject },
	{ "findspace",			&Critter_BobProgram::parse_findspace },
	{ "walk",				&Critter_BobProgram::parse_walk },
	{ "animation",			&Critter_BobProgram::parse_animation },
	{ "return",				&Critter_BobProgram::parse_return },
	{ "object",				&Critter_BobProgram::parse_object },
	{ "plant",				&Critter_BobProgram::parse_plant },
   { "create_bob",		&Critter_BobProgram::parse_create_bob },
	{ "removeobject",		&Critter_BobProgram::parse_removeobject },
	{ "geologist",			&Critter_BobProgram::parse_geologist },
	{ "geologist-find",	&Critter_BobProgram::parse_geologist_find },
*/
	{ "remove",	&Critter_BobProgram::parse_remove},
	{ 0, 0 }
};


/*
===============
Critter_BobProgram::Critter_BobProgram

Initialize a program
===============
*/
Critter_BobProgram::Critter_BobProgram(std::string name)
{
	m_name = name;
}


/*
===============
Critter_BobProgram::parse

Parse a program
===============
*/
void Critter_BobProgram::parse(Parser* parser, std::string name)
{
	Section* sprogram = parser->prof->get_safe_section(name.c_str());

	for(uint idx = 0; ; ++idx) {
		try
		{
			char buf[32];
			const char* string;
			std::vector<std::string> cmd;

			snprintf(buf, sizeof(buf), "%i", idx);
			string = sprogram->get_string(buf, 0);
			if (!string)
				break;

			split_string(string, &cmd, " \t\r\n");
			if (!cmd.size())
				continue;

         // Find the appropriate parser
			Critter_BobAction act;
			uint mapidx;

			for(mapidx = 0; s_parsemap[mapidx].name; ++mapidx)
				if (cmd[0] == s_parsemap[mapidx].name)
					break;

			if (!s_parsemap[mapidx].name)
				throw wexception("unknown command '%s'", cmd[0].c_str());

			(this->*s_parsemap[mapidx].function)(&act, parser, cmd);

			m_actions.push_back(act);
		}
		catch(std::exception& e)
		{
			throw wexception("Line %i: %s", idx, e.what());
		}
	}

	// Check for line numbering problems
	if (sprogram->get_num_values() != m_actions.size())
		throw wexception("Line numbers appear to be wrong");
}

/*
===================================================

    PROGRAMS

==================================================
*/

/*
==============================

remove

Remove this critter

==============================
*/
void Critter_BobProgram::parse_remove(Critter_BobAction* act, Parser* parser, const std::vector<std::string>& cmd)
{
	if (cmd.size() != 1)
		throw wexception("Usage: remove");

   act->function = &Critter_Bob::run_remove;
}

bool Critter_Bob::run_remove(Game* g, State* state, const Critter_BobAction* act)
{

	state->ivar1++;
	// Bye,bye cruel world
   schedule_destroy(g);
   return true;
}


/*
===========================================================================

 CRITTER BOB DESCR

===========================================================================
*/

/*
 * Constructor
 */
Critter_Bob_Descr::Critter_Bob_Descr(const char *name, Tribe_Descr* tribe)
	: Bob_Descr(name, tribe)
{
	m_swimming = 0;
}

/*
===============
Critter_Bob_Descr::get_program

Get a program from the workers description.
===============
*/
const Critter_BobProgram* Critter_Bob_Descr::get_program(std::string name) const
{
	ProgramMap::const_iterator it = m_programs.find(name);

	if (it == m_programs.end())
		throw wexception("%s has no program '%s'", get_name(), name.c_str());

	return it->second;
}


void Critter_Bob_Descr::parse(const char *directory, Profile *prof, const EncodeData *encdata)
{
	Bob_Descr::parse(directory, prof, encdata);

	Section *s = prof->get_safe_section("global");

	m_swimming = s->get_bool("swimming", false);

   // Read all walking animations.
	// Default settings are in [walk]
	char sectname[256];

   // Pretty name
   m_descname = s->get_safe_string("descname");

   snprintf(sectname, sizeof(sectname), "%s_walk_??", m_name);
	m_walk_anims.parse(this, directory, prof, sectname, prof->get_section("walk"), encdata);

	Section *sglobal = prof->get_safe_section("global");
   const char* string;
   // Read programs
	while(sglobal->get_next_string("program", &string)) {
		Critter_BobProgram* prog = 0;

		try
		{
			Critter_BobProgram::Parser parser;

			parser.descr = this;
			parser.directory = directory;
			parser.prof = prof;
         parser.encdata = encdata;

			prog = new Critter_BobProgram(string);
			prog->parse(&parser, string);
			m_programs[prog->get_name()] = prog;
		}
		catch(std::exception& e)
		{
			if (prog)
				delete prog;

			throw wexception("Parse error in program %s: %s", string, e.what());
		}
	}
}



/*
==============================================================================

class Critter_Bob

==============================================================================
*/

//
// Implementation
//
#define CRITTER_MAX_WAIT_TIME_BETWEEN_WALK 2000 // wait up to 12 seconds between moves

Critter_Bob::Critter_Bob(Critter_Bob_Descr *d)
	: Bob(d)
{
}

Critter_Bob::~Critter_Bob()
{
}

uint Critter_Bob::get_movecaps() { return get_descr()->is_swimming() ? MOVECAPS_SWIM : MOVECAPS_WALK; }


/*
==============================

PROGRAM task

Follow the steps of a configuration-defined program.
ivar1 is the next action to be performed.
ivar2 is used to store description indices selected by setdescription
objvar1 is used to store objects found by findobject
coords is used to store target coordinates found by findspace

==============================
*/

Bob::Task Critter_Bob::taskProgram = {
	"program",

	(Bob::Ptr)&Critter_Bob::program_update,
	(Bob::Ptr)&Critter_Bob::program_signal,
	0
};


/*
===============
Critter_Bob::start_task_program

Start the given program.
===============
*/
void Critter_Bob::start_task_program(Game* g, std::string name)
{
	State* state;

	push_task(g, &taskProgram);

	state = get_state();
	state->program = get_descr()->get_program(name);
	state->ivar1 = 0;
}


/*
===============
Critter_Bob::program_update
===============
*/
void Critter_Bob::program_update(Game* g, State* state)
{
	const Critter_BobAction* act;

	for(;;)
	{
      const Critter_BobProgram* program=static_cast<const Critter_BobProgram*>(state->program);
		molog("[program]: %s#%i\n", program->get_name().c_str(), state->ivar1);

		if (state->ivar1 >= program->get_size()) {
			molog("  End of program\n");
			pop_task(g);
			return;
		}

		act = program->get_action(state->ivar1);

		if ((this->*(act->function))(g, state, act))
			return;
	}
}


/*
===============
Critter_Bob::program_signal
===============
*/
void Critter_Bob::program_signal(Game* g, State* state)
{
	molog("[program]: Interrupted by signal '%s'\n", get_signal().c_str());
	pop_task(g);
}



/*
==============================

ROAM task

Simply roam the map

==============================
*/

Bob::Task Critter_Bob::taskRoam = {
	"roam",

	(Bob::Ptr)&Critter_Bob::roam_update,
	(Bob::Ptr)&Critter_Bob::roam_signal,
	0,
};

void Critter_Bob::roam_update(Game* g, State* state)
{

	// ignore all signals
	if (get_signal().size())
		set_signal("");

	// alternately move and idle
	if (state->ivar1) {
		if
			(start_task_movepath
			 (g,
			  g->random_location(get_position(), 2), //  Pick a target at random.
			  3,
			  get_descr()->get_walk_anims()))
		{
			state->ivar1 = 0;
			return;
		}

		//molog("        Failed\n");

		start_task_idle(g, get_descr()->get_animation("idle"), 1 + g->logic_rand()%1000);
	}
	else
	{
		state->ivar1 = 1;

		//molog("[roam]: Idle\n");

		start_task_idle(g, get_descr()->get_animation("idle"), 1000 + g->logic_rand() % CRITTER_MAX_WAIT_TIME_BETWEEN_WALK);
	}
}

void Critter_Bob::roam_signal(Game* g, State* state) {
   pop_task(g);
}

void Critter_Bob::init_auto_task(Game* g)
{
	push_task(g, &taskRoam);

	get_state()->ivar1 = 0;
}

Bob *Critter_Bob_Descr::create_object()
{
	return new Critter_Bob(this);
}
