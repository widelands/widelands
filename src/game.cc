/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#include "widelands.h"
#include "cursor.h"
#include "map.h"
#include "cmd_queue.h"
#include "descr_maintainer.h"
#include "bob.h"
#include "ware.h"
#include "worker.h"
#include "tribe.h"
#include "game.h"

/** class Game
 *
 * This game handels one game. This class is not a all portabel,
 * it depends on nearly everything else in widelands
 */

/** Game::Game(void)
 *
 * init
 */
Game::Game(void) {
   hinst= new Instance_Handler(MAX_OBJS);
   queue = new Cmd_Queue(this);
   map=0;
   frame_count=0;
}


/** Game::~Game(void)
 *
 * cleanup
 */
Game::~Game(void) {
   delete hinst; 
   delete queue;
   delete map;
}

//
//this function loads the map for the game. this must be done, before
//Game::run() is called
//
int Game::set_map(const char* mapname) {
   if(map) return ERR_FAILED;

   queue->queue(SENDER_LOADER, CMD_LOAD_MAP, 0, 0, strdup(mapname));

   return RET_OK;
}
/** void Game::run(void)
 *
 * This runs a game 
 */
#include "ui.h"
#include "fileloc.h"
#include "worldfiletypes.h"
#include "myfile.h"
#include "graphic.h"
void Game::run(void) {
   // TEMP
   tribe= new Tribe_Descr(); 
   const char* str=g_fileloc.locate_file("testtribe.wtf", TYPE_TRIBE);
   assert(str);
   assert(!tribe->load(str));
   // TEMP

   // run the cmd queue, so all the load cmds are worked through
   queue->run_queue();
   
   ipl = new Interactive_Player(this);
   counter.start();
   ipl->run();
   delete ipl;
   delete tribe;
}

//
// This is called by IntPlayer, in each of his loop. This
// function checks, if a frame (time) has passed, if it has, it 
// runs the command queue. 
// 
// think(), mmh, i don't know if i like the name
// 
void Game::think(void) {
   static ulong lticks;
   ulong curticks=counter.get_ticks();
   
   while(curticks-lticks > FRAME_LENGTH) {
//      cerr << "Working frame number: " << frame_count << endl;

      uint i;
      Instance* inst;
      for(i=0; i<MAX_OBJS; i++) {
         inst=hinst->get_inst(i);
         if(inst->get_state() == Instance::USED) {
            if(frame_count >= inst->get_next_acting_frame()) {
               inst->act(this);
               //        cerr << i << " is acting!" << endl;
            }
         }
      }

      queue->run_queue();
      frame_count++;
      lticks=curticks;
   }
}
