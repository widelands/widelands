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
Flags, Roads, the logic behind ware pulls and pushes.
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
	m_owner = 0;
	m_economy = 0;
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

	Flag *flag = new Flag;
	flag->m_owner = owner;
	flag->m_position = coords;
	
	if (imm && imm->get_type() == Map_Object::ROAD) 
	{
		// we split a road
		road = (Road*)imm;
		road->get_flag_start()->get_economy()->add_flag(flag);
	}
	else
	{
		// a new, standalone flag is created
		(new Economy(owner))->add_flag(flag);
	}
	
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
Flag::set_economy

Call this only from Economy code!
===============
*/
void Flag::set_economy(Economy *e)
{
	// Remove from current economy
	if (m_economy) {
		if (m_building)
			m_building->remove_from_economy(m_economy);
	}

	m_economy = e;
	
	// Add to new economy
	if (m_economy) {
		if (m_building)
			m_building->add_to_economy(m_economy);
	}
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
	
	m_building->add_to_economy(m_economy);
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

	m_building->remove_from_economy(m_economy);
	
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
Flag::get_neighbours

Return neighbouring flags.
===============
*/
void Flag::get_neighbours(Neighbour_list *neighbours)
{
	for(int i = 0; i < 6; i++) {
		Road *road = m_roads[i];
		if (!road)
			continue;

		Neighbour n;
		n.road = road;
		n.flag = road->get_flag_end();
		if (n.flag != this)
			n.cost = road->get_cost(true);
		else {
			n.flag = road->get_flag_start();
			n.cost = road->get_cost(false);
		}
		
		assert(n.flag != this);
		neighbours->push_back(n);
	}
	
	// I guess this would be the place to add other ports if a port building
	// is attached to this flag
	// Or maybe other hosts of a carrier pigeon service, or a wormhole connection
	// point or whatever ;)
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
		m_building->remove(g); // immediate death
		assert(!m_building);
	}
	
	for(int i = 0; i < 6; i++) {
		if (m_roads[i]) {
			m_roads[i]->remove(g); // immediate death
			assert(!m_roads[i]);
		}
	}

	m_economy->remove_flag(this);
		
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
	m_carrier_request = 0;
}

