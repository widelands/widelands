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
   
   cerr << "Running queue!" << endl;

   uint i=0;
   Cmd* c;
   while(i<ncmds) {
      c=&cmds[i];
      
      switch(c->cmd) {
         case CMD_LOAD_MAP:
            g->map = new Map();
            if(g->map->load_map((char*)c->arg3, this)) {
               // TODO: make this better
               assert(0) ;
            }
            free(c->arg3); c->arg3=0;
            break;

         default:
            cerr << "Unknown Queue_Cmd: " << c->cmd << endl;
            break;
      }
      
      i++;
   }

   ncmds=0;

   return RET_OK;
}

