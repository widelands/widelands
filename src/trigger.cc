/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

#include "game.h"
#include "map.h"
#include "trigger.h"

Cmd_CheckTrigger::Cmd_CheckTrigger (int t, int tid):BaseCommand (t)
{
	trigger_id=tid;
}

void Cmd_CheckTrigger::execute (Game* g)
{
	trigger_id++;
	
	log("Trigger: looking if check is needed for trigger %i\n", trigger_id);
	
	if(trigger_id>= g->get_map()->get_number_of_triggers()) {
    		// either we wrapped around the end of all triggers
		// if so, restart. if there are no triggers at all,
		// requeue in about a 10 seconds to check if this state has changed
		// (a new trigger could be registered) (this should only happen at the beginning
		// of the game and should not harm at all, and 30seconds means nearly no CPU time
		// for non trigger games)
		
    		if (g->get_map()->get_number_of_triggers())
			trigger_id=0;
		else
			g->enqueue_command (
			    new Cmd_CheckTrigger(g->get_gametime() + 30000, -1));
		
		return;
	}
	
	Trigger* trig=g->get_map()->get_trigger(trigger_id);
	assert(trig);
	log("Trigger %s is going to get checked!\n", trig->get_name());
	
	bool trig_state=trig->is_set();
	trig->check_set_conditions(g);
	if(trig->is_set()!=trig_state) {
		log("Trigger has changed state: %s gone to %i\n", trig->get_name(), trig->is_set());

		for (int i=0; i<g->get_map()->get_number_of_events(); i++) {
			Event* ev=g->get_map()->get_event(i);
			log("Checking event: %s\n", ev->get_name());
			bool is_one_timer=false;

			if (ev->check_triggers()) {
				// This event is ready to run
				is_one_timer=ev->is_one_time_event();
				log("Run event!\n");
				ev->run(g);
				log("Run done!\n");
				
				// The event was a one timer and got therefore deleted. we have to make sure that i stays valid
				if(is_one_timer)
					--i;
			}
		}
	}
	
	// recheck next trigger in the time that all triggers get checked at least once ever 10 seconds
	int delay=g->get_map()->get_number_of_triggers() ? 1000/g->get_map()->get_number_of_triggers() : 30000;
	g->enqueue_command (new Cmd_CheckTrigger(g->get_gametime() + delay, trigger_id));
}

