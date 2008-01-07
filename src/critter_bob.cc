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

#include "critter_bob.h"

#include "critter_bob_program.h"
#include "field.h"
#include "game.h"
#include "helper.h"
#include "profile.h"
#include "wexception.h"

#include <stdio.h>


const Critter_BobProgram::ParseMap Critter_BobProgram::s_parsemap[] = {
#if 0
	{"mine",              &Critter_BobProgram::parse_mine},
	{"createitem",        &Critter_BobProgram::parse_createitem},
	{"setdescription",    &Critter_BobProgram::parse_setdescription},
	{"setbobdescription", &Critter_BobProgram::parse_setbobdescription},
	{"findobject",        &Critter_BobProgram::parse_findobject},
	{"findspace",         &Critter_BobProgram::parse_findspace},
	{"walk",              &Critter_BobProgram::parse_walk},
	{"animation",         &Critter_BobProgram::parse_animation},
	{"return",            &Critter_BobProgram::parse_return},
	{"object",            &Critter_BobProgram::parse_object},
	{"plant",             &Critter_BobProgram::parse_plant},
	{"create_bob",        &Critter_BobProgram::parse_create_bob},
	{"removeobject",      &Critter_BobProgram::parse_removeobject},
	{"geologist",         &Critter_BobProgram::parse_geologist},
	{"geologist-find",    &Critter_BobProgram::parse_geologist_find},
#endif
	{"remove",            &Critter_BobProgram::parse_remove},
	{0,                   0}
};


/*
===============
Critter_BobProgram::parse

Parse a program
===============
*/
void Critter_BobProgram::parse(Parser* parser, std::string name)
{
	Section* sprogram = parser->prof->get_safe_section(name.c_str());

	for (uint32_t idx = 0; ; ++idx) {
		try
		{
			char buffer[32];

			snprintf(buffer, sizeof(buffer), "%i", idx);
			const char * const string = sprogram->get_string(buffer, 0);
			if (!string)
				break;

			const std::vector<std::string> cmd(split_string(string, " \t\r\n"));
			if (!cmd.size())
				continue;

         // Find the appropriate parser
			Critter_BobAction act;
			uint32_t mapidx;

			for (mapidx = 0; s_parsemap[mapidx].name; ++mapidx)
				if (cmd[0] == s_parsemap[mapidx].name)
					break;

			if (!s_parsemap[mapidx].name)
				throw wexception("unknown command '%s'", cmd[0].c_str());

			(this->*s_parsemap[mapidx].function)(&act, parser, cmd);

			m_actions.push_back(act);
		}
		catch (std::exception& e)
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
void Critter_BobProgram::parse_remove
(Critter_BobAction * act, Parser *, const std::vector<std::string> & cmd)
{
	if (cmd.size() != 1)
		throw wexception("Usage: remove");

   act->function = &Critter_Bob::run_remove;
}

bool Critter_Bob::run_remove(Game * g, State * state, const Critter_BobAction *)
{

	++state->ivar1;
	// Bye, bye cruel world
   schedule_destroy(g);
   return true;
}


/*
===========================================================================

 CRITTER BOB DESCR

===========================================================================
*/

Critter_Bob_Descr::Critter_Bob_Descr
(const Tribe_Descr * const tribe_descr, const std::string & critter_bob_name)
: Bob::Descr(tribe_descr, critter_bob_name), m_swimming(0)
{}


Critter_Bob_Descr::~Critter_Bob_Descr() {
	const ProgramMap::const_iterator programs_end = m_programs.end();
	for
		(ProgramMap::const_iterator it = m_programs.begin();
		 it != programs_end;
		 ++it)
		delete it->second;
}


/*
===============
Critter_Bob_Descr::get_program

Get a program from the workers description.
===============
*/
const Critter_BobProgram* Critter_Bob_Descr::get_program
(std::string programname)
const
{
	const ProgramMap::const_iterator it = m_programs.find(programname);
	if (it == m_programs.end())
		throw wexception
			("%s has no program '%s'", name().c_str(), programname.c_str());
	return it->second;
}


void Critter_Bob_Descr::parse(const char *directory, Profile *prof, const EncodeData *encdata)
{
	Bob::Descr::parse(directory, prof, encdata);

	Section *s = prof->get_safe_section("global");

	m_swimming = s->get_bool("swimming", false);

	// Pretty name
   m_descname = s->get_safe_string("descname");

	m_walk_anims.parse
		(this,
		 directory,
		 prof,
		 (m_name + "_walk_??").c_str(),
		 prof->get_section("walk"),
		 encdata);

	Section *sglobal = prof->get_safe_section("global");
   const char* string;
   // Read programs
	while (sglobal->get_next_string("program", &string)) {
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
		catch (std::exception& e)
		{
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

Critter_Bob::Critter_Bob(const Critter_Bob_Descr & critter_bob_descr) :
Bob(critter_bob_descr)
{
}

Critter_Bob::~Critter_Bob()
{
}

uint32_t Critter_Bob::get_movecaps() const throw ()
{return descr().is_swimming() ? MOVECAPS_SWIM : MOVECAPS_WALK;}


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
void Critter_Bob::start_task_program(const std::string & programname) {
	push_task(taskProgram);
	State & state = top_state();
	state.program = descr().get_program(programname);
	state.ivar1   = 0;
}


/*
===============
Critter_Bob::program_update
===============
*/
void Critter_Bob::program_update(Game* g, State* state)
{
	const Critter_BobAction* action;

	for (;;)
	{
      const Critter_BobProgram* program=static_cast<const Critter_BobProgram*>(state->program);

		if (state->ivar1 >= program->get_size()) {
			molog("  End of program\n");
			pop_task();
			return;
		}

		action = program->get_action(state->ivar1);

		if ((this->*(action->function))(g, state, action))
			return;
	}
}


/*
===============
Critter_Bob::program_signal
===============
*/
void Critter_Bob::program_signal(Game *, State *) {
	molog("[program]: Interrupted by signal '%s'\n", get_signal().c_str());
	pop_task();
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
			  descr().get_walk_anims()))
		{
			state->ivar1 = 0;
			return;
		}

		//molog("        Failed\n");

		start_task_idle(g, descr().get_animation("idle"), 1 + g->logic_rand()%1000);
	}
	else
	{
		state->ivar1 = 1;

		//molog("[roam]: Idle\n");

		start_task_idle(g, descr().get_animation("idle"), 1000 + g->logic_rand() % CRITTER_MAX_WAIT_TIME_BETWEEN_WALK);
	}
}

void Critter_Bob::roam_signal(Game *, State *) {pop_task();}

void Critter_Bob::init_auto_task(Game *) {
	push_task(taskRoam);
	top_state().ivar1 = 0;
}

Bob * Critter_Bob_Descr::create_object() const {return new Critter_Bob(*this);}
