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

#include "filesystem.h"
#include "game.h"
#include "game_saver.h"
#include "IntPlayer.h"
#include "map.h"
#include "overlay_manager.h"
#include "player.h"
#include "queue_cmd_factory.h"
#include "transport.h"
#include "widelands_map_saver.h"
#include "widelands_map_loader.h"

#include "game_cmd_queue_data_packet.h"
#include "game_data_packet_ids.h"
#include "game_game_class_data_packet.h"
#include "game_map_data_packet.h"
#include "game_preload_data_packet.h"
#include "game_interactive_player_data_packet.h"
#include "game_player_economies_data_packet.h"
#include "game_player_info_data_packet.h"


/*
 * Game Saver, creation and destruction
 */
Game_Saver::Game_Saver(std::string filename, Game* game) {
   m_filename=filename;
   m_game=game;
}

Game_Saver::~Game_Saver(void) {
}

/*
 * The core save function
 */
void Game_Saver::save(void) throw(wexception) {
   FileWrite fw;
   Game_Data_Packet* gp;
   Game_Map_Data_Packet* gmdp;
   
   log("Game: Writing Preload Data ... ");
   gp = new Game_Preload_Data_Packet();
   gp->Write(&fw, m_game, 0); 
   delete gp;
   log(" done\n");

   log("Game: Writing Game Class Data ... ");
   gp = new Game_Game_Class_Data_Packet();
   gp->Write(&fw, m_game, 0); 
   delete gp;
   log(" done\n");

   log("Game: Writing Player Info ... ");
   gp = new Game_Player_Info_Data_Packet();
   gp->Write(&fw, m_game, 0); 
   delete gp;
   log(" done\n");
   
   log("Game: Writing Map Data!\n");
   gmdp = new Game_Map_Data_Packet();
   gmdp->Write(&fw, m_game, 0);
   Widelands_Map_Map_Object_Saver *mos = gmdp->get_map_object_saver();
   log("Game: Writing Map Data done!\n");

   log("Game: Writing Player Economies Info ... ");
   gp = new Game_Player_Economies_Data_Packet();
   gp->Write(&fw, m_game, mos); 
   delete gp;
   log(" done\n");
   
   log("Game: Writing Command Queue Data ... ");
   gp = new Game_Cmd_Queue_Data_Packet();
   gp->Write(&fw, m_game, mos); 
   delete gp;
   log(" done\n");
 
   log("Game: Writing Interactive Player Data ... ");
   gp = new Game_Interactive_Player_Data_Packet();
   gp->Write(&fw, m_game, mos); 
   delete gp;
   log(" done\n");

   delete gmdp; // Deletes our map object saver

   fw.Write(g_fs, m_filename.c_str());
}

/*
void Game_Saver::load(void) throw (wexception) {
   FileRead fr;
   Widelands_Map_Map_Object_Loader* m_mol;
  
   fr.Open(g_fs, m_filename.c_str());
  
   ALIVE();

   // First of all, save some kind of header.
   fr.Unsigned32();
   fr.CString();

   ALIVE();

   // Now write the game
   load_game_class(&fr);

   ALIVE();
   // Now Load the map as it would be a normal map saving
   Widelands_Map_Loader wml(&fr, m_game->get_map());
   int filepos = fr.GetFilePos();
   
   // Preload map
   wml.preload_map(1);
   // Reset filepos
   fr.SetFilePos(filepos);

   // Now create the players accordingly
   for(uint i=0; i<m_game->get_map()->get_nrplayers(); i++) {
      std::string name = m_game->get_map()->get_scenario_player_name(i+1);
      std::string tribe = m_game->get_map()->get_scenario_player_tribe(i+1);
      
      if(name == "" && tribe == "") continue; // doesn't exists

      log("Creating player %i: <%s> with tribe <%s>\n", i+1, m_game->get_map()->get_scenario_player_tribe(i+1).c_str(), m_game->get_map()->get_scenario_player_name(i+1).c_str());
      m_game->add_player(i+1, i==0 ? Player::playerLocal : Player::playerAI, 
            m_game->get_map()->get_scenario_player_tribe(i+1).c_str(),
            m_game->get_map()->get_scenario_player_name(i+1).c_str());      // TODO: this must be saved somewhere 
      m_game->get_player(i+1)->init(m_game,0); 
   }
  
   // Now load the map
   wml.load_map_complete(m_game, true);
   ALIVE();
   m_mol=wml.get_map_object_loader();
   ALIVE();
   
     log(" Loading player economies!\n");
   Map* map=m_game->get_map();
   for(uint i=1; i<=m_game->get_map()->get_nrplayers(); i++) {
      Player* plr=m_game->get_player(i);
      if(!plr) continue;

      uint nr_economies=fr.Unsigned16();
      assert(nr_economies == plr->m_economies.size());

      std::vector<Economy*> ecos;
      ecos.resize(nr_economies);

      for(uint j=0; j<plr->m_economies.size(); j++) {
         int x=fr.Unsigned16();
         int y=fr.Unsigned16();
         Flag* flag=static_cast<Flag*>(map->get_field(Coords(x,y))->get_immovable());
         assert(flag);
         ecos[j]=flag->get_economy();
      }
      for(uint i=0; i<ecos.size(); i++) { 
         plr->m_economies[i]=ecos[i];
         ecos[i]->balance_requestsupply(); // Issue first balance
      }
   }

   // Now write the command queue
   ALIVE();
   load_cmd_queue_class(&fr, m_mol);
   ALIVE();

}
*/
