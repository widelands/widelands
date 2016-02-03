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

#include "logic/map_objects/world/critter.h"

#include <cstdio>
#include <memory>
#include <string>

#include <boost/format.hpp>
#include <stdint.h>

#include "base/wexception.h"
#include "graphic/graphic.h"
#include "helper.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/field.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/world/critter_program.h"
#include "logic/map_objects/world/world.h"
#include "map_io/world_legacy_lookup_table.h"
#include "scripting/lua_table.h"


namespace Widelands {

void CritterProgram::parse(const std::vector<std::string>& lines) {
	for (const std::string& line : lines) {
		try {
			const std::vector<std::string> cmd(split_string(line, " \t\r\n"));
			if (cmd.empty())
				continue;

			CritterAction act;
			if (cmd[0] == "remove") {
				if (cmd.size() != 1)
					throw wexception("Usage: remove");
				act.function = &Critter::run_remove;
			} else {
				throw wexception("unknown command type \"%s\"", cmd[0].c_str());
			}

			m_actions.push_back(act);
		}
		catch (const std::exception& e) {
			throw wexception("Line '%s': %s", line.c_str(), e.what());
		}
	}
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
bool Critter::run_remove
	(Game & game, State & state, const CritterAction &)
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

CritterDescr::CritterDescr(const std::string& init_descname, const LuaTable& table)
	: BobDescr(init_descname, MapObjectType::CRITTER, MapObjectDescr::OwnerType::kWorld, table)
{
	add_directional_animation(&m_walk_anims, "walk");

	add_attributes(
	   table.get_table("attributes")->array_entries<std::string>(), std::set<uint32_t>());

	std::unique_ptr<LuaTable> programs = table.get_table("programs");
	for (const std::string& program_name : programs->keys<std::string>()) {
		try {
			std::unique_ptr<CritterProgram> prog(new CritterProgram(program_name));
			prog->parse(programs->get_table(program_name)->array_entries<std::string>());
			m_programs[program_name] = prog.release();
		} catch (const std::exception& e) {
			throw wexception("Parse error in program %s: %s", program_name.c_str(), e.what());
		}
	}
}

CritterDescr::~CritterDescr() {
	for (std::pair<std::string, CritterProgram *> program : m_programs) {
		delete program.second;
	}
}

bool CritterDescr::is_swimming() const {
	const static uint32_t swimming_attribute = get_attribute_id("swimming");
	return has_attribute(swimming_attribute);
}


/*
===============
Get a program from the workers description.
===============
*/
CritterProgram const * CritterDescr::get_program
	(const std::string & programname) const
{
	Programs::const_iterator const it = m_programs.find(programname);
	if (it == m_programs.end())
		throw wexception
			("%s has no program '%s'", name().c_str(), programname.c_str());
	return it->second;
}


uint32_t CritterDescr::movecaps() const {
	return is_swimming() ? MOVECAPS_SWIM : MOVECAPS_WALK;
}


/*
==============================================================================

class Critter

==============================================================================
*/

//
// Implementation
//

// wait up to 12 seconds between moves
#define CRITTER_MAX_WAIT_TIME_BETWEEN_WALK 2000

Critter::Critter(const CritterDescr & critter_descr) :
Bob(critter_descr)
{}


/*
==============================

PROGRAM task

Follow the steps of a configuration-defined program.
ivar1 is the next action to be performed.
ivar2 is used to store description indices selected by plant/setbobdescription
objvar1 is used to store objects found by findobject
coords is used to store target coordinates found by findspace

==============================
*/

Bob::Task const Critter::taskProgram = {
	"program",
	static_cast<Bob::Ptr>(&Critter::program_update),
	nullptr,
	nullptr,
	true
};


void Critter::start_task_program
	(Game & game, const std::string & programname)
{
	push_task(game, taskProgram);
	State & state = top_state();
	state.program = descr().get_program(programname);
	state.ivar1   = 0;
}


void Critter::program_update(Game & game, State & state)
{
	if (get_signal().size()) {
		molog("[program]: Interrupted by signal '%s'\n", get_signal().c_str());
		return pop_task(game);
	}

	for (;;) {
		const CritterProgram & program =
			dynamic_cast<const CritterProgram&>(*state.program);

		if (state.ivar1 >= program.get_size())
			return pop_task(game);

		const CritterAction & action = program[state.ivar1];

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

Bob::Task const Critter::taskRoam = {
	"roam",
	static_cast<Bob::Ptr>(&Critter::roam_update),
	nullptr,
	nullptr,
	true
};

void Critter::roam_update(Game & game, State & state)
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

void Critter::init_auto_task(Game & game) {
	push_task(game, taskRoam);
	top_state().ivar1 = 0;
}

Bob & CritterDescr::create_object() const {
	return *new Critter(*this);
}

/*
==============================

Load / Save implementation

==============================
*/

constexpr uint8_t kCurrentPacketVersion = 1;

Critter::Loader::Loader()
{
}

const Bob::Task * Critter::Loader::get_task(const std::string & name)
{
	if (name == "roam") return &taskRoam;
	if (name == "program") return &taskProgram;
	return Bob::Loader::get_task(name);
}

const BobProgramBase * Critter::Loader::get_program
	(const std::string & name)
{
	Critter & critter = get<Critter>();
	return critter.descr().get_program(name);
}

MapObject::Loader* Critter::load(EditorGameBase& egbase,
												  MapObjectLoader& mol,
                                      FileRead& fr,
                                      const WorldLegacyLookupTable& lookup_table) {
	std::unique_ptr<Loader> loader(new Loader);

	try {
		// The header has been peeled away by the caller

		uint8_t const packet_version = fr.unsigned_8();
		// Supporting older versions for map loading
		if (1 <= packet_version && packet_version  <= kCurrentPacketVersion) {
			const std::string owner = fr.c_string();
			std::string critter_name = fr.c_string();
			const CritterDescr * descr = nullptr;

			if (owner == "world") {
				critter_name = lookup_table.lookup_critter(critter_name);
				descr =
					dynamic_cast<const CritterDescr*>(egbase.world().get_bob_descr(critter_name));
			} else {
				throw GameDataError
					("Tribes don't have critters %s/%s", owner.c_str(), critter_name.c_str());
			}

			if (!descr)
				throw GameDataError
					("undefined critter %s/%s", owner.c_str(), critter_name.c_str());

			loader->init(egbase, mol, descr->create_object());
			loader->load(fr);
		} else {
			throw UnhandledVersionError("Critter", packet_version, kCurrentPacketVersion);
		}
	} catch (const std::exception & e) {
		throw wexception("loading critter: %s", e.what());
	}

	return loader.release();
}

void Critter::save
	(EditorGameBase & egbase, MapObjectSaver & mos, FileWrite & fw)
{
	fw.unsigned_8(HeaderCritter);
	fw.unsigned_8(kCurrentPacketVersion);

	const std::string owner =
		descr().get_owner_type() == MapObjectDescr::OwnerType::kTribe ?
				"" : // Tribes don't have critters
				"world";
	fw.c_string(owner);
	fw.c_string(descr().name());

	Bob::save(egbase, mos, fw);
}

}
