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

Flag IMPLEMENTATION

==============================================================================
*/

// dummy class because Map_Object needs a description
class Flag_Descr : public Map_Object_Descr {
public:
	Flag_Descr();
};

Flag_Descr::Flag_Descr()
{
}

Flag_Descr g_flag_descr;

/*
===============
Flag::Flag

Create the flag. Initially, it doesn't have any attachments.
===============
*/
Flag::Flag()
	: BaseImmovable(&g_flag_descr)
{
	m_anim = 0;
	m_building = 0;
	for(int i = 0; i < 6; i++)
		m_roads[i] = 0;
}


/*
===============
Flag::~Flag

Shouldn't be necessary to do anything, since die() always calls
cleanup() first.
===============
*/
Flag::~Flag()
{
	if (m_building)
		log("Flag: ouch! building left\n");
	
	for(int i = 0; i < 6; i++)
		if (m_roads[i])
			log("Flag: ouch! road left\n");
}

/*
===============
Flag::create [static]

Create a flag at the given location
===============
*/
Flag *Flag::create(Game *g, Player *owner, Coords coords)
{
	Road *road = 0;
	BaseImmovable *imm = g->get_map()->get_immovable(coords);

	if (imm && imm->get_type() == Map_Object::ROAD)
		road = (Road*)imm;
	
	Flag *flag = new Flag;
	flag->m_owner = owner;
	flag->m_position = coords;
	if (road)
		road->presplit(g, coords);
	flag->init(g);
	if (road)
		road->postsplit(g, flag);
	return flag;
}

/*
===============
Flag::get_type
Flag::get_size
Flag::get_passable
===============
*/
int Flag::get_type()
{
	return FLAG;
}
	
int Flag::get_size()
{
	return SMALL;
}

bool Flag::get_passable()
{
	return true;
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
	
	Map *map = g->get_map();
	Coords tln;
	
	map->get_tln(m_position, &tln);
	map->get_field(tln)->set_road(Road_SouthEast, Road_Normal);
}


/*
===============
Flag::detach_building

Call this only from the Building cleanup!
===============
*/
void Flag::detach_building(Game *g)
{
	assert(m_building);

	Map *map = g->get_map();
	Coords tln;
	
	map->get_tln(m_position, &tln);
	map->get_field(tln)->set_road(Road_SouthEast, Road_None);
	
	m_building = 0;
}


/*
===============
Flag::attach_road	

Call this only from the Road init!
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

Call this only from the Road init!
===============
*/
void Flag::detach_road(int dir)
{
	assert(m_roads[dir-1]);

	m_roads[dir-1] = 0;
}


/*
===============
Flag::init
===============
*/
void Flag::init(Game *g)
{
	BaseImmovable::init(g);

	set_position(g, m_position);
	
	m_anim = m_owner->get_tribe()->get_flag_anim();
	m_animstart = g->get_gametime();
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
		m_building->die(g); // immediate death
		m_building = 0;
	}
	
	for(int i = 0; i < 6; i++) {
		if (m_roads[i]) {
			m_roads[i]->die(g); // immediate death
			m_roads[i] = 0;
		}
	}
	
	unset_position(g, m_position);
	
	BaseImmovable::cleanup(g);
}

/*
===============
Flag::draw

Draw the flag.
===============
*/
void Flag::draw(Game* game, Bitmap* dst, FCoords coords, int posx, int posy)
{
	copy_animation_pic(dst, m_anim, game->get_gametime() - m_animstart, posx, posy, m_owner->get_playercolor_rgb());
	
	// TODO: draw wares
}

/*
==============================================================================

Road IMPLEMENTATION

==============================================================================
*/

// dummy class because Map_Object needs a description
class Road_Descr : public Map_Object_Descr {
public:
	Road_Descr();
};

Road_Descr::Road_Descr()
{
}

Road_Descr g_road_descr;

/*
===============
Road::Road
Road::~Road

Construction and destruction. Most of the actual work is done in init/cleanup.
===============
*/
Road::Road()
	: BaseImmovable(&g_road_descr)
{
	m_type = 0;
	m_start = m_end = 0;
}

Road::~Road()
{
}


/*
===============
Road::create [static]

Create a road between the given flags, using the given path.
===============
*/
Road *Road::create(Game *g, int type, Flag *start, Flag *end, const Path &path)
{
	assert(start->get_position() == path.get_start());
	assert(end->get_position() == path.get_end());

	Road *r = new Road;
	r->m_type = type;
	r->m_start = start;
	r->m_end = end;
	r->m_path = path;
	r->init(g);
	return r;
}

	
/*
===============
Road::get_type
Road::get_size
Road::get_passable
===============
*/
int Road::get_type()
{
	return ROAD;
}

int Road::get_size()
{
	return SMALL;
}

bool Road::get_passable()
{
	return true;
}

