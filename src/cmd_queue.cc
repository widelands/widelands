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
#include "player.h"
#include "worker.h"


// 
// class Cmd_Queue
//
Cmd_Queue::Cmd_Queue(Game *g)
{
	m_game = g;
}

Cmd_Queue::~Cmd_Queue(void)
{
	while(!m_cmds.empty()) {
		const Cmd &c = m_cmds.top();
		clear_cmd((Cmd *)&c);
		m_cmds.pop();
	}
}

/*
===============
Cmd_Queue::queue

Insert a new command into the queue; it will be executed at the given time
===============
*/
void Cmd_Queue::queue(int time, char sender, int cmd, int arg1, int arg2, int arg3)
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
 * the game_time_var represents the current game time, which we update and with 
 * which we must mess around (to run all queued cmd.s) and which we update (add
 * the interval)
 */
int Cmd_Queue::run_queue(int interval, int* game_time_var)
{
	int final = *game_time_var + interval;
	int cnt = 0;


	while(!m_cmds.empty()) {
		const Cmd &c = m_cmds.top();
		if (final - c.time < 0)
			break;
		
		//cerr << "run(" << c.time << ", " << c.arg1 << ")" << endl;
			
		*game_time_var = c.time;
		exec_cmd(&c);
		clear_cmd((Cmd *)&c);
		
		m_cmds.pop();
	}
	
	*game_time_var = final;
   
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
			obj->act(m_game, c->arg2);
		// the object must queue the next CMD_ACT itself if necessary
		break;
	}
	
	case CMD_DESTROY:
	{
		Map_Object* obj = m_game->get_objects()->get_object(c->arg1);
		if (obj)
			obj->destroy(m_game);
		break;
	}
	
	case CMD_INCORPORATE:
	{
		Worker *worker = (Worker*)m_game->get_objects()->get_object(c->arg1);
		if (worker)
			worker->incorporate(m_game);
		break;
	}
	
	case CMD_BUILD_FLAG:
	{
		Player *plr = m_game->get_player(c->sender);
		plr->build_flag(Coords(c->arg1, c->arg2));
		break;
	}
	
	case CMD_RIP_FLAG:
	{
		Player *plr = m_game->get_player(c->sender);
		plr->rip_flag(Coords(c->arg1, c->arg2));
		break;
	}
	
	case CMD_BUILD_ROAD:
	{
		Player *plr = m_game->get_player(c->sender);
		plr->build_road((Path*)c->arg1);
		break;
	}
	
	case CMD_REMOVE_ROAD:
	{
		assert(c->arg1);
		Player *plr = m_game->get_player(c->sender);
		Map_Object* obj = m_game->get_objects()->get_object(c->arg1);
		if (obj && obj->get_type() == Map_Object::ROAD)
			plr->remove_road((Road*)obj);
		break;
	}
	
	case CMD_BUILD:
	{
		Player* plr = m_game->get_player(c->sender);
		plr->build(Coords(c->arg1, c->arg2), c->arg3);
		break;
	}
	
	default:
		assert(0);
		break;
	}
}

/*
===============
Cmd_Queue::clear_cmd

Free dynamically allocated arguments, if any
===============
*/
void Cmd_Queue::clear_cmd(Cmd *c)
{
	switch(c->cmd) {
	case CMD_BUILD_ROAD:
		if (c->arg1)
			delete (Path*)c->arg1;
		break;
	}
}