Road::~Road()
{
	if (m_carrier_request)
		delete m_carrier_request;
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
	r->m_economy = start->get_economy();
	r->set_path(g, path);
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
Road::get_cost

Return the cost of getting from start to end.
If reverse is true, return the cost of getting from end to start.
The two will be different on slopes.
===============
*/
int Road::get_cost(bool reverse)
{
	if (!reverse)
		return m_cost_forward;
	else
		return m_cost_backward;
}

/*
===============
Road::set_path

Set the new path, calculate costs.
===============
*/
void Road::set_path(Game *g, const Path &path)
{
	assert(path.get_start() == m_start->get_position());
	assert(path.get_end() == m_end->get_position());
	
	m_path = path;
	g->get_map()->calc_cost(path, &m_cost_forward, &m_cost_backward);
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
	
	Economy::check_merge(m_start, m_end);	
	
	// Mark Fields
	mark_map(g);
	
	// Request Carrier
	if (!m_carrier.get(g) && !m_carrier_request) {
		m_carrier_request = new Request(this, g->get_safe_ware_id("carrier"));
		m_economy->add_request(m_carrier_request);
	}
}


/*
===============
Road::cleanup

Cleanup the road
===============
*/
void Road::cleanup(Game *g)
{
	// Release carrier
	if (m_carrier_request) {
		m_economy->remove_request(m_carrier_request);
		delete m_carrier_request;
		m_carrier_request = 0;
	}
	// TODO: release carrier if we have one
	//if (m_carrier.get(g))

	// Unmark Fields
	unmark_map(g);

	// Unlink from flags
	int dir;
	
	dir = m_path.get_step(0);
	m_start->detach_road(dir);
	
	dir = get_reverse_dir(m_path.get_step(m_path.get_nsteps()-1));
	m_end->detach_road(dir);
	
	Economy::check_split(m_start, m_end);
	
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
	m_end = flag;
	set_path(g, path);

	dir = get_reverse_dir(m_path.get_step(m_path.get_nsteps()-1));
	m_end->attach_road(dir, this);
	
	// recreate road markings
	mark_map(g);
	
	// create the new road
	Road *newroad = new Road;
	newroad->m_type = m_type;
	newroad->m_start = flag;
	newroad->m_end = oldend;
	newroad->m_economy = m_economy;
	newroad->set_path(g, secondpath);
	
	// Reassign carrier(s)
	Carrier *carrier = (Carrier *)m_carrier.get(g);
	if (carrier) {
		// TODO: Figure out whether the carrier remains on this road or moves
		// This largely depends on where the carrier is right now
	}

	// Initialize the new road
	newroad->init(g);
}

/*
===============
Road::set_economy

Called whenever the road changes economy as a result of split/merge.
We need to reassign all wares that currently belong to this road:
- the carrier(s) [implicitly reassigns the wares that are currently 
  being carried]
- all workers that are walking along the road right now

Note: we do not reassign the carrier request. This is handled by the economy.
===============
*/
void Road::set_economy(Economy *e)
{
	if (m_economy == e)
		return;
	
	// TODO: reassign carrier
	// if (m_carrier.get(g))
	
	m_economy = e;
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


/*
==============================================================================

Request IMPLEMENTATION

==============================================================================
*/

/*
===============
Request::Request
Request::~Request
===============
*/
Request::Request(BaseImmovable *target, int ware)
{
	assert(target->get_type() == Map_Object::ROAD || target->get_type() == Map_Object::BUILDING);
	
	m_target = target;
	m_ware = ware;
}

Request::~Request()
{
	// Request must have been removed from the economy before it's deleted
}

/*
==============================================================================

Economy IMPLEMENTATION

==============================================================================
*/

/*
===============
Economy::Economy
Economy::~Economy
===============
*/
Economy::Economy(Player *player)
{
	m_owner = player;
	mpf_cycle = 0;
}

Economy::~Economy()
{
	if (m_flags.size())
		log("Warning: Economy still has flags left on destruction\n");
	if (m_warehouses.size())
		log("Warning: Economy still has warehouses left on destruction\n");
}

/*
===============
Economy::check_merge [static]

Two flags have been connected; check whether their economies should be
merged.
Since we could merge into both directions, we preserver the economy that is
currently bigger (should be more efficient).
===============
*/
void Economy::check_merge(Flag *f1, Flag *f2)
{
	Economy *e1 = f1->get_economy();
	Economy *e2 = f2->get_economy();
	
	if (e1 == e2)
		return;

	if (e2->get_nrflags() > e1->get_nrflags())
		std::swap(e1, e2);
	
	e1->do_merge(e2);
}

/*
===============
Economy::check_split [static]

If the two flags can no longer reach each other (pathfinding!), the economy
gets split.
===============
*/
void Economy::check_split(Flag *f1, Flag *f2)
{
	assert(f1 != f2);
	assert(f1->get_economy() == f2->get_economy());
	
	Economy *e = f1->get_economy();
	
	if (e->find_route(f1, f2, 0))
		return;

	// Should we split off f1 or f2? Ideally, we'd split off so that the
	// new economy will be relatively small.
	// Unfortunately, there's no easy way to tell in advance which of the two
	// resulting economies will be smaller
	// Using f2 is just a guess, but if anything f2 is probably best:
	// it will be the end point of a road. Since roads are typically built 
	// from the center of a country outwards, and since splits
	// are more likely to happen outwards, the economy at the end point is
	// probably smaller in average. It's all just guesswork though ;)
	e->do_split(f2);
}

/*
class FlagQueue

Provides the flexible priority queue to maintain the open list.
This is more flexible than a standard priority_queue (fast boost() to
adjust cost)
*/
class FlagQueue {
	std::vector<Flag*> m_data;

public:
	FlagQueue() { }
	~FlagQueue() { }

	void flush() { m_data.clear(); }

	// Return the best node and readjust the tree
	// Basic idea behind the algorithm:
	//  1. the top slot of the tree is empty
	//  2. if this slot has both children:
	//       fill this slot with one of its children or with slot[_size], whichever
	//       is best;
	//       if we filled with slot[_size], stop
	//       otherwise, repeat the algorithm with the child slot
	//     if it doesn't have any children (l >= _size)
	//       put slot[_size] in its place and stop
	//     if only the left child is there
	//       arrange left child and slot[_size] correctly and stop
	Flag* pop()
	{
		if (m_data.empty())
			return 0;

		Flag* head = m_data[0];

		unsigned nsize = m_data.size()-1;
		unsigned fix = 0;
		while(fix < nsize) {
			unsigned l = fix*2 + 1;
			unsigned r = fix*2 + 2;
			if (l >= nsize) {
				m_data[fix] = m_data[nsize];
				m_data[fix]->mpf_heapindex = fix;
				break;
			}
			if (r >= nsize) {
				if (m_data[nsize]->cost() <= m_data[l]->cost()) {
					m_data[fix] = m_data[nsize];
					m_data[fix]->mpf_heapindex = fix;
				} else {
					m_data[fix] = m_data[l];
					m_data[fix]->mpf_heapindex = fix;
					m_data[l] = m_data[nsize];
					m_data[l]->mpf_heapindex = l;
				}
				break;
			}

			if (m_data[nsize]->cost() <= m_data[l]->cost() && m_data[nsize]->cost() <= m_data[r]->cost()) {
				m_data[fix] = m_data[nsize];
				m_data[fix]->mpf_heapindex = fix;
				break;
			}
			if (m_data[l]->cost() <= m_data[r]->cost()) {
				m_data[fix] = m_data[l];
				m_data[fix]->mpf_heapindex = fix;
				fix = l;
			} else {
				m_data[fix] = m_data[r];
				m_data[fix]->mpf_heapindex = fix;
				fix = r;
			}
		}
		
		m_data.pop_back();

		debug(0, "pop");
		
		head->mpf_heapindex = -1;
		return head;
	}

	// Add a new node and readjust the tree
	// Basic idea:
	//  1. Put the new node in the last slot
	//  2. If parent slot is worse than self, exchange places and recurse
	// Note that I rearranged this a bit so swap isn't necessary
	void push(Flag *t)
	{
		unsigned slot = m_data.size();
		m_data.push_back(0);

		while(slot > 0) {
			unsigned parent = (slot - 1) / 2;

			if (m_data[parent]->cost() < t->cost())
				break;

			m_data[slot] = m_data[parent];
			m_data[slot]->mpf_heapindex = slot;
			slot = parent;
		}
		m_data[slot] = t;
		t->mpf_heapindex = slot;
		
		debug(0, "push");
	}

	// Rearrange the tree after a node has become better, i.e. move the
	// node up
	// Pushing algorithm is basically the same as in push()
	void boost(Flag *t)
	{
		unsigned slot = t->mpf_heapindex;

		assert(m_data[slot] == t);

		while(slot > 0) {
			unsigned parent = (slot - 1) / 2;

			if (m_data[parent]->cost() <= t->cost())
				break;

			m_data[slot] = m_data[parent];
			m_data[slot]->mpf_heapindex = slot;
			slot = parent;
		}
		m_data[slot] = t;
		t->mpf_heapindex = slot;
		
		debug(0, "boost");
	}

	// Recursively check integrity
	void debug(unsigned node, const char *str)
	{
		unsigned l = node*2 + 1;
		unsigned r = node*2 + 2;
		if (m_data[node]->mpf_heapindex != (int)node) {
			fprintf(stderr, "%s: mpf_heapindex integrity!\n", str);
			exit(-1);
		}
		if (l < m_data.size()) {
			if (m_data[node]->cost() > m_data[l]->cost()) {
				fprintf(stderr, "%s: Integrity failure\n", str);
				exit(-1);
			}
			debug(l, str);
		}
		if (r < m_data.size()) {
			if (m_data[node]->cost() > m_data[r]->cost()) {
				fprintf(stderr, "%s: Integrity failure\n", str);
				exit(-1);
			}
			debug(r, str);
		}
	}

};

/*
===============
Economy::find_route

Route between the two flags.
Store the route in route if it is not null.
Returns true if a route has been found.

This function should always be successfull, except for when it's called from
check_split().
===============
*/
bool Economy::find_route(Flag *start, Flag *end, Route *route)
{
	assert(start->get_economy() == this);
	assert(end->get_economy() == this);

	Map *map = get_owner()->get_game()->get_map();
	
	// advance the path-finding cycle	
	mpf_cycle++;
	if (!mpf_cycle) { // reset all cycle fields
		for(uint i = 0; i < m_flags.size(); i++)
			m_flags[i]->mpf_cycle = 0;
		mpf_cycle++;
	}
	
	// Add the starting flag into the open list
	FlagQueue Open;
	Flag *current;
	
	start->mpf_cycle = mpf_cycle;
	start->mpf_backlink = 0;
	start->mpf_realcost = 0;
	start->mpf_estimate = map->calc_cost_estimate(start->get_position(), end->get_position());
	
	Open.push(start);
	
	while((current = Open.pop())) {
		if (current == end)
			break; // found our goal
		
		// Loop through all neighbouring flags
		Neighbour_list neighbours;
		
		current->get_neighbours(&neighbours);
		
		for(uint i = 0; i < neighbours.size(); i++) {
			Flag *neighbour = neighbours[i].flag;
			int cost;

			// don't need to find the optimal path if we're just checking connectivity			
			if (neighbour == end && !route)
				return true;
			
			cost = current->mpf_realcost + neighbours[i].cost;

			if (neighbour->mpf_cycle != mpf_cycle) {
				// add to open list
				neighbour->mpf_cycle = mpf_cycle;
				neighbour->mpf_realcost = cost;
				neighbour->mpf_estimate = map->calc_cost_estimate(neighbour->get_position(), end->get_position());
				neighbour->mpf_backlink = current;
				Open.push(neighbour);
			} else if (neighbour->cost() > cost+neighbour->mpf_estimate) {
				// found a better path to a field that's already Open
				neighbour->mpf_realcost = cost;
				neighbour->mpf_backlink = current;
				Open.boost(neighbour);
			}
		}
	}
	
	if (!current) // path not found
		return false;
	
	// Unwind the path to form the route
	if (route) {
		Flag *flag;
		
		route->m_totalcost = end->mpf_realcost;
		
		flag = end;
		for(;;) {
			route->m_route.insert(route->m_route.begin(), flag);
			if (flag == start)
				break;
			flag = flag->mpf_backlink;
		}
	}
	
	return true;
}

/*
===============
Economy::add_flag

Add a flag to the flag array.
Only call from Flag init and split/merger code!
===============
*/
void Economy::add_flag(Flag *flag)
{
	assert(flag->get_economy() == 0);

	m_flags.push_back(flag);
	flag->set_economy(this);
	flag->mpf_cycle = 0;
	
	// TODO: add wares associated with this flag to the economy
}

/*
===============
Economy::remove_flag

Remove a flag from the flag array.
Only call from Flag cleanup and split/merger code!
===============
*/
void Economy::remove_flag(Flag *flag)
{
	assert(flag->get_economy() == this);

	// TODO: remove wares associated with this flag to the economy
	
	flag->set_economy(0);

	// fast remove
	uint i;
	for(i = 0; i < m_flags.size(); i++) {
		if (m_flags[i] == flag) {
			if (i < m_flags.size()-1)
				m_flags[i] = m_flags[m_flags.size()-1];
			break;
		}
	}
	assert(i != m_flags.size());
	m_flags.pop_back();

	// automatically delete the economy when it becomes empty.
	if (!m_flags.size())
		delete this;
}

/*
===============
Economy::add_wares

Call this whenever some entity created a ware, e.g. when a lumberjack
has felled a tree.
This is also called when a ware is added to the economy through trade or
a merger.
===============
*/
void Economy::add_wares(int id, int count)
{
	m_wares.add(id, count);
	
	// TODO: add to global player inventory?
}

/*
===============
Economy::remove_wares

Call this whenever a ware his destroyed or consumed, e.g. food has been 
eaten or a warehouse has been destroyed.
This is also called when a ware is removed from the economy through trade or
a split of the Economy.
===============
*/
void Economy::remove_wares(int id, int count)
{
	m_wares.remove(id, count);

	// TODO: remove from global player inventory?
}

/*
===============
Economy::add_warehouse

Add the warehouse to our list of warehouses.
This also adds the wares in the warehouse to the economy. However, if wares are
added to the warehouse in the future, add_wares() must be called.
===============
*/
void Economy::add_warehouse(Warehouse *wh)
{
	m_warehouses.push_back(wh);
	m_wares.add(wh->get_wares());
}

/*
===============
Economy::remove_warehouse

Remove the warehouse and its wares from the economy.
===============
*/
void Economy::remove_warehouse(Warehouse *wh)
{
	m_wares.remove(wh->get_wares());

	// fast remove
	uint i;
	for(i = 0; i < m_warehouses.size(); i++) {
		if (m_warehouses[i] == wh) {
			if (i < m_warehouses.size()-1)
				m_warehouses[i] = m_warehouses[m_warehouses.size()-1];
			break;
		}
	}
	assert(i != m_warehouses.size());
	m_warehouses.pop_back();
}

/*
===============
Economy::add_request

Consider the request, try to fulfill it immediately or queue it for later.
===============
*/
void Economy::add_request(Request *req)
{
	// TODO: handle new request
}

/*
===============
Economy::remove_request

Remove the request from this economy.
Only call remove_request() when a request is actually cancelled. That is, 
_do not_ remove a request when the owning economy changes (split/merge). This
is handled by the actual split/merge functions.
===============
*/
void Economy::remove_request(Request *req)
{
	// TODO: cancel the request
}

/*
===============
Economy::do_merge

Add e's flags to this economy.
Also transfer all wares and wares request. Try to resolve the new ware requests
if possible.
===============
*/
void Economy::do_merge(Economy *e)
{
	log("Economy: merge %i + %i\n", get_nrflags(), e->get_nrflags());

	// Be careful around here. The last e->remove_flag() will cause the other
	// economy to delete itself.
	int i = e->get_nrflags();
	while(i--) {
		assert(i+1 == e->get_nrflags());
		
		Flag *flag = e->m_flags[0];
		
		Neighbour_list neighbours;
		flag->get_neighbours(&neighbours);
		
		for(uint i = 0; i < neighbours.size(); i++)
			neighbours[i].road->set_economy(this);
		
		e->remove_flag(flag);
		add_flag(flag);
	}
	
	// TODO: merge requests and force a check of pending requests
}

/*
===============
Economy::do_split

Flag f and all its direct and indirect neighbours are put into a new economy.
===============
*/
void Economy::do_split(Flag *f)
{
	log("Economy: split %i\n", get_nrflags());

	Economy *e = new Economy(m_owner);
	
	std::set<Flag*> open;
	
	open.insert(f);
	while(open.size()) {
		f = *open.begin();
		open.erase(open.begin());

		// move this flag to the new economy
		remove_flag(f);
		e->add_flag(f);

		//	check all neighbours; if they aren't in the new economy yet, add them
		// to the list
		// make sure roads are reassigned, too
		Neighbour_list neighbours;
		f->get_neighbours(&neighbours);
		
		for(uint i = 0; i < neighbours.size(); i++) {
			Flag *n = neighbours[i].flag;
			
			neighbours[i].road->set_economy(e);
			
			if (n->get_economy() == this)
				open.insert(n);
		}
	}
	
	log("  split %i flags\n", e->get_nrflags());
	
	// TODO: manually split requests (check their owner); check whether they can still be fulfilled
}
