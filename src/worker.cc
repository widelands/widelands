/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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


#include "carrier.h"
#include "cmd_incorporate.h"
#include "critter_bob.h"
#include "game.h"
#include "graphic.h"
#include "helper.h"
#include "interactive_player.h"
#include "player.h"
#include "profile.h"
#include "soldier.h"
#include "sound/sound_handler.h"
#include "tribe.h"
#include "warehouse.h"
#include "wexception.h"
#include "worker.h"
#include "worker_program.h"

#include "upcast.h"

namespace Widelands {

/**
 * createitem \<waretype\>
 *
 * The worker will create and carry an item of the given type.
 *
 * sparam1 = ware name
 */
bool Worker::run_createitem(Game* g, State* state, const Action* action)
{
	WareInstance* item;
	int32_t wareid;

	molog("  CreateItem(%s)\n", action->sparam1.c_str());

	item = fetch_carried_item(g);
	if (item) {
		molog("  Still carrying an item! Delete it.\n");
		item->schedule_destroy(g);
	}

	wareid = get_owner()->tribe().get_safe_ware_index(action->sparam1.c_str());
	item = new WareInstance(wareid, get_owner()->tribe().get_ware_descr(wareid));
	item->init(g);

	set_carried_item(g, item);

	// For statistics, inform the user that a ware was produced
	get_owner()->ware_produced(wareid);

	++state->ivar1;
	schedule_act(g, 10);
	return true;
}


/**
 * mine \<resource\> \<area\>
 *
 * Mine on the current coordinates (from walk or so) for resources decrease,
 * go home
 *
 * iparam1 = area
 * sparam1 = resource
 */
bool Worker::run_mine(Game* g, State* state, const Action* action)
{
   molog("  Mine(%s, %i)\n", action->sparam1.c_str(), action->iparam1);

	Map & map = *g->get_map();

	const Resource_Descr::Index res =
		map.get_world()->get_resource(action->sparam1.c_str());
	if (static_cast<int8_t>(res)==-1) //FIXME: ARGH!!
		throw wexception
			(" Worker::run_mine: Should mine resource %s, which "
			 "doesn't exist in world. Tribe is not compatible with "
			 "world!!\n",  action->sparam1.c_str());

	// Select one of the fields randomly
	uint32_t totalres = 0;
	uint32_t totalchance = 0;
	int32_t pick;
	MapRegion<Area<FCoords> > mr
		(map, Area<FCoords>(map.get_fcoords(get_position()), action->iparam1));
	do {
		uint8_t fres  = mr.location().field->get_resources();
		uint32_t amount = mr.location().field->get_resources_amount();

		// In the future, we might want to support amount = 0 for
		// fields that can produce an infinite amount of resources.
		// Rather -1 or something similar. not 0
		if (fres != res)
			amount = 0;

		totalres += amount;
		totalchance += 8 * amount;

		// Add penalty for fields that are running out
		if (amount == 0)
			// we already know it's completely empty, so punish is less
			totalchance += 1;
		else if (amount <= 2)
			totalchance += 6;
		else if (amount <= 4)
			totalchance += 4;
		else if (amount <= 6)
			totalchance += 2;
	} while (mr.advance(map));

	if (totalres == 0) {
		molog("  Run out of resources\n");
		return false;
	}

	// Second pass through fields
	pick = g->logic_rand() % totalchance;

	do {
		uint8_t fres  = mr.location().field->get_resources();
		uint32_t amount = mr.location().field->get_resources_amount();;

		if (fres != res)
			amount = 0;

		pick -= 8*amount;
		if (pick < 0) {
			assert(amount > 0);

			--amount;

			mr.location().field->set_resources(res, amount);
			break;
		}
	} while (mr.advance(map));

	if (pick >= 0) {
		molog("  Not successful this time\n");
		return false;
	}

	molog("  Mined one item\n");

	++state->ivar1;
	schedule_act(g, 10);
	return true;
}


/**
 * setdescription \<immovable name\> \<immovable name\> ...
 *
 * Randomly select an immovable name that can be used in subsequent commands
 * (e.g. plant).
 *
 * sparamv = possible bobs
 */
bool Worker::run_setdescription(Game* g, State* state, const Action* action)
{
	int32_t idx = g->logic_rand() % action->sparamv.size();

	molog("  SetDescription: %s\n", action->sparamv[idx].c_str());

	const std::vector<std::string> list(split_string(action->sparamv[idx], ":"));
	std::string bob;
	if (list.size()==1) {
		state->svar1 = "world";
		bob=list[0];
	} else {
		state->svar1 = "tribe";
		bob=list[1];
	}

	state->ivar2 =
		state->svar1 == "world" ?
		g->map().world().get_immovable_index(bob.c_str())
		:
		descr ().tribe().get_immovable_index(bob.c_str());

	if (state->ivar2 < 0) {
		molog("  WARNING: Unknown immovable %s\n", action->sparamv[idx].c_str());
		set_signal("fail");
		pop_task();
		return true;
	}

	++state->ivar1;
	schedule_act(g, 10);
	return true;
}


/**
 * setbobdescription \<bob name\> \<bob name\> ...
 *
 * Randomly select a bob name that can be used in subsequent commands
 * (e.g. create_bob).
 *
 * sparamv = possible bobs
 */
bool Worker::run_setbobdescription(Game* g, State* state, const Action* action)
{
	int32_t idx = g->logic_rand() % action->sparamv.size();

	molog("  SetBobDescription: %s\n", action->sparamv[idx].c_str());

	const std::vector<std::string> list(split_string(action->sparamv[idx], ":"));
	std::string bob;
	if (list.size()==1) {
		state->svar1 = "world";
		bob=list[0];
	} else {
		state->svar1 = "tribe";
		bob=list[1];
	}

	state->ivar2 =
		state->svar1 == "world" ?
		g->map().world().get_bob(bob.c_str())
		:
		descr ().tribe().get_bob(bob.c_str());

	if (state->ivar2 < 0) {
		molog("  WARNING: Unknown bob %s\n", action->sparamv[idx].c_str());
		set_signal("fail");
		pop_task();
		return true;
	}

	++state->ivar1;
	schedule_act(g, 10);
	return true;
}


/**
 * findobject key:value key:value ...
 *
 * Find and select an object based on a number of predicates.
 * The object can be used in other commands like walk or object.
 *
 * Predicates:
 * radius:\<dist\>
 * Find objects within the given radius
 *
 * attrib:\<attribute\>  (optional)
 * Find objects with the given attribute
 *
 * type:\<what\>         (optional, defaults to immovable)
 * Find only objects of this type
 *
 * iparam1 = radius predicate
 * iparam2 = attribute predicate (if >= 0)
 * sparam1 = type
 */
bool Worker::run_findobject(Game* g, State* state, const Action* action)
{
	molog
		("  FindObject(%i, %i, %s)\n",
		 action->iparam1, action->iparam2, action->sparam1.c_str());

	CheckStepWalkOn cstep(get_movecaps(), false);

	Map & map = g->map();
	const Area<FCoords> area (map.get_fcoords(get_position()), action->iparam1);
	if (action->sparam1 == "immovable") {
		std::vector<ImmovableFound> list;
		if (action->iparam2 < 0)
			map.find_reachable_immovables(area, &list, cstep);
		else
			map.find_reachable_immovables(area, &list, cstep,
			                           FindImmovableAttribute(action->iparam2));

		if (!list.size()) {
			set_signal("fail"); // no object found, cannot run program
			pop_task();
			return true;
		}

		int32_t sel = g->logic_rand() % list.size();
		state->objvar1 = list[sel].object;
		molog("  %i found\n", list.size());
	} else {
		std::vector<Bob*> list;
		log("BOB: searching bob with attribute (%i)\n", action->iparam2);
		if (action->iparam2 < 0)
			map.find_reachable_bobs(area, &list, cstep);
		else
			map.find_reachable_bobs(area, &list, cstep,
			                        FindBobAttribute(action->iparam2));

		if (!list.size()) {
			set_signal("fail"); // no object found, cannot run program
			pop_task();
			return true;
		}
		int32_t sel = g->logic_rand() % list.size();
		state->objvar1 = list[sel];
		molog("  %i found\n", list.size());
	}

	++state->ivar1;
	schedule_act(g, 10);
	return true;
}



/**
 * findspace key:value key:value ...
 *
 * Find a field based on a number of predicates.
 * The field can later be used in other commands, e.g. walk.
 *
 * Predicates:
 * radius:\<dist\>
 * Search for fields within the given radius around the worker.
 *
 * size:[any|build|small|medium|big|mine|port]
 * Search for fields with the given amount of space.
 *
 * resource:\<resname\>
 * Resource to search for. This is mainly intended for fisher and
 * therelike (non detectable Resources and default resources)
 *
 * iparam1 = radius
 * iparam2 = FindNodeSize::sizeXXX
 * sparam1 = Resource
 */
bool Worker::run_findspace(Game* g, State* state, const Action* action)
{
	std::vector<Coords> list;
	Map & map = g->map();
	World * const w = &map.world();

	CheckStepDefault cstep(get_movecaps());

	int32_t const res =
		action->sparam1.size() ? w->get_resource(action->sparam1.c_str()) : -1;

	Area<FCoords> area(map.get_fcoords(get_position()), action->iparam1);

	if
		(!
		 (res != -1 ?
		  map.find_reachable_fields
		  (area, &list, cstep,
		   FindNodeSizeResource
		   (static_cast<FindNodeSize::Size>(action->iparam2), res))
		  :
		  map.find_reachable_fields
		  (area, &list, cstep,
		   FindNodeSize
		   (static_cast<FindNodeSize::Size>(action->iparam2)))))
	{
		molog("  no space found\n");
		set_signal("fail");
		pop_task();
		return true;
	}

	// Pick a location at random
	int32_t sel = g->logic_rand() % list.size();

	state->coords = list[sel];

	molog("  selected %i, %i\n", state->coords.x, state->coords.y);

	++state->ivar1;
	schedule_act(g, 10);
	return true;
}


/**
 * walk \<where\>
 *
 * Walk to a previously selected destination. where can be one of:
 * object  walk to a previously found and selected object
 * coords  walk to a previously found and selected field/coordinate
 *
 * iparam1 = walkXXX
 */
bool Worker::run_walk(Game* g, State* state, const Action* action)
{
	BaseImmovable* imm = g->get_map()->get_immovable(get_position());
	Coords dest;
	bool forceonlast = false;
	int32_t max_steps = -1;

	Building & owner = dynamic_cast<Building &>(*get_location(g));

	molog("  Walk(%i)\n", action->iparam1);

	// First of all, make sure we're outside
	if (imm == &owner) {
		start_task_leavebuilding(g, false);
		return true;
	}

	// Determine the coords we need to walk towards
	switch (action->iparam1) {
	case Action::walkObject: {
		Map_Object* obj = state->objvar1.get(g);

		if (!obj) {
			molog("  object(nil)\n");
			set_signal("fail");
			pop_task();
			return true;
		}

		molog("  object(%u): type = %i\n", obj->get_serial(), obj->get_type());

		if      (upcast(Bob       const, bob,       obj))
			dest = bob      ->get_position();
		else if (upcast(Immovable const, immovable, obj))
			dest = immovable->get_position();
		else
			throw wexception
				("MO(%u): [actWalk]: bad object type = %i",
				 get_serial(), obj->get_type());

		max_steps=1; // Only take one step, then rethink (object may have moved)
		forceonlast = true;
		break;
	}
	case Action::walkCoords: {
		molog("  coords(%i, %i)\n", state->coords.x, state->coords.y);
		dest = state->coords;
		break;
	}
	default:
		throw wexception("MO(%u): [actWalk]: bad action->iparam1 = %i", get_serial(), action->iparam1);
	}

	// If we've already reached our destination, that's cool
	if (get_position() == dest) {
		molog("  reached\n");
		++state->ivar1;
		return false; // next instruction
	}

	// Walk towards it
	if (not start_task_movepath(g, dest, 10,
	                            descr().get_right_walk_anims(does_carry_ware()),
	                            forceonlast, max_steps))
	{
		molog("  couldn't find path\n");
		set_signal("fail");
		pop_task();
		return true;
	}

	return true;
}


/**
 * animation \<name\> \<duration\>
 *
 * Play the given animation for the given amount of time.
 *
 * iparam1 = anim id
 * iparam2 = duration
 */
bool Worker::run_animation(Game* g, State* state, const Action* action)
{
	set_animation(g, action->iparam1);

	++state->ivar1;
	schedule_act(g, action->iparam2);
	return true;
}



/**
 * Return home, drop any item we're carrying onto our building's flag.
 *
 * iparam1 = 0: don't drop item on flag, 1: do drop item on flag
 */
bool Worker::run_return(Game* g, State* state, const Action* action)
{
	molog("  Return(%i)\n", action->iparam1);

	++state->ivar1;
	start_task_return(g, action->iparam1);
	return true;
}


/**
 * object \<command\>
 *
 * Cause the currently selected object to execute the given program.
 *
 * sparam1 = object command name
 */
bool Worker::run_object(Game* g, State* state, const Action* action)
{
	Map_Object* obj;

	molog("  Object(%s)\n", action->sparam1.c_str());

	obj = state->objvar1.get(g);

	if (!obj) {
		molog("  object(nil)\n");
		set_signal("fail");
		pop_task();
		return true;
	}

	molog("  object(%u): type = %i\n", obj->get_serial(), obj->get_type());

	if      (upcast(Immovable, immovable, obj))
		immovable->switch_program(g, action->sparam1);
	else if (upcast(Bob,       bob,       obj)) {
		if        (upcast(Critter_Bob, crit, bob)) {
			crit->send_signal(g, "interrupt_now");
			crit->start_task_program(action->sparam1);
		} else if (upcast(Worker,      w,    bob)) {
			w   ->send_signal(g, "interrupt_now");
			w   ->start_task_program(action->sparam1);
		} else
			throw wexception
				("MO(%i): [actObject]: bab bob type = %i",
				 get_serial(), bob->get_bob_type());
	} else
		throw wexception
			("MO(%u): [actObject]: bad object type = %i",
			 get_serial(), obj->get_type());

	++state->ivar1;
	schedule_act(g, 10);
	return true;
}


/**
 * Plant an immovable on the current position. The immovable type must have
 * been selected by a previous command (i.e. setdescription)
 */
bool Worker::run_plant(Game * g, State * state, const Action *)
{
	Coords pos = get_position();

	molog("  Plant: %i at %i, %i\n", state->ivar2, pos.x, pos.y);

	// Check if the map is still free here
	BaseImmovable* imm = g->get_map()->get_immovable(pos);

	if (imm && imm->get_size() >= BaseImmovable::SMALL) {
		molog("  field no longer free\n");
		set_signal("fail");
		pop_task();
		return true;
	}

	g->create_immovable
		(pos, state->ivar2, state->svar1 == "world" ? 0 : &descr().tribe());

	++state->ivar1;
	schedule_act(g, 10);
	return true;
}


/**
 * Plants a bob (critter usually, maybe also worker later on). The immovable
 * type must have been selected by a previous command (i.e. setbobdescription).
 */
bool Worker::run_create_bob(Game * g, State * state, const Action *)
{
	Coords pos = get_position();

	molog("  Create Bob: %i at %i, %i\n", state->ivar2, pos.x, pos.y);

	g->create_bob
		(pos, state->ivar2, state->svar1 == "world" ? 0 : &descr().tribe());

	++state->ivar1;
	schedule_act(g, 10);
	return true;
}


/**
 * Simply remove the currently selected object - make no fuss about it.
 */
bool Worker::run_removeobject(Game * g, State * state, const Action *)
{
	Map_Object* obj;

	obj = state->objvar1.get(g);
	if (obj) {
		obj->remove(g);
		state->objvar1 = 0;
	}

	++state->ivar1;
	schedule_act(g, 10);
	return true;
}


/**
 * geologist \<repeat #\> \<radius\> \<subcommand\>
 *
 * Walk around the starting point randomly within a certain radius, and
 * execute the subcommand for some of the fields.
 *
 * iparam1 = maximum repeat #
 * iparam2 = radius
 * sparam1 = subcommand
 */
bool Worker::run_geologist(Game* g, State* state, const Action* action)
{
	dynamic_cast<const Flag &>(*get_location(g));

	molog("  Start Geologist (%i attempts, %i radius -> %s)\n", action->iparam1,
	      action->iparam2, action->sparam1.c_str());

	++state->ivar1;
	start_task_geologist(action->iparam1, action->iparam2, action->sparam1);
	return true;
}


/**
 * Check resources at the current position, and plant a marker object when
 * possible.
 */
bool Worker::run_geologist_find(Game * g, State * state, const Action *)
{
	const Map & map = g->map();
	const FCoords position = map.get_fcoords(get_position());
	BaseImmovable* imm = position.field->get_immovable();

	if (imm && imm->get_size() > BaseImmovable::NONE) {
		//NoLog("  Field is no longer empty\n");
	} else if
		(const Resource_Descr * const rdescr =
		 map.world().get_resource(position.field->get_resources()))
	{
		Tribe_Descr const & t = tribe();
		g->create_immovable
			(position,
			 t.get_resource_indicator
			 (rdescr,
			  rdescr->is_detectable() ?
			  position.field->get_resources_amount() : 0),
			 &t);
	}

	++state->ivar1;
	return false;
}


/**
 * Demand from the g_sound_handler to play a certain sound effect.
 * Whether the effect actually gets played is decided only by the sound server.
 */
bool Worker::run_playFX(Game* g, State* state, const Action* action)
{
	g_sound_handler.play_fx(action->sparam1, get_position(), action->iparam1);

	++state->ivar1;
	schedule_act(g, 10);
	return true;
}


Worker::Worker(const Worker_Descr & worker_descr):Bob          (worker_descr),
                                                  m_economy    (0),
                                                  m_supply     (0),
                                                  m_needed_exp (0),
                                                  m_current_exp(0)
{
}

Worker::~Worker()
{
	assert(!m_location.is_set());
}


/**
 * Log basic informations
 */
void Worker::log_general_info(Editor_Game_Base* egbase)
{
	Bob::log_general_info(egbase);

	PlayerImmovable* loc=static_cast<PlayerImmovable*>(m_location.get(egbase));

	molog("m_location: %p\n", loc);
	if (loc) {
		molog("* Owner: (%p)\n", loc->get_owner());
		molog("** Owner (plrnr): %i\n", loc->get_owner()->get_player_number());
		molog("* Economy: %p\n", loc->get_economy());
	}

	molog("Economy: %p\n", m_economy);

	WareInstance* ware=static_cast<WareInstance*>(m_carried_item.get(egbase));
	molog("m_carried_item: %p\n", ware);
	if (ware) {
		molog("* m_carried_item->get_ware() (id): %i\n", ware->descr_index());
		molog("* m_carried_item->get_economy() (): %p\n", ware->get_economy());
	}

	molog("m_needed_exp: %i\n", m_needed_exp);
	molog("m_current_exp: %i\n", m_current_exp);

	molog("m_supply: %p\n", m_supply);
}


uint32_t Worker::get_movecaps() const throw ()
{
	return MOVECAPS_WALK;
}


/**
 * Change the location. This should be called in the following situations:
 * \li worker creation (usually, location is a warehouse)
 * \li worker moves along a route (location is a road and finally building)
 * \li current location is destroyed (building burnt down etc...)
 */
void Worker::set_location(PlayerImmovable *location)
{
	PlayerImmovable *oldlocation = get_location(&get_owner()->egbase());

	if (oldlocation == location)
		return;

	if (oldlocation)
		// Note: even though we have an oldlocation, m_economy may be zero
		// (oldlocation got deleted)
		oldlocation->remove_worker(this);
	else
		assert(!m_economy);

	m_location = location;

	if (location) {
		Economy *eco = location->get_economy();

		if (!m_economy)
			set_economy(eco);
		else if (m_economy != eco)
			throw wexception("Worker::set_location changes economy");

		location->add_worker(this);
	} else {
		// Our location has been destroyed, we are now fugitives.
		// Interrupt whatever we've been doing.
		set_economy(0);

		send_signal(static_cast<Game *>(&get_owner()->egbase()),
		            "location");
	}
}


/**
 * Change the worker's current economy. This is called:
 * \li by set_location() when appropriate
 * \li by the current location, when the location's economy changes
 */
void Worker::set_economy(Economy *economy)
{
	if (economy == m_economy)
		return;

	if (m_economy)
		m_economy->remove_workers(descr().tribe().get_worker_index(name().c_str()), 1);

	m_economy = economy;

	if (WareInstance * const item = get_carried_item(&get_owner()->egbase()))
		item->set_economy(m_economy);
	if (m_supply)
		m_supply->set_economy(m_economy);

	if (m_economy)
		m_economy->add_workers(descr().tribe().get_worker_index(name().c_str()), 1);
}


/**
 * Initialize the worker
 */
void Worker::init(Editor_Game_Base *g)
{
	Bob::init(g);

	// a worker should always start out at a fixed location
	// (this assert is not longer true for save games. Where it lives
	// is unknown to this worker till he is initialized
	// assert(get_location(g));

	if (upcast(Game, game, g))
		create_needed_experience(game); // Set his experience
}


/**
 * Remove the worker.
 */
void Worker::cleanup(Editor_Game_Base *g)
{
	WareInstance* item = get_carried_item(g);

	delete m_supply;
	m_supply = 0;

	if (item)
		if (g->objects().object_still_available(item))
			item->destroy(g);

	// We are destroyed, but we were maybe idling
	// or doing something else. Get Location might
	// init a gowarehouse task or something and this results
	// in a dirty stack. Nono, we do not want to end like this
	if (upcast(Game, game, g))
		reset_tasks(game);

	if (get_location(g))
		set_location(0);

	assert(!get_economy());

	Bob::cleanup(g);
}


/**
 * Set the item we carry.
 * If we carry an item right now, it will be destroyed (see
 * fetch_carried_item()).
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


/**
 * Stop carrying the current item, and return a pointer to it.
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


/**
 * Schedule an immediate CMD_INCORPORATE, which will integrate this worker into
 * the warehouse he is standing on.
 */
void Worker::schedule_incorporate(Game* g)
{
	g->get_cmdqueue()->enqueue (new Cmd_Incorporate(g->get_gametime(), this));
	force_skip_act();
}


/**
 * Incorporate the worker into the warehouse it's standing on immediately.
 * This will delete the worker.
 */
void Worker::incorporate(Game *g)
{
	if (upcast(Warehouse, wh, get_location(g))) {
		wh->incorporate_worker(g, this);
		return;
	}

	// our location has been deleted from under us
	send_signal(g, "fail");
}


/**
 * Calculate needed experience.
 *
 * This sets the needed experience on a value between max and min
 */
void Worker::create_needed_experience(Game* g)
{
	if (descr().get_min_exp() == -1 && descr().get_max_exp() == -1) {
		m_needed_exp = m_current_exp = -1;
		return;
	}

	int32_t range = descr().get_max_exp() - descr().get_min_exp();
	int32_t value = g->logic_rand() % range;
	m_needed_exp = value + descr().get_min_exp();
	m_current_exp = 0;
}


/**
 * Gain experience
 *
 * This function increases the experience
 * of the worker by one, if he reaches
 * needed_experience he levels
 */
void Worker::gain_experience(Game* g)
{
	if (m_needed_exp == -1)
		return; // This worker can not level

	++m_current_exp;

	if (m_current_exp == m_needed_exp)
		level(g);
}


/**
 * Level this worker to the next higher level. this includes creating a
 * new worker with his propertys and removing this worker
 */
void Worker::level(Game* g)
{

	// We do not really remove this worker, all we do
	// is to overwrite his description with the new one and to
	// reset his needed experience. Congratulations to promotion!
	// This silently expects that the new worker is the same type as the old
	// worker and can fullfill the same jobs (which should be given in all
	// circumstances)
	assert(get_becomes());
	const Tribe_Descr & t = tribe();

	// Inform the economy, that something has changed
	m_economy->remove_workers   (t.get_worker_index(descr().name().c_str()), 1);
	m_descr = t.get_worker_descr(t.get_worker_index(get_becomes()));
	m_economy->add_workers      (t.get_worker_index(descr().name().c_str()), 1);

	create_needed_experience(g);
}


/**
 * Set a fallback task.
 */
void Worker::init_auto_task(Game* g)
{
	PlayerImmovable* location = get_location(g);

	if (location) {
		if (get_economy()->get_nr_warehouses()) {
			molog("init_auto_task: go warehouse\n");

			start_task_gowarehouse();
			return;
		}

		set_location(0);
	}

	molog("init_auto_task: become fugitive\n");

	start_task_fugitive(g);
}


/**
 * Follow the given transfer.
 *
 * Signal "cancel" to cancel the transfer.
 */
Bob::Task Worker::taskTransfer = {
	"transfer",
	static_cast<Bob::Ptr>(&Worker::transfer_update),
	static_cast<Bob::Ptr>(&Worker::transfer_signal),
	static_cast<Bob::Ptr>(&Worker::transfer_mask),
};


/**
 * Tell the worker to follow the Transfer
 */
void Worker::start_task_transfer(Game* g, Transfer* t)
{
	State* state;

	// hackish override for gowarehouse
	state = get_state(&taskGowarehouse);
	if (state) {
		assert(!state->transfer);

		state->transfer = t;
		send_signal(g, "transfer");
		return;
	}

	// just start a normal transfer
	push_task(taskTransfer);

	state = get_state();
	state->transfer = t;
}


void Worker::transfer_update(Game* g, State* state)
{
	PlayerImmovable* location = get_location(g);

	assert(location); // 'location' signal expected otherwise

	// The request is no longer valid, the task has failed
	if (!state->transfer) {
		molog("[transfer]: Fail (without transfer)\n");

		set_signal("fail");
		pop_task();
		return;
	}

	// If our location is a building, make sure we're actually in it.
	// If we're a building's worker, and we've just been released from
	// the building, we may be somewhere else entirely (e.g. lumberjack, soldier)
	// or we may be on the building's flag for a fetch_from_flag or dropoff
	// task.
	if (dynamic_cast<Building const *>(location)) {
		BaseImmovable* position = g->get_map()->get_immovable(get_position());

		if (position != location) {
			if (upcast(Flag, flag, position)) {
				location = flag;
				set_location(flag);
			} else {
				set_location(0);
				return;
			}
		}
	}

	// Figure out where to go
	bool success;
	PlayerImmovable* nextstep =
		state->transfer->get_next_step(location, &success);

	if (!nextstep) {
		Transfer* t = state->transfer;

		state->transfer = 0;

		if (success) {
			molog("[transfer]: Success\n");
			pop_task();

			t->has_finished();
			return;
		} else {
			molog("[transfer]: Failed\n");
			set_signal("fail");
			pop_task();

			t->has_failed();
			return;
		}
	}

	// Initiate the next step
	if (upcast(Building, building, location)) {
		if (building->get_base_flag() != nextstep)
			throw wexception
				("MO(%u): [transfer]: in building, nextstep is not building's "
				 "flag",
				 get_serial());

		molog("[transfer]: move from building to flag\n");
		start_task_leavebuilding(g, true);
		return;
	}

	if (upcast(Flag, flag, location)) {
		// Flag to Building
		if (upcast(Building, building, nextstep)) {
			if (building->get_base_flag() != location)
				throw wexception
					("MO(%u): [transfer]: next step is building, but we are "
					 "nowhere near",
					 get_serial());

			molog("[transfer]: move from flag to building\n");
			start_task_forcemove
				(WALK_NW, descr().get_right_walk_anims(does_carry_ware()));
			set_location(building);
			return;
		}

		// Flag to Flag
		if (upcast(Flag, nextflag, nextstep)) {
			Road & road = *flag->get_road(nextflag);

			molog
				("[transfer]: move to next flag via road %u\n", road.get_serial());

			Path path(road.get_path());

			if (nextstep != road.get_flag(Road::FlagEnd))
				path.reverse();

			start_task_movepath
				(path, descr().get_right_walk_anims(does_carry_ware()));
			set_location(&road);
			return;
		}

		// Flag to Road
		if (upcast(Road, road, nextstep)) {
			if
				(road->get_flag(Road::FlagStart) != location
				 and
				 road->get_flag(Road::FlagEnd)   != location)
				throw wexception
					("MO(%u): [transfer]: nextstep is road, but we are nowhere near",
					 get_serial());

			molog("[transfer]: set location to road %u\n", road->get_serial());
			set_location(road);
			set_animation(g, descr().get_animation("idle"));
			schedule_act(g, 10); // wait a little
			return;
		}

		throw wexception("MO(%u): [transfer]: flag to bad nextstep %u",
		                 get_serial(), nextstep->get_serial());
	}

	if (upcast(Road, road, location)) {
		// Road to Flag
		if (nextstep->get_type() == FLAG) {
			const Path& path = road->get_path();
			int32_t index;

			if (nextstep == road->get_flag(Road::FlagStart))
				index = 0;
			else if (nextstep == road->get_flag(Road::FlagEnd))
				index = path.get_nsteps();
			else
				index = -1;

			molog("[transfer]: on road %u, to flag %u, index is %i\n",
			      road->get_serial(), nextstep->get_serial(), index);

			if (index >= 0) {
				if
					(start_task_movepath
					 (g->map(),
					  path,
					  index,
					  descr().get_right_walk_anims(does_carry_ware())))
				{
					molog("[transfer]: from road %u to flag %u nextstep %u\n",
					      get_serial(), road->get_serial(),
					      nextstep->get_serial());
					return;
				}
			} else
				if (nextstep != g->get_map()->get_immovable(get_position()))
					throw wexception("MO(%u): [transfer]: road to flag, but "
					                 "flag is nowhere near", get_serial());

			molog("[transfer]: arrive at flag %u\n", nextstep->get_serial());
			set_location(dynamic_cast<Flag *>(nextstep));
			set_animation(g, descr().get_animation("idle"));
			schedule_act(g, 10); // wait a little
			return;
		}

		throw wexception
			("MO(%u): [transfer]: from road to bad nextstep %u (type %u)",
			 get_serial(), nextstep->get_serial(), nextstep->get_type());
	}

	throw wexception
		("MO(%u): location %u has bad type %u",
		 get_serial(), location->get_serial(), location->get_type());
}


void Worker::transfer_signal(Game *, State *)
{
	std::string signal = get_signal();

	// The caller requested a route update, or the previously calulcated route
	// failed.
	// We will recalculate the route on the next update().
	if (signal == "road" || signal == "fail") {
		molog("[transfer]: Got signal '%s' -> recalculate\n", signal.c_str());

		set_signal("");
		return;
	}

	molog("[transfer]: Cancel due to signal '%s'\n", signal.c_str());
	pop_task();
}


void Worker::transfer_mask(Game *, State * state)
{
	std::string signal = get_signal();

	if (signal == "cancel")
		state->transfer = 0; // dont't call transfer_fail/finish when cancelled
}


/**
 * Called by transport code when the transfer has been cancelled & destroyed.
 */
void Worker::cancel_task_transfer(Game* g)
{
	send_signal(g, "cancel");
}


/**
 * Endless loop, in which the worker calls the owning building's
 * get_building_work() function to intiate subtasks.
 * The signal "update" is used to wake the worker up after a sleeping time
 * (initiated by a false return value from get_building_work()).
 *
 * ivar1 - 0: no task has failed; 1: currently in buildingwork;
 *         2: signal failure of buildingwork
 */
Bob::Task Worker::taskBuildingwork = {
	"buildingwork",
	static_cast<Bob::Ptr>(&Worker::buildingwork_update),
	static_cast<Bob::Ptr>(&Worker::buildingwork_signal),
	0
};


/**
 * Begin work at a building.
 */
void Worker::start_task_buildingwork()
{
	push_task(taskBuildingwork);
	top_state().ivar1 = 0;
}


void Worker::buildingwork_update(Game* g, State* state)
{
	std::string signal = get_signal();

	if (signal == "location") {
		pop_task();
		return;
	}

	// Reset any other signals

	set_signal("");

	if (state->ivar1 == 1)
	{
		if (signal == "fail")
			state->ivar1 = 2;
		else
			state->ivar1 = 0;
	}

	// Return to building, if necessary
	Building & building = dynamic_cast<Building &>(*get_location(g));

	if (g->map().get_immovable(get_position()) != &building) {
		molog("[buildingwork]: Something went wrong, return home.\n");

		start_task_return(g, false); // don't drop item
		return;
	}

	// Get the new job
	bool success = state->ivar1 != 2;

	// Set this *before* calling to get_building_work, because the
	// state pointer might become invalid
	state->ivar1 = 1;

	if (not building.get_building_work(g, this, success)) {
		set_animation(g, 0);
		skip_act();
	}
}


void Worker::buildingwork_signal(Game * g, State *)
{
	std::string signal = get_signal();

	if (signal == "update")
		set_signal("");

	schedule_act(g, 1);
}


/**
 * Wake up the buildingwork task if it was sleeping.
 * Otherwise, the buildingwork task will update as soon as the previous task
 * is finished.
 */
void Worker::update_task_buildingwork(Game* g)
{
	State* state = get_state();

	if (state->task == &taskBuildingwork)
		send_signal(g, "update");
}


/**
 * Return to our owning building.
 * If dropitem (ivar1) is true, we'll drop our carried item (if any) on the
 * building's flag, if possible.
 * Blocks all signals except for "location".
 */
Bob::Task Worker::taskReturn = {
	"return",
	static_cast<Bob::Ptr>(&Worker::return_update),
	static_cast<Bob::Ptr>(&Worker::return_signal),
	0,
};


/**
 * Return to our owning building.
 */
void Worker::start_task_return(Game* g, bool dropitem)
{
	PlayerImmovable* location = get_location(g);

	molog("start_task_return\n");

	if (!location || location->get_type() != BUILDING)
		throw wexception("MO(%u): start_task_return(): not owned by building", get_serial());

	push_task(taskReturn);

	molog("pushed task\n");

	get_state()->ivar1 = dropitem ? 1 : 0;

	molog("done\n");
}


void Worker::return_update(Game* g, State* state)
{
	Building & location = dynamic_cast<Building &>(*get_location(g));
	if (BaseImmovable * const pos = g->map().get_immovable(get_position())) {
		if (pos == &location) {
			set_animation(g, 0);
			pop_task();
			return;
		}

		if (upcast(Flag, flag, pos)) {
			// Is this "our" flag?
			if (flag->get_building() == &location) {
				if (state->ivar1 && flag->has_capacity()) {
					WareInstance* item = fetch_carried_item(g);

					if (item) {
						molog("[return]: Drop item on flag\n");

						flag->add_item(g, item);

						set_animation(g, descr().get_animation("idle"));
						schedule_act(g, 20); // rest a while
						return;
					}
				}

				molog("[return]: Move back into building\n");

				start_task_forcemove
					(WALK_NW, descr().get_right_walk_anims(does_carry_ware()));
				return;
			}
		}
	}

	// Determine the building's flag and move to it

	molog("[return]: Move to building's flag\n");

	if
		(not
		 start_task_movepath
		 (g,
		  location.get_base_flag()->get_position(),
		  15,
		  descr().get_right_walk_anims(does_carry_ware())))
	{
		molog("[return]: Failed to return\n");

		set_location(0);
		return;
	}
}


void Worker::return_signal(Game *, State *)
{
	std::string signal = get_signal();

	if (signal == "location") {
		molog("[return]: Interrupted by signal '%s'\n", signal.c_str());
		pop_task();
		return;
	}

	molog("[return]: Blocking signal '%s'\n", signal.c_str());
	set_signal("");
}


/**
 * Follow the steps of a configuration-defined program.
 * ivar1 is the next action to be performed.
 * ivar2 is used to store description indices selected by setdescription
 * objvar1 is used to store objects found by findobject
 * coords is used to store target coordinates found by findspace
 */
Bob::Task Worker::taskProgram = {
	"program",
	static_cast<Bob::Ptr>(&Worker::program_update),
	static_cast<Bob::Ptr>(&Worker::program_signal),
	0
};


/**
 * Start the given program.
 */
void Worker::start_task_program(const std::string & programname)
{
	push_task(taskProgram);
	State & state = top_state();
	state.program = descr().get_program(programname);
	state.ivar1 = 0;
}


void Worker::program_update(Game* g, State* state)
{
	const Action* action;

	for (;;) {
		const WorkerProgram* program =
			static_cast<const WorkerProgram*>(state->program);

		if (state->ivar1 >= program->get_size()) {
			molog("  End of program\n");
			pop_task();
			return;
		}

		action = program->get_action(state->ivar1);

		if ((this->*(action->function))(g, state, action))
			return;
	}
}


void Worker::program_signal(Game *, State *)
{
	molog("[program]: Interrupted by signal '%s'\n", get_signal().c_str());
	pop_task();
}


Bob::Task Worker::taskGowarehouse = {
	"gowarehouse",
	static_cast<Bob::Ptr>(&Worker::gowarehouse_update),
	static_cast<Bob::Ptr>(&Worker::gowarehouse_signal),
	0,
};


/**
 * Get the worker to move to the nearest warehouse.
 * The worker is added to the list of usable wares, so he may be reassigned to
 * a new task immediately.
 */
void Worker::start_task_gowarehouse()
{
	assert(!m_supply);

	push_task(taskGowarehouse);

	m_supply = new IdleWorkerSupply(this);
}


void Worker::gowarehouse_update(Game* g, State* state)
{
	PlayerImmovable *location = get_location(g);

	assert(location); // 'location' signal expected otherwise

	if
		(dynamic_cast<const Building *>(location)
		 and
		 location->has_attribute(WAREHOUSE))
	{

		molog("[gowarehouse]: Back in warehouse, schedule incorporate\n");

		delete m_supply;
		m_supply = 0;

		schedule_incorporate(g);
		return;
	}

	// If we got a transfer, use it
	if (state->transfer) {
		Transfer* t = state->transfer;

		molog("[gowarehouse]: Got a Transfer\n");

		state->transfer = 0;
		delete m_supply;
		m_supply = 0;

		pop_task();
		start_task_transfer(g, t);
		return;
	}

	if (!get_economy()->get_nr_warehouses()) {
		molog("[gowarehouse]: No warehouse left in Economy\n");

		delete m_supply;
		m_supply = 0;

		pop_task();
		return;
	}

	// Idle until we are assigned a transfer.
	// The delay length is rather arbitrary, but we need some kind of
	// check against disappearing warehouses, or the worker will just
	// idle on a flag until the end of days (actually, until either the
	// flag is removed or a warehouse connects to the Economy).
	molog("[gowarehouse]: Idle\n");

	start_task_idle(g, get_animation("idle"), 1000);
}


void Worker::gowarehouse_signal(Game * g, State *)
{
	std::string signal = get_signal();

	// if routing has failed, try a different warehouse/route on next update()
	if (signal == "fail") {
		molog("[gowarehouse]: caught 'fail'\n");

		set_signal("");
		return;
	}

	if (signal == "transfer") {
		molog("[gowarehouse]: transfer signal\n");

		delete m_supply;
		m_supply = 0;

		schedule_act(g, 1);
		set_signal("");
		return;
	}

	molog("[gowarehouse]: cancel for signal '%s'\n", signal.c_str());

	delete m_supply;
	m_supply = 0;
	pop_task();
}


Bob::Task Worker::taskDropoff = {
	"dropoff",
	static_cast<Bob::Ptr>(&Worker::dropoff_update),
	0,
	0,
};

/**
 * Walk to the building's flag, drop the given item, and walk back inside.
 */
void Worker::start_task_dropoff(Game* g, WareInstance* item)
{
	assert(item);

	set_carried_item(g, item);

	push_task(taskDropoff);
}


void Worker::dropoff_update(Game * g, State *)
{
	std::string signal = get_signal();

	if (signal.size()) {
		molog("[dropoff]: Interrupted by signal '%s'\n", signal.c_str());
		pop_task();
		return;
	}

	WareInstance* item = get_carried_item(g);
	BaseImmovable* location = g->get_map()->get_immovable(get_position());
#ifndef NDEBUG
	Building & ploc = dynamic_cast<Building &>(*get_location(g));
	assert(&ploc == location || ploc.get_base_flag() == location);
#endif

	// Deliver the item
	if (item) {
		// We're in the building, walk onto the flag
		if (upcast(Building, building, location)) {
			Flag * const flag = building->get_base_flag();

			if (start_task_waitforcapacity(g, flag))
				return;

			// Exit throttle
			molog("[dropoff]: move from building to flag\n");
			start_task_leavebuilding(g, false);
			return;
		}

		// We're on the flag, drop the item and pause a little
		if (upcast(Flag, flag, location)) {
			if (flag->has_capacity()) {
				molog("[dropoff]: dropping the item\n");

				item = fetch_carried_item(g);
				flag->add_item(g, item);

				set_animation(g, descr().get_animation("idle"));
				schedule_act(g, 50);
				return;
			}

			molog("[dropoff]: flag is overloaded\n");
			start_task_forcemove
				(WALK_NW, descr().get_right_walk_anims(does_carry_ware()));
			return;
		}

		throw wexception("MO(%u): [dropoff]: not on building or on flag - fishy", get_serial());
	}

	// We don't have the item any more, return home
	if (location->get_type() == Map_Object::FLAG) {
		start_task_forcemove
			(WALK_NW, descr().get_right_walk_anims(does_carry_ware()));
		return;
	}

	if (location->get_type() != Map_Object::BUILDING)
		throw wexception("MO(%u): [dropoff]: not on building on return", get_serial());

	if (location->has_attribute(WAREHOUSE)) {
		schedule_incorporate(g);
		return;
	}

	// Our parent task should know what to do
	molog("[dropoff]: back in building\n");
	pop_task();
}


/**
 * ivar1 is set to 0 if we should move to the flag and fetch the item, and it
 * is set to 1 if we should move into the building.
 */
Bob::Task Worker::taskFetchfromflag = {
	"fetchfromflag",
	static_cast<Bob::Ptr>(&Worker::fetchfromflag_update),
	0,
	0,
};


/**
 * Walk to the building's flag, fetch an item from the flag that is destined for
 * the building, and walk back inside.
 */
void Worker::start_task_fetchfromflag()
{
	push_task(taskFetchfromflag);
	top_state().ivar1 = 0;
}


void Worker::fetchfromflag_update(Game *g, State* state)
{
	PlayerImmovable & owner = *get_location(g);
	PlayerImmovable * const location =
		dynamic_cast<PlayerImmovable *>(g->map().get_immovable(get_position()));

	// If we haven't got the item yet, walk onto the flag
	if (!get_carried_item(g) && !state->ivar1) {
		if (dynamic_cast<Building const *>(location)) {
			molog("[fetchfromflag]: move from building to flag\n");
			start_task_leavebuilding(g, false);
			return;
		}

		state->ivar1 = 1; // force return to building

		// The item has decided that it doesn't want to go to us after all
		// In order to return to the warehouse, we're switching to State_DropOff
		if
			(WareInstance * const item =
			 dynamic_cast<Flag &>(*location).fetch_pending_item(g, &owner))
			set_carried_item(g, item);

		set_animation(g, descr().get_animation("idle"));
		schedule_act(g, 20);
		return;
	}

	// Go back into the building
	if (dynamic_cast<Flag const *>(location)) {
		molog("[fetchfromflag]: return to building\n");

		start_task_forcemove
			(WALK_NW, descr().get_right_walk_anims(does_carry_ware()));
		return;
	}

	if (not dynamic_cast<Building const *>(location))
		throw wexception("MO(%u): [fetchfromflag]: building disappeared", get_serial());

	assert(location == &owner);

	molog("[fetchfromflag]: back home\n");

	if (WareInstance * const item = fetch_carried_item(g)) {
		item->set_location(g, location);
		item->update(g); // this might remove the item and ack any requests
	}

	// We're back!
	if (dynamic_cast<Warehouse const *>(location)) {
		schedule_incorporate(g);
		return;
	}

	pop_task(); // assume our parent task knows what to do
}


/**
 * Wait for available capacity on a flag.
 */
Bob::Task Worker::taskWaitforcapacity = {
	"waitforcapacity",
	static_cast<Bob::Ptr>(&Worker::waitforcapacity_update),
	static_cast<Bob::Ptr>(&Worker::waitforcapacity_signal),
	0,
};

/**
 * Checks the capacity of the flag.
 *
 * If there is none, a wait task is pushed, and the worker is added to the
 * flag's wait queue. The function returns true in this case.
 * If the flag still has capacity, the function returns false and doesn't
 * act at all.
 */
bool Worker::start_task_waitforcapacity(Game* g, Flag* flag)
{
	if (flag->has_capacity())
		return false;

	push_task(taskWaitforcapacity);

	get_state()->objvar1 = flag;

	flag->wait_for_capacity(g, this);

	return true;
}


void Worker::waitforcapacity_update(Game *, State *)
{
	skip_act(); // wait indefinitely
}


void Worker::waitforcapacity_signal(Game *, State *)
{
	// The 'wakeup' signal is to be expected; don't propagate it
	if (get_signal() == "wakeup")
		set_signal("");

	pop_task();
}


/**
 * Called when the flag we waited on has now got capacity left.
 * Return true if we actually woke up due to this.
 */
bool Worker::wakeup_flag_capacity(Game* g, Flag* flag)
{
	State* state = get_state();

	if (state && state->task == &taskWaitforcapacity) {
		molog("[waitforcapacity]: Wake up: flag capacity.\n");

		if (state->objvar1.get(g) != flag)
			throw wexception("MO(%u): wakeup_flag_capacity: Flags don't match.",
			                 get_serial());

		send_signal(g, "wakeup");
		return true;
	}

	return false;
}


/**
 * ivar1 - 0: don't change location; 1: change location to the flag
 * objvar1 - the building we're leaving
 */
Bob::Task Worker::taskLeavebuilding = {
	"leavebuilding",
	static_cast<Bob::Ptr>(&Worker::leavebuilding_update),
	static_cast<Bob::Ptr>(&Worker::leavebuilding_signal),
	0,
};


/**
 * Leave the current building.
 * Waits on the buildings leave wait queue if necessary.
 *
 * If changelocation is true, change the location to the flag once we're
 * outside.
 */
void Worker::start_task_leavebuilding(Game* g, bool changelocation)
{
	Building & building = dynamic_cast<Building &>(*get_location(g));

	// Set the wait task
	push_task(taskLeavebuilding);

	get_state()->ivar1 = changelocation ? 1 : 0;
	get_state()->objvar1 = &building;
}


void Worker::leavebuilding_update(Game* g, State* state)
{
	if (upcast(Building, building, g->map().get_immovable(get_position()))) {
	assert(building == state->objvar1.get(g));

	if (!building->leave_check_and_wait(g, this)) {
		molog("[leavebuilding]: Wait\n");
		skip_act();
		return;
	}

		molog
			("[leavebuilding]: Leave (%s location)\n",
			 state->ivar1 ? "change" : "stay in");

	if (state->ivar1)
		set_location(building->get_base_flag());

		start_task_forcemove
			(WALK_SE, descr().get_right_walk_anims(does_carry_ware()));
	} else
		pop_task();
}


void Worker::leavebuilding_signal(Game * g, State *)
{
	std::string signal = get_signal();

	if (signal == "wakeup") {
		molog("[leavebuilding]: Wake up\n");
		set_signal("");
		schedule_act(g, 1);
		return;
	}

	molog("[leavebuilding]: Interrupted by signal '%s'\n", signal.c_str());
	pop_task();
}


/**
 * Called when the given building allows us to leave it.
 * \return true if we actually woke up due to this.
 */
bool Worker::wakeup_leave_building(Game* g, Building* building)
{
	State* state = get_state();

	molog("wakeup_leave_building called\n");

	if (state && state->task == &taskLeavebuilding) {
		molog("[leavebuilding]: wakeup\n");

		if (state->objvar1.get(g) != building)
			throw wexception("MO(%u): [waitleavebuilding]: buildings don't match", get_serial());

		send_signal(g, "wakeup");
		return true;
	}

	return false;
}



/**
 * Run around aimlessly until we find a warehouse.
 */
Bob::Task Worker::taskFugitive = {
	"fugitive",
	static_cast<Bob::Ptr>(&Worker::fugitive_update),
	static_cast<Bob::Ptr>(&Worker::fugitive_signal),
	0,
};


void Worker::start_task_fugitive(Game* g)
{
	push_task(taskFugitive);

	// Fugitives survive for two to four minutes
	get_state()->ivar1 = g->get_gametime() + 120000 + 200*(g->logic_rand() % 600);
}


void Worker::fugitive_update(Game* g, State* state)
{
	Map *map = g->get_map();
	PlayerImmovable *location = get_location(g);

	if (location && location->get_owner() == get_owner()) {
		molog("[fugitive]: we're on location\n");

		if (location->has_attribute(WAREHOUSE)) {
			schedule_incorporate(g);
			return;
		}

		set_location(0);
		location = 0;
	}

	// check whether we're on a flag and it's time to return home
	if (upcast(Flag, flag, map->get_immovable(get_position())))
		if (upcast(Warehouse, warehouse, flag->get_building())) {
			if (warehouse->get_owner() == get_owner()) {
			molog("[fugitive]: move into warehouse\n");
			start_task_forcemove
				(WALK_NW, descr().get_right_walk_anims(does_carry_ware()));
				set_location(warehouse);
			return;
			}
		} else if
			(flag->get_owner() == get_owner()
			 and
			 flag->economy().get_nr_warehouses())
		{
			set_location(flag);
			pop_task();
			start_task_gowarehouse();
			return;
		}

	//  try to find a flag connected to a warehouse that we can return to
	std::vector<ImmovableFound> flags;
	struct FindFlagWithPlayersWarehouse : public FindImmovable {
		FindFlagWithPlayersWarehouse(Player const & owner) : m_owner(owner) {}
		bool accept(BaseImmovable * const imm) const {
			if (upcast(Flag const, flag, imm))
				if (flag->get_owner() == &m_owner)
					if(flag->economy().get_nr_warehouses())
						return true;
			return false;
		}
	private:
		Player const & m_owner;
	};
	if
		(map->find_immovables
		 (Area<FCoords>(map->get_fcoords(get_position()), vision_range()),
		  &flags, FindFlagWithPlayersWarehouse(*get_owner())))
	{
		int32_t bestdist = -1;
		Flag *  best     =  0;

		molog("[fugitive]: found a flag connected to warehouse(s)\n");

		std::vector<ImmovableFound>::const_iterator flags_end = flags.end();
		for
			(std::vector<ImmovableFound>::const_iterator it = flags.begin();
			 it != flags_end;
			 ++it)
		{
			Flag & flag = dynamic_cast<Flag &>(*it->object);

			int32_t const dist = map->calc_distance(get_position(), it->coords);

			if (!best || dist < bestdist) {
				best = &flag;
				bestdist = dist;
			}
		}

		if (best) {
			if (static_cast<int32_t>((g->logic_rand() % 30)) <= (30 - bestdist)) {
				molog("[fugitive]: try to move to flag\n");

				// the warehouse could be on a different island, so check for failure
				if
					(start_task_movepath
					 (g,
					  best->get_position(),
					  0,
					  descr().get_right_walk_anims(does_carry_ware())))

					return;
			}
		}
	}

	if (state->ivar1 < g->get_gametime()) {//  time to die?
		molog("[fugitive]: die\n");
		schedule_destroy(g);
		return;
	}

	molog("[fugitive]: wander randomly\n");

	if
		(start_task_movepath
		 (g,
		  g->random_location(get_position(), vision_range()),
		  4,
		  descr().get_right_walk_anims(does_carry_ware())))
		return;

	start_task_idle(g, descr().get_animation("idle"), 50);
}


void Worker::fugitive_signal(Game *, State *)
{
	molog("[fugitive]: interrupted by signal '%s'\n", get_signal().c_str());
	pop_task();
}


/**
 * Walk in a circle around our owner, calling a subprogram on currently
 * empty fields.
 *
 * ivar1 - number of attempts
 * ivar2 - radius to search
 * svar1 - name of subcommand
 *
 * Failure of path movement is caught, all other signals terminate this task.
 */
Bob::Task Worker::taskGeologist = {
	"geologist",
	static_cast<Bob::Ptr>(&Worker::geologist_update),
	0,
	0,
};


void Worker::start_task_geologist(const int32_t attempts, const int32_t radius,
                                  const std::string & subcommand)
{
	push_task(taskGeologist);
	State & state = top_state();
	state.ivar1   = attempts;
	state.ivar2   = radius;
	state.svar1   = subcommand;
}


void Worker::geologist_update(Game* g, State* state)
{
	std::string signal = get_signal();

	if (signal == "fail") {
		molog("[geologist]: Caught signal '%s'\n", signal.c_str());
		set_signal("");
	} else if (signal.size()) {
		molog("[geologist]: Interrupted by signal '%s'\n", signal.c_str());
		pop_task();
		return;
	}

	//
	Map & map = g->map();
	const World & world = map.world();
	Flag & owner = dynamic_cast<Flag &>(*get_location(g));
	Area<FCoords> owner_area
		(map.get_fcoords(owner.get_position()), state->ivar2);

	// Check if it's time to go home
	if (state->ivar1 > 0) {
		// Check to see if we're on suitable terrain
		BaseImmovable * const imm = map.get_immovable(get_position());

		if
			(not imm
			 or
			 (imm->get_size() == BaseImmovable::NONE
			  and
			  not imm->has_attribute(RESI)))
		{
			molog("[geologist]: Starting program '%s'\n", state->svar1.c_str());

			--state->ivar1;
			start_task_program(state->svar1);
			return;
		}

		// Find a suitable field and walk towards it
		std::vector<Coords> list;
		CheckStepDefault cstep(get_movecaps());
		FindNodeImmovableSize ffis(FindNodeImmovableSize::sizeNone);
		FindNodeImmovableAttribute ffia(RESI);
		FindNodeAnd ffa;

		ffa.add(&ffis, false);
		ffa.add(&ffia, true);

		if (map.find_reachable_fields(owner_area, &list, cstep, ffa)) {
			FCoords target;

			// is center a mountain piece?
			bool is_center_mountain =
				world.terrain_descr(owner_area.field->terrain_d()).get_is()
				&
				TERRAIN_MOUNTAIN
				|
				world.terrain_descr(owner_area.field->terrain_r()).get_is()
				&
				TERRAIN_MOUNTAIN;
			// Only run towards fields that are on a mountain (or not)
			// depending on position of center
			bool is_target_mountain;
			uint32_t n=list.size();
			uint32_t i=g->logic_rand() % list.size();
			do {
				molog("[geologist] Searching for a suitable field!\n");
				target =
					map.get_fcoords(list[g->logic_rand() % list.size()]);
				is_target_mountain =
					world.terrain_descr(target.field->terrain_d()).get_is()
					&
					TERRAIN_MOUNTAIN
					|
					world.terrain_descr(target.field->terrain_r()).get_is()
					&
					TERRAIN_MOUNTAIN;
				if (i==0) i=list.size();
				--i;
				--n;
			} while ((is_center_mountain != is_target_mountain) && n);

			if (!n) {
				// no suitable field found, this is no fail, there's just
				// nothing else to do so let's go home
				// FALLTHROUGH TO RETURN HOME
			} else {
				molog("[geologist]: Walk towards free field\n");
				if
					(!
					 start_task_movepath
					 (g,
					  target,
					  0,
					  descr().get_right_walk_anims(does_carry_ware())))
				{

					molog("[geologist]: BUG: couldn't find path\n");
					set_signal("fail");
					pop_task();
					return;
				}
				return;
			}
		}

		molog("[geologist]: Found no applicable field, going home\n");
		state->ivar1 = 0;
	}

	if (get_position() == owner_area) {
		molog("[geologist]: We're home\n");
		pop_task();
		return;
	}

	molog("[geologist]: Return home\n");
	if (not start_task_movepath(g, owner_area, 0,
	                            descr().get_right_walk_anims(does_carry_ware())))
	{
		molog("[geologist]: Couldn't find path home\n");
		set_signal("fail");
		pop_task();
		return;
	}
}


void Worker::draw_inner(const Editor_Game_Base & game, RenderTarget & dst,
                        const Point drawpos) const
{
	dst.drawanim
		(drawpos,
		 get_current_anim(),
		 game.get_gametime() - get_animstart(),
		 get_owner());

	const WareInstance * const carried_item = get_carried_item(&game);
	if (carried_item)
		dst.drawanim
			(drawpos - Point(0, 15),
			 carried_item->descr().get_animation("idle"),
			 0,
			 get_owner());
}


/**
 * Draw the worker, taking the carried item into account.
 */
void Worker::draw(const Editor_Game_Base & game, RenderTarget & dst,
                  const Point pos) const
{
	if (get_current_anim())
		draw_inner(game, dst, calc_drawpos(game, pos));
}

};
