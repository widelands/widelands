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

#include <vector>
#include "widelands.h"
#include "game.h"
#include "map.h"
#include "cmd_queue.h"

/*
TODO:
- try to avoid malloc()ing and free()ing all the time
- use a better algorithm (heap?)
*/

// 
// class Cmd_Queue
//
Cmd_Queue::Cmd_Queue(Game *g)
{
	m_game = g;
	m_time = 0;
}

Cmd_Queue::~Cmd_Queue(void)
{
	while(!m_cmds.empty()) {
		const Cmd &c = m_cmds.top();
		if (c.arg3)
			free(c.arg3);
		m_cmds.pop();
	}
}

/** Cmd_Queue::queue(int time, char sender, int cmd, int arg1=0, int arg2=0, void *arg3=0)
 *
 * Insert a new command into the queue; it will be executed at the given time
 */
void Cmd_Queue::queue(int time, char sender, int cmd, int arg1, int arg2, void *arg3)
{
	Cmd c; // our command
	
	c.time = time;
	c.sender = sender;
	c.cmd = cmd;
	c.arg1 = arg1;
	c.arg2 = arg2;	
	c.arg3 = arg3;
	
	m_cmds.push(c);
	
	//cerr << "queue(" << time << ", " << arg1 << ")" << endl;
}

/** Cmd_Queue::run_queue(int interval)
 *
 * Run all commands scheduled for the next interval milliseconds, and update the
 * internal time as well.
 */
int Cmd_Queue::run_queue(int interval)
{
	int final = m_time + interval;
	int cnt = 0;

	while(!m_cmds.empty()) {
		const Cmd &c = m_cmds.top();
		if (final - c.time < 0)
			break;
		
		//cerr << "run(" << c.time << ", " << c.arg1 << ")" << endl;
			
		m_time = c.time;
		exec_cmd(&c);
		
		if (c.arg3)
			free(c.arg3);
		m_cmds.pop();
	}
	
	m_time = final;
	
	return cnt;
}

/** Cmd_Queue::exec_cmd(const Cmd *c) [private]
 *
 * Execute the given command now
 */
void Cmd_Queue::exec_cmd(const Cmd *c)
{
	switch(c->cmd) {
	case CMD_ACT:
	{
		// an object has requested to act again after a time
		// arg1: serial number of the Map_Object
		assert(c->arg1);
		Map_Object* obj = m_game->get_objects()->get_object(c->arg1);
		if (obj)
			obj->act(m_game);
		// the object must queue the next CMD_ACT itself if necessary
		break;
	}
	
	case CMD_REMOVE:
	{
		// remove a Map_Object safely
		assert(c->arg1);
		Map_Object *obj = m_game->get_objects()->get_object(c->arg1);
		if (obj)
			m_game->get_objects()->free_object(m_game, obj);
		break;
	}
	
	default:
		assert(0);
		break;
	}
}
