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
#include "player.h"
#include "queue_cmd_factory.h"
#include "transport.h"
#include "widelands_map_saver.h"
#include "widelands_map_loader.h"


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
   Widelands_Map_Map_Object_Saver* m_mos;
   
   // First of all, save some kind of header.
   fw.Unsigned32(m_game->get_gametime()); // Time in milliseconds of elapsed game time (without pauses)
   fw.CString(m_game->get_map()->get_name()); // Name of map

   // Now write the game
   save_game_class(&fw);

   // Now Write the map as it would be a normal map saving
   Widelands_Map_Saver wms(&fw, m_game);
   wms.save();
   m_mos=wms.get_map_object_saver();

   log(" Writing Player Economies!");
   bool done=false;
   for(uint i=1; i<=m_game->get_map()->get_nrplayers(); i++) {
      Player* plr=m_game->get_player(i);
      fw.Unsigned16(plr->m_economies.size());
      for(uint j=0; j<plr->m_economies.size(); j++) {
         done=false;
         // Walk the map so that we find a representant
         Map* map=m_game->get_map();
         for(ushort y=0; y<map->get_height(); y++) {
            for(ushort x=0; x<map->get_width(); x++) {
               BaseImmovable* imm=map->get_field(Coords(x,y))->get_immovable();
               if(!imm) continue;

               if(imm->get_type()==Map_Object::FLAG) {
                  Flag* flag=static_cast<Flag*>(imm);
                  if(flag->get_economy() == plr->m_economies[j]) {
                     fw.Unsigned16(x);
                     fw.Unsigned16(y);
                     done=true;
                  }
               }
               if(done) break;
            }
            if(done) break;
         }
         if(done) continue;
      }
   }
   
   // Now write the command queue
   log(" Writing cmd_queue!\n");
   save_cmd_queue_class(&fw, m_mos);
   
   fw.Write(g_fs, m_filename.c_str());
}

/*
 * Saves all data concerning the game class
 */
void Game_Saver::save_game_class(FileWrite* fw) throw(wexception) {
      // GAME CLASS
   // Can't save netgames
   assert(!m_game->m_netgame);

   // State is running, we do not need to save this
	// Save speed
   fw->Signed16(m_game->m_speed);

   // From the interactive player, we only
   // need the player number
   fw->Unsigned8(m_game->ipl->get_player_number());
	
   // WE DO NOT SAVE COMPUTER PLAYERS AT THE MOMENT // TODO
 	
   // CMD Queue is saved later
   // We do not care for real time. 

       // EDITOR GAME CLASS
   // Write gametime
   fw->Unsigned32(m_game->m_gametime);
   
   // We do not care for players, since they were set
   // on game initialization to match Map::scenario_player_[names|tribes]
   // or vice versa, so this is handled by map loader
   
   // Objects are loaded and saved by map
   
   // Tribes and wares are handled by map
   // Interactive_Base doesn't need saving

   // Map is handled by map saving

   // Track pointers are not saved in save games
}

/*
 * Saves all data concerning the cmd_queue class
 */
void Game_Saver::save_cmd_queue_class(FileWrite* fw, Widelands_Map_Map_Object_Saver* mos) throw(wexception) {
   Cmd_Queue* cmdq=m_game->get_cmdqueue();

   // nothing to be done for m_game
   
   // Next serial
   fw->Unsigned32(cmdq->nextserial);

   // Number of cmds
   fw->Unsigned16(cmdq->m_cmds.size());

   // Write all commands
   std::priority_queue<Cmd_Queue::cmditem> p;
   
   // Make a copy, so we can pop stuff
   p=cmdq->m_cmds;
   
   assert(p.top().serial==cmdq->m_cmds.top().serial);
   assert(p.top().cmd==cmdq->m_cmds.top().cmd);

   while(p.size()) {
      // Serial number
      fw->Unsigned32(p.top().serial);
     
      // Now the id
      fw->Unsigned16(p.top().cmd->get_id());

      // Now the command itself
      p.top().cmd->Write(fw, m_game, mos);
      // DONE: next command
      p.pop();
   }
}

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

   m_game->get_map()->recalc_whole_map();

   log(" Loading player economies!\n");
   Map* map=m_game->get_map();
   for(uint i=1; i<=m_game->get_map()->get_nrplayers(); i++) {
      Player* plr=m_game->get_player(i);
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

void Game_Saver::load_cmd_queue_class(FileRead* fr, Widelands_Map_Map_Object_Loader* mol) throw (wexception) {
   Cmd_Queue* cmdq=m_game->get_cmdqueue();

   // nothing to be done for m_game
   
   // Next serial
   cmdq->nextserial=fr->Unsigned32();

   // Erase all currently pending commands in the queue
   while(!cmdq->m_cmds.empty())
      cmdq->m_cmds.pop();
   
   // Number of cmds
   uint ncmds=fr->Unsigned16();

   uint i=0;
   while(i<ncmds) {
      Cmd_Queue::cmditem item;
      item.serial=fr->Unsigned32();

      uint packet_id=fr->Unsigned16();
      log("Creating queue command for id: %i (serial: %li) ... ", packet_id, item.serial);
      BaseCommand* cmd=Queue_Cmd_Factory::create_correct_queue_command(packet_id);
      cmd->Read(fr, m_game, mol);
      log("done\n");

      item.cmd=cmd;

      cmdq->m_cmds.push(item);
      ++i;
   }
}

void Game_Saver::load_game_class(FileRead* fr) throw (wexception) {
      // GAME CLASS
   // Can't save netgames
   m_game->m_netgame=0;

   m_game->m_state=gs_running;
   m_game->m_speed=fr->Signed16();

   // From the interactive player, we only
   // need the player number
   m_game->ipl->set_player_number(fr->Unsigned8());
	
   // WE DO NOT SAVE COMPUTER PLAYERS AT THE MOMENT // TODO
 	
   // CMD Queue is saved later
   // We do not care for real time. 

       // EDITOR GAME CLASS
   // Write gametime
   m_game->m_gametime=fr->Unsigned32();
   
   // We do not care for players, since they were set
   // on game initialization to match Map::scenario_player_[names|tribes]
   // or vice versa, so this is handled by map loader
   
   // Objects are loaded and saved by map
   
   // Tribes and wares are handled by map
   // Interactive_Base doesn't need saving

   // Map is handled by map saving

   // Track pointers are not saved in save games
}

