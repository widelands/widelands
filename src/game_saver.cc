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
#include "game.h"
#include "game_cmd_queue_data_packet.h"
#include "game_computer_player_data_packet.h"
#include "game_data_packet_ids.h"
#include "game_game_class_data_packet.h"
#include "game_map_data_packet.h"
#include "game_preload_data_packet.h"
#include "game_interactive_player_data_packet.h"
#include "game_player_economies_data_packet.h"
#include "game_player_info_data_packet.h"
#include "game_saver.h"


/*
 * Game Saver, creation and destruction
 */
Game_Saver::Game_Saver(FileSystem* fs, Game* game) {
   m_fs=fs;
   m_game=game;
}

Game_Saver::~Game_Saver(void) {
}

/*
 * The core save function
 */
void Game_Saver::save(void) throw(wexception) {
   Game_Data_Packet* gp;
   Game_Map_Data_Packet* gmdp;
  
   m_fs->EnsureDirectoryExists( "binary" );

   log("Game: Writing Preload Data ... ");
   gp = new Game_Preload_Data_Packet();
   gp->Write(m_fs, m_game, 0); 
   delete gp;
   log(" done\n");

   log("Game: Writing Game Class Data ... ");
   gp = new Game_Game_Class_Data_Packet();
   gp->Write(m_fs, m_game, 0); 
   delete gp;
   log(" done\n");

   log("Game: Writing Player Info ... ");
   gp = new Game_Player_Info_Data_Packet();
   gp->Write(m_fs, m_game, 0); 
   delete gp;
   log(" done\n");
   
   log("Game: Writing Map Data!\n");
   gmdp = new Game_Map_Data_Packet();
   gmdp->Write(m_fs, m_game, 0);
   Widelands_Map_Map_Object_Saver *mos = gmdp->get_map_object_saver();
   log("Game: Writing Map Data done!\n");

   log("Game: Writing Player Economies Info ... ");
   gp = new Game_Player_Economies_Data_Packet();
   gp->Write(m_fs, m_game, mos); 
   delete gp;
   log(" done\n");
   
   log("Game: Writing Command Queue Data ... ");
   gp = new Game_Cmd_Queue_Data_Packet();
   gp->Write(m_fs, m_game, mos); 
   delete gp;
   log(" done\n");
 
   log("Game: Writing Interactive Player Data ... ");
   gp = new Game_Interactive_Player_Data_Packet();
   gp->Write(m_fs, m_game, mos); 
   delete gp;
   log(" done\n");

   log("Game: Writing Computer Player Data ... ");
   gp = new Game_Computer_Player_Data_Packet();
   gp->Write(m_fs, m_game, mos); 
   delete gp;
   log(" done\n");

   delete gmdp; // Deletes our map object saver
}
