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
#include "profile.h"
#include "bob.h"
#include "worker.h"
#include "player.h"
#include "tribe.h"
#include "transport.h"


/*
==============================================================================

Worker IMPLEMENTATION

==============================================================================
*/

/*
===============
Worker_Descr::Worker_Descr
Worker_Descr::~Worker_Descr
===============
*/
Worker_Descr::Worker_Descr(Tribe_Descr *tribe, const char *name)
	: Bob_Descr(name)
{
	m_tribe = tribe;
	m_menu_pic = 0;
	m_menu_pic_fname = 0;
	m_ware_id = -1;
}

Worker_Descr::~Worker_Descr(void)
{
	if (m_menu_pic_fname)
		free(m_menu_pic_fname);
}


/*
===============
Worker_Descr::load_graphics

Load graphics (other than animations).
===============
*/
void Worker_Descr::load_graphics()
{
	m_menu_pic = g_gr->get_picture(PicMod_Game, m_menu_pic_fname);
}


/*
===============
Worker_Descr::set_ware_id

Set the worker's ware id. Called by Game::init_wares.
===============
*/
void Worker_Descr::set_ware_id(int idx)
{
	m_ware_id = idx;
}

/*
===============
Worker_Descr::create

Custom creation routing that accounts for the location.
===============
*/
Worker *Worker_Descr::create(Editor_Game_Base *gg, Player *owner, PlayerImmovable *location, Coords coords, bool logic)
{
   assert(logic); // a worker without logic doesn't make sense
   Game* g=static_cast<Game*>(gg);

   Worker *worker = (Worker*)create_object(g->is_game());
	worker->set_owner(owner);
	worker->set_location(location);
	worker->set_position(g, coords);
	worker->init(g);
	return worker;
}

/*
===============
Worker_Descr::parse

Parse the worker data from configuration
===============
*/
void Worker_Descr::parse(const char *directory, Profile *prof, const EncodeData *encdata)
{
	const char *string;
	char buf[256];
	char fname[256];
	Section *s;

	Bob_Descr::parse(directory, prof, encdata);

	s = prof->get_safe_section("global");

	snprintf(buf, sizeof(buf),	"%s_menu.bmp", get_name());
	string = s->get_string("menu_pic", buf);
	
	snprintf(fname, sizeof(fname), "%s/%s", directory, string);
	m_menu_pic_fname = strdup(fname);
	
	// Read the walking animations
	s = prof->get_section("walk");
	m_walk_anims.parse(directory, prof, "walk_??", s, encdata);

	s = prof->get_section("walkload");
	m_walkload_anims.parse(directory, prof, "walkload_??", s, encdata);
}


/*
==============================

IMPLEMENTATION

==============================
*/

/*
===============
Worker::Worker
Worker::~Worker
===============
*/
Worker::Worker(Worker_Descr *descr, bool logic)
	: Bob(descr, logic)
{
	m_economy = 0;
	m_location = 0;
	m_state = State_None;
	m_carried_ware = -1;
	m_route = 0;
}

Worker::~Worker()
{
}

/*
===============
Worker::get_movecaps
===============
*/
uint Worker::get_movecaps()
{
	return MOVECAPS_WALK;
}

/*
===============
Worker::set_location

Change the location. This should be called in the following situations:
- worker creation (usually, location is a warehouse)
- worker moves along a route (location is a road and finally building)
- current location is destroyed (building burnt down etc...)
===============
*/
void Worker::set_location(PlayerImmovable *location)
{
	PlayerImmovable *oldlocation = get_location(get_owner()->get_game());

	if (oldlocation)
	{
		// Note: even though we have an oldlocation, m_economy may be zero
		// (oldlocation got deleted)

		oldlocation->remove_worker(this);
	}
	else
	{
		assert(!m_economy);
	}

	m_location = location;

	if (location)
	{
		Economy *eco = location->get_economy();

		if (!m_economy)
			set_economy(eco);
		else if (m_economy != eco)
			throw wexception("Worker::set_location changes economy");

		location->add_worker(this);
	}
	else
	{
		// Our location has been destroyed, we are now fugitives.
		// Interrupt whatever we've been doing.
		if (m_state > State_None)
			interrupt_task(static_cast<Game*>(get_owner()->get_game()), false);

		set_economy(0);
	}
}

