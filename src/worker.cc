/*
 * Copyright (C) 2002, 2003 by the Widelands Development Team
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
		set_economy(0);

		send_signal((Game*)get_owner()->get_game(), "location");
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

	assert(!get_economy());

	Bob::cleanup(g);
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
Worker::schedule_incorporate

Schedule an immediate CMD_INCORPORATE, which will integrate this worker into
the warehouse he is standing on.
===============
*/
void Worker::schedule_incorporate(Game* g)
{
	g->get_cmdqueue()->queue(g->get_gametime(), SENDER_MAPOBJECT, CMD_INCORPORATE, m_serial);
	force_skip_act(g);
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
	send_signal(g, "fail");
}


/*
===============
Worker::init_auto_task

Set a fallback task.
===============
*/
void Worker::init_auto_task(Game* g)
{
	PlayerImmovable* location = get_location(g);

	if (location) {
		molog("init_auto_task: go warehouse\n");

		start_task_gowarehouse(g);
		return;
	}

	molog("init_auto_task: become fugitive\n");

	start_task_fugitive(g);
}


/*
==============================

REQUEST task

Follow the given request.
Signal "update" to force a recalculation of the route.
Signal "cancel" to cancel the request.

==============================
*/

Bob::Task Worker::taskRequest = {
	"request",

	(Bob::Ptr)&Worker::request_update,
	(Bob::Ptr)&Worker::request_signal,
	(Bob::Ptr)&Worker::request_mask,
};


/*
===============
Worker::start_task_request

Tell the worker to fulfill the request
===============
*/
void Worker::start_task_request(Game* g, Request *req)
{
	State* state;

	push_task(g, &taskRequest);

	state = get_state();
	state->request = req;
}


/*
===============
Worker::request_update
===============
*/
void Worker::request_update(Game* g, State* state)
{
	PlayerImmovable* location = get_location(g);

	assert(location); // 'location' signal expected otherwise

	// The request is no longer valid, the task has failed
	if (!state->request) {
		molog("[request]: Fail (without request)\n");

		set_signal("fail");
		pop_task(g);
		return;
	}

	// Figure out where to go
	PlayerImmovable* target = state->request->get_target(g);

	if (get_location(g) == target) {
		Request* rq = state->request;

		molog("[request]: finish\n");

		state->request = 0; // don't fail, we were successful!
		pop_task(g);

		rq->transfer_finish(g);
		return;
	}

	Route* route = new Route;

	if (!get_economy()->find_route(get_location(g)->get_base_flag(), target->get_base_flag(), route)) {
		molog("[request]: Can't find route\n");

		set_signal("fail");
		pop_task(g);
		return;
	}

	start_task_route(g, route, target);
}


/*
===============
Worker::request_signal
===============
*/
void Worker::request_signal(Game* g, State* state)
{
	std::string signal = get_signal();

	// The caller requested a route update, or the previously calulcated route
	// failed.
	// We will recalculate the route on the next update().
	if (signal == "update" || signal == "fail") {
		molog("[request]: Got signal '%s' -> recalculate\n", signal.c_str());

		set_signal("");
		return;
	}

	molog("[request]: Cancel due to signal '%s'\n", signal.c_str());
	pop_task(g);
}


/*
===============
Worker::request_mask
===============
*/
void Worker::request_mask(Game* g, State* state)
{
	std::string signal = get_signal();

	if (signal == "cancel")
		state->request = 0; // dont't call transfer_fail/finish when cancelled
}


/*
===============
Worker::update_task_request

Called by Economy code when:
- the request has been cancelled & destroyed (cancel is true)
- the current route has been broken (cancel is false)
===============
*/
void Worker::update_task_request(Game* g, bool cancel)
{
	send_signal(g, cancel ? "cancel" : "update");
}


/*
==============================

BUILDINGWORK task

Endless loop, in which the worker calls the owning building's
get_building_work() function to intiate subtasks.
The signal "update" is used to wake the worker up after a sleeping time
(initiated by a false return value from get_building_work()).

==============================
*/

Bob::Task Worker::taskBuildingwork = {
	"buildingwork",

	(Bob::Ptr)&Worker::buildingwork_update,
	(Bob::Ptr)&Worker::buildingwork_signal,
	0
};


/*
===============
Worker::start_task_buildingwork

Begin work at a building.
===============
*/
void Worker::start_task_buildingwork(Game* g)
{
	push_task(g, &taskBuildingwork);
}


