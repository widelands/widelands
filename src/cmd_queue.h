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
		void set_duetime(int t) { duetime=t; }
};

class Cmd_Queue {
	struct cmditem {
		BaseCommand*	cmd;
		unsigned long	serial;
		
		bool operator< (const cmditem& c) const
		{
			if (cmd->get_duetime()==c.cmd->get_duetime())
				return serial > c.serial;
			else
				return cmd->get_duetime() > c.cmd->get_duetime();
		}
	};

   public:
	Cmd_Queue(Game *g);
	~Cmd_Queue(void);

	void enqueue (BaseCommand*);
	int run_queue (int interval, int* game_time_var);

   private:
	Game*				m_game;
	std::priority_queue<cmditem>	m_cmds;
	unsigned long			nextserial;
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
