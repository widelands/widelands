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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "logic/cmd_queue.h"

#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/instances.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "logic/widelands_fileread.h"
#include "logic/worker.h"
#include "machdep.h"
#include "upcast.h"
#include "wexception.h"

namespace Widelands {

//
// class Cmd_Queue
//
Cmd_Queue::Cmd_Queue(Game & game) :
	m_game(game),
	nextserial(0),
	m_ncmds(0),
	m_cmds(CMD_QUEUE_BUCKET_SIZE, std::priority_queue<cmditem>()) {}

Cmd_Queue::~Cmd_Queue()
{
	flush();
}

/*
 * flushs all commands from the queue. Needed for
 * game loading (while in game)
 * FIXME ...but game loading while in game is not possible!
 * Note: Order of destruction of Items is not guaranteed
 */
void Cmd_Queue::flush() {
	uint32_t cbucket = 0;
	while (m_ncmds and cbucket < CMD_QUEUE_BUCKET_SIZE) {
		std::priority_queue<cmditem> & current_cmds = m_cmds[cbucket];

		while (!current_cmds.empty()) {
			Command * cmd = current_cmds.top().cmd;
			current_cmds.pop();
			delete cmd;
			--m_ncmds;
		}
		++ cbucket;
	}
	assert(m_ncmds == 0);
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

	m_cmds[cmd->duetime() % CMD_QUEUE_BUCKET_SIZE].push(ci);
	++ m_ncmds;
}

int32_t Cmd_Queue::run_queue(int32_t const interval, int32_t & game_time_var) {
	int32_t const final = game_time_var + interval;
	int32_t cnt = 0;

	while (game_time_var < final) {
		std::priority_queue<cmditem> & current_cmds = m_cmds[game_time_var % CMD_QUEUE_BUCKET_SIZE];

		while (!current_cmds.empty()) {
			Command & c = *current_cmds.top().cmd;
			if (game_time_var < c.duetime())
				break;

			current_cmds.pop();
			-- m_ncmds;
			assert(game_time_var == c.duetime());

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
		++game_time_var;
	}

	assert(final - game_time_var == 0);
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
				throw game_data_error
					("duetime (%i) < gametime (%i)", duetime(), gametime);
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("game logic: %s", e.what());
	}
}

}
