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

/*
TODO:
- try to avoid malloc()ing and free()ing all the time
- use a better algorithm (heap?)
*/

//
// This struct defines the commands, which are possible
//
// [I must've accidently deleted a comment about different access rights
//  here; either way, filtering commands at the network level should
//  really be enough]
//
struct Cmd_Queue::Cmd {
   Cmd* next; // next command in queue
	int time; // scheduled time of execution
	char sender;
	int cmd;
	int arg1;
	int arg2;
	void *arg3; // pointer to malloc()ed memory
};


// 
// class Cmd_Queue
//
Cmd_Queue::Cmd_Queue(Game *g)
{
	m_game = g;
	m_cmds = 0;
	m_time = 0;
}

Cmd_Queue::~Cmd_Queue(void)
{
	while(m_cmds) {
		Cmd *c = m_cmds;
		m_cmds = c->next;
		
		free_cmd(c);
	}
}

/** Cmd_Queue::queue(int time, char sender, int cmd, int arg1=0, int arg2=0, void *arg3=0)
 *
 * Insert a new command into the queue; it will be executed at the given time
 */
void Cmd_Queue::queue(int time, char sender, int cmd, int arg1, int arg2, void *arg3)
{
	Cmd *c; // our command
	Cmd **pp; // where we put it
	
	// Initialize the command 
	c = (Cmd *)malloc(sizeof(Cmd));
	c->time = time;
	c->sender = sender;
	c->cmd = cmd;
	c->arg1 = arg1;
	c->arg2 = arg2;	
	c->arg3 = arg3;
	
	// Insert it into the queue
	pp = &m_cmds;
	while(*pp && (*pp)->time <= time)
		pp = &(*pp)->next;
	
	c->next = *pp;
	*pp = c;
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

	while(m_cmds && final - m_cmds->time >= 0) {
		Cmd *c = m_cmds;
		m_cmds = c->next;
		
		m_time = c->time;
		exec_cmd(c);
		free_cmd(c);
		cnt++;
	}
	
	m_time = final;
	
	return cnt;
}

/** Cmd_Queue::exec_cmd(Cmd *c) [private]
 *
 * Execute the given command now
 */
void Cmd_Queue::exec_cmd(Cmd *c)
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
	
	default:
		assert(0);
		break;
	}
}

/** Cmd_Queue::free_cmd(Cmd *c) [private]
 *
 * Unuse a command structure
 */
void Cmd_Queue::free_cmd(Cmd *c)
{
	if (c->arg3)
		free(c->arg3);
	free(c);
}