/*
===============
Worker::set_economy

Change the worker's current economy. This is called:
- by set_location() when appropriate
- by the current location, when the location's economy changes
===============
*/
void Worker::set_economy(Economy *economy)
{
	if (economy == m_economy)
		return;

	if (m_economy) {
		m_economy->remove_wares(get_descr()->get_ware_id(), 1);

		if (m_carried_ware >= 0)
			m_economy->remove_wares(m_carried_ware, 1);
	}

	m_economy = economy;

	if (m_economy) {
		m_economy->add_wares(get_descr()->get_ware_id(), 1);

		if (m_carried_ware >= 0)
			m_economy->add_wares(m_carried_ware, 1);
	}
}

/*
===============
Worker::set_job_request

Tell the worker to walk the route to the request target
===============
*/
void Worker::set_job_request(Request *req, const Route *route)
{
	assert(m_state == State_None);

	m_state = State_Request;

	m_request = req;
	m_route = new Route(*route);

	// Hmm.. whenever this is called, a CMD_ACT should already be pending,
	// issued by either Bob::init() or by whatever the guy is currently doing
}

/*
===============
Worker::change_job_request

Called by Economy code when:
- the request has been cancelled & destroyed (cancel is true)
- the current route has been broken (cancel is false)
===============
*/
void Worker::change_job_request(bool cancel)
{
	assert(m_state == State_Request);

	if (cancel) {
		m_request = 0;
	} else {
		if (m_route && !m_route->verify(static_cast<Game*>(get_owner()->get_game()))) {
			delete m_route;
			m_route = 0;
		}
	}
}

/*
===============
Worker::set_job_gowarehouse

Get the worker to move to the nearest warehouse.

Note: It might be interesting to add the worker to the list of usable wares in
the economy. On the other hand, S2 did not do that, and we really should give
those poor workers some rest ;-)
===============
*/
void Worker::set_job_gowarehouse()
{
	m_state = State_GoWarehouse;
	m_gowarehouse = 0;
}

/*
===============
Worker::schedule_incorporate

Schedule an immediate CMD_INCORPORATE, which will integrate this worker into
the warehouse he is standing on.
===============
*/
void Worker::schedule_incorporate(Game *g)
{
	g->get_cmdqueue()->queue(g->get_gametime(), SENDER_MAPOBJECT, CMD_INCORPORATE, m_serial);
}

/*
===============
Worker::incorporate

Incorporate the worker into the warehouse it's standing on immediately.
This will delete the worker.
===============
*/
void Worker::incorporate(Game *g)
{
	PlayerImmovable *location = get_location(g);

	if (location && location->has_attribute(WAREHOUSE)) {
		Warehouse *wh = (Warehouse*)location;

		wh->incorporate_worker(g, this);
		return;
	}

	// our location has been deleted from under us
}

/*
===============
Worker::init

Initialize the worker
===============
*/
void Worker::init(Editor_Game_Base *g)
{
	Bob::init(g);

	// a worker should always start out at a fixed location
	assert(get_location(g));
}

/*
===============
Worker::cleanup

Remove the worker.
===============
*/
void Worker::cleanup(Editor_Game_Base *g)
{
	if (get_location(g))
		set_location(0);

	Bob::cleanup(g);
}

/*
===============
Worker::task_start_best [virtual]

Give the worker something to do after the last task has finished.
===============
*/
void Worker::task_start_best(Game* g, uint prev, bool success, uint nexthint)
{
	switch(m_state) {
	case State_None:
		{
			// this will be executed once before our creator gets the chance to
			// assign a job; therefore, just idle if this is the first call
			if (prev != TASK_NONE) {
				PlayerImmovable *location = get_location(g);

				if (location && location->get_type() == Map_Object::FLAG) {
					log("Worker::task_start_best [State_None]: return to warehouse\n");
					set_job_gowarehouse();
				}

				if (m_state == State_None) {
					log("Worker::task_start_best [State_None]: become fugitive\n");

					set_location(0);
					m_state = State_Fugitive;
					m_fugitive_death = g->get_gametime() + 60000 + 100*(g->logic_rand() % 300);
				}
			}

			start_task_idle(g, get_descr()->get_idle_anim(), 2);
		}
		return;

	case State_Request:
		run_state_request(g, prev, success, nexthint);
		return;

	case State_Fugitive:
		run_state_fugitive(g, prev, success, nexthint);
		return;

	case State_GoWarehouse:
		run_state_gowarehouse(g, prev, success, nexthint);
		return;
	}

	throw wexception("Worker::task_start_best: unhandled");
}


