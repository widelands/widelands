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
	CMD_START_STOP_BUILDING,  // arg1 = serialnummer of building start/stop building
   CMD_ENHANCE_BUILDING // enhance this building to another arg1 = serialnummer of building, arg2 = Tribe_Descr::get_building() id of new building
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

class BaseCommand {
	private:
		int duetime;

	public:
		BaseCommand (int);
		virtual ~BaseCommand ();

		virtual void execute (Game*)=0;
		
		int get_duetime() const { return duetime; }
};

class Cmd_Queue {
	struct CmdCompare {
		bool operator() (const BaseCommand* c1, const BaseCommand* c2) {
			return c1->get_duetime() > c2->get_duetime();
		}
	};

	typedef std::priority_queue<BaseCommand*, std::vector<BaseCommand*>, CmdCompare> queue_t;


   public:
	Cmd_Queue(Game *g);
	~Cmd_Queue(void);

	void enqueue (BaseCommand*);
	int run_queue (int interval, int* game_time_var);

   private:
	Game *m_game;
	queue_t m_cmds;
};


class Cmd_Call:public BaseCommand {
    public:
	typedef void (call_fn_t) (Game* g, int arg1, int arg2);
	
	Cmd_Call (int t, call_fn_t* c, int a1, int a2) : BaseCommand (t)
	{
		callee=c;
		arg1=a1;
		arg2=a2;
	}	
	
	void execute (Game* g)
	{
		callee (g, arg1, arg2);
	}
	
    private:
	call_fn_t*	callee;
	int		arg1, arg2;
};

#endif // __S__CMD_QUEUE_H
