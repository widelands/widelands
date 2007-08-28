/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "cmd_queue.h"
#include "error.h"
#include "game.h"
#include "queue_cmd_factory.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"


#define CURRENT_PACKET_VERSION 2


Game_Cmd_Queue_Data_Packet::~Game_Cmd_Queue_Data_Packet() {}


void Game_Cmd_Queue_Data_Packet::Read
(FileSystem & fs, Game* game, Widelands_Map_Map_Object_Loader * const ol)
throw (_wexception)
{
	WidelandsFileRead fr;
   fr.Open(fs, "binary/cmd_queue");

   // read packet version
   int packet_version=fr.Unsigned16();

   if (packet_version >= 1) {
      Cmd_Queue* cmdq=game->get_cmdqueue();

      // nothing to be done for m_game

      // Next serial
      cmdq->nextserial=fr.Unsigned32();

      // Erase all currently pending commands in the queue
      while (!cmdq->m_cmds.empty())
         cmdq->m_cmds.pop();

      // Number of cmds
      uint ncmds=fr.Unsigned16();

      uint i=0;
      while (i<ncmds) {
			Cmd_Queue::cmditem item;
			if (packet_version == CURRENT_PACKET_VERSION)
				item.category = fr.Signed32();
			else
				item.category = Cmd_Queue::cat_gamelogic;
			item.serial = fr.Unsigned32();

         uint packet_id=fr.Unsigned16();
         GameLogicCommand* cmd=Queue_Cmd_Factory::create_correct_queue_command(packet_id);
			cmd->Read(fr, *game, *ol);

         item.cmd=cmd;

         cmdq->m_cmds.push(item);
         ++i;
		}
      // DONE
      return;
	} else
      throw wexception("Unknown version in Game_Cmd_Queue_Data_Packet: %i\n", packet_version);

   assert(0); // never here
}

/*
 * Write Function
 */
void Game_Cmd_Queue_Data_Packet::Write
(FileSystem & fs, Game* game, Widelands_Map_Map_Object_Saver * const os)
throw (_wexception)
{
	WidelandsFileWrite fw;

   // Now packet version
   fw.Unsigned16(CURRENT_PACKET_VERSION);

   Cmd_Queue* cmdq=game->get_cmdqueue();

   // nothing to be done for m_game

   // Next serial
   fw.Unsigned32(cmdq->nextserial);

   // Number of cmds
   fw.Unsigned16(cmdq->m_cmds.size());

   // Write all commands
   std::priority_queue<Cmd_Queue::cmditem> p;

   // Make a copy, so we can pop stuff
   p=cmdq->m_cmds;

   assert(p.top().serial==cmdq->m_cmds.top().serial);
   assert(p.top().cmd==cmdq->m_cmds.top().cmd);

   while (p.size()) {
		const Cmd_Queue::cmditem& it = p.top();

		if (GameLogicCommand* cmd = dynamic_cast<GameLogicCommand*>(it.cmd)) {
			// Serial number
			fw.Signed32(it.category);
			fw.Unsigned32(it.serial);

			// Now the id
			fw.Unsigned16(cmd->get_id());

			// Now the command itself
			cmd->Write(fw, *game, *os);
		}

		// DONE: next command
		p.pop();
	}

   fw.Write(fs, "binary/cmd_queue");
}