/*
===============
Worker::set_state

Change the current state. However, the current task is not interrupted.
===============
*/
void Worker::set_state(int state)
{
	assert(m_state == State_None);

	m_state = state;
}


/*
===============
Worker::end_state

Called by the run_state_XXX functions to indicate the current state has
finished.

In this function, we zero any state before calling any callback functions.
This is because callback functions might wish to set a new state.
===============
*/
void Worker::end_state(Game *g, bool success)
{
	int oldstate = m_state;

	m_state = State_None;

	do_end_state(g, oldstate, success);

	// 2ms idling to get task_start_best() called again for the new state
	start_task_idle(g, get_descr()->get_idle_anim(), 2);
}


/*
===============
Worker::do_end_state [virtual]

Called by end_state().
Free data that belongs to the state, possibly set a new state.
===============
*/
void Worker::do_end_state(Game* g, int oldstate, bool success)
{
	switch(oldstate) {
	case State_None:
		log("Worker::end_state [State_None]\n");
		break;

	case State_Request:
		{
			Request *request = m_request;

			log("Worker::end_state [State_Request] %s\n", success ? "success" : "fail");

			if (m_route) {
				delete m_route;
				m_route = 0;
			}
			m_request = 0;

			if (request) {
				if (success)
					request->transfer_finish(g);
				else
					request->transfer_fail(g);
			}
		}
		break;

	case State_Fugitive:
		// note that run_state_fugitive should have scheduled either incorporate or destroy
		log("Worker::end_state [State_Fugitive]\n");
		break;

	case State_GoWarehouse:
		if (m_route) {
			delete m_route;
			m_route = 0;
		}
		log("Worker::end_state [State_GoWarehouse]\n");
		break;

	default:
		throw wexception("Worker::do_end_state: unhandled");
	}
}


/*
===============
Worker::run_state_request

Decide what to do in State_Request.

Note: We call run_route as soon as possible. This way, run_route() can
decide what to do and setup location and stuff. This makes it easier to
go into GoWarehouse mode instead of becoming a fugitive.
===============
*/
void Worker::run_state_request(Game *g, uint prev, bool success, uint nexthing)
{
	// if our previous task failed, reset to State_None
	if (!success) {
		log("State_Request: previous task failed\n");
		end_state(g, false);
		return;
	}

	// Figure out our target
	PlayerImmovable *target = 0;

	if (m_request)
		target = m_request->get_target(g);

	int ret = run_route(g, prev, m_route, target);
	PlayerImmovable *location = get_location(g);

	if (ret == -2)
	{
		// we won't ever be able to walk any route again
		log("State_Request: total failure\n");
		end_state(g, false);
		return;
	}
	else if (ret == -1)
	{
		// the current route won't lead us to the target
		if (!target || target->get_economy() != get_economy()) {
			log("State_Request: target unreachable\n");
			end_state(g, false);
			return;
		}

		if (!m_route)
			m_route = new Route;

		if (!get_economy()->find_route(location->get_base_flag(), target->get_base_flag(), m_route))
			throw wexception("State_Request: find_route failed");

		start_task_idle(g, get_descr()->get_idle_anim(), 10);
	}
	else if (ret == 1) // reached the target
	{
		end_state(g, true);
	}
}