/*
===============
Worker::buildingwork_update
===============
*/
void Worker::buildingwork_update(Game* g, State* state)
{
	std::string signal = get_signal();

	if (signal == "location") {
		pop_task(g);
		return;
	}

	set_signal("");

	// Get the new job
	PlayerImmovable* location = get_location(g);

	assert(location);
	assert(location->get_type() == BUILDING);

	if (!((Building*)location)->get_building_work(g, this, !signal.size())) {
		molog("[buildingwork]: Nothing to be done.\n");
		set_animation(g, 0);
		skip_act(g);
	}
}


/*
===============
Worker::buildingwork_signal
===============
*/
void Worker::buildingwork_signal(Game* g, State* state)
{
	std::string signal = get_signal();

	if (signal == "update")
		set_signal("");

	schedule_act(g, 1);
}


/*
===============
Worker::update_task_buildingwork

Wake up the buildingwork task.
===============
*/
void Worker::update_task_buildingwork(Game* g)
{
	State* state = get_state();

	if (state->task == &taskBuildingwork)
		send_signal(g, "update");
}


/*
==============================

GOWAREHOUSE task

==============================
*/

Bob::Task Worker::taskGowarehouse = {
	"gowarehouse",

	(Bob::Ptr)&Worker::gowarehouse_update,
	(Bob::Ptr)&Worker::gowarehouse_signal,
	0,
};

/*
===============
Worker::start_task_gowarehouse

Get the worker to move to the nearest warehouse.

Note: It might be interesting to add the worker to the list of usable wares in
the economy. On the other hand, S2 did not do that, it's not that simple,
and we really should give those poor workers some rest ;-)
===============
*/
void Worker::start_task_gowarehouse(Game* g)
{
	push_task(g, &taskGowarehouse);
}


/*
===============
Worker::gowarehouse_update
===============
*/
void Worker::gowarehouse_update(Game* g, State* state)
{
	PlayerImmovable *location = get_location(g);

	assert(location); // 'location' signal expected otherwise

	if (location->get_type() == BUILDING && location->has_attribute(WAREHOUSE)) {
		molog("[gowarehouse]: Back in warehouse, schedule incorporate\n");
		schedule_incorporate(g);
		return;
	}

	// Find nearest warehouse and move to it
	Warehouse* wh;
	Route* route = new Route;

	wh = get_economy()->find_nearest_warehouse(location->get_base_flag(), route);
	if (!wh) {
		molog("[gowarehouse]: no warehouse found\n");

		set_location(0);
		return;
	}

	molog("[gowarehouse]: found warehouse (%u)\n", wh->get_serial());
	start_task_route(g, route, wh);
}

/*
===============
Worker::gowarehouse_signal
===============
*/
void Worker::gowarehouse_signal(Game* g, State* state)
{
	std::string signal = get_signal();

	// if routing has failed, try a different warehouse/route on next update()
	if (signal == "fail") {
		molog("[gowarehouse]: caught 'fail'\n");

		set_signal("");
		return;
	}

	molog("[gowarehouse]: cancel for signal '%s'\n", signal.c_str());
	pop_task(g);
}


/*
==============================

DROPOFF task

==============================
*/

Bob::Task Worker::taskDropoff = {
	"dropoff",

	(Bob::Ptr)&Worker::dropoff_update,
	0,
	0,
};

/*
===============
Worker::set_job_dropoff

Walk to the building's flag, drop the given item, and walk back inside.
===============
*/
void Worker::start_task_dropoff(Game* g, WareInstance* item)
{
	assert(item);

	set_carried_item(g, item);

	push_task(g, &taskDropoff);
}


/*
===============
Worker::dropoff_update
===============
*/
void Worker::dropoff_update(Game* g, State* state)
{
	PlayerImmovable* owner = get_location(g);
	WareInstance* item = get_carried_item(g);

	assert(owner); // expect 'location' signal

	BaseImmovable* location = g->get_map()->get_immovable(get_position());

	// Deliver the item
	if (item)
	{
		// We're in the building, walk onto the flag
		if (location->get_type() == Map_Object::BUILDING)
		{
			Flag* flag = ((PlayerImmovable*)location)->get_base_flag();

			if (start_task_waitforcapacity(g, flag))
				return;

			// Exit throttle
			if (start_task_waitleavebuilding(g))
				return;

			molog("[dropoff]: move from building to flag\n");
			start_task_forcemove(g, WALK_SE, get_descr()->get_walk_anims());
			return;
		}

		// We're on the flag, drop the item and pause a little
		if (location->get_type() == Map_Object::FLAG)
		{
			Flag* flag = (Flag*)location;

			if (flag->has_capacity()) {
				molog("[dropoff]: dropping the item\n");

				item = fetch_carried_item(g);
				flag->add_item(g, item);

				set_animation(g, get_descr()->get_idle_anim());
				schedule_act(g, 50);
				return;
			}

			molog("[dropoff]: flag is overloaded\n");
			start_task_forcemove(g, WALK_NW, get_descr()->get_walk_anims());
			return;
		}

		throw wexception("MO(%u): [dropoff]: not on building or on flag - fishy", get_serial());
	}

	// We don't have the item any more, return home
	if (location->get_type() == Map_Object::FLAG) {
		start_task_forcemove(g, WALK_NW, get_descr()->get_walk_anims());
		return;
	}

	if (location->get_type() != Map_Object::BUILDING)
		throw wexception("MO(%u): [dropoff]: not on building on return", get_serial());

	if (location->has_attribute(WAREHOUSE)) {
		schedule_incorporate(g);
		return;
	}

	throw wexception("Worker: DropOff: TODO: return into non-warehouse");
}


