/*
 * Copyright (C) 2002-2004 by the Widelands Development Team
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

#include <queue>

// Define here all the possible users
#define SENDER_MAPOBJECT 0
#define SENDER_PLAYER1 1 // those are just place holder, a player can send commands with
#define SENDER_PLAYER2 2 // it's player number
#define SENDER_PLAYER3 3
#define SENDER_PLAYER4 4
#define SENDER_PLAYER5 5
#define SENDER_PLAYER6 6
#define SENDER_PLAYER7 7
#define SENDER_PLAYER8 8
#define SENDER_CMDQUEUE 100   // The Cmdqueue sends itself some action request


// ---------------------- BEGINN OF CMDS ----------------------------------
enum {
   UNUSED = 0,
   CMD_ACT,				// arg1 = serialnum, arg2 = user-defined data
	CMD_DESTROY,		// arg1 = serialnum
	CMD_INCORPORATE,	// arg1 = serialnum (Worker)
	CMD_CALL,			// arg1 = fnptr (Cmd_Queue::call_fn), arg2, arg3 = args for function
   CMD_CHECK_TRIGGER,// arg1 = last checked trigger number or -1 if no trigger was ever checked
	// Begin commands sent by players
	CMD_BUILD_FLAG,	// arg1 = x, arg2 = y
	CMD_BUILD_ROAD,	// arg1 = Path*
	CMD_BUILD,			// arg1 = x, arg2 = y, arg3 = build_descr idx
	CMD_BULLDOZE,		// arg1 = serialnum (PlayerImmovable)

	CMD_FLAGACTION,	// arg1 = serialnum of flag, arg2 = action number
};

// arg2 of CMD_FLAGACTION is one of these:
enum {
	FLAGACTION_GEOLOGIST = 0,	// call a geologist
};
// ---------------------- END    OF CMDS ----------------------------------

//
// This is finally the command queue. It is fully widelands specific,
// it needs to know nearly all modules.
//
class Game;

class Cmd_Queue {
	struct Cmd {
		int time; // scheduled time of execution
		char sender;
		int cmd;
		int arg1; // the meaning of arg? depends on cmd
		int arg2;
		int arg3;
	};
	struct CmdCompare {
	public:
		bool operator() (const Cmd& c1, const Cmd& c2) {
			return c1.time > c2.time;
		}
	};
	typedef std::priority_queue<Cmd, std::vector<Cmd>, CmdCompare> queue_t;

	public:
		typedef void (call_fn_t)(Game* g, int arg1, int arg2);

   public:
      Cmd_Queue(Game *g);
      ~Cmd_Queue(void);

		void queue(int time, char sender, int cmd, int arg1=0, int arg2=0, int arg3=0);
      int run_queue(int interval, int* game_time_var);

   private:
		void exec_cmd(const Cmd *c);
		void clear_cmd(const Cmd* c);

      Game *m_game;
		queue_t m_cmds;
};

#endif // __S__CMD_QUEUE_H
