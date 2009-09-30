/*
 * Copyright (C) 2002-2004, 2007-2008 by the Widelands Development Team
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

#include "cmd_queue.h"

#include "io/filewrite.h"
#include "game.h"
#include "instances.h"
#include "machdep.h"
#include "player.h"
#include "playercommand.h"
#include "trigger/trigger.h"
#include "wexception.h"
#include "widelands_fileread.h"
#include "worker.h"

#include "upcast.h"

namespace Widelands {

//
// class Cmd_Queue
//
Cmd_Queue::Cmd_Queue(Game & game) : m_game(game), nextserial(0) {}

Cmd_Queue::~Cmd_Queue()
{
	flush();
}

/*
 * flushs all commands from the queue. Needed for
 * game loading (while in game)
 * FIXME ...but game loading while in game is not possible!
 */
void Cmd_Queue::flush() {
	while (!m_cmds.empty()) {
		delete m_cmds.top().cmd;
		m_cmds.pop();
	}
}

/*
===============
Insert a new command into the queue; it will be executed at the given time
===============
*/
void Cmd_Queue::enqueue (Command * const cmd)
{
	cmditem ci;

	ci.cmd = cmd;
	if (upcast(PlayerCommand, plcmd, cmd)) {
		ci.category = cat_playercommand;
		ci.serial = plcmd->cmdserial();
	} else if (dynamic_cast<GameLogicCommand *>(cmd)) {
		ci.category = cat_gamelogic;
		ci.serial = nextserial++;
	} else {
		// the order of non-gamelogic commands matters only with respect to
		// gamelogic commands; the order of non-gamelogic commands wrt other
		// non-gamelogic commands shouldn't matter, so we can assign a
		// constant serial number.
		ci.category = cat_nongamelogic;
		ci.serial = 0;
	}

	m_cmds.push(ci);
}

/**
 * Run all commands scheduled for the next interval milliseconds, and update the
 * internal time as well.
 * the game_time_var represents the current game time, which we update and with
 * which we must mess around (to run all queued cmd.s) and which we update (add
 * the interval)
 */
int32_t Cmd_Queue::run_queue(int32_t const interval, int32_t & game_time_var) {
	int32_t const final = game_time_var + interval;
	int32_t cnt = 0;

	while (!m_cmds.empty()) {
		Command & c = *m_cmds.top().cmd;
		if (final <= c.duetime())
			break;

		m_cmds.pop();

		assert(game_time_var <= c.duetime());
		game_time_var = c.duetime();

		if (dynamic_cast<GameLogicCommand *>(&c)) {
			StreamWrite & ss = m_game.syncstream();
			static uint8_t const tag[] = {0xde, 0xad, 0x00};
			ss.Data(tag, 3); // provide an easy-to-find pattern as debugging aid
			ss.Unsigned32(c.duetime());
			ss.Unsigned32(c.id());
		}

		c.execute (m_game);

		delete &c;
	}

	assert(final - game_time_var >= 0);
	game_time_var = final;

	return cnt;
}


Command::~Command () {}


#define BASE_CMD_VERSION 1

/**
 * Write variables from the base command to a file.
 *
 * \note This function must be called by deriving objects that override it.
 */
void GameLogicCommand::Write
	(FileWrite & fw,
#ifndef NDEBUG
	 Editor_Game_Base & egbase,
#else
	 Editor_Game_Base &,
#endif
	 Map_Map_Object_Saver &)
{
	// First version
	fw.Unsigned16(BASE_CMD_VERSION);

	// Write duetime
	assert(egbase.get_gametime() <= duetime());
	fw.Unsigned32(duetime());
}

/**
 * Read variables for the base command from a file.
 *
 * \note This function must be called by deriving objects that override it.
 */
void GameLogicCommand::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader &)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == BASE_CMD_VERSION) {
			set_duetime(fr.Unsigned32());
			int32_t const gametime = egbase.get_gametime();
			if (duetime() < gametime)
				throw wexception
					("duetime (%i) < gametime (%i)", duetime(), gametime);
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("game logic: %s", e.what());
	}
}

}
