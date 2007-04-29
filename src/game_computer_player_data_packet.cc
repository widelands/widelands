/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#include "game_computer_player_data_packet.h"

#include "computer_player.h"
#include "game.h"
#include "fileread.h"
#include "filewrite.h"
#include "mapview.h"

#define CURRENT_PACKET_VERSION 1


Game_Computer_Player_Data_Packet::~Game_Computer_Player_Data_Packet() {}


void Game_Computer_Player_Data_Packet::Read
(FileSystem & fs, Game *, Widelands_Map_Map_Object_Loader * const)
throw (_wexception)
{
   FileRead fr;

   fr.Open( fs, "binary/computer_player");
   // read packet version
   int packet_version=fr.Unsigned16();

   if(packet_version==CURRENT_PACKET_VERSION) {
      // This packet currently does nothing at all. The AI doesn't has any saved data
      // if you add anything here, remember to increase the version count (for backwards compatibility)

      // DONE
      return;
   } else
      throw wexception("Unknown version in Game_Computer_Player_Data_Packet: %i\n", packet_version);

   assert(0); // never here
}

/*
 * Write Function
 */
void Game_Computer_Player_Data_Packet::Write
(FileSystem & fs, Game *, Widelands_Map_Map_Object_Saver * const)
throw (_wexception)
{
   FileWrite fw;

   // Now packet version
   fw.Unsigned16(CURRENT_PACKET_VERSION);

   // Sorry, not anything more currently
   fw.Write( fs, "binary/computer_player");
}
