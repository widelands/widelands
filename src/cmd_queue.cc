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
}

Cmd_Queue::~Cmd_Queue(void)
{
	while(!m_cmds.empty()) {
		const Cmd &c = m_cmds.top();
		clear_cmd(&c);
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
		Cmd c = m_cmds.top();
		if (final - c.time < 0)
			break;

		m_cmds.pop();

		*game_time_var = c.time;
		exec_cmd(&c);
		clear_cmd(&c);
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

	case CMD_CALL:
	{
		call_fn_t* fn = (call_fn_t*)c->arg1;

		(*fn)(m_game, c->arg2, c->arg3);
		break;
	}

   case CMD_CHECK_TRIGGER:
   {
      int trigger_id=c->arg1+1;
      log("Trigger: looking if check is needed for trigger %i\n", trigger_id);
      if(trigger_id>= m_game->get_map()->get_number_of_triggers()) {
         // either we swapped around the end of all triggers
         // if so, restart. if there are no triggers at all,
         // requeue in about a 10 seconds to check if this state has changed
         // (a new trigger could be registered) (this should only happen at the beginning
         // of the game and should not harm at all, and 30seconds means nearly no CPU time
         // for non trigger games)
         if(m_game->get_map()->get_number_of_triggers()) {
            trigger_id=0;
         } else {
            queue(m_game->get_gametime() + 30000, SENDER_CMDQUEUE, CMD_CHECK_TRIGGER, -1, 0, 0);
            break;
         }
      }
      Trigger* trig=m_game->get_map()->get_trigger(trigger_id); 
      assert(trig);
      log("Trigger %s is going to get checked!\n", trig->get_name()); 
      bool trig_state=trig->is_set();
      trig->check_set_conditions(m_game);
      if(trig->is_set()!=trig_state) { 
         log("Trigger has changed state: %s gone to %i\n", trig->get_name(), trig->is_set());
         int i=0;
         for(i=0; i<m_game->get_map()->get_number_of_events(); i++) {
            Event* ev=m_game->get_map()->get_event(i);
            log("Checking event: %s\n", ev->get_name());
            bool is_one_timer=false;
            if(ev->check_triggers()) {
               // This event is ready to run
               is_one_timer=ev->is_one_time_event();
               log("Run event!\n");
               ev->run(m_game);
               log("Run done!\n");
               if(is_one_timer) // The event was a one timer and got therefore deleted. we have to make sure that i stays valid
                  --i;
            }
         }
      }
      // recheck next trigger in the time that all triggers get checked at least once ever 10 seconds
      int time= m_game->get_map()->get_number_of_triggers() ? 1000/m_game->get_map()->get_number_of_triggers() : 30000;
      queue(m_game->get_gametime() + time, SENDER_CMDQUEUE, CMD_CHECK_TRIGGER, trigger_id, 0, 0);
      break;
   }

	case CMD_BUILD_FLAG:
	{
		Player *plr = m_game->get_player(c->sender);
		plr->build_flag(Coords(c->arg1, c->arg2));
		break;
	}

	case CMD_BUILD_ROAD:
	{
		Player *plr = m_game->get_player(c->sender);
		plr->build_road((Path*)c->arg1);
		break;
	}

	case CMD_BUILD:
	{
		Player* plr = m_game->get_player(c->sender);
		plr->build(Coords(c->arg1, c->arg2), c->arg3);
		break;
	}

	case CMD_BULLDOZE:
	{
		assert(c->arg1);
		Player* plr = m_game->get_player(c->sender);
		Map_Object* obj = m_game->get_objects()->get_object(c->arg1);
		if (obj && obj->get_type() >= Map_Object::BUILDING)
			plr->bulldoze((PlayerImmovable*)obj);
		break;
	}

	case CMD_FLAGACTION:
	{
		assert(c->arg1);
		Player* plr = m_game->get_player(c->sender);
		Map_Object* obj = m_game->get_objects()->get_object(c->arg1);
		if (obj && obj->get_type() == Map_Object::FLAG) {
			if (((PlayerImmovable*)obj)->get_owner() == plr)
				plr->flagaction((Flag*)obj, c->arg2);
		}
		break;
	}

	case CMD_START_STOP_BUILDING:
	{
		assert(c->arg1);
		Player* plr = m_game->get_player(c->sender);
		Map_Object* obj = m_game->get_objects()->get_object(c->arg1);
		if (obj && obj->get_type() >= Map_Object::BUILDING)
		plr->start_stop_building((PlayerImmovable*)obj);
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
void Cmd_Queue::clear_cmd(const Cmd* c)
{
	switch(c->cmd) {
	case CMD_BUILD_ROAD:
		delete (Path*)c->arg1;
		break;
	}
}