/*
===============
Road::mark_map

Add road markings to the map
===============
*/
void Road::mark_map(Game *g)
{
	Map *map = g->get_map();
	FCoords curf(m_path.get_start(), map->get_field(m_path.get_start()));
	
	for(int steps = 0; steps <= m_path.get_nsteps(); steps++) {
		if (steps > 0 && steps < m_path.get_nsteps())
			set_position(g, curf);
		
		// mark the road that leads up to this field
		if (steps > 0) {
			int dir = get_reverse_dir(m_path.get_step(steps-1));
			int rdir = 2*(dir - Map_Object::WALK_E);
			
			if (rdir >= 0 && rdir <= 4)
				curf.field->set_road(rdir, m_type);
		}
		
		// mark the road that leads away from this field
		if (steps < m_path.get_nsteps()) {
			int dir = m_path.get_step(steps);
			int rdir = 2*(dir - Map_Object::WALK_E);
			
			if (rdir >= 0 && rdir <= 4)
				curf.field->set_road(rdir, m_type);
			
			map->get_neighbour(curf, dir, &curf);
		}
	}
}

/*
===============
Road::unmark_map

Remove road markings from the map
===============
*/
void Road::unmark_map(Game *g)
{
	Map *map = g->get_map();
	FCoords curf(m_path.get_start(), map->get_field(m_path.get_start()));
	
	for(int steps = 0; steps <= m_path.get_nsteps(); steps++) {
		if (steps > 0 && steps < m_path.get_nsteps())
			unset_position(g, curf);
		
		// mark the road that leads up to this field
		if (steps > 0) {
			int dir = get_reverse_dir(m_path.get_step(steps-1));
			int rdir = 2*(dir - Map_Object::WALK_E);
			
			if (rdir >= 0 && rdir <= 4)
				curf.field->set_road(rdir, Road_None);
		}
		
		// mark the road that leads away from this field
		if (steps < m_path.get_nsteps()) {
			int dir = m_path.get_step(steps);
			int rdir = 2*(dir - Map_Object::WALK_E);
			
			if (rdir >= 0 && rdir <= 4)
				curf.field->set_road(rdir, Road_None);
		
			map->get_neighbour(curf, dir, &curf);
		}
	}
}

/*
===============
Road::init

Initialize the road.
===============
*/
void Road::init(Game *g)
{
	assert(m_path.get_nsteps() >= 2);

	BaseImmovable::init(g);

	// Link into the flags
	int dir;
	
	dir = m_path.get_step(0);
	m_start->attach_road(dir, this);
	
	dir = get_reverse_dir(m_path.get_step(m_path.get_nsteps()-1));
	m_end->attach_road(dir, this);
	
	// Mark Fields
	mark_map(g);
	
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
	// Unmark Fields
	unmark_map(g);

	// Unlink from flags
	int dir;
	
	dir = m_path.get_step(0);
	m_start->detach_road(dir);
	
	dir = get_reverse_dir(m_path.get_step(m_path.get_nsteps()-1));
	m_end->detach_road(dir);
	
	BaseImmovable::cleanup(g);
}

/*
===============
Road::presplit

A flag has been placed that splits this road. This function is called before
the new flag initializes. We remove markings to avoid interference with the
flag.
===============
*/
void Road::presplit(Game *g, Coords split)
{
	unmark_map(g);
}

/*
===============
Road::postsplit

The flag that splits this road has been initialized. Perform the actual
splitting.
After the split, this road will span [start...new flag]. A new road will
be created to span [new flag...end]
===============
*/
void Road::postsplit(Game *g, Flag *flag)
{
	Flag *oldend = m_end;
	int dir;
	
	// detach from end
	dir = get_reverse_dir(m_path.get_step(m_path.get_nsteps()-1));
	m_end->detach_road(dir);
	
	// build our new path and the new road's path
	CoordPath path(m_path);
	CoordPath secondpath(path);
	int index = path.get_index(flag->get_position());
	
	assert(index > 0 && index < path.get_nsteps()-1);
	
	path.truncate(index);
	secondpath.starttrim(index);
	
	// change road size and reattach
	m_path = path;
	m_end = flag;

	dir = get_reverse_dir(m_path.get_step(m_path.get_nsteps()-1));
	m_end->attach_road(dir, this);
	
	// recreate road markings
	mark_map(g);
	
	// create the new road
	Road *newroad = new Road;
	newroad->m_type = m_type;
	newroad->m_start = flag;
	newroad->m_end = oldend;
	newroad->m_path = secondpath;
	newroad->init(g);
	
	// TODO: reassign carrier(s)
}

/*
===============
Road::draw

The road is drawn by the terrain renderer via marked fields.
===============
*/
void Road::draw(Game* game, Bitmap* dst, FCoords coords, int posx, int posy)
{
}