/*
===============
Worker::run_state_fugitive

Wander around aimlessly until we find a warehouse
===============
*/
void Worker::run_state_fugitive(Game *g, uint prev, bool success, uint nexthint)
{
	Map *map = g->get_map();
	PlayerImmovable *location = get_location(g);

	if (location) {
		log("Worker: State_Fugitive: we're on location\n");

		if (location->has_attribute(WAREHOUSE)) {
			schedule_incorporate(g);
			end_state(g, true);
			return;
		}

		set_location(0);
		location = 0;
	}

	// check whether we're on a flag and it's time to return home
	if (prev == TASK_MOVEPATH) {
		BaseImmovable *imm = map->get_immovable(get_position());

		if (imm && imm->get_type() == FLAG) {
			Flag *flag = (Flag*)imm;
			Building *building = flag->get_building();

			if (building && building->has_attribute(WAREHOUSE)) {
				log("Worker: State_Fugitive: move into warehouse\n");
				start_task_forcemove(g, WALK_NW, get_descr()->get_walk_anims());
				set_location(building);
				return;
			}
		}

		if (g->get_gametime() - m_fugitive_death >= 0) {
			log("Worker: State_Fugitive: die\n");
			schedule_destroy(g);
			return;
		}

		start_task_idle(g, get_descr()->get_idle_anim(), 10 + g->logic_rand()%20);
		return;
	}

	// Try to find a warehouse we can return to
	std::vector<ImmovableFound> warehouses;

	if (map->find_immovables(get_position(), 15, &warehouses, FindImmovableAttribute(WAREHOUSE))) {
		int bestdist = -1;
		Warehouse *best = 0;

		log("Worker: State_Fugitive: found warehouse(s)\n");

		for(uint i = 0; i < warehouses.size(); i++) {
			Warehouse *wh = (Warehouse*)warehouses[i].object;
			int dist = map->calc_distance(get_position(), warehouses[i].coords);

			if (!best || dist < bestdist) {
				best = wh;
				bestdist = dist;
			}
		}

		bool use;

		if (bestdist < 6)
			use = true;
		else
			use = (g->logic_rand() % (bestdist-4)) == 0;

		// okay, move towards the flag of this warehouse
		if (use) {
			Flag *flag = best->get_base_flag();

			log("Worker: State_Fugitive: try to move to warehouse\n");

			// the warehouse could be on a different island, so check for failure
			if (start_task_movepath(g, flag->get_position(), 0, get_descr()->get_walk_anims()))
				return;
		}
	}

	// just walk into a random direction
	Coords dst;

	log("Worker: State_Fugitive: wander randomly\n");

	dst.x = m_position.x + (g->logic_rand()%5) - 2;
	dst.y = m_position.y + (g->logic_rand()%5) - 2;

	if (start_task_movepath(g, dst, 4, get_descr()->get_walk_anims()))
		return;

	start_task_idle(g, get_descr()->get_idle_anim(), 50);
}

/*
===============
Worker::run_state_gowarehouse

Try to reach the nearest warehouse. Become fugitive if no warehouse is in
this economy.
===============
*/
void Worker::run_state_gowarehouse(Game *g, uint prev, bool success, uint nexthint)
{
	PlayerImmovable *location = get_location(g);

	if (!success) {
		log("State_GoWarehouse: previous task failed\n");
		end_state(g, false);
		return;
	}

	if (!location) {
		log("State_GoWarehouse: location removed\n");
		end_state(g, false);
		return;
	}

	// if necessary find a warehouse to move to
	Warehouse *wh = (Warehouse*)m_gowarehouse.get(g);

	if (!m_route) {
		assert(location->get_type() == Map_Object::FLAG);

		m_route = new Route;

		wh = get_economy()->find_nearest_warehouse(location->get_base_flag(), m_route);
		if (!wh) {
			set_location(0); // no warehouse in this economy, become a fugitive
			end_state(g, false);
			return;
		}

		m_gowarehouse = wh;
	}

	int ret = run_route(g, prev, m_route, wh);

	if (ret == -2) {
		end_state(g, false); // something awful happened
	} else if (ret == -1) {
		if (m_route) { // need to recalc our route / find a new warehouse
			delete m_route;
			m_route = 0;
		}
		m_gowarehouse = 0;
		start_task_idle(g, get_descr()->get_idle_anim(), 10);
	} else if (ret == 1) {
		schedule_incorporate(g); // we're in the warehouse
		end_state(g, true);
	}
}

