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

#include "base/macros.h"
#include "base/wexception.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "io/machdep.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/instances.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "logic/worker.h"

namespace Widelands {

//
// class Cmd_Queue
//
CmdQueue::CmdQueue(Game & game) :
	m_game(game),
	nextserial(0),
	m_ncmds(0),
	m_cmds(CMD_QUEUE_BUCKET_SIZE, std::priority_queue<CmdItem>()) {}

CmdQueue::~CmdQueue()
{
	flush();
}

/*
 * flushs all commands from the queue. Needed for
 * game loading (while in game)
 */
// TODO(unknown): ...but game loading while in game is not possible!
// Note: Order of destruction of Items is not guaranteed
void CmdQueue::flush() {
	uint32_t cbucket = 0;
	while (m_ncmds && cbucket < CMD_QUEUE_BUCKET_SIZE) {
		std::priority_queue<CmdItem> & current_cmds = m_cmds[cbucket];

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
void CmdQueue::enqueue (Command * const cmd)
{
	CmdItem ci;

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

int32_t CmdQueue::run_queue(int32_t const interval, int32_t & game_time_var) {
	int32_t const final = game_time_var + interval;
	int32_t cnt = 0;

	while (game_time_var < final) {
		std::priority_queue<CmdItem> & current_cmds = m_cmds[game_time_var % CMD_QUEUE_BUCKET_SIZE];

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
				ss.data(tag, 3); // provide an easy-to-find pattern as debugging aid
				ss.unsigned_32(c.duetime());
				ss.unsigned_32(c.id());
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

constexpr uint16_t kCurrentPacketVersion = 1;

/**
 * Write variables from the base command to a file.
 *
 * \note This function must be called by deriving objects that override it.
 */
void GameLogicCommand::write
	(FileWrite & fw,
#ifndef NDEBUG
	 EditorGameBase & egbase,
#else
	 EditorGameBase &,
#endif
	 MapObjectSaver &)
{
	fw.unsigned_16(kCurrentPacketVersion);

	// Write duetime
	assert(egbase.get_gametime() <= duetime());
	fw.unsigned_32(duetime());
}

/**
 * Read variables for the base command from a file.
 *
 * \note This function must be called by deriving objects that override it.
 */
void GameLogicCommand::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader &)
{
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			set_duetime(fr.unsigned_32());
			int32_t const gametime = egbase.get_gametime();
			if (duetime() < gametime)
				throw GameDataError
					("duetime (%i) < gametime (%i)", duetime(), gametime);
		} else {
			throw UnhandledVersionError(packet_version, kCurrentPacketVersion);
		}
	} catch (const WException & e) {
		throw GameDataError("game logic: %s", e.what());
	}
}

}
