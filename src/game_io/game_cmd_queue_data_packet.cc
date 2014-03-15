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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "game_io/game_cmd_queue_data_packet.h"

#include "logic/cmd_queue.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/queue_cmd_factory.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"
#include "upcast.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 2


void Game_Cmd_Queue_Data_Packet::Read
	(FileSystem & fs, Game & game, Map_Map_Object_Loader * const ol)
{
	try {
		FileRead fr;
		fr.Open(fs, "binary/cmd_queue");
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CURRENT_PACKET_VERSION) {
			Cmd_Queue & cmdq = game.cmdqueue();

			// nothing to be done for m_game

			// Next serial
			cmdq.nextserial = fr.Unsigned32();

			// Erase all currently pending commands in the queue
			cmdq.flush();

			for (;;) {
				uint32_t const packet_id = fr.Unsigned16();

				if (!packet_id)
					break;

				Cmd_Queue::cmditem item;
				item.category = fr.Signed32();
				item.serial = fr.Unsigned32();

				if (packet_id == 129) {
					// For backwards compatibility with savegames up to build15:
					// Discard old CheckEventChain commands
					fr.Unsigned16(); // CheckEventChain version
					fr.Unsigned16(); // GameLogicCommand version
					fr.Unsigned32(); // GameLogicCommand duetime
					fr.Unsigned16(); // CheckEventChain ID
					continue;
				}

				GameLogicCommand & cmd =
					Queue_Cmd_Factory::create_correct_queue_command(packet_id);
				cmd.Read(fr, game, *ol);

				item.cmd = &cmd;

				cmdq.m_cmds[cmd.duetime() % CMD_QUEUE_BUCKET_SIZE].push(item);
				++ cmdq.m_ncmds;
			}
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("command queue: %s", e.what());
	}
}


void Game_Cmd_Queue_Data_Packet::Write
	(FileSystem & fs, Game & game, Map_Map_Object_Saver * const os)
{
	FileWrite fw;

	// Now packet version
	fw.Unsigned16(CURRENT_PACKET_VERSION);

	const Cmd_Queue & cmdq = game.cmdqueue();

	// nothing to be done for m_game

	// Next serial
	fw.Unsigned32(cmdq.nextserial);

	// Write all commands

	// Find all the items in the current cmdqueue
	int32_t time = game.get_gametime();
	size_t nhandled = 0;

	while (nhandled < cmdq.m_ncmds) {
		// Make a copy, so we can pop stuff
		std::priority_queue<Cmd_Queue::cmditem> p = cmdq.m_cmds[time % CMD_QUEUE_BUCKET_SIZE];

		while (!p.empty()) {
			const Cmd_Queue::cmditem & it = p.top();
			if (it.cmd->duetime() == time) {
				if (upcast(GameLogicCommand, cmd, it.cmd)) {
					// The id (aka command type)
					fw.Unsigned16(cmd->id());

					// Serial number
					fw.Signed32(it.category);
					fw.Unsigned32(it.serial);

					// Now the command itself
					cmd->Write(fw, game, *os);
				}
				++ nhandled;
			}

			// DONE: next command
			p.pop();
		}
		++time;
	}


	fw.Unsigned16(0); // end of command queue

	fw.Write(fs, "binary/cmd_queue");
}

}
