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
#include "game.h"
#include "map.h"
#include "cmd_queue.h"

// 
// class Cmd_Queue
//
Cmd_Queue::Cmd_Queue(Game* gg) {
   g=gg;
   ncmds=0;
}

Cmd_Queue::~Cmd_Queue(void) {
   uint i;
   for(i=0; i<ncmds; i++) {
      assert(!cmds[i].arg3);
   }
}

// Queue a new command
void Cmd_Queue::queue(uchar sender, ushort cmd, ulong arg1, ulong arg2, void* arg3) {
   cerr << "queue: ncmds:" << ncmds << endl;
   if(ncmds == MAX_CMDS-1) run_queue();
   
   cmds[ncmds].sender=sender;
   cmds[ncmds].cmd=cmd;
   cmds[ncmds].arg1=arg1;
   cmds[ncmds].arg2=arg2;
   cmds[ncmds].arg3=arg3;
   ++ncmds;
}

// main function, run the queued commands
int Cmd_Queue::run_queue(void) {
   
   uint i=0, temp=0;
   Cmd* c;
   Instance* inst;
   ushort cmd;
   
   cerr << "running queue! ncmds:" << ncmds << endl;

   while(i<ncmds) {
      c=&cmds[i];
      cmd=c->cmd;
      c->cmd=SKIP;
      
      switch(cmd) {
         case CMD_LOAD_MAP:
            // arg3 is file name of map
            g->map = new Map();
            if(g->map->load_map((char*)c->arg3, this)) {
               // TODO: make this better
               assert(0) ;
            }
            break;

         case CMD_WARP_BUILDING:
            // create a building in a instance (without build time)
            // arg1==player number to own the building
            // arg2==index of building
            // arg3==pointer to point struct where to build the building
            temp=g->hinst->get_free_inst_id();
            inst=g->hinst->get_inst(temp);
            cerr << "Hooking instance: " << ((Point*) c->arg3)->x << ":" << ((Point*) c->arg3)->y << endl;
            g->get_map()->get_field(((Point*) c->arg3)->x, ((Point*) c->arg3)->y)->hook_instance(inst);
            temp=g->get_player_tribe(c->arg1)->get_building_descr(c->arg2)->create_instance(inst);
            inst->set_owned_by(c->arg1);
            inst->set_next_acting_frame(g->get_frame()+temp);
            break;

         case CMD_CREATE_BOB:
            // create a bob in a instance 
            // arg1==bob_index
            // arg2 unused!
            // arg3=pointer to point struct where to build the bobs
            temp=g->hinst->get_free_inst_id();
            inst=g->hinst->get_inst(temp);
            g->get_map()->get_field(((Point*) c->arg3)->x, ((Point*) c->arg3)->y)->hook_instance(inst);
            temp=g->get_map()->get_world()->get_bob_descr(c->arg1)->create_instance(inst);
            inst->set_owned_by(SENDER_LOADER);
            inst->set_next_acting_frame(g->get_frame()+temp);
            break;
            
         default:
            cerr << "Unknown Queue_Cmd: " << c->cmd << endl;
            break;
      }
            
      if(c->arg3) {
         free(c->arg3); 
         c->arg3=0;
      }
      
      i++;
   }

   cerr << "queue ends1!: ncmds" << ncmds << endl;
   ncmds=0;
   cerr << "queue ends2!: ncmds" << ncmds << endl;

   return RET_OK;
}

