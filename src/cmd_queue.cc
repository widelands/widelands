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
   cur_frame=0;
   cur_cmd=0;
   
   uint i;
   for(i=0; i<FRAMES_IN_ADVANCE; i++) {
         frames[i].first.sender=0;
         frames[i].first.cmd=0;
         frames[i].first.arg1=0;
         frames[i].first.arg2=0;
         frames[i].first.arg3=0;
         frames[i].first.next=0;
         frames[i].last=&frames[i].first;
   }
   
   for(i=0; i<MAX_CMDS; i++) {
      cmds[i].sender=0;
      cmds[i].cmd=0;
      cmds[i].arg1=0;
      cmds[i].arg2=0;
      cmds[i].arg3=0;
      cmds[i].next=0;
   }
}

Cmd_Queue::~Cmd_Queue(void) {
   // nothig to clean
}

// Queue a new command
void Cmd_Queue::queue(uint frame, uchar sender, ushort cmd, ulong arg1, ulong arg2, void* arg3) {
   assert(frame < FRAMES_IN_ADVANCE);
   frame+=cur_frame;
   if(frame>=FRAMES_IN_ADVANCE) frame-=FRAMES_IN_ADVANCE;

   Cmd* pcmd=&cmds[cur_cmd];
   while(pcmd->cmd) { pcmd=&cmds[cur_cmd++]; if(cur_cmd==MAX_CMDS-1) cur_cmd=0; }

   pcmd->sender=sender;
   pcmd->cmd=cmd;
   pcmd->arg1=arg1;
   pcmd->arg2=arg2;
   pcmd->arg3=arg3;
   pcmd->next=0;

   frames[frame].last->next=pcmd;
   frames[frame].last=pcmd;
}

// main function, run the queued commands
int Cmd_Queue::run_queue(void) {
   int temp=0;
   Cmd* c, *tempc;
   Instance* inst;
  
   c=frames[cur_frame].first.next;


   while(c) {
      switch(c->cmd) {
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
            g->get_map()->get_field(((Point*) c->arg3)->x, ((Point*) c->arg3)->y)->hook_instance(inst);
            temp=g->get_player_tribe(c->arg1)->get_building_descr(c->arg2)->create_instance(inst);
            inst->set_owned_by((uchar)c->arg1);
            if(temp>=0) queue(temp, SENDER_QUEUE, CMD_ACT, 0, 0, inst);
            //inst->set_next_acting_frame(g->get_frame()+temp);
            break;

         case CMD_CREATE_BOB:
            // create a bob in a instance 
            // arg1==bob_index
            // arg2 unused!
            // arg3=pointer to point struct where to build the bobs
            temp=g->hinst->get_free_inst_id();
            inst=g->hinst->get_inst(temp);
            g->get_map()->get_field(((Point*) c->arg3)->x, ((Point*) c->arg3)->y)->hook_instance(inst);
            temp=g->get_map()->get_world()->get_bob_descr((ushort)c->arg1)->create_instance(inst);
            inst->set_owned_by(SENDER_LOADER);
            if(temp>=0) queue(temp, SENDER_QUEUE, CMD_ACT, 0, 0, inst);
            //inst->set_next_acting_frame(g->get_frame()+temp);
            break;

         case CMD_ACT:
            // an instance has requested to act again after a time
            // arg3==(Instance*) pointer_to_instance to act
            assert(c->arg3);
            inst=(Instance*) c->arg3;
            temp=inst->act(g);
            if(temp>=0) queue(temp, SENDER_QUEUE, CMD_ACT, 0, 0, inst);
            c->arg3=0; // prevent from freeing
            break;

         default:
            break;
      }
      
     
      tempc=c->next;
      // cleanup this cmd
      if(c->arg3) {
         free(c->arg3); 
         c->arg3=0;
      }
      c->cmd=0; // make public again
      c->next=0; // make public again
      
      c=tempc;
   }

   frames[cur_frame].first.next=0;
   frames[cur_frame].last=&frames[cur_frame].first;

   ++cur_frame;
   if(cur_frame==FRAMES_IN_ADVANCE) cur_frame=0;

   return RET_OK;
}