/*
==============================

FETCHFROMFLAG task

ivar1 is set to 0 if we should move to the flag and fetch the item, and it is
set to 1 if we should move into the building.

==============================
*/

Bob::Task Worker::taskFetchfromflag = {
	"fetchfromflag",

	(Bob::Ptr)&Worker::fetchfromflag_update,
	0,
	0,
};


/*
===============
Worker::start_task_fetchfromflag

Walk to the building's flag, fetch an item from the flag that is destined for
the building, and walk back inside.
===============
*/
void Worker::start_task_fetchfromflag(Game* g)
{
	push_task(g, &taskFetchfromflag);

	get_state()->ivar1 = 0;
}


/*
===============
Worker::fetchfromflag_update
===============
*/
void Worker::fetchfromflag_update(Game *g, State* state)
{
	PlayerImmovable* owner = get_location(g);
	WareInstance* item = get_carried_item(g);

	assert(owner); // expect 'location' signal

	BaseImmovable* location = g->get_map()->get_immovable(get_position());

	// If we haven't got the item yet, walk onto the flag
	if (!item && !state->ivar1) {
		if (location->get_type() == BUILDING) {
			// Exit throttle
			if (start_task_waitleavebuilding(g))
				return;

			molog("[fetchfromflag]: move from building to flag\n");
			start_task_forcemove(g, WALK_SE, get_descr()->get_walk_anims());
			return;
		}

		// This can't happen because of the owner check above
		if (location->get_type() != FLAG)
			throw wexception("MO(%u): [fetchfromflag]: flag disappeared", get_serial());

		item = ((Flag*)location)->fetch_pending_item(g, owner);
		state->ivar1 = 1; // force return to building

		// The item has decided that it doesn't want to go to us after all
		// In order to return to the warehouse, we're switching to State_DropOff
		if (item)
			set_carried_item(g, item);

		set_animation(g, get_descr()->get_idle_anim());
		schedule_act(g, 20);
		return;
	}

	// Go back into the building
	if (location->get_type() == FLAG) {
		molog("[fetchfromflag]: return to building\n");

		start_task_forcemove(g, WALK_NW, get_descr()->get_walk_anims());
		return;
	}

	if (location->get_type() != BUILDING)
		throw wexception("MO(%u): [fetchfromflag]: building disappeared", get_serial());

	assert(location == owner);

	molog("[fetchfromflag]: back home\n");

	item = fetch_carried_item(g);
	item->set_location(g, location);
	item->update(g); // this might remove the item and ack any requests

	// We're back!
	if (location->has_attribute(WAREHOUSE)) {
		schedule_incorporate(g);
		return;
	}

	throw wexception("Worker: FetchFromFlag: TODO: return into non-warehouse");
}


/*
==============================

WAITFORCAPACITY task

Wait for available capacity on a flag.

==============================
*/

Bob::Task Worker::taskWaitforcapacity = {
	"waitforcapacity",

	(Bob::Ptr)&Worker::waitforcapacity_update,
	(Bob::Ptr)&Worker::waitforcapacity_signal,
	0,
};

/*
===============
Worker::start_task_waitforcapacity

Checks the capacity of the flag.
If there is none, a wait task is pushed, and the worker is added to the flag's
wait queue. The function returns true in this case.
If the flag still has capacity, the function returns false and doesn't act at
all.
===============
*/
bool Worker::start_task_waitforcapacity(Game* g, Flag* flag)
{
	if (flag->has_capacity())
		return false;

	push_task(g, &taskWaitforcapacity);

	get_state()->objvar1 = flag;

	flag->wait_for_capacity(g, this);

	return true;
}


/*
===============
Worker::waitforcapacity_update
===============
*/
void Worker::waitforcapacity_update(Game* g, State* state)
{
	skip_act(g); // wait indefinitely
}


/*
===============
Worker::waitforcapacity_signal
===============
*/
void Worker::waitforcapacity_signal(Game* g, State* state)
{
	// The 'wakeup' signal is to be expected; don't propagate it
	if (get_signal() == "wakeup")
		set_signal("");

	pop_task(g);
}

/*
===============
Worker::wakeup_flag_capacity

Called when the flag we waited on has now got capacity left.
Return true if we actually woke up due to this.
===============
*/
bool Worker::wakeup_flag_capacity(Game* g, Flag* flag)
{
	State* state = get_state();

	if (state && state->task == &taskWaitforcapacity) {
		molog("[waitforcapacity]: Wake up: flag capacity.\n");

		if (state->objvar1.get(g) != flag)
			throw wexception("MO(%u): wakeup_flag_capacity: Flags don't match.", get_serial());

		send_signal(g, "wakeup");
		return true;
	}

	return false;
}


/*
==============================

WAITLEAVEBUILDING task

==============================
*/

Bob::Task Worker::taskWaitleavebuilding = {
	"waitleavebuilding",

	(Bob::Ptr)&Worker::waitleavebuilding_update,
	(Bob::Ptr)&Worker::waitleavebuilding_signal,
	0,
};


/*
===============
Worker::start_task_waitleavebuilding

Returns false if we can just leave our location building right now.
Return true if we have to wait on the building's leave queue. In the latter
case, this function takes care of all the task scheduling etc.
===============
*/
bool Worker::start_task_waitleavebuilding(Game* g)
{
	PlayerImmovable* location = get_location(g);

	assert(location && location->get_type() == BUILDING);

	Building* building = (Building*)location;

	if (building->leave_check_and_wait(g, this))
		return false;

	// Set the wait task
	push_task(g, &taskWaitleavebuilding);

	get_state()->objvar1 = building;

	return true;
}


/*
===============
Worker::waitleavebuilding_update
===============
*/
void Worker::waitleavebuilding_update(Game* g, State* state)
{
	skip_act(g);
}


/*
===============
Worker::waitleavebuilding_signal
===============
*/
void Worker::waitleavebuilding_signal(Game* g, State* state)
{
	molog("[waitleavebuilding]: Woke up by signal '%s'\n", get_signal().c_str());

	if (get_signal() == "wakeup")
		set_signal("");

	pop_task(g);
}


/*
===============
Worker::wakeup_leave_building

Called when the given building allows us to leave it.
Return true if we actually woke up due to this.
===============
*/
bool Worker::wakeup_leave_building(Game* g, Building* building)
{
	State* state = get_state();

	molog("wakeup_leave_building called\n");

	if (state && state->task == &taskWaitleavebuilding) {
		molog("[waitleavebuilding]: wakeup\n");

		if (state->objvar1.get(g) != building)
			throw wexception("MO(%u): [waitleavebuilding]: buildings don't match", get_serial());

		send_signal(g, "wakeup");
		return true;
	}

	return false;
}


/*
==============================

ROUTE task

Follow the given route. If we have an explicit target, move onto this target
before finishing the task.
If the route is broken or something else happens, we do not attempt to reroute.
Let the caller do this.

Note that we temporarily mask all signals except for "location" and "wakeup",
so that walking isn't interrupted in the middle of a road.
The masked out signal is stored in svar1.

==============================
*/

Bob::Task Worker::taskRoute = {
	"route",

	(Bob::Ptr)&Worker::route_update,
	0,
	(Bob::Ptr)&Worker::route_mask,
};


/*
===============
Worker::start_task_route

Begin walking the given route. If target is not null, we also walk in/onto the
target when we arrive.
Note that if one segment of the route is destroyed, the route is not recreated
automatically.

We claim ownership of the route, i.e. the caller must not free it.
===============
*/
void Worker::start_task_route(Game* g, Route* route, PlayerImmovable* target)
{
	State* state;

	push_task(g, &taskRoute);

	state = get_state();
	state->objvar1 = target;
	state->route = route;
}


/*
===============
Worker::route_update
===============
*/
void Worker::route_update(Game* g, State* state)
{
	PlayerImmovable* location = get_location(g);
	PlayerImmovable* nextstep;

	if (!location)
		set_signal("location");

	// abort on signals
	if (get_signal().size()) {
		molog("[route]: Interrupted by signal '%s'\n", get_signal().c_str());
		pop_task(g);
		return;
	}

	//molog("[route]: update\n");

	// issue post-poned signals
	if (state->svar1.size()) {
		set_signal(state->svar1);
		schedule_act(g, 5);
		return;
	}

	// Find out where we should go next
	if (state->route)
	{
		// Verify that the route is still good
		if (!state->route->verify(g)) {
			molog("[route]: route no longer valid\n");
			set_signal("fail");
			pop_task(g);
			return;
		}

		nextstep = state->route->get_flag(g, 0);

		// Special case for roads #1:
		// If we're on a road, and both the first and the second flag on the
		// route are endpoints of this road, move to the second flag instead
		// of the first
		if (state->route->get_nrsteps() && location->get_type() == ROAD) {
			Flag* otherflag = state->route->get_flag(g, 1);
			Road* road = (Road*)location;

			if ((road->get_flag(Road::FlagStart) == nextstep && road->get_flag(Road::FlagEnd) == otherflag) ||
			    (road->get_flag(Road::FlagEnd) == nextstep && road->get_flag(Road::FlagStart) == otherflag))
			{
				nextstep = otherflag;
				state->route->starttrim(1);
			}
		}


		// Special case for roads #2:
		// Make sure we don't walk unnecessarily across a road if the road is
		// our final target.
		PlayerImmovable* finalgoal = (PlayerImmovable*)state->objvar1.get(g);

		if (finalgoal->get_type() == Map_Object::ROAD) {
			Road *finalroad = (Road*)finalgoal;

			if (finalroad->get_flag(Road::FlagEnd) == location) {
				molog("[route]: arrived at end flag of target road\n");

				nextstep = finalgoal;
				delete state->route;
				state->route = 0;
			}
		}
	}
	else
	{
		// We no longer have a route; there must be a final goal we have to reach
		nextstep = (PlayerImmovable*)state->objvar1.get(g);

		if (!nextstep) {
			molog("[route]: no route and no final goal\n");
			set_signal("fail");
			pop_task(g);
			return;
		}
	}

	// If location == nextstep, consume one part of the route and wait a little
	if (nextstep == location) {
		if (state->route)
		{
			if (state->route->get_nrsteps())
				state->route->starttrim(1);
			else {
				delete state->route;
				state->route = 0;
			}
		}
		else
		{
			state->objvar1 = 0;
		}

		if (!state->route && !state->objvar1.get(g)) {
			molog("[route]: target reached\n");
			pop_task(g);
			return;
		}

		set_animation(g, get_descr()->get_idle_anim());
		schedule_act(g, 50);
		return;
	}

	// Building to Flag
	if (location->get_type() == BUILDING) {
		if (location->get_base_flag() != nextstep)
			throw wexception("MO(%u): [route]: in building, nextstep is not building's flag", get_serial());

		if (start_task_waitleavebuilding(g))
			return;

		molog("[route]: move from building to flag\n");
		start_task_forcemove(g, WALK_SE, get_descr()->get_walk_anims());
		set_location(nextstep);
		return;
	}

	if (location->get_type() == FLAG) {
		// Flag to Building
		if (nextstep->get_type() == BUILDING) {
			if (nextstep->get_base_flag() != location)
				throw wexception("MO(%u): [route]: next step is building, but we're nowhere near", get_serial());

			molog("[route]: move from flag to building\n");
			start_task_forcemove(g, WALK_NW, get_descr()->get_walk_anims());
			set_location(nextstep);
			return;
		}

		// Flag to Flag
		if (nextstep->get_type() == FLAG) {
			Road* road = ((Flag*)location)->get_road((Flag*)nextstep);

			molog("[route]: move to next flag\n");

			Path path(road->get_path());

			if (nextstep != road->get_flag(Road::FlagEnd))
				path.reverse();

			start_task_movepath(g, path, get_descr()->get_walk_anims());
			set_location(road);
			return;
		}

		// Flag to Road
		if (nextstep->get_type() == ROAD) {
			Road* road = (Road*)nextstep;

			if (road->get_flag(Road::FlagStart) != location && road->get_flag(Road::FlagEnd) != location)
				throw wexception("MO(%u): [route]: nextstep is road, but we're nowhere near", get_serial());

			molog("[route]: set location to road %u\n", road->get_serial());
			set_location(road);
			set_animation(g, get_descr()->get_idle_anim());
			schedule_act(g, 10); // wait a little
			return;
		}

		throw wexception("MO(%u): [route]: flag to bad nextstep %u", get_serial(), nextstep->get_serial());
	}

	if (location->get_type() == ROAD) {
		// Road to Flag
		if (nextstep->get_type() == FLAG) {
			Road* road = (Road*)location;
			const Path& path = road->get_path();
			int index;

			if (nextstep == road->get_flag(Road::FlagStart))
				index = 0;
			else if (nextstep == road->get_flag(Road::FlagEnd))
				index = path.get_nsteps();
			else
				index = -1;

			if (index >= 0)
			{
				if (start_task_movepath(g, path, index, get_descr()->get_walk_anims())) {
					molog("[route]: from road %u to flag %u\n", get_serial(), road->get_serial(),
									nextstep->get_serial());
					return;
				}
			}
			else
			{
				if (nextstep != g->get_map()->get_immovable(get_position()))
					throw wexception("MO(%u): [route]: road to flag, but flag is nowhere near", get_serial());
			}

			molog("[route]: arrive at flag %u\n", nextstep->get_serial());
			set_location((Flag*)nextstep);
			set_animation(g, get_descr()->get_idle_anim());
			schedule_act(g, 10); // wait a little
			return;
		}

		throw wexception("MO(%u): [route]: from road to bad nextstep %u (type %u)", get_serial(),
					nextstep->get_serial(), nextstep->get_type());
	}

	throw wexception("MO(%u): location %u has bad type %u", get_serial(),
					location->get_serial(), location->get_type());
}


/*
===============
Worker::route_mask
===============
*/
void Worker::route_mask(Game* g, State* state)
{
	std::string signal = get_signal();

	//molog("[route]: Filter signal '%s'\n", signal.c_str());

	// 'location' and wakeup are allowed to get through
	if (signal == "location" || signal == "wakeup")
		return;

	state->svar1 = signal; // delay the signal
	set_signal("");
}



/*
==============================

FUGITIVE task

Run around aimlessly until we find a warehouse.

==============================
*/

Bob::Task Worker::taskFugitive = {
	"fugitive",

	(Bob::Ptr)&Worker::fugitive_update,
	(Bob::Ptr)&Worker::fugitive_signal,
	0,
};


/*
===============
Worker::start_task_fugitive
===============
*/
void Worker::start_task_fugitive(Game* g)
{
	push_task(g, &taskFugitive);

	get_state()->ivar1 = g->get_gametime() + 60000 + 100*(g->logic_rand() % 300);
}


/*
===============
Worker::fugitive_update
===============
*/
void Worker::fugitive_update(Game* g, State* state)
{
	Map *map = g->get_map();
	PlayerImmovable *location = get_location(g);

	if (location) {
		molog("[fugitive]: we're on location\n");

		if (location->has_attribute(WAREHOUSE)) {
			schedule_incorporate(g);
			return;
		}

		set_location(0);
		location = 0;
	}

	// check whether we're on a flag and it's time to return home
	BaseImmovable *imm = map->get_immovable(get_position());

	if (imm && imm->get_type() == FLAG) {
		Flag *flag = (Flag*)imm;
		Building *building = flag->get_building();

		if (building && building->has_attribute(WAREHOUSE)) {
			molog("[fugitive]: move into warehouse\n");
			start_task_forcemove(g, WALK_NW, get_descr()->get_walk_anims());
			set_location(building);
			return;
		}
	}

	// time to die?
	if (g->get_gametime() - state->ivar1 >= 0) {
		molog("[fugitive]: die\n");

		schedule_destroy(g);
		return;
	}

	// Try to find a warehouse we can return to
	std::vector<ImmovableFound> warehouses;

	if (map->find_immovables(get_position(), 15, &warehouses, FindImmovableAttribute(WAREHOUSE))) {
		int bestdist = -1;
		Warehouse *best = 0;

		molog("[fugitive]: found warehouse(s)\n");

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

			molog("[fugitive]: try to move to warehouse\n");

			// the warehouse could be on a different island, so check for failure
			if (start_task_movepath(g, flag->get_position(), 0, get_descr()->get_walk_anims()))
				return;
		}
	}

	// just walk into a random direction
	Coords dst;

	molog("[fugitive]: wander randomly\n");

	dst.x = get_position().x + (g->logic_rand()%5) - 2;
	dst.y = get_position().y + (g->logic_rand()%5) - 2;

	if (start_task_movepath(g, dst, 4, get_descr()->get_walk_anims()))
		return;

	start_task_idle(g, get_descr()->get_idle_anim(), 50);
}


/*
===============
Worker::fugitive_signal
===============
*/
void Worker::fugitive_signal(Game* g, State* state)
{
	molog("[fugitive]: interrupted by signal '%s'\n", get_signal().c_str());
	pop_task(g);
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
	m_acked_ware = -1;
}

Carrier::~Carrier()
{
}


/*
==============================

ROAD task

Signal "update" on road split.
Signal "ware" when a ware has arrived.

==============================
*/

Bob::Task Carrier::taskRoad = {
	"road",

	(Bob::Ptr)&Carrier::road_update,
	(Bob::Ptr)&Carrier::road_signal,
	0,
};


/*
===============
Carrier::start_task_road

Work on the given road, assume the location is correct.
===============
*/
void Carrier::start_task_road(Game* g, Road* road)
{
	assert(get_location(g) == road);

	push_task(g, &taskRoad);

	get_state()->ivar1 = 0;

	m_acked_ware = -1;
}


/*
===============
Carrier::update_task_road

Called by Road code when the road is split.
===============
*/
void Carrier::update_task_road(Game* g)
{
	send_signal(g, "update");
}


/*
===============
Carrier::road_update
===============
*/
void Carrier::road_update(Game* g, State* state)
{
	Road* road = (Road*)get_location(g);

	assert(road); // expect 'location' signal

	// Check for pending items
	if (m_acked_ware < 0)
		find_pending_item(g);

	if (m_acked_ware >= 0) {
		if (state->ivar1)
		{
			molog("[road]: Go pick up item from %i\n", m_acked_ware);

			state->ivar1 = 0;
			start_task_transport(g, m_acked_ware);
		}
		else
		{
			// Short delay before we move to pick up
			state->ivar1 = 1;

			set_animation(g, get_descr()->get_idle_anim());
			schedule_act(g, 50);
		}

		return;
	}

	// Move into idle position if necessary
	if (start_task_movepath(g, road->get_path(), road->get_idle_index(), get_descr()->get_walk_anims()))
		return;

	// Be bored. There's nothing good on TV, either.
	molog("[road]: Idle.\n");

	// TODO: idle animations

	set_animation(g, get_descr()->get_idle_anim());
	skip_act(g); // wait until signal
	state->ivar1 = 1; // we're available immediately after an idle phase
}


/*
===============
Carrier::road_signal
===============
*/
void Carrier::road_signal(Game* g, State* state)
{
	std::string signal = get_signal();

	if (signal == "update" || signal == "ware") {
		set_signal(""); // update() will do the rest
		schedule_act(g, 10);
		return;
	}

	molog("[road]: Terminated by signal '%s'\n", signal.c_str());
	pop_task(g);
}


/*
==============================

TRANSPORT task

Fetch an item from a flag, drop it on the other flag.
ivar1 is the flag we fetch from, or -1 when we're in the target building.

Signal "update" when the road has been split etc.

==============================
*/

Bob::Task Carrier::taskTransport = {
	"transport",

	(Bob::Ptr)&Carrier::transport_update,
	(Bob::Ptr)&Carrier::transport_signal,
	0,
};


/*
===============
Carrier::start_task_transport

Begin the transport task.
===============
*/
void Carrier::start_task_transport(Game* g, int fromflag)
{
	State* state;

	assert(!get_carried_item(g));

	push_task(g, &taskTransport);

	state = get_state();
	state->ivar1 = fromflag;
}


/*
===============
Carrier::transport_update
===============
*/
void Carrier::transport_update(Game* g, State* state)
{
	WareInstance* item;
	Road* road = (Road*)get_location(g);

	assert(road); // expect 'location' signal

	//molog("[transport]\n");

	// If we're "in" the target building, special code applies
	if (state->ivar1 == -1)
	{
		BaseImmovable* pos = g->get_map()->get_immovable(get_position());

		// tough luck, the building has disappeared
		if (!pos) {
			molog("[transport]: Building disappeared while in building.\n");

			set_location(0);
			return;
		}

		// Drop the item, indicating success
		if (pos->get_type() == Map_Object::BUILDING) {
			item = fetch_carried_item(g);

			if (item) {
				molog("[transport]: Arrived in building.\n");
				item->set_location(g, (Building*)pos);
				item->update(g);

				set_animation(g, get_descr()->get_idle_anim());
				schedule_act(g, 20);
				return;
			}

			// Now walk back onto the flag
			molog("[transport]: Move out of building.\n");
			start_task_forcemove(g, WALK_SE, get_descr()->get_walk_anims());
			return;
		}

		// We're done
		if (pos->get_type() != Map_Object::FLAG)
			throw wexception("MO(%u): [transport]: inbuilding, but neither on building nor on flag", get_serial());

		pop_task(g);
		return;
	}

	// If we don't carry something, walk to the flag
	if (!get_carried_item(g)) {
		Flag* flag;
		Flag* otherflag;

		if (start_task_walktoflag(g, state->ivar1))
			return;

		molog("[transport]: pick up from flag.\n");

		flag = road->get_flag((Road::FlagId)state->ivar1);
		otherflag = road->get_flag((Road::FlagId)(state->ivar1 ^ 1));
		item = flag->fetch_pending_item(g, otherflag);

		if (!item) {
			molog("[transport]: Nothing on flag.\n");
			pop_task(g);
			return;
		}

		set_carried_item(g, item);
		m_acked_ware = -1;

		set_animation(g, get_descr()->get_idle_anim());
		schedule_act(g, 20);
		return;
	}

	// If the item should go to the building attached to our flag, walk directly
	// into said building
	Flag* flag;

	item = get_carried_item(g);
	flag = road->get_flag((Road::FlagId)(state->ivar1 ^ 1));

	assert(item->get_location(g) == this);

	// A sanity check is necessary, in case the building has been destroyed
	if (item->is_moving(g))
	{
		PlayerImmovable* final = item->get_final_move_step(g);

		if (final != flag && final->get_base_flag() == flag)
		{
			if (start_task_walktoflag(g, state->ivar1 ^ 1))
				return;

			molog("[transport]: Move into building.\n");
			start_task_forcemove(g, WALK_NW, get_descr()->get_walk_anims());
			state->ivar1 = -1;
			return;
		}
	}

	// Move into waiting position if the flag is overloaded
	if (!flag->has_capacity())
	{
		if (start_task_walktoflag(g, state->ivar1 ^ 1, true))
			return;

		// Wait one field away
		start_task_waitforcapacity(g, flag);
		return;
	}

	// If there is capacity, walk to the flag
	if (start_task_walktoflag(g, state->ivar1 ^ 1))
		return;

	item = fetch_carried_item(g);
	flag->add_item(g, item);

	molog("[transport]: back to idle.\n");
	pop_task(g);
}


/*
===============
Carrier::transport_signal
===============
*/
void Carrier::transport_signal(Game* g, State* state)
{
	std::string signal = get_signal();

	if (signal == "update") {
		set_signal("");
		schedule_act(g, 10);
		return;
	}

	molog("[transport]: Interrupted by signal '%s'\n", signal.c_str());
	pop_task(g);
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
	State* state = get_state();

	// Check if we've already acked something
	if (m_acked_ware >= 0) {
		molog("notify_ware(%i): already acked %i\n", flag, m_acked_ware);
		return false;
	}

	// Don't ack if we're currently moving away from that flag
	m_acked_ware = flag;

	if (state->task == &taskRoad)
		send_signal(g, "ware");

	return true;
}


/*
===============
Carrier::find_pending_item

Find a pending item on one of the road's flags, ack it and set m_acked_ware
accordingly.
===============
*/
void Carrier::find_pending_item(Game* g)
{
	Road* road = (Road*)get_location(g);
	CoordPath startpath;
	CoordPath endpath;
	int curidx = -1;
	uint haveitembits = 0;

	assert(m_acked_ware < 0);

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

		m_acked_ware = 0;

		ok = road->get_flag(Road::FlagStart)->ack_pending_item(g, road->get_flag(Road::FlagEnd));
		if (!ok)
			throw wexception("Carrier::find_pending_item: start flag is messed up");

		return;
	}

	if (haveitembits == 2)
	{
		bool ok = false;

		m_acked_ware = 1;

		ok = road->get_flag(Road::FlagEnd)->ack_pending_item(g, road->get_flag(Road::FlagStart));
		if (!ok)
			throw wexception("Carrier::find_pending_item: end flag is messed up");

		return;
	}
}


/*
===============
Carrier::start_task_walktoflag

Walk to the given flag, or one field before it if offset is true.
Returns true if a move task has been started, or false if we're already on
the target field.
===============
*/
bool Carrier::start_task_walktoflag(Game* g, int flag, bool offset)
{
	Road* road = (Road*)get_location(g);
	const Path& path = road->get_path();
	int idx;

	if (!flag) {
		idx = 0;
		if (offset)
			idx++;
	} else {
		idx = path.get_nsteps();
		if (offset)
			idx--;
	}

	return start_task_movepath(g, path, idx, get_descr()->get_walk_anims());
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