/*
===============
Worker::run_route

Start tasks to move the worker further towards the given goal, using the given
road.
The function returns one of the following values:
-2 something happened so that we will never be able to reach the goal
-1 the route has become invalid; recalculate it
 0 moved further towards the goal
 1 we have reached the goal
Note that run_route has only scheduled a task if it returns 0. If it returns
anything else, the caller is responsible for setting up the next task.
===============
*/
int Worker::run_route(Game *g, uint prev, Route *route, PlayerImmovable *finalgoal)
{
	PlayerImmovable *location = get_location(g);

	if (!location) {
		log("run_route: location has been removed from under us\n");
		return -2;
	}

	// We were waiting; now continue on the next step of the route
	if (prev == TASK_IDLE)
	{
		if (!finalgoal || !route) {
			log("run_route: goal/route not available\n");
			return -1;
		}

		// Verify that the route is still good
		if (!route->verify(g))
			return -1;

		Flag *current = route->get_flag(g, 0);

		// We may have to move out of a building
		if (current != location) {
			assert(location->get_type() == Map_Object::BUILDING);
			assert(location->get_base_flag() == current);

			log("run_route: move from building to flag\n");

			start_task_forcemove(g, WALK_SE, get_descr()->get_walk_anims());
			return 0;
		}

		// Special case for roads: we always try to get to the starting flag
		// of a road (it's the base flag). This case makes sure we don't walk
		// across the target road unnecessarily
		if (finalgoal->get_type() == Map_Object::ROAD) {
			Road *finalroad = (Road*)finalgoal;

			if (finalroad->get_flag_end() == current) {
				log("run_route: arrived at end flag of target road\n");

				set_location(finalroad);
				return 1;
			}
		}

		// Move on to the next step if there is one
		if (m_route->get_nrsteps()) {
			Flag *dest = m_route->get_flag(g, 1);
			Road *road = current->get_road(dest);

			log("run_route: move to next flag\n");

			Path path(road->get_path());

			if (dest != road->get_flag_end())
				path.reverse();

			start_task_movepath(g, path, get_descr()->get_walk_anims());
			set_location(road);
			m_route->starttrim(1);
			return 0;
		}

		// Finally, we may have to move toward the building
		if (finalgoal->get_type() == Map_Object::BUILDING) {
			assert(finalgoal->get_base_flag() == current);

			log("run_route: move from flag to building\n");

			start_task_forcemove(g, WALK_NW, get_descr()->get_walk_anims());
			set_location(finalgoal);
			return 0;
		}

		log("run_route: arrived at goal (road/flag)\n");

		if (finalgoal->get_type() == Map_Object::ROAD)
			set_location(finalgoal);

		return 1; // we're done
	}

	// Check if we've arrived at the target
	if (location == finalgoal) {
		assert(finalgoal->get_type() == Map_Object::BUILDING);

		log("run_route: arrived at goal (building)\n");

		return 1;
	}

	// Wait a little bit on the flag
	log("run_route: wait on flag\n");

	BaseImmovable *imm = g->get_map()->get_immovable(get_position());
	if (!imm || imm->get_type() != FLAG)
		throw wexception("run_route: arrived on flag but no flag\n");

	set_location((PlayerImmovable*)imm);

	start_task_idle(g, get_descr()->get_idle_anim(), 50);

	return 0;
}

/*
==============================================================================

Carrier IMPLEMENTATION

==============================================================================
*/

/*
===============
Carrier_Descr::Carrier_Descr
Carrier_Descr::~Carrier_Descr
===============
*/
Carrier_Descr::Carrier_Descr(Tribe_Descr *tribe, const char *name)
	: Worker_Descr(tribe, name)
{
}

Carrier_Descr::~Carrier_Descr(void)
{
}

/*
===============
Carrier_Descr::parse

Parse carrier-specific configuration data
===============
*/
void Carrier_Descr::parse(const char *directory, Profile *prof, const EncodeData *encdata)
{
	Worker_Descr::parse(directory, prof, encdata);
}


/*
==============================

IMPLEMENTATION

==============================
*/

/*
===============
Carrier::Carrier
Carrier::~Carrier
===============
*/
Carrier::Carrier(Carrier_Descr *descr, bool logic)
	: Worker(descr, logic)
{
}

Carrier::~Carrier()
{
}


