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
/*
The entire transport subsystem comes into this file.

What does _not_ belong in here: road renderer, client-side road building.

What _does_ belong in here:
Flags, Roads, Carriers, the logic behind ware pulls and pushes.
*/

#include "widelands.h"
#include "game.h"
#include "player.h"
#include "transport.h"
#include "building.h"


/*
==============================================================================

Flag_Descr IMPLEMENTATION

==============================================================================
*/

Flag_Descr::Flag_Descr()
	: Map_Object_Descr()
{
	add_attribute(Map_Object::ROBUST);
	add_attribute(Map_Object::SMALL);
	add_attribute(Map_Object::FLAG);
}

Map_Object *Flag_Descr::create_object()
{
	return new Flag(this);
}

Flag_Descr g_flag_descr;

/*
==============================================================================

Flag IMPLEMENTATION

==============================================================================
*/

/*
===============
Flag::Flag

Create the flag. Initially, it doesn't have any attachments.
===============
*/
Flag::Flag(Flag_Descr *descr)
	: Map_Object(descr)
{
	m_building = 0;
	for(int i = 0; i < 6; i++)
		m_roads[i] = 0;
}


/*
===============
Flag::~Flag

Shouldn't be necessary to do anything, since Object_Manager always calls
cleanup() first.
===============
*/
Flag::~Flag()
{
	if (m_building)
		log("Flag: ouch! building left\n");
	
	for(int i = 0; i < 6; i++)
		if (m_roads[i])
			log("Flag: puch! road left\n");
}

/*
===============
Flag::create [static]

Create a flag at the given location
===============
*/
Flag *Flag::create(Game *g, int owner, Coords &coords)
{
	return (Flag *)g->get_objects()->create_object(g, &g_flag_descr, owner, coords.x, coords.y);
}

/*
===============
Flag::attach_building

Call this only from the Building init!
===============
*/
void Flag::attach_building(Game *g, Building *building)
{
	assert(!m_building);
	
	m_building = building;
	
	Field *f = g->get_map()->get_field(building->get_position());
	f->set_road(Road_SouthEast, Road_Normal);
}


/*
===============
Flag::detach_building

Call this only from the Building destructor!
===============
*/
void Flag::detach_building(Game *g)
{
	assert(m_building);

	Field *f = g->get_map()->get_field(m_building->get_position());
	f->set_road(Road_SouthEast, Road_None);
	
	m_building = 0;
}


/*
===============
Flag::attach_road	

Call this only from the Road constructor!
===============
*/
void Flag::attach_road(int dir, Road *road)
{
	assert(!m_roads[dir-1]);
	
	m_roads[dir-1] = road;
}


/*
===============
Flag::detach_road

Call this only from the Road destructor!
===============
*/
void Flag::detach_road(int dir)
{
	m_roads[dir-1] = 0;
}


/*
===============
Flag::init
===============
*/
void Flag::init(Game *g)
{
	Map_Object::init(g);
}

/*
===============
Flag::cleanup

Detach building and free roads.
===============
*/
void Flag::cleanup(Game *g)
{
	if (m_building) {
		m_building->die(g);
		// awkward... we won't be around anymore when the building cleans up
		detach_building(g);
	}
	
	for(int i = 0; i < 6; i++) {
		if (m_roads[i]) {
			m_roads[i]->cleanup(g);
			delete m_roads[i];
		}
	}
	
	Map_Object::cleanup(g);
}

/*
===============
Flag::task_start_best

A flag never does anything itself
===============
*/
void Flag::task_start_best(Game *g, uint prev, bool success, uint nexthint)
{
	assert(get_owned_by());

	Player *player = g->get_player(get_owned_by());
	start_task_idle(g, player->get_tribe()->get_flag_anim(), -1);
}


/*
==============================================================================

Road IMPLEMENTATION

==============================================================================
*/

/*
===============
Road::Road

Initialize the road
===============
*/
Road::Road(int type, Flag *start, Flag *end, const Path &path)
{
	m_type = type;
	m_start = start;
	m_end = end;
	m_path = path;
	
	assert(m_path.get_start() == m_start->get_position());
	assert(m_path.get_end() == m_end->get_position());
	
	int dir;
	
	dir = m_path.get_step(0);
	m_start->attach_road(dir, this);
	
	dir = get_reverse_dir(m_path.get_step(m_path.get_nsteps()-1));
	m_end->attach_road(dir, this);
}


/*
===============
Road::~Road

Cleanup the road
===============
*/
Road::~Road()
{
	int dir;
	
	dir = m_path.get_step(0);
	m_start->detach_road(dir);
	
	dir = get_reverse_dir(m_path.get_step(m_path.get_nsteps()-1));
	m_end->detach_road(dir);
}

	
/*
===============
Road::init

Initialize the road.
===============
*/
void Road::init(Game *g)
{
	// TODO: mark Fields
	
	// TODO: request Carrier
}


/*
===============
Road::cleanup

Cleanup the road
===============
*/
void Road::cleanup(Game *g)
{
	// TODO: unmark Fields
}

