/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#include "game_cmd_queue_data_packet.h"

#include "logic/cmd_queue.h"
#include "logic/game.h"
#include "queue_cmd_factory.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"

#include "upcast.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 2


void Game_Cmd_Queue_Data_Packet::Read
	(FileSystem & fs, Game & game, Map_Map_Object_Loader * const ol)
throw (_wexception)
{
	FileRead fr;
	try {
		fr.Open(fs, "binary/cmd_queue");
		uint16_t const packet_version = fr.Unsigned16();
		if (1 <= packet_version) {
			Cmd_Queue & cmdq = game.cmdqueue();

			// nothing to be done for m_game

			// Next serial
			cmdq.nextserial = fr.Unsigned32();

			// Erase all currently pending commands in the queue
			while (!cmdq.m_cmds.empty())
				cmdq.m_cmds.pop();

			if (packet_version == CURRENT_PACKET_VERSION)
				for (;;) {
					uint32_t const packet_id = fr.Unsigned16();

					if (!packet_id)
						break;

					Cmd_Queue::cmditem item;
					item.category = fr.Signed32();
					item.serial = fr.Unsigned32();

					GameLogicCommand & cmd =
						Queue_Cmd_Factory::create_correct_queue_command(packet_id);
					cmd.Read(fr, game, *ol);

					item.cmd = &cmd;

					cmdq.m_cmds.push(item);
				}
			else {
				// Old-style (version 1) command list
				uint32_t const ncmds = fr.Unsigned16();

				uint32_t i = 0;
				while (i < ncmds) {
					Cmd_Queue::cmditem item;
					item.category = Cmd_Queue::cat_gamelogic;
					item.serial = fr.Unsigned32();

					uint32_t const packet_id = fr.Unsigned16();
					GameLogicCommand & cmd =
						Queue_Cmd_Factory::create_correct_queue_command(packet_id);
					cmd.Read(fr, game, *ol);

					item.cmd = &cmd;

					cmdq.m_cmds.push(item);
					++i;
				}
			}
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("command queue: %s", e.what());
	}
}


void Game_Cmd_Queue_Data_Packet::Write
	(FileSystem & fs, Game & game, Map_Map_Object_Saver * const os)
throw (_wexception)
{
	FileWrite fw;

	// Now packet version
	fw.Unsigned16(CURRENT_PACKET_VERSION);

	Cmd_Queue const & cmdq = game.cmdqueue();

	// nothing to be done for m_game

	// Next serial
	fw.Unsigned32(cmdq.nextserial);

	// Write all commands

	// Make a copy, so we can pop stuff
	std::priority_queue<Cmd_Queue::cmditem> p = cmdq.m_cmds;

	assert(p.top().serial == cmdq.m_cmds.top().serial);
	assert(p.top().cmd    == cmdq.m_cmds.top().cmd);

	while (p.size()) {
		Cmd_Queue::cmditem const & it = p.top();

		if (upcast(GameLogicCommand, cmd, it.cmd)) {
			// The id (aka command type)
			fw.Unsigned16(cmd->id());

			// Serial number
			fw.Signed32(it.category);
			fw.Unsigned32(it.serial);

			// Now the command itself
			cmd->Write(fw, game, *os);
		}

		// DONE: next command
		p.pop();
	}

	fw.Unsigned16(0); // end of command queue

	fw.Write(fs, "binary/cmd_queue");
}

};