/*
===============
Carrier::set_job_road

Begin working on the given road. This is also called after a road split,
even if the location remains the same.

If road is not null, this implicitly puts the worker into working mode.
If road is null, it automatically makes the worker a fugitive.
===============
*/
void Carrier::set_job_road(Game* g, Road* road)
{
	assert(get_location(g) == road);

	if (road)
	{
		if (get_state() < State_WorkIdle)
			set_state(State_WorkIdle);
		else
		{
			if (get_state() == State_WorkIdle)
			{
				interrupt_task(g, false); // Wake up and readjust our sleeping position
			}
			else
				throw wexception("Carrier::set_job_road: unhandled state\n");
		}
	}
	else
	{
		assert(get_state() == State_WorkIdle);

		// TODO: drop ware

		if (get_current_task() == TASK_MOVEPATH)
			interrupt_task(g, false);

		set_state(State_None); // become a fugitive
	}
}


/*
===============
Carrier::task_start_best

Handle carrier-specific states.
===============
*/
void Carrier::task_start_best(Game* g, uint prev, bool success, uint nexthint)
{
	switch(get_state()) {
	case State_WorkIdle:
		run_state_workidle(g, prev, success, nexthint);
		return;
	}

	Worker::task_start_best(g, prev, success, nexthint);
}


/*
===============
Carrier::do_end_state [virtual]

Cleanup after a state.
===============
*/
void Carrier::do_end_state(Game *g, int oldstate, bool success)
{
	switch(oldstate) {
	case State_WorkIdle:
		break;

	default:
		Worker::do_end_state(g, oldstate, success);
	}
}


/*
===============
Carrier::run_state_workidle

Determine the next task for idling.
===============
*/
void Carrier::run_state_workidle(Game *g, uint prev, bool success, uint nexthint)
{
	Road* road = (Road*)get_location(g);

	log("Carrier::run_state_workidle.\n");

	// Note: success is false when new wares appear and when the road is destroyed,
	// so it isn't very helpful in this case.
	if (!road) {
		log("Carrier::run_state_workidle: Road disappeared, become fugitive.\n");
		end_state(g, false);
		return;
	}

	// TODO: Check if we can pick up an item at the flag

	// Move into idle position if necessary
	CoordPath path(road->get_path());
	int curidx = path.get_index(get_position());
	int idleidx = road->get_idle_index();

	assert(curidx != -1);

	if (curidx != idleidx) {
		log("Carrier::run_state_workidle: Move into idle position\n");

		if (curidx < idleidx) {
			path.truncate(idleidx);
			path.starttrim(curidx);

			start_task_movepath(g, path, get_descr()->get_walk_anims());
		} else {
			path.truncate(curidx);
			path.starttrim(idleidx);
			path.reverse();

			start_task_movepath(g, path, get_descr()->get_walk_anims());
		}

		return;
	}

	// Be bored. There's nothing good on TV, either.
	log("Carrier::run_state_workidle: Idle.\n");

	// TODO: idle animations

	start_task_idle(g, get_descr()->get_idle_anim(), -1);
}


/*
===============
Carrier_Descr::create_object

Create a carrier of this type.
===============
*/
Bob *Carrier_Descr::create_object(bool logic)
{
	return new Carrier(this, logic);
}


/*
==============================================================================

Worker factory

==============================================================================
*/

/*
===============
Worker_Descr::create_from_dir [static]

Automatically create the appropriate Worker_Descr type from the given
config data.
May return 0.
===============
*/
Worker_Descr *Worker_Descr::create_from_dir(Tribe_Descr *tribe, const char *directory, const EncodeData *encdata)
{
	const char *name;

	// name = last element of path
	const char *slash = strrchr(directory, '/');
	const char *backslash = strrchr(directory, '\\');

	if (backslash && (!slash || backslash > slash))
		slash = backslash;

	if (slash)
		name = slash+1;
	else
		name = directory;

	// Open the config file
	Worker_Descr *descr = 0;
	char fname[256];

	snprintf(fname, sizeof(fname), "%s/conf", directory);

	if (!g_fs->FileExists(fname))
		return 0;

	try
	{
		Profile prof(fname);
		Section *s = prof.get_safe_section("global");
		const char *type = s->get_safe_string("type");

		if (!strcasecmp(type, "carrier"))
			descr = new Carrier_Descr(tribe, name);
		else
			throw wexception("Unknown carrier type '%s' [supported: carrier]", type);
		
		descr->parse(directory, &prof, encdata);
		prof.check_used();
	}
	catch(std::exception &e) {
		if (descr)
			delete descr;
		throw wexception("Error reading worker %s: %s", name, e.what());
	}
	catch(...) {
		if (descr)
			delete descr;
		throw;
	}
	
	return descr;
}


