/*
 * Copyright (C) 2002 by Florian Bluemel
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

#include "creature.h"
#include "growablearray.h"
#include "map.h"
#include "building.h"
#include "bob.h"

Creature::Task::Task(TaskAction a, void* p)
{
	this->action = a;
	this->param = p;
}

Creature::Creature(CreatureDesc* d)
{
	this->desc = d;
	this->tasks = new Growable_Array(8, 8);
	this->field = NULL;
	this->carrying = -1;
}

Creature::~Creature()
{
	this->tasks->flush();
	delete this->tasks;
}

void Creature::add_task(TaskAction action, void* param)
{
	tasks->add(new Task(action, param));
}

void Creature::next_task()
{
	delete (Task*)tasks->remove(0);
	taskTime = 0;
}

void Creature::clear_tasks()
{
	tasks->flush();
}

void Creature::live(uint timekey)
{
	Task* currentTask = (Task*)tasks->element_at(0);
	if (!currentTask)
		return;
	bool taskFinished = false;
	switch (currentTask->action)
	{
		case TASK_GO:
			// go to field
			// while walking, creature belongs to field currently on
			break;
		case TASK_GOHOME:
			// find nearest store
			// go there
			break;
		case TASK_WORK:
			// try to work in building on current field
			this->work();
			taskFinished = true;
			break;
		case TASK_WORKONROAD:
			// bind this creature to the road
			// walk to the road
			// start transporting
			break;
		case TASK_DROP:
			// if a flag or building accepting the ware is here, drop
			// if not, something went wrong.
			/*
			if (field->building)
				if (field->building->add_ware(tribe->get_ware(this->carrying)))
					this->carrying = -1;
			*/
			break;
		case TASK_PRODUCE:
		{
			// try to produce ware on current field
			// if that worked, go and drop ware on param field
			int timeLeft = this->produce(timekey);
			if (timeLeft == 0)
			{
				tasks->insert_at(1, new Task(TASK_GO, currentTask->param));
				tasks->insert_at(2, new Task(TASK_DROP));
			}
			if (timeLeft <= 0)
				taskFinished = true;
			break;
		}
		case TASK_TRANSPORT:
			// transport wares on a road
			// if creature carries something, deliver it to one end of the
			// road; if not, check if wares on the ends of the road are to
			// be transported over this road
			// else move to middle of road
		// case ...
			break;
	}
	if (taskFinished)
		next_task();
}

Field* Creature::find_bob_to_consume()
{
	// try to find a bob that provides everything needed to produce
	// this search is based on a) resource/wares needed b) search range
	// return the field that bob is on
	return NULL;
}

void Creature::work()
{
	if (field->building)
	{
		field->remove_creature(this);
		field->building->worker_idle(this);
		field = NULL;
	}
}

int Creature::produce(uint timekey)
{
	if (taskTime == 0)
	{
		taskTime = timekey;
		switch (desc->consumeType)
		{
/*			case CONSUME_BOB:
				if (field->bob && field->bob->resource() == desc->consume)
					field->bob->consume();
				else
					return -1;
				break;
			case CONSUME_RESOURCE:
				if (field->resource() == desc->consume)
					field->consume();
				else
					return -1;
				break;*/
			default:
				return -1;
		}
		carrying = desc->produce;
	}
	int ret = taskTime + desc->prodTime - timekey;
	return ret > 0 ? ret : 0;
}