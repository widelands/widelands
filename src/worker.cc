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

	add_attribute(Map_Object::WORKER);
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
Worker *Worker_Descr::create(Editor_Game_Base *gg, Player *owner, PlayerImmovable *location, Coords coords)
{
   Game* g=static_cast<Game*>(gg);

   Worker *worker = (Worker*)create_object();
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

	m_descname = s->get_string("descname", get_name());
	m_helptext = s->get_string("help", "Doh... someone forgot the help text!");

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
Worker::Worker(Worker_Descr *descr)
	: Bob(descr)
{
	m_economy = 0;
	m_location = 0;
	m_state = State_None;
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
			end_state(static_cast<Game*>(get_owner()->get_game()), false);

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

	if (m_economy)
		m_economy->remove_wares(get_descr()->get_ware_id(), 1);

	m_economy = economy;

	WareInstance* item = get_carried_item(get_owner()->get_game());

	if (item)
		item->set_economy(m_economy);

	if (m_economy)
		m_economy->add_wares(get_descr()->get_ware_id(), 1);
}


/*
===============
Worker::set_carried_item

Set the item we carry.
If we carry an item right now, it will be destroyed (see fetch_carried_item()).
===============
*/
void Worker::set_carried_item(Game* g, WareInstance* item)
{
	WareInstance* olditem = get_carried_item(g);

	if (olditem) {
		olditem->cleanup(g);
		delete olditem;
	}

	m_carried_item = item;
	item->set_location(g, this);
	item->update(g);
}


/*
===============
Worker::fetch_carried_item

Stop carrying the current item, and return a pointer to it.
===============
*/
WareInstance* Worker::fetch_carried_item(Game* g)
{
	WareInstance* item = get_carried_item(g);

	if (item) {
		item->set_location(g, 0);
		m_carried_item = 0;
	}

	return item;
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

	if (cancel)
	{
		m_request = 0;
		if (m_route) {
			delete m_route;
			m_route = 0;
		}
	}
	else
	{
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
the economy. On the other hand, S2 did not do that, it's not that simple,
and we really should give those poor workers some rest ;-)
===============
*/
void Worker::set_job_gowarehouse()
{
	m_state = State_GoWarehouse;
	m_gowarehouse = 0;
}


/*
===============
Worker::set_job_idleloop

Loop the same animation over and over again in an idle loop.
===============
*/
void Worker::set_job_idleloop(Game* g, uint anim)
{
	assert(m_state == State_None);

	m_state = State_IdleLoop;
	m_job_anim = anim;
}


/*
===============
Worker::stop_job_idleloop

Stop a running idleloop.
===============
*/
void Worker::stop_job_idleloop(Game* g)
{
	assert(m_state == State_IdleLoop);

	end_state(g, false);
}


/*
===============
Worker::set_job_dropoff

Walk to the building's flag, drop the given item, and walk back inside.
===============
*/
void Worker::set_job_dropoff(Game* g, WareInstance* item)
{
	assert(m_state == State_None);
	assert(item);

	m_state = State_DropOff;
	set_carried_item(g, item);
}


/*
===============
Worker::set_job_fetchfromflag

Walk to the building's flag, fetch an item from the flag that is destined for
the building, and walk back inside.
===============
*/
void Worker::set_job_fetchfromflag(Game* g)
{
	assert(m_state == State_None);

	m_state = State_FetchFromFlag;
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
	WareInstance* item = get_carried_item(g);

	if (item)
		item->destroy(g);

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
			// This will be executed once before our creator gets the chance to
			// assign a job; it can also be called as a result of interrupt_task()
			// from end_state()
			// In those cases, we simply idle a bit to give the creator/owner a
			// chance to assign a job.
			if (prev != TASK_NONE && nexthint != TASK_IDLE) {
				PlayerImmovable *location = get_location(g);

				if (get_carried_item(g))
					get_carried_item(g)->cancel_moving(g);

				if (location && location->get_type() == Map_Object::FLAG) {
					molog("Worker::task_start_best [State_None]: return to warehouse\n");
					set_job_gowarehouse();
				}

				if (m_state == State_None) {
					molog("Worker::task_start_best [State_None]: become fugitive\n");

					set_location(0);
					m_state = State_Fugitive;
					m_fugitive_death = g->get_gametime() + 60000 + 100*(g->logic_rand() % 300);
				}
			}

			start_task_idle(g, get_descr()->get_idle_anim(), 1);
		}
		break;

	case State_IdleLoop:
		start_task_idle(g, m_job_anim, -1);
		break;

	case State_Request:
		run_state_request(g, prev, success, nexthint);
		break;

	case State_Fugitive:
		run_state_fugitive(g, prev, success, nexthint);
		break;

	case State_GoWarehouse:
		run_state_gowarehouse(g, prev, success, nexthint);
		break;

	case State_DropOff:
		run_state_dropoff(g, prev, success, nexthint);
		break;

	case State_FetchFromFlag:
		run_state_fetchfromflag(g, prev, success, nexthint);
		break;

	default:
		throw wexception("Worker::task_start_best: unhandled");
	}
}


/*
===============
Worker::wakeup_flag_capacity [virtual]

Called when the flag we waited on has now got capacity left.
Return true if we actually woke up due to this.
===============
*/
bool Worker::wakeup_flag_capacity(Game* g, Flag* flag)
{
	if (m_state == State_DropOff && get_carried_item(g)) {
		molog("State_DropOff: Wake up: flag capacity.\n");

		interrupt_task(g, false);
		return true;
	}

	return false;
}


/*
===============
Worker::wakeup_leave_building [virtual]

Called when the given building allows us to leave it.
Return true if we actually woke up due to this.
===============
*/
bool Worker::wakeup_leave_building(Game* g, Building* building)
{
	if (get_current_task() == TASK_IDLE)
	{
		BaseImmovable* location = g->get_map()->get_immovable(get_position());

		if (building == location) {
			interrupt_task(g, false, 0, true);
			return true;
		}
	}

	return false;
}


/*
===============
Worker::set_state

Change the current state. However, the current task is not interrupted.
===============
*/
void Worker::set_state(int state)
{
	molog("Set state to %i\n", state);

	assert(m_state == State_None);

	m_state = state;
}


/*
===============
Worker::end_state

Called by the run_state_XXX functions to indicate the current state has
finished.
You can also call this from anywhere else, but do be careful.

In this function, we zero any state before calling any callback functions.
This is because callback functions might wish to set a new state.
===============
*/
void Worker::end_state(Game *g, bool success)
{
	int oldstate = m_state;

	m_state = State_None;

	do_end_state(g, oldstate, success);

	// Make sure task_start_best() is called again soon
	if (get_current_task())
		interrupt_task(g, false, TASK_IDLE);
	else
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
		molog("Worker::end_state [State_None]\n");
		break;

	case State_IdleLoop:
		molog("Worker::end_state [State_IdleLoop]\n");
		break;

	case State_Request:
		{
			Request *request = m_request;

			molog("Worker::end_state [State_Request] %s\n", success ? "success" : "fail");

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
		molog("Worker::end_state [State_Fugitive]\n");
		break;

	case State_GoWarehouse:
		if (m_route) {
			delete m_route;
			m_route = 0;
		}
		molog("Worker::end_state [State_GoWarehouse]\n");
		break;

	case State_DropOff:
		molog("Worker::end_state [State_DropOff]\n");
		break;

	case State_FetchFromFlag:
		molog("Worker::end_state [State_FetchFromFlag]\n");
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
		molog("State_Request: previous task failed\n");
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
		molog("State_Request: total failure\n");
		end_state(g, false);
		return;
	}
	else if (ret == -1)
	{
		// the current route won't lead us to the target
		if (!target || target->get_economy() != get_economy()) {
			molog("State_Request: target unreachable\n");
			end_state(g, false);
			return;
		}

		assert(get_economy() == location->get_economy());

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
		molog("Worker: State_Fugitive: we're on location\n");

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
				molog("Worker: State_Fugitive: move into warehouse\n");
				start_task_forcemove(g, WALK_NW, get_descr()->get_walk_anims());
				set_location(building);
				return;
			}
		}

		if (g->get_gametime() - m_fugitive_death >= 0) {
			molog("Worker: State_Fugitive: die\n");
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

		molog("Worker: State_Fugitive: found warehouse(s)\n");

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

			molog("Worker: State_Fugitive: try to move to warehouse\n");

			// the warehouse could be on a different island, so check for failure
			if (start_task_movepath(g, flag->get_position(), 0, get_descr()->get_walk_anims()))
				return;
		}
	}

	// just walk into a random direction
	Coords dst;

	molog("Worker: State_Fugitive: wander randomly\n");

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
		molog("State_GoWarehouse: previous task failed\n");
		end_state(g, false);
		return;
	}

	if (!location) {
		molog("State_GoWarehouse: location removed\n");
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
Worker::run_state_dropoff

Drop the carried item on the base flag of the current building, and return
into the building.
===============
*/
void Worker::run_state_dropoff(Game *g, uint prev, bool success, uint nexthint)
{
	PlayerImmovable* owner = get_location(g);
	WareInstance* item = get_carried_item(g);

	if (!owner) {
		molog("Worker: DropOff: owner disappeared\n");
		end_state(g, false);
		return;
	}

	BaseImmovable* location = g->get_map()->get_immovable(get_position());

	// Deliver the item
	if (item)
	{
		// We're in the building, walk onto the flag
		if (location->get_type() == Map_Object::BUILDING)
		{
			Flag* flag = ((PlayerImmovable*)location)->get_base_flag();

			if (!flag->has_capacity()) {
				molog("Worker: DropOff: wait for capacity\n");

				flag->wait_for_capacity(g, this);
				start_task_idle(g, get_descr()->get_idle_anim(), -1);
				return;
			}

			// Exit throttle
			if (!((Building*)location)->leave_check_and_wait(g, this)) {
				molog("Worker: DropOff: wait on building's leave queue\n");
				start_task_idle(g, get_descr()->get_idle_anim(), -1);
				return;
			}

			molog("Worker: DropOff: move from building to flag\n");
			start_task_forcemove(g, WALK_SE, get_descr()->get_walk_anims());
			return;
		}

		// We're on the flag, drop the item and pause a little
		if (location->get_type() == Map_Object::FLAG)
		{
			Flag* flag = (Flag*)location;

			if (flag->has_capacity()) {
				molog("Worker: DropOff: dropping the item\n");

				item = fetch_carried_item(g);
				flag->add_item(g, item);

				start_task_idle(g, get_descr()->get_idle_anim(), 50);
				return;
			}

			molog("Worker: DropOff: flag is overloaded\n");

			start_task_forcemove(g, WALK_NW, get_descr()->get_walk_anims());
			return;
		}

		throw wexception("Worker: DropOff: not on building or on flag - fishy");
	}

	// We don't have the item any more, return home
	if (location->get_type() == Map_Object::FLAG) {
		start_task_forcemove(g, WALK_NW, get_descr()->get_walk_anims());
		return;
	}

	if (location->get_type() != Map_Object::BUILDING)
		throw wexception("Worker: DropOff: not on building or on flag on return");

	if (location->has_attribute(WAREHOUSE)) {
		schedule_incorporate(g);
		end_state(g, true);
		return;
	}

	throw wexception("Worker: DropOff: TODO: return into non-warehouse");
}


/*
===============
Worker::run_state_fetchfromflag

Go to the building's flag, fetch an item for the building, walk it into the
building and assign back to building.
===============
*/
void Worker::run_state_fetchfromflag(Game *g, uint prev, bool success, uint nexthint)
{
	PlayerImmovable* owner = get_location(g);
	WareInstance* item = get_carried_item(g);

	if (!owner) {
		molog("Worker: FetchFromFlag: owner disappeared\n");
		end_state(g, false);
		return;
	}

	BaseImmovable* location = g->get_map()->get_immovable(get_position());

	// If we haven't got the item yet, walk onto the flag
	if (!item) {
		if (location->get_type() == BUILDING) {
			// Exit throttle
			if (!((Building*)location)->leave_check_and_wait(g, this)) {
				molog("Worker: FetchFromFlag: wait on building's leave queue\n");
				start_task_idle(g, get_descr()->get_idle_anim(), -1);
				return;
			}

			molog("Worker: FetchFromFlag: move from building to flag\n");
			start_task_forcemove(g, WALK_SE, get_descr()->get_walk_anims());
			return;
		}

		// This can't happen because of the owner check above
		if (location->get_type() != FLAG)
			throw wexception("MO(%u): FetchFromFlag: flag disappeared", get_serial());

		item = ((Flag*)location)->fetch_pending_item(g, owner);

		// The item has decided that it doesn't want to go to us after all
		// In order to return to the warehouse, we're switching to State_DropOff
		if (!item) {
			molog("Worker: FetchFromFlag: item no longer on flag, switch to DropOff\n");

			m_state = State_DropOff;
			start_task_idle(g, get_descr()->get_idle_anim(), 20);
			return;
		}

		set_carried_item(g, item);
		start_task_idle(g, get_descr()->get_idle_anim(), 20);
		return;
	}

	// Go back into the building
	if (location->get_type() == FLAG) {
		molog("Worker: FetchFromFlag: return to building\n");

		start_task_forcemove(g, WALK_NW, get_descr()->get_walk_anims());
		return;
	}

	if (location->get_type() != BUILDING)
		throw wexception("MO(%u): FetchFromFlag: building disappeared", get_serial());

	assert(location == owner);

	molog("Worker: FetchFromFlag: drop item\n");

	item = fetch_carried_item(g);
	item->set_location(g, location);
	item->update(g); // this might remove the item and ack any requests

	// We're back!
	if (location->has_attribute(WAREHOUSE)) {
		schedule_incorporate(g); // implicitly adds the ware
		end_state(g, true);
		return;
	}

	throw wexception("Worker: FetchFromFlag: TODO: return into non-warehouse");
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
		molog("run_route: location has been removed from under us\n");
		return -2;
	}

	// We were waiting; now continue on the next step of the route
	if (prev == TASK_IDLE)
	{
		if (!finalgoal || !route) {
			molog("run_route: goal/route not available\n");
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

			// Exit throttle
			if (!((Building*)location)->leave_check_and_wait(g, this)) {
				molog("run_route: wait on building's leave queue\n");
				start_task_idle(g, get_descr()->get_idle_anim(), -1);
				return 0;
			}

			molog("run_route: move from building to flag\n");
			start_task_forcemove(g, WALK_SE, get_descr()->get_walk_anims());
			return 0;
		}

		// Special case for roads: we always try to get to the starting flag
		// of a road (it's the base flag). This case makes sure we don't walk
		// across the target road unnecessarily
		if (finalgoal->get_type() == Map_Object::ROAD) {
			Road *finalroad = (Road*)finalgoal;

			if (finalroad->get_flag(Road::FlagEnd) == current) {
				molog("run_route: arrived at end flag of target road\n");

				set_location(finalroad);
				return 1;
			}
		}

		// Move on to the next step if there is one
		if (m_route->get_nrsteps()) {
			Flag *dest = m_route->get_flag(g, 1);
			Road *road = current->get_road(dest);

			molog("run_route: move to next flag\n");

			Path path(road->get_path());

			if (dest != road->get_flag(Road::FlagEnd))
				path.reverse();

			start_task_movepath(g, path, get_descr()->get_walk_anims());
			set_location(road);
			m_route->starttrim(1);
			return 0;
		}

		// Finally, we may have to move toward the building
		if (finalgoal->get_type() == Map_Object::BUILDING) {
			assert(finalgoal->get_base_flag() == current);

			molog("run_route: move from flag to building\n");

			start_task_forcemove(g, WALK_NW, get_descr()->get_walk_anims());
			set_location(finalgoal);
			return 0;
		}

		molog("run_route: arrived at goal (road/flag)\n");

		if (finalgoal->get_type() == Map_Object::ROAD)
			set_location(finalgoal);

		return 1; // we're done
	}

	// Check if we've arrived at the target
	if (location == finalgoal) {
		assert(finalgoal->get_type() == Map_Object::BUILDING);

		molog("run_route: arrived at goal (building)\n");

		return 1;
	}

	// Wait a little bit on the flag
	molog("run_route: wait on flag\n");

	BaseImmovable *imm = g->get_map()->get_immovable(get_position());
	if (!imm || imm->get_type() != FLAG)
		throw wexception("run_route: arrived on flag but no flag\n");

	set_location((PlayerImmovable*)imm);

	start_task_idle(g, get_descr()->get_idle_anim(), 50);

	return 0;
}


/*
===============
Worker::draw

Draw the worker, taking the carried item into account.
===============
*/
void Worker::draw(Editor_Game_Base* g, RenderTarget* dst, Point pos)
{
	uint anim = get_current_anim();

	if (!anim)
		return;

	const RGBColor* playercolors = 0;
	Point drawpos;

	calc_drawpos(g, pos, &drawpos);

	if (get_owner())
		playercolors = get_owner()->get_playercolor();

	dst->drawanim(drawpos.x, drawpos.y, anim, g->get_gametime() - get_animstart(), playercolors);

	// Draw the currently carried item
	WareInstance* item = get_carried_item(g);

	if (item) {
		uint itemanim = item->get_ware_descr()->get_idle_anim();

		dst->drawanim(drawpos.x, drawpos.y - 15, itemanim, 0, playercolors);
	}
}


/*
===============
Worker_Descr::create_object

Create a generic worker of this type.
===============
*/
Bob* Worker_Descr::create_object()
{
	return new Worker(this);
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
Carrier::Carrier(Carrier_Descr *descr)
	: Worker(descr)
{
	m_inbuilding = false;
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
				// Wake up and readjust our sleeping position
				interrupt_task(g, false);
			}
			else if (get_state() == State_WorkTransport)
			{
				// This should do the right thing. Since the orientation of the
				// flags does not change, the item will still travel in the right
				// direction, and run_state_worktransport() is rather
				// fault-tolerant.
				molog("Carrier::set_job_road: Interrupt while transporting\n");
				interrupt_task(g, false);
			}
			else
				throw wexception("Carrier::set_job_road: unhandled state\n");
		}
	}
	else
	{
		assert(get_state() == State_WorkIdle || get_state() == State_WorkTransport);

		// Wake up from idling
		interrupt_task(g, false);
		end_state(g, false); // become a fugitive
	}
}


/*
===============
Carrier::notify_ware

Called by Road code to indicate that a new item has arrived on a flag
(0 = start, 1 = end).
Returns true if the carrier is going to fetch it.
===============
*/
bool Carrier::notify_ware(Game* g, int flag)
{
	if (get_state() != State_WorkIdle)
		return false;

	end_state(g, false); // This also interrupts the idling
	set_state(State_WorkTransport);

	m_fetch_flag = flag;

	return true;
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

	case State_WorkTransport:
		run_state_worktransport(g, prev, success, nexthint);
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
	case State_WorkTransport:
		break;

	default:
		Worker::do_end_state(g, oldstate, success);
	}
}


/*
===============
Carrier::find_pending_item

Find a pending item on one of the road's flags, and set m_fetch_flag
accordingly.
Returns true if an item has been found, or false otherwise.
===============
*/
bool Carrier::find_pending_item(Game* g)
{
	Road* road = (Road*)get_location(g);
	CoordPath startpath;
	CoordPath endpath;
	int curidx = -1;
	uint haveitembits = 0;

	if (road->get_flag(Road::FlagStart)->has_pending_item(g, road->get_flag(Road::FlagEnd)))
	{
		haveitembits |= 1;

		startpath = road->get_path();

		curidx = startpath.get_index(get_position());

		startpath.truncate(curidx);
		startpath.reverse();
	}

	if (road->get_flag(Road::FlagEnd)->has_pending_item(g, road->get_flag(Road::FlagStart)))
	{
		haveitembits |= 2;

		endpath = road->get_path();

		if (curidx < 0)
			curidx = endpath.get_index(get_position());

		endpath.starttrim(curidx);
	}

	// If both roads have an item, we pick the one closer to us
	if (haveitembits == 3)
	{
		int startcost, endcost;

		g->get_map()->calc_cost(startpath, &startcost, 0);
		g->get_map()->calc_cost(endpath, &endcost, 0);

		if (startcost < endcost)
			haveitembits = 1;
		else
			haveitembits = 2;
	}

	// Ack our decision
	if (haveitembits == 1)
	{
		bool ok = false;

		m_fetch_flag = 0;

		ok = road->get_flag(Road::FlagStart)->ack_pending_item(g, road->get_flag(Road::FlagEnd));
		if (!ok)
			throw wexception("Carrier::find_pending_item: start flag is messed up");

		return true;
	}

	if (haveitembits == 2)
	{
		bool ok = false;

		m_fetch_flag = 1;

		ok = road->get_flag(Road::FlagEnd)->ack_pending_item(g, road->get_flag(Road::FlagStart));
		if (!ok)
			throw wexception("Carrier::find_pending_item: end flag is messed up");

		return true;
	}

	return false;
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

	molog("Carrier::run_state_workidle.\n");

	// Note: success is false when new wares appear and when the road is destroyed,
	// so it isn't very helpful in this case.
	if (!road) {
		molog("Carrier::run_state_workidle: Road disappeared, become fugitive.\n");
		end_state(g, false);
		return;
	}

	// Check if we can pick up an item at the flag
	if (find_pending_item(g)) {
		molog("Carrier: WorkIdle: Go pick up item from %i\n", m_fetch_flag);
		end_state(g, true);
		set_state(State_WorkTransport);
		return;
	}

	// Move into idle position if necessary
	if (walk_to_index(g, road->get_idle_index()))
		return;

	// Be bored. There's nothing good on TV, either.
	molog("Carrier::run_state_workidle: Idle.\n");

	// TODO: idle animations

	start_task_idle(g, get_descr()->get_idle_anim(), -1);
}


/*
===============
Carrier::run_state_worktransport

Fetch an item at one flag and drop it at the other.

This function has to be rather fault tolerant, especially because the road we're
on could get split while we're on it.
===============
*/
void Carrier::run_state_worktransport(Game* g, uint prev, bool success, uint nexthint)
{
	WareInstance* item;
	Road* road = (Road*)get_location(g);

	molog("Carrier::run_state_worktransport.\n");

	if (!road) {
		molog("Carrier: WorkTransport: Road disappeared, become fugitive.\n");
		m_inbuilding = false;
		end_state(g, false);
		return;
	}

	// If we're "in" the target building, special code applies
	if (m_inbuilding)
	{
		BaseImmovable* pos = g->get_map()->get_immovable(get_position());

		// tough luck, the building has disappeared
		if (!pos) {
			molog("Carrier: WorkTransport: Building disappeared while in building.\n");
			m_inbuilding = false;
			end_state(g, false);
			return;
		}

		// Drop the item, indicating success
		if (pos->get_type() == Map_Object::BUILDING) {
			item = fetch_carried_item(g);

			if (item) {
				molog("Carrier: WorkTransport: Arrived in building.\n");
				item->set_location(g, (Building*)pos);
				item->update(g);

				start_task_idle(g, get_descr()->get_idle_anim(), 20);
				return;
			}

			// Now walk back onto the flag
			molog("Carrier: WorkTransport: Move out of building.\n");
			start_task_forcemove(g, WALK_SE, get_descr()->get_walk_anims());
			return;
		}

		// Wait a little on
		if (pos->get_type() != Map_Object::FLAG)
			throw wexception("Carrier: WorkTransport: inbuilding, but neither on building nor flag");

		m_inbuilding = false;

		// Check for pending items
		if (!find_pending_item(g)) {
			molog("Carrier: WorkTransport: back to idle.\n");
			end_state(g, true);
			set_state(State_WorkIdle);
			return;
		}

		start_task_idle(g, get_descr()->get_idle_anim(), 50);
		return;
	}

	// If we don't carry something, walk to the flag
	if (!get_carried_item(g)) {
		Flag* flag;
		Flag* otherflag;

		if (walk_to_flag(g, m_fetch_flag))
			return;

		molog("Carrier: WorkTransport: pick up from flag.\n");

		flag = road->get_flag((Road::FlagId)m_fetch_flag);
		otherflag = road->get_flag((Road::FlagId)(m_fetch_flag ^ 1));
		item = flag->fetch_pending_item(g, otherflag);

		if (!item) {
			molog("Carrier: WorkTransport: Nothing on flag.\n");
			end_state(g, false);
			set_state(State_WorkIdle);
			return;
		}

		set_carried_item(g, item);
		start_task_idle(g, get_descr()->get_idle_anim(), 20);
		return;
	}

	// If the item should go to the building attached to our flag, walk directly
	// into said building
	Flag* flag;

	item = get_carried_item(g);
	flag = road->get_flag((Road::FlagId)(m_fetch_flag ^ 1));

	assert(item->get_location(g) == this);

	// A sanity check is necessary, in case the building has been destroyed
	if (item->is_moving(g))
	{
		PlayerImmovable* final = item->get_final_move_step(g);

		if (final != flag && final->get_base_flag() == flag)
		{
			if (walk_to_flag(g, m_fetch_flag ^ 1))
				return;

			molog("Carrier: WorkTransport: Move into building.\n");
			start_task_forcemove(g, WALK_NW, get_descr()->get_walk_anims());
			m_inbuilding = true;
			return;
		}
	}

	// Move into waiting position if the flag is overloaded
	if (!flag->has_capacity())
	{
		if (walk_to_flag(g, m_fetch_flag ^ 1, true))
			return;

		// Wait one field away
		flag->wait_for_capacity(g, this);
		start_task_idle(g, get_descr()->get_idle_anim(), -1);
		return;
	}

	// If there is capacity, walk to the flag
	if (walk_to_flag(g, m_fetch_flag ^ 1))
		return;

	item = fetch_carried_item(g);
	flag->add_item(g, item);

	// Check for pending items
	if (!find_pending_item(g)) {
		molog("Carrier: WorkTransport: back to idle.\n");
		end_state(g, true);
		set_state(State_WorkIdle);
		return;
	}

	// Even the strongest human can't work non-stop
	start_task_idle(g, get_descr()->get_idle_anim(), 50);
}


/*
===============
Carrier::wakeup_flag_capacity [virtual]

Called by flag code when our target flag has capacity.
===============
*/
bool Carrier::wakeup_flag_capacity(Game* g, Flag* flag)
{
	if (get_state() == State_WorkTransport && get_carried_item(g)) {
		interrupt_task(g, false);
		return true;
	}

	return Worker::wakeup_flag_capacity(g, flag);
}


/*
===============
Carrier::walk_to_index

Walk to the given index on the owning road.
Returns true if a move task has been started, or false if we're already on
the target field.
===============
*/
bool Carrier::walk_to_index(Game* g, int index)
{
	Road* road = (Road*)get_location(g);
	CoordPath path(road->get_path());
	int curidx = path.get_index(get_position());

	assert(curidx != -1);

	if (curidx != index) {
		molog("Carrier::walk_to_index: Move from %i to %i.\n", curidx, index);

		if (curidx < index) {
			path.truncate(index);
			path.starttrim(curidx);

			start_task_movepath(g, path, get_descr()->get_walk_anims());
		} else {
			path.truncate(curidx);
			path.starttrim(index);
			path.reverse();

			start_task_movepath(g, path, get_descr()->get_walk_anims());
		}

		return true;
	}

	return false;
}


/*
===============
Carrier::walk_to_flag

Walk to the given flag, or one field before it if offset is true.
Returns true if a move task has been started, or false if we're already on
the target field.
===============
*/
bool Carrier::walk_to_flag(Game* g, int flag, bool offset)
{
	int idx;

	if (!flag) {
		idx = 0;
		if (offset)
			idx++;
	} else {
		Road* road = (Road*)get_location(g);
		const Path& path = road->get_path();

		idx = path.get_nsteps();
		if (offset)
			idx--;
	}

	return walk_to_index(g, idx);
}


/*
===============
Carrier_Descr::create_object

Create a carrier of this type.
===============
*/
Bob *Carrier_Descr::create_object()
{
	return new Carrier(this);
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

		if (!strcasecmp(type, "generic"))
			descr = new Worker_Descr(tribe, name);
		else if (!strcasecmp(type, "carrier"))
			descr = new Carrier_Descr(tribe, name);
		else
			throw wexception("Unknown worker type '%s' [supported: carrier]", type);

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
