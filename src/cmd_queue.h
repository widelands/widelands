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

#ifndef __S__CMD_QUEUE_H
#define __S__CMD_QUEUE_H

// Define here all the possible users
#define SENDER_LOADER 0
#define SENDER_PLAYER1 1 // those are just place holder, a player can send commands with
#define SENDER_PLAYER2 2 // it's player number 
#define SENDER_PLAYER3 3 
#define SENDER_PLAYER4 4 
#define SENDER_PLAYER5 5 
#define SENDER_PLAYER6 6 
#define SENDER_PLAYER7 7
#define SENDER_PLAYER8 8
#define SENDER_ENGINE  50 
#define SENDER_EVENT   51 // a map event has been triggered

// ---------------------- BEGINN OF CMDS ----------------------------------
#define CMD_LOAD_MAP             0
// ---------------------- END    OF CMDS ----------------------------------

//
// This struct defines the commands, which are possible
//
// TODO: think about differencing 'rights', e.g. the 'loader' 
// is allowed to do everything, while a 'player' may not warp a 
// building or so.
// 
struct Cmd {
   uchar sender;
   ushort cmd;
   ulong arg1;
   ulong arg2;
   void* arg3; // don't use, if you can avoid it!
};

#define MAX_CMDS  100    // if more than this commands are queued in one frame, 
                         // the game might freeze, since the cmd queue runs those cmds
                         // first

// 
// This is finally the command queue. It is fully widelands specific,
// it needs to know nearly all modules.
// 
class Game;

class Cmd_Queue {
   public:
      Cmd_Queue(Game *);
      ~Cmd_Queue(void);
     
      void queue(uchar, ushort, ulong=0, ulong=0, void* =0);
      int run_queue(void);
      
   private:
      Cmd cmds[MAX_CMDS];
      Game* g;
      uint ncmds;
};


#endif // __S__CMD_QUEUE_H 
