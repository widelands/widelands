/*
 * Copyright (C) 2002-2009 by the Widelands Development Team
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

#include "cmd_check_eventchain.h"

#include "event.h"
#include "game.h"
#include "trigger_conditional.h"

namespace Widelands {

void Cmd_CheckEventChain::execute (Game & game) {
	++m_eventchain_id;

	Map & map = game.map();
	Manager<EventChain> & mcm = map.mcm();
	Manager<EventChain>::Index nr_eventchains = mcm.size();
	if (nr_eventchains <= m_eventchain_id) {
		//  either we wrapped around the end of all eventchains if so, restart.
		//  if there are no eventchains at all, requeue in about 30 seconds to
		//  check if this state has changed (a new trigger could be registered)
		//  (this should only happen at the beginning of the game and should not
		//  harm at all, and 30 seconds means nearly no CPU time for non trigger
		//  games)

		if (nr_eventchains)
			m_eventchain_id = 0;
		else
			return
				game.enqueue_command
					(new Cmd_CheckEventChain
					 	(game.get_gametime() + 30000, static_cast<uint16_t>(-1)));
	}

	EventChain & evchain = mcm[m_eventchain_id];

	switch (evchain.get_state()) {
	case EventChain::INIT:
		//  This is initialized, look if it needs running.
		if (evchain.get_trigcond()->eval(game))
			evchain.run(game); //  Hooray, we can start the shit off
		break;
	case EventChain::RUNNING:
		// This chain is currently running. Continue to run it.
		evchain.run(game);
		break;
	case EventChain::DONE:
		throw wexception
			("Cmd_CheckEventChain: Done event chain found. no good no good!");
		break;
	}

	if (evchain.get_state() == EventChain::DONE) {
		mcm.remove(evchain);
		nr_eventchains = mcm.size();
		map.mem().remove_unreferenced();
		map.mtm().remove_unreferenced();
	}

	//  recheck next in the time that all eventchains get checked at least once
	//  every 10 seconds
	int32_t const delay = nr_eventchains ? 1000 / nr_eventchains : 30000;

	game.enqueue_command
		(new Cmd_CheckEventChain(game.get_gametime() + delay, m_eventchain_id));
}

#define CMD_CHECK_EVENTCHAIN_VERSION 1
void Cmd_CheckEventChain::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CMD_CHECK_EVENTCHAIN_VERSION) {
			GameLogicCommand::Read(fr, egbase, mol);
			m_eventchain_id = fr.Unsigned16();
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("check eventchain: %s", e.what());
	}
}
void Cmd_CheckEventChain::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	fw.Unsigned16(CMD_CHECK_EVENTCHAIN_VERSION);
	GameLogicCommand::Write(fw, egbase, mos);
	fw.Unsigned16(m_eventchain_id);
}

};