#if 0
//
// class Soldier_Descr
//
int Soldier_Descr::read(FileRead* f)
{
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   
   energy = f->Unsigned16();
/*
   attack_l.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   attack_l.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   attack_l.read(f);
   attack1_l.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   attack1_l.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   attack1_l.read(f);
   evade_l.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   evade_l.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   evade_l.read(f);
   evade1_l.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   evade1_l.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   evade1_l.read(f);

   attack_r.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   attack_r.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   attack_r.read(f);
   attack1_r.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   attack1_r.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   attack1_r.read(f);
   evade_r.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   evade_r.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   evade_r.read(f);
   evade1_r.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   evade1_r.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   evade1_r.read(f);
*/     
   return RET_OK;
}

// Worker class read functions
int Has_Working_Worker_Descr::read(FileRead* f) {
/*
   working.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   working.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   working.read(f);
*/
   return RET_OK;
}

int Has_Working1_Worker_Descr::read(FileRead* f) {
/*
   working1.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   working1.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   working1.read(f);
*/
   return RET_OK;
}
int Has_Walk1_Worker_Descr::read(FileRead* f) {
/*
   walk_ne1.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   walk_nw1.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   walk_w1.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   walk_sw1.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   walk_se1.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   walk_w1.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   walk_ne1.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   walk_nw1.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   walk_w1.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   walk_sw1.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   walk_se1.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   walk_w1.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());

   walk_ne1.read(f);
   walk_e1.read(f);
   walk_se1.read(f);
   walk_sw1.read(f);
   walk_w1.read(f);
   walk_nw1.read(f);
*/
   return RET_OK;
}
int Scientist::read(FileRead* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   
   return RET_OK;
}

int Searcher::read(FileRead* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   Has_Walk1_Worker_Descr::read(f);
   Has_Working_Worker_Descr::read(f);

   // nothing additional
   return RET_OK;
}

int Grower::read(FileRead* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   Has_Walk1_Worker_Descr::read(f);
   Has_Working_Worker_Descr::read(f);
   Has_Working1_Worker_Descr::read(f);

   return RET_OK;
}

int Planter::read(FileRead* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   Has_Walk1_Worker_Descr::read(f);
   Has_Working_Worker_Descr::read(f);
   
   return RET_OK;
}
int SitDigger_Base::read(FileRead* f) {
   // Nothing to do
   return RET_OK;
}
int SitDigger::read(FileRead* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   SitDigger_Base::read(f);
   
   // Nothing of our own stuff to read
   return RET_OK;
}

int Carrier::read(FileRead* f) {

   // nothing to do
   return RET_OK;
}

int Def_Carrier::read(FileRead* f) {
   Worker_Descr::read(f);
   SitDigger_Base::read(f);
   Carrier::read(f);
   Has_Walk1_Worker_Descr::read(f);
   Has_Working_Worker_Descr::read(f);
   Has_Working1_Worker_Descr::read(f);

   return RET_OK;
}

int Add_Carrier::read(FileRead* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   SitDigger_Base::read(f);
   Carrier::read(f);
   Has_Walk1_Worker_Descr::read(f);
   Has_Working_Worker_Descr::read(f);
   Has_Working1_Worker_Descr::read(f);

   return RET_OK;
}
int Builder::read(FileRead* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   SitDigger_Base::read(f);
   Has_Working_Worker_Descr::read(f);
   Has_Working1_Worker_Descr::read(f);

   return RET_OK;
}

int Planer::read(FileRead* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   SitDigger_Base::read(f);
   Has_Working_Worker_Descr::read(f);
      
   return RET_OK;
}

int Explorer::read(FileRead* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   SitDigger_Base::read(f);

   return RET_OK;
}

int Geologist::read(FileRead* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   SitDigger_Base::read(f);
   Has_Working_Worker_Descr::read(f);
   Has_Working1_Worker_Descr::read(f);
   
   return RET_OK;
}
#endif
