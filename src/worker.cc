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
#include "pic.h"
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
	m_ware_id = -1;
}

Worker_Descr::~Worker_Descr(void)
{
	if (m_menu_pic)
		delete m_menu_pic;
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
Worker *Worker_Descr::create(Game *g, Player *owner, PlayerImmovable *location, Coords coords)
{
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

	snprintf(buf, sizeof(buf),	"%s_menu.bmp", get_name());
	string = s->get_string("menu_pic", buf);
	
	snprintf(fname, sizeof(fname), "%s/%s", directory, string);
	m_menu_pic = new Pic;
	m_menu_pic->load(fname);
	
	if (m_menu_pic->get_w() != WARE_MENU_PIC_W || m_menu_pic->get_h() != WARE_MENU_PIC_H)
		throw wexception("%s: menu pic must be %ix%i pixels.", fname, WARE_MENU_PIC_W, WARE_MENU_PIC_H);
	
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
	m_carried_ware = -1;
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
		assert(m_economy);
		
		oldlocation->remove_worker(this);
	}
	else
	{
		assert(!m_economy);
	}
	
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
			interrupt_task(get_owner()->get_game());
	
		set_economy(0);
	}
	
	m_location = location;
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
	
	throw wexception("TODO: implement change_job_request");
	// if (cancel) m_request = 0;
}

/*
===============
Worker::init

Initialize the worker
===============
*/
void Worker::init(Game *g)
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
void Worker::cleanup(Game *g)
{
	if (get_location(g))
		set_location(0);

	Bob::cleanup(g);
}

/*
===============
Worker::task_start_best

Give the worker something to do after the last task has finished.
===============
*/
void Worker::task_start_best(Game* g, uint prev, bool success, uint nexthint)
{
	switch(m_state) {
	case State_None:
		{
			log("Worker::task_start_best [State_None]\n");
		
			// this will be executed once before our creator gets the chance to 
			// assign a job; therefore, the 1ms grace idling
			if (prev == TASK_NONE)
				start_task_idle(g, get_descr()->get_idle_anim(), 1);
			else {
				assert(m_economy == 0); // this should only happen to "fugitives", if at all
			
				throw wexception("TODO: task_start_best, become fugitive");
			}
		}
		return;
	
	case State_Request:
		log("Worker::task_start_best [State_Request]\n");
		run_state_request(g, prev, success, nexthint);
		return;
	}
	
	throw wexception("Worker::task_start_best: unhandled");
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
			
			if (success)
				request->transfer_finish(g);
			else
				request->transfer_fail(g);
		}
		break;
	
	default:
		throw wexception("Worker::end_state: unhandled");
	}

	// 1ms idling to get task_start_best() called again for the new state
	start_task_idle(g, get_descr()->get_idle_anim(), 1);
}

/*
===============
Worker::run_state_request

Decide what to do in State_Request.
===============
*/
void Worker::run_state_request(Game *g, uint prev, bool success, uint nexthing)
{
	// if our previous task failed, reset to State_None
	if (!success) {
		log("Worker: State_Request: previous task failed\n");
		end_state(g, false);
		return;
	}

	PlayerImmovable *location = get_location(g);
	
	// We were waiting; now continue on the next step of the route
	if (prev == TASK_IDLE)
	{
		Flag *current = m_route->get_flag(g, 0);
		assert(current);
		
		// We may have to move out of a building
		if (current != location) {
			assert(location->get_type() == Map_Object::BUILDING);
			assert(location->get_base_flag() == current);

			log("Worker: State_Request: move from building to flag\n");
			
			Path path(g->get_map(), get_position());
			path.append(Map_Object::WALK_SE);

			start_task_movepath(g, path, get_descr()->get_walk_anims());
			return;
		}
		
		// No, move on to the next step if there is one
		if (m_route->get_nrsteps()) {
			Flag *dest = m_route->get_flag(g, 1);
			Road *road = current->get_road(dest);
			
			log("Worker: State_Request: move to next flag\n");
			
			if (!road)
				throw wexception("TODO: recalculate route");
			
			Path path(road->get_path());
			
			if (dest != road->get_flag_end())
				path.reverse();
			
			start_task_movepath(g, path, get_descr()->get_walk_anims());
			set_location(road);
			m_route->starttrim(1);
			return;
		}
		
		delete m_route;
		m_route = 0;
		
		// Finally, we may have to move toward the building
		PlayerImmovable *target = m_request->get_target(g);
		
		if (target->get_type() == Map_Object::BUILDING) {
			assert(target->get_base_flag() == current);
			
			log("Worker: State_Request: move from flag to building\n");
			
			Path path(g->get_map(), get_position());
			path.append(Map_Object::WALK_NW);
			
			start_task_movepath(g, path, get_descr()->get_walk_anims());
			set_location(target);
			return;
		}
		
		log("Worker: State_Request: finished (road/flag)\n");
			
		if (target->get_type() == Map_Object::ROAD)
			set_location(target);
		
		// we're done
		end_state(g, true);
		return;
	}
	
	// Check if we've arrived at the target
	if (location == m_request->get_target(g)) {
		log("Worker: State_Request: finished (building)\n");
			
		end_state(g, true);
		return;
	}
	
	// Wait a little bit on the flag
	log("Worker: State_Request: wait on flag\n");
			
	Flag *current = m_route->get_flag(g, 0);
	assert(get_position() == current->get_position());
	set_location(current);
	
	start_task_idle(g, get_descr()->get_idle_anim(), 50);
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
}

Carrier::~Carrier()
{
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
