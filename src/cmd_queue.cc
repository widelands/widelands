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

#include "cmd_queue.h"
#include "game.h"
#include "instances.h"
#include "player.h"
#include "worker.h"
#include "trigger.h"
#include "error.h"

//
// class Cmd_Queue
//
Cmd_Queue::Cmd_Queue(Game *g)
{
	m_game = g;
	nextserial = 0;
}

Cmd_Queue::~Cmd_Queue(void)
{
	while(!m_cmds.empty()) {
		delete m_cmds.top().cmd;
		m_cmds.pop();
	}
}

/*
===============
Cmd_Queue::queue

Insert a new command into the queue; it will be executed at the given time
===============
*/
void Cmd_Queue::enqueue (BaseCommand* cmd)
{
	cmditem ci;
	
	ci.cmd=cmd;
	ci.serial=nextserial++;
	
	m_cmds.push(ci);
}

/** Cmd_Queue::run_queue(int interval)
 *
 * Run all commands scheduled for the next interval milliseconds, and update the
 * internal time as well.
 * the game_time_var represents the current game time, which we update and with
 * which we must mess around (to run all queued cmd.s) and which we update (add
 * the interval)
 */
int Cmd_Queue::run_queue(int interval, int* game_time_var)
{
	int final = *game_time_var + interval;
	int cnt = 0;


	while(!m_cmds.empty()) {
		BaseCommand* c = m_cmds.top().cmd;
		if (final - c->get_duetime() <= 0)
			break;

		m_cmds.pop();

		*game_time_var = c->get_duetime();
		
		c->execute (m_game);
		
		delete c;
	}

	*game_time_var = final;

	return cnt;
}


BaseCommand::BaseCommand (int t)
{
	duetime=t;
}

BaseCommand::~BaseCommand ()
{
}

