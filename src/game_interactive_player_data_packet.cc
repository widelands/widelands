/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

#include "game.h"
#include "game_interactive_player_data_packet.h"
#include "IntPlayer.h"
#include "mapview.h"
#include "overlay_manager.h"

#define CURRENT_PACKET_VERSION 1

// Forward declaration. Defined in IntPlayer.cc
int Int_Player_overlay_callback_function(FCoords& fc, void* data, int);

/*
 * Destructor
 */
Game_Interactive_Player_Data_Packet::~Game_Interactive_Player_Data_Packet(void) {
}

/*
 * Read Function
 */
void Game_Interactive_Player_Data_Packet::Read(FileRead* fr, Game* game, Widelands_Map_Map_Object_Loader*) throw(wexception) {
   // read packet version
   int packet_version=fr->Unsigned16();

   if(packet_version==CURRENT_PACKET_VERSION) {
      Interactive_Player* plr = game->get_ipl();

      plr->m_player_number = fr->Unsigned8();

      // Main Menu is not closed
      
      if(plr->m_fieldaction.window) {
         delete plr->m_fieldaction.window;
         plr->m_fieldaction.window = 0;
      }

      // Map Position
      int x = fr->Unsigned16();
      int y = fr->Unsigned16();
      plr->m_mapview->set_viewpoint(Point(x,y));

      plr->m_display_flags = fr->Unsigned32();

      if(plr->m_minimap.window) {
         delete plr->m_minimap.window;
         plr->m_minimap.window = 0;
      }

      // Now only restore the callback functions. assumes, map is already loaded
      game->get_map()->get_overlay_manager()->show_buildhelp(false);
      game->get_map()->get_overlay_manager()->register_overlay_callback_function(&Int_Player_overlay_callback_function, static_cast<void*>(plr));

      game->get_map()->recalc_whole_map();
      // DONE
      return;
   } else
      throw wexception("Unknown version in Game_Interactive_Player_Data_Packet: %i\n", packet_version);
   assert(0); // never here
}

/*
 * Write Function
 */
void Game_Interactive_Player_Data_Packet::Write(FileWrite* fw, Game* game, Widelands_Map_Map_Object_Saver*) throw(wexception) {
   // First, id
   fw->Unsigned16(PACKET_INTERACTIVE_PLAYER_DATA);
   
   // Now packet version
   fw->Unsigned16(CURRENT_PACKET_VERSION);
		

   Interactive_Player* plr = game->get_ipl();

   // Player number
   fw->Unsigned8(plr->get_player_number());

   // Map Position
   fw->Unsigned16(plr->m_mapview->get_viewpoint().x);
   fw->Unsigned16(plr->m_mapview->get_viewpoint().y);

   // Display flags
   fw->Unsigned32(plr->m_display_flags);
}
