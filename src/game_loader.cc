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

#include "error.h"
#include "filesystem.h"
#include "game_loader.h"
#include "game_data_packet_factory.h"
#include "game_data_packet_ids.h"
#include "game_map_data_packet.h"
#include "game_preload_data_packet.h"
#include "widelands_map_map_object_loader.h"

/*
 * Constructor
 */
Game_Loader::Game_Loader(const char* filename, Game* game) { 
      m_game=game;
      m_filename=filename;
}

/*
 * Destructor
 */
Game_Loader::~Game_Loader(void) {
}

/*
 * This function preloads a game
 */
int Game_Loader::preload_game(Game_Preload_Data_Packet* mp) {
   FileRead fr;
   fr.Open(g_fs, m_filename.c_str());

   // Load elemental data block
   mp->Read(&fr, m_game);
   
   return 0;
}

/*
 * Load the complete file 
 */
int Game_Loader::load_game(void) {
   
   FileRead fr;
   fr.Open(g_fs, m_filename.c_str());

   // Load elemental data block (again)
   Game_Preload_Data_Packet mp;
   mp.Read(&fr, m_game);
   
   Widelands_Map_Map_Object_Loader* m_mol = 0;
   
   // ok, now go on and load the rest
   Game_Data_Packet_Factory fac;

   Game_Data_Packet* gmdp = 0;
   
   ushort id;
   Game_Data_Packet* pak;
   while(!fr.IsEOF()) {
      id=fr.Unsigned16();
      if(id==PACKET_END_OF_GAME_DATA) break;
      log("Creating Game-packet for id: %i. Reading packet ... ", id);
      pak=fac.create_correct_packet(id);
      pak->Read(&fr, m_game, m_mol);
      log("done\n");
      if(id == PACKET_MAP_DATA) { 
         m_mol = static_cast<Game_Map_Data_Packet*>(pak)->get_map_object_loader();
         gmdp = pak;
      } else 
         delete pak;
   }

   delete gmdp; // deletes m_mol too

   return 0;
}
