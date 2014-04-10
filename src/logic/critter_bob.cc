/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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

#include "logic/critter_bob.h"

#include <cstdio>

#include "helper.h"
#include "logic/critter_bob_program.h"
#include "logic/field.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/tribe.h"
#include "profile/profile.h"
#include "wexception.h"


namespace Widelands {

const Critter_BobProgram::ParseMap Critter_BobProgram::s_parsemap[] = {
	{"remove",            &Critter_BobProgram::parse_remove},
	{nullptr,                   nullptr}
};


void Critter_BobProgram::parse(Parser * const parser, char const * const name)
{
	Section & program_s = parser->prof->get_safe_section(name);

	for (uint32_t idx = 0;; ++idx) {
		try {
			char buffer[32];

			snprintf(buffer, sizeof(buffer), "%i", idx);
			char const * const string = program_s.get_string(buffer, nullptr);
			if (!string)
				break;

			const std::vector<std::string> cmd(split_string(string, " \t\r\n"));
			if (cmd.empty())
				continue;

			//  find the appropriate parser
			Critter_BobAction act;
			uint32_t mapidx;

			for (mapidx = 0; s_parsemap[mapidx].name; ++mapidx)
				if (cmd[0] == s_parsemap[mapidx].name)
					break;

			if (!s_parsemap[mapidx].name)
				throw wexception("unknown command type \"%s\"", cmd[0].c_str());

			(this->*s_parsemap[mapidx].function)(&act, parser, cmd);

			m_actions.push_back(act);
		} catch (const std::exception & e) {
			throw wexception("Line %i: %s", idx, e.what());
		}
	}

	// Check for line numbering problems
	if (program_s.get_num_values() != m_actions.size())
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

bool Critter_Bob::run_remove
	(Game & game, State & state, const Critter_BobAction &)
{
	++state.ivar1;
	// Bye, bye cruel world
	schedule_destroy(game);
	return true;
}


/*
===========================================================================

 CRITTER BOB DESCR

===========================================================================
*/

Critter_Bob_Descr::Critter_Bob_Descr
	(char const * const _name, char const * const _descname,
	 const std::string & directory, Profile & prof, Section & global_s,
	 Tribe_Descr const * const _tribe)
	:
	BobDescr(_name, _descname, directory, prof, global_s, _tribe),
	m_swimming(global_s.get_bool("swimming", false))
{
	char defaultpics[256];
	snprintf(defaultpics, sizeof(defaultpics), "%s_walk_!!_??.png", _name);
	m_walk_anims.parse(*this, directory, prof, "walk", false, defaultpics);

	while (Section::Value const * const v = global_s.get_next_val("program")) {
		std::string program_name = v->get_string();
		std::transform
			(program_name.begin(), program_name.end(), program_name.begin(),
			 tolower);
		Critter_BobProgram * prog = nullptr;
		try {
			if (m_programs.count(program_name))
				throw wexception("this program has already been declared");
			Critter_BobProgram::Parser parser;

			parser.descr = this;
			parser.directory = directory;
			parser.prof = &prof;

			prog = new Critter_BobProgram(v->get_string());
			prog->parse(&parser, v->get_string());
			m_programs[program_name] = prog;
		} catch (const std::exception & e) {
			delete prog;
			throw wexception
				("Parse error in program %s: %s", v->get_string(), e.what());
		}
	}
}


Critter_Bob_Descr::~Critter_Bob_Descr() {
	container_iterate_const(Programs, m_programs, i)
		delete i.current->second;
}


/*
===============
Get a program from the workers description.
===============
*/
Critter_BobProgram const * Critter_Bob_Descr::get_program
	(const std::string & programname) const
{
	Programs::const_iterator const it = m_programs.find(programname);
	if (it == m_programs.end())
		throw wexception
			("%s has no program '%s'", name().c_str(), programname.c_str());
	return it->second;
}


uint32_t Critter_Bob_Descr::movecaps() const {
	return is_swimming() ? MOVECAPS_SWIM : MOVECAPS_WALK;
}


/*
==============================================================================

class Critter_Bob

==============================================================================
*/

//
// Implementation
//

// wait up to 12 seconds between moves
#define CRITTER_MAX_WAIT_TIME_BETWEEN_WALK 2000

Critter_Bob::Critter_Bob(const Critter_Bob_Descr & critter_bob_descr) :
Bob(critter_bob_descr)
{}


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

Bob::Task const Critter_Bob::taskProgram = {
	"program",
	static_cast<Bob::Ptr>(&Critter_Bob::program_update),
	nullptr,
	nullptr,
	true
};


void Critter_Bob::start_task_program
	(Game & game, const std::string & programname)
{
	push_task(game, taskProgram);
	State & state = top_state();
	state.program = descr().get_program(programname);
	state.ivar1   = 0;
}


void Critter_Bob::program_update(Game & game, State & state)
{
	if (get_signal().size()) {
		molog("[program]: Interrupted by signal '%s'\n", get_signal().c_str());
		return pop_task(game);
	}

	for (;;) {
		const Critter_BobProgram & program =
			ref_cast<Critter_BobProgram const, BobProgramBase const>
				(*state.program);

		if (state.ivar1 >= program.get_size())
			return pop_task(game);

		const Critter_BobAction & action = program[state.ivar1];

		if ((this->*(action.function))(game, state, action))
			return;
	}
}


/*
==============================

ROAM task

Simply roam the map

==============================
*/

Bob::Task const Critter_Bob::taskRoam = {
	"roam",
	static_cast<Bob::Ptr>(&Critter_Bob::roam_update),
	nullptr,
	nullptr,
	true
};

void Critter_Bob::roam_update(Game & game, State & state)
{
	if (get_signal().size())
		return pop_task(game);

	// alternately move and idle
	Time idle_time_min = 1000;
	Time idle_time_rnd = CRITTER_MAX_WAIT_TIME_BETWEEN_WALK;
	if (state.ivar1) {
		state.ivar1 = 0;
		if
			(start_task_movepath
			 	(game,
			 	 game.random_location(get_position(), 2), //  Pick a random target.
			 	 3,
			 	 descr().get_walk_anims()))
			return;
		idle_time_min = 1, idle_time_rnd = 1000;
	}
	state.ivar1 = 1;
	return
		start_task_idle
			(game,
			 descr().get_animation("idle"),
			 idle_time_min + game.logic_rand() % idle_time_rnd);
}

void Critter_Bob::init_auto_task(Game & game) {
	push_task(game, taskRoam);
	top_state().ivar1 = 0;
}

Bob & Critter_Bob_Descr::create_object() const {
	return *new Critter_Bob(*this);
}

/*
==============================

Load / Save implementation

==============================
*/

#define CRITTER_SAVEGAME_VERSION 1

Critter_Bob::Loader::Loader()
{
}

const Bob::Task * Critter_Bob::Loader::get_task(const std::string & name)
{
	if (name == "roam") return &taskRoam;
	if (name == "program") return &taskProgram;
	return Bob::Loader::get_task(name);
}

const BobProgramBase * Critter_Bob::Loader::get_program
	(const std::string & name)
{
	Critter_Bob & critter = get<Critter_Bob>();
	return critter.descr().get_program(name);
}


Map_Object::Loader * Critter_Bob::load
	(Editor_Game_Base & egbase, Map_Map_Object_Loader & mol, FileRead & fr)
{
	std::unique_ptr<Loader> loader(new Loader);

	try {
		// The header has been peeled away by the caller

		uint8_t const version = fr.Unsigned8();
		if (1 <= version && version <= CRITTER_SAVEGAME_VERSION) {
			std::string owner = fr.CString();
			std::string name = fr.CString();
			const Critter_Bob_Descr * descr = nullptr;

			if (owner == "world") {
				descr = dynamic_cast<const Critter_Bob_Descr *>
					(egbase.map().world().get_bob_descr(name));
			} else {
				egbase.manually_load_tribe(owner);

				if (const Tribe_Descr * tribe = egbase.get_tribe(owner))
					descr = dynamic_cast<const Critter_Bob_Descr *>
						(tribe->get_bob_descr(name));
			}

			if (!descr)
				throw game_data_error
					("undefined critter %s/%s", owner.c_str(), name.c_str());

			loader->init(egbase, mol, descr->create_object());
			loader->load(fr);
		} else
			throw game_data_error("unknown/unhandled version %u", version);
	} catch (const std::exception & e) {
		throw wexception("loading critter: %s", e.what());
	}

	return loader.release();
}

void Critter_Bob::save
	(Editor_Game_Base & egbase, Map_Map_Object_Saver & mos, FileWrite & fw)
{
	fw.Unsigned8(header_Critter);
	fw.Unsigned8(CRITTER_SAVEGAME_VERSION);

	std::string owner =
		descr().get_owner_tribe() ? descr().get_owner_tribe()->name() : "world";
	fw.CString(owner);
	fw.CString(descr().name());

	Bob::save(egbase, mos, fw);
}

}
