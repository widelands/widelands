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

#if 0 

#include "widelands.h"
#include "map.h"
#include "building.h"
#include "creature.h"

Building::Building()
{
	this->worker = 0;
	this->working = false;
}

Building::~Building()
{
}

void Building::get_pic(int timekey)
{
}

void Building::work(int timekey)
{
	if (worker)
	{
		if (desc.professionType == PROFESSION_SEARCH)
		{
			Field* field = worker->find_bob_to_consume();
			worker->add_task(Creature::TASK_GO, field);
			worker->add_task(Creature::TASK_PRODUCE, location->get_brn());
			worker->add_task(Creature::TASK_GO, location);
			worker->add_task(Creature::TASK_WORK);
			worker->live(timekey);
		}
		else
		{
			worker->add_task(Creature::TASK_PRODUCE, location->get_brn());
			worker->add_task(Creature::TASK_GO, location);
			worker->add_task(Creature::TASK_WORK);
			worker->live(timekey);
		}
		worker = NULL;
		working = true;	// fuer die animation; mag sein, dass man den
						// schmied beim klopfen sieht, mag sein, dass
						// aus der huette des holzfaellers kein rauch
						// kommt oder das licht aus ist oder was weiss ich

	}
	// ausserdem hier: waren anfordern!
}

void Building::worker_idle(Creature* w)
{
	worker = w;
	working = false;
}

void Building::destroy()
{
	if (worker)
	{
		worker->clear_tasks();
		worker->add_task(Creature::TASK_GOHOME);
	}
}

bool Building::add_ware(uint ware)
{
	// add ware to this building's stock
	// return false if this building does not accept that ware
	return false;
}

#endif // 0
