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

#include "worker.h"

#include "carrier.h"
#include "checkstep.h"
#include "cmd_incorporate.h"
#include "critter_bob.h"
#include "findimmovable.h"
#include "findnode.h"
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

	molog("  CreateItem(%s)\n", action->sparam1.c_str());

	item = fetch_carried_item(g);
	if (item) {
		molog("  Still carrying an item! Delete it.\n");
		item->schedule_destroy(g);
	}

	Player & player = *get_owner();
	Ware_Index wareid = player.tribe().ware_index(action->sparam1.c_str());
	item = new WareInstance(wareid, get_owner()->tribe().get_ware_descr(wareid));
	item->init(g);

	set_carried_item(g, item);

	// For statistics, inform the user that a ware was produced
	player.ware_produced(wareid);

	++state->ivar1;
	schedule_act(g, 10);
	return true;
}


/**
 * Mine on the current coordinates for resources decrease, go home.
 *
 * Syntax in conffile: mine \<resource\> \<area\>
 *
 * \param g
 * \param state
 * \param action Which resource to mine (action.sparam1) and where to look for
 * it (in a radius of action.iparam1 around current location)
 *
 * \todo Lots of magic numbers in here
 * \todo Document parameters g and state
 */
bool Worker::run_mine(Game* g, State* state, const Action* action)
{
	molog("  Mine(%s, %i)\n", action->sparam1.c_str(), action->iparam1);

	Map & map = *g->get_map();

	//Make sure that the specified resource is available in this world
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
		send_signal(g, "fail"); // mine empty, abort program
		pop_task(g);
		return true;
	}

	// Second pass through fields
	pick = g->logic_rand() % totalchance;

	do {
		uint8_t fres  = mr.location().field->get_resources();
		uint32_t amount = mr.location().field->get_resources_amount();

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
		send_signal(g, "fail"); // not successful, abort program
		pop_task(g);
		return true;
	}

	molog("  Mined one item\n");

	// Advance program state
	++state->ivar1;
	schedule_act(g, 10);
	return true;
}


/**
 * Breed on the current coordinates for resource increase, go home.
 *
 * Syntax in conffile: breed \<resource\> \<area\>
 *
 * \param g
 * \param state
 * \param action Which resource to breed (action.sparam1) and where to put
 * it (in a radius of action.iparam1 around current location)
 *
 * \todo Lots of magic numbers in here
 * \todo Document parameters g and state
 */
bool Worker::run_breed(Game* g, State* state, const Action* action)
{
	molog(" Breed(%s, %i)\n", action->sparam1.c_str(), action->iparam1);

	Map & map = *g->get_map();

	//Make sure that the specified resource is available in this world
	const Resource_Descr::Index res =
		map.get_world()->get_resource(action->sparam1.c_str());
	if (static_cast<int8_t>(res)==-1) //FIXME: ARGH!!
		throw wexception
			(" Worker::run_breed: Should breed resource %s, which "
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
		uint32_t amount = mr.location().field->get_starting_res_amount() - mr.location().field->get_resources_amount();

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
		molog("  All resources full\n");
		send_signal(g, "fail"); // no space for more, abort program
		pop_task(g);
		return true;
	}

	// Second pass through fields
	pick = g->logic_rand() % totalchance;

	do {
		uint8_t fres  = mr.location().field->get_resources();
		uint32_t amount = mr.location().field->get_starting_res_amount() - mr.location().field->get_resources_amount();

		if (fres != res)
			amount = 0;

		pick -= 8*amount;
		if (pick < 0) {
			assert(amount > 0);

			--amount;

			mr.location().field->set_resources(res, mr.location().field->get_starting_res_amount() - amount);
			break;
		}
	} while (mr.advance(map));

	if (pick >= 0) {
		molog("  Not successful this time\n");
		send_signal(g, "fail"); // not successful, abort program
		pop_task(g);
		return true;
	}

	molog("  Bred one item\n");

	// Advance program state
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
		send_signal(g, "fail");
		pop_task(g);
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
		send_signal(g, "fail");
		pop_task(g);
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

	CheckStepWalkOn cstep(descr().movecaps(), false);

	Map & map = g->map();
	const Area<FCoords> area (map.get_fcoords(get_position()), action->iparam1);
	if (action->sparam1 == "immovable") {
		std::vector<ImmovableFound> list;
		if (action->iparam2 < 0)
			map.find_reachable_immovables
				(area, &list, cstep);
		else
			map.find_reachable_immovables
				(area, &list, cstep, FindImmovableAttribute(action->iparam2));

		if (!list.size()) {
			send_signal(g, "fail"); // no object found, cannot run program
			pop_task(g);
			return true;
		}

		int32_t sel = g->logic_rand() % list.size();
		state->objvar1 = list[sel].object;
		molog("  %lu found\n", static_cast<long unsigned int>(list.size()));
	} else {
		std::vector<Bob*> list;
		log("BOB: searching bob with attribute (%i)\n", action->iparam2);
		if (action->iparam2 < 0)
			map.find_reachable_bobs
				(area, &list, cstep);
		else
			map.find_reachable_bobs
				(area, &list, cstep, FindBobAttribute(action->iparam2));

		if (!list.size()) {
			send_signal(g, "fail"); // no object found, cannot run program
			pop_task(g);
			return true;
		}
		int32_t sel = g->logic_rand() % list.size();
		state->objvar1 = list[sel];
		molog("  %lu found\n", static_cast<long unsigned int>(list.size()));
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
 * space
 * Find only fields that are walkable such that all neighbours
 * are also walkable (an exception is made if one of the neighbouring
 * fields is owned by this worker's location).
 *
 * iparam1 = radius
 * iparam2 = FindNodeSize::sizeXXX
 * iparam3 = whether the "space" flag is set
 * iparam4 = whether the "breed" flag is set
 * sparam1 = Resource
 */
struct FindNodeSpace {
	FindNodeSpace(BaseImmovable* ignoreimm)
		: ignoreimmovable(ignoreimm) {}

	bool accept(const Map& map, const FCoords& coords) const {
		if (!(coords.field->get_caps() & MOVECAPS_WALK))
			return false;

		for (uint8_t dir = Map_Object::FIRST_DIRECTION; dir <= Map_Object::LAST_DIRECTION; ++dir) {
			FCoords neighb = map.get_neighbour(coords, dir);

			if
				(!(neighb.field->get_caps() & MOVECAPS_WALK) &&
				 neighb.field->get_immovable() != ignoreimmovable)
				return false;
		}

		return true;
	}

private:
	BaseImmovable* ignoreimmovable;
};

bool Worker::run_findspace(Game* g, State* state, const Action* action)
{
	std::vector<Coords> list;
	Map & map = g->map();
	World * const w = &map.world();

	CheckStepDefault cstep(descr().movecaps());

	Area<FCoords> area(map.get_fcoords(get_position()), action->iparam1);

	FindNodeAnd functor;
	functor.add(FindNodeSize(static_cast<FindNodeSize::Size>(action->iparam2)));
	if (action->sparam1.size()) {
		if (action->iparam4)
			functor.add(FindNodeResourceBreedable(w->get_resource(action->sparam1.c_str())));
		else
			functor.add(FindNodeResource(w->get_resource(action->sparam1.c_str())));
	}

	if (action->iparam3)
		functor.add(FindNodeSpace(get_location(g)));

	if (!map.find_reachable_fields(area, &list, cstep, functor))
	{
		molog("  no space found\n");
		send_signal(g, "fail");
		pop_task(g);
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
	BaseImmovable const * const imm = g->map()[get_position()].get_immovable();
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
			send_signal(g, "fail");
			pop_task(g);
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
	if
		(not
		 start_task_movepath
		 	(g,
		 	 dest,
		 	 10,
		 	 descr().get_right_walk_anims(does_carry_ware()),
		 	 forceonlast, max_steps))
	{
		molog("  couldn't find path\n");
		send_signal(g, "fail");
		pop_task(g);
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
		send_signal(g, "fail");
		pop_task(g);
		return true;
	}

	molog("  object(%u): type = %i\n", obj->get_serial(), obj->get_type());

	if      (upcast(Immovable, immovable, obj))
		immovable->switch_program(g, action->sparam1);
	else if (upcast(Bob,       bob,       obj)) {
		if        (upcast(Critter_Bob, crit, bob)) {
			crit->reset_tasks(g); //TODO: We should ask the critter more nicely
			crit->start_task_program(g, action->sparam1);
		} else if (upcast(Worker,      w,    bob)) {
			w   ->reset_tasks(g); //TODO: We should ask the worker more nicely
			w   ->start_task_program(g, action->sparam1);
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
	if (BaseImmovable const * const imm = g->map()[pos].get_immovable())
		if (imm->get_size() >= BaseImmovable::SMALL) {
			molog("  field no longer free\n");
			send_signal(g, "fail");
			pop_task(g);
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

	molog
		("  Start Geologist (%i attempts, %i radius -> %s)\n",
		 action->iparam1, action->iparam2, action->sparam1.c_str());

	++state->ivar1;
	start_task_geologist(g, action->iparam1, action->iparam2, action->sparam1);
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


Worker::Worker(const Worker_Descr & worker_descr)
	:
	Bob          (worker_descr),
	m_economy    (0),
	m_supply     (0),
	m_needed_exp (0),
	m_current_exp(0)
{}

Worker::~Worker()
{
	assert(!m_location.is_set());
}


/// Log basic information.
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
		molog
			("* m_carried_item->get_ware() (id): %i\n",
			 ware->descr_index().value());
		molog("* m_carried_item->get_economy() (): %p\n", ware->get_economy());
	}

	molog("m_needed_exp: %i\n", m_needed_exp);
	molog("m_current_exp: %i\n", m_current_exp);

	molog("m_supply: %p\n", m_supply);
}


/**
 * Change the location. This should be called in the following situations:
 * \li worker creation (usually, location is a warehouse)
 * \li worker moves along a route (location is a road and finally building)
 * \li current location is destroyed (building burnt down etc...)
 */
void Worker::set_location(PlayerImmovable *location)
{
	assert(not location or Object_Ptr(location).get(&get_owner()->egbase()));
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

		send_signal(static_cast<Game *>(&get_owner()->egbase()), "location");
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
		m_economy->remove_workers(descr().tribe().worker_index(name().c_str()), 1);

	m_economy = economy;

	if (WareInstance * const item = get_carried_item(&get_owner()->egbase()))
		item->set_economy(m_economy);
	if (m_supply)
		m_supply->set_economy(m_economy);

	if (m_economy)
		m_economy->add_workers(descr().tribe().worker_index(name().c_str()), 1);
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
		create_needed_experience(*game); //  set his experience
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
	skip_act();
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
void Worker::create_needed_experience(Game & game)
{
	if (descr().get_min_exp() == -1 && descr().get_max_exp() == -1) {
		m_needed_exp = m_current_exp = -1;
		return;
	}

	m_needed_exp =
		descr().get_min_exp()
		+
		game.logic_rand() % (descr().get_max_exp() - descr().get_min_exp());
	m_current_exp = 0;
}


/**
 * Gain experience
 *
 * This function increases the experience
 * of the worker by one, if he reaches
 * needed_experience he levels
 */
Ware_Index Worker::gain_experience(Game & game) {
	return
		m_needed_exp == -1              ? Ware_Index::Null() :
		++m_current_exp == m_needed_exp ? level(game)          :
		Ware_Index::Null();
}


/**
 * Level this worker to the next higher level. this includes creating a
 * new worker with his propertys and removing this worker
 */
Ware_Index Worker::level(Game & game) {

	// We do not really remove this worker, all we do
	// is to overwrite his description with the new one and to
	// reset his needed experience. Congratulations to promotion!
	// This silently expects that the new worker is the same type as the old
	// worker and can fullfill the same jobs (which should be given in all
	// circumstances)
	assert(becomes());
	const Tribe_Descr & t = tribe();
	Ware_Index const old_index = t.worker_index(descr().name().c_str());
	Ware_Index const new_index = becomes();
	m_descr = t.get_worker_descr(new_index);
	assert(new_index);

	// Inform the economy, that something has changed
	m_economy->remove_workers(old_index, 1);
	m_economy->add_workers   (new_index, 1);

	create_needed_experience(game);
	return old_index; //  So that the caller knows what to replace him with.
}


/**
 * Set a fallback task.
 */
void Worker::init_auto_task(Game * game) {
	if (get_location(game)) {
		if (get_economy()->get_nr_warehouses())
			return start_task_gowarehouse(game);

		set_location(0);
	}

	molog("init_auto_task: become fugitive\n");

	return start_task_fugitive(game);
}


/**
 * Follow the given transfer.
 *
 * Signal "cancel" to cancel the transfer.
 */
const Bob::Task Worker::taskTransfer = {
	"transfer",
	static_cast<Bob::Ptr>(&Worker::transfer_update),
	static_cast<Bob::PtrSignal>(&Worker::transfer_signalimmediate),
	0
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

		molog("start_task_transfer while in gowarehouse\n");
		state->transfer = t;
		send_signal(g, "transfer");
		return;
	}

	// just start a normal transfer
	push_task(g, taskTransfer);

	state = get_state();
	state->transfer = t;
}


void Worker::transfer_update(Game * g, State * state) {
	Map & map = g->map();
	PlayerImmovable * location = get_location(g);

	// We expect to always have a location at this point,
	// but this assumption may fail when loading a corrupted savegame.
	if (!location) {
		send_signal(g, "location");
		return pop_task(g);
	}

	// The request is no longer valid, the task has failed
	if (!state->transfer) {
		molog("[transfer]: Fail (without transfer)\n");

		send_signal(g, "fail");
		return pop_task(g);
	}

	// Signal handling
	std::string signal = get_signal();

	if (signal.size()) {
		// The caller requested a route update, or the previously calulcated route
		// failed.
		// We will recalculate the route on the next update().
		if (signal == "road" || signal == "fail") {
			molog("[transfer]: Got signal '%s' -> recalculate\n", signal.c_str());

			signal_handled();
		} else {
			molog("[transfer]: Cancel due to signal '%s'\n", signal.c_str());
			return pop_task(g);
		}
	}

	// If our location is a building, make sure we're actually in it.
	// If we're a building's worker, and we've just been released from
	// the building, we may be somewhere else entirely (e.g. lumberjack, soldier)
	// or we may be on the building's flag for a fetch_from_flag or dropoff
	// task.
	// Similarly for flags.
	if (dynamic_cast<Building const *>(location)) {
		BaseImmovable * const position = map[get_position()].get_immovable();

		if (position != location) {
			if (upcast(Flag, flag, position)) {
				location = flag;
				set_location(flag);
			} else
				return set_location(0);
		}
	} else if (upcast(Flag, flag, location)) {
		BaseImmovable * const position = map[get_position()].get_immovable();

		if (position != flag) {
			if (position == flag->get_building()) {
				Building* building = static_cast<Building*>(position);
				set_location(building);
				location = building;
			} else
				return set_location(0);
		}
	}

	// Figure out where to go
	bool success;
	PlayerImmovable * const nextstep =
		state->transfer->get_next_step(location, &success);

	if (!nextstep) {
		Transfer* t = state->transfer;

		state->transfer = 0;

		if (success) {
			molog("[transfer]: Success\n");
			pop_task(g);

			t->has_finished();
		} else {
			molog("[transfer]: Failed\n");
			send_signal(g, "fail");
			pop_task(g);

			t->has_failed();
		}
		return;
	}

	// Initiate the next step
	if        (upcast(Building, building, location)) {
		if (building->get_base_flag() != nextstep)
			throw wexception
				("MO(%u): [transfer]: in building, nextstep is not building's "
				 "flag",
				 get_serial());

		return start_task_leavebuilding(g, true);
	} else if (upcast(Flag,     flag,     location)) {
		if        (upcast(Building, nextbuild, nextstep)) { //  Flag to Building
			if (nextbuild->get_base_flag() != location)
				throw wexception
					("MO(%u): [transfer]: next step is building, but we are "
					 "nowhere near",
					 get_serial());

			return
				start_task_move
					(g,
					 WALK_NW, &descr().get_right_walk_anims(does_carry_ware()),
					 true);
		} else if (upcast(Flag,     nextflag,  nextstep)) { //  Flag to Flag
			Road & road = *flag->get_road(nextflag);

			molog
				("[transfer]: move to next flag via road %u\n", road.get_serial());

			Path path(road.get_path());

			if (nextstep != road.get_flag(Road::FlagEnd))
				path.reverse();

			start_task_movepath
				(g, path, descr().get_right_walk_anims(does_carry_ware()));
			set_location(&road);
		} else if (upcast(Road,    road,      nextstep)) { //  Flag to Road
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
			schedule_act(g, 10); //  wait a little
		} else
			throw wexception
				("MO(%u): [transfer]: flag to bad nextstep %u",
				 get_serial(), nextstep->get_serial());
	} else if (upcast(Road,     road,     location)) {
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

			molog
				("[transfer]: on road %u, to flag %u, index is %i\n",
				 road->get_serial(), nextstep->get_serial(), index);

			if (index >= 0) {
				if
					(start_task_movepath
					 	(g,
					 	 map,
					 	 path,
					 	 index,
					 	 descr().get_right_walk_anims(does_carry_ware())))
				{
					molog
						("[transfer]: from road %u to flag %u nextstep %u\n",
						 get_serial(), road->get_serial(), nextstep->get_serial());
					return;
				}
			} else if (nextstep != map[get_position()].get_immovable())
				throw wexception
					("MO(%u): [transfer]: road to flag, but flag is nowhere near",
					 get_serial());

			molog("[transfer]: arrive at flag %u\n", nextstep->get_serial());
			set_location(dynamic_cast<Flag *>(nextstep));
			set_animation(g, descr().get_animation("idle"));
			schedule_act(g, 10); //  wait a little
		} else
			throw wexception
				("MO(%u): [transfer]: from road to bad nextstep %u",
				 get_serial(), nextstep->get_serial());
	} else
		throw wexception
			("MO(%u): location %u has bad type",
			 get_serial(), location->get_serial());
}


void Worker::transfer_signalimmediate(Game *, State * state, const std::string& signal)
{
	if (signal == "cancel")
		state->transfer = 0; //  do not call transfer_fail/finish when cancelled
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
const Bob::Task Worker::taskBuildingwork = {
	"buildingwork",
	static_cast<Bob::Ptr>(&Worker::buildingwork_update),
	0,
	0
};


/**
 * Begin work at a building.
 */
void Worker::start_task_buildingwork(Game* g)
{
	push_task(g, taskBuildingwork);
	top_state().ivar1 = 0;
}


void Worker::buildingwork_update(Game* g, State* state)
{
	// Reset any signals that are not related to location
	std::string signal = get_signal();
	signal_handled();

	if (state->ivar1 == 1)
		state->ivar1 = (signal == "fail") * 2;

	// Return to building, if necessary
	Building* building = dynamic_cast<Building*>(get_location(g));
	if (!building)
		return pop_task(g);

	if (g->map().get_immovable(get_position()) != building) {
		molog("[buildingwork]: Something went wrong, return home.\n");
		return start_task_return(g, false); // don't drop item
	}

	// Get the new job
	bool success = state->ivar1 != 2;

	// Set this *before* calling to get_building_work, because the
	// state pointer might become invalid
	state->ivar1 = 1;

	if (not building->get_building_work(g, this, success)) {
		set_animation(g, 0);
		skip_act();
	}
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
const Bob::Task Worker::taskReturn = {
	"return",
	static_cast<Bob::Ptr>(&Worker::return_update),
	0,
	0
};


/**
 * Return to our owning building.
 */
void Worker::start_task_return(Game* g, bool dropitem)
{
	PlayerImmovable* location = get_location(g);

	if (!location || location->get_type() != BUILDING)
		throw wexception("MO(%u): start_task_return(): not owned by building", get_serial());

	push_task(g, taskReturn);
	get_state()->ivar1 = dropitem ? 1 : 0;
}


void Worker::return_update(Game* g, State* state)
{
	std::string signal = get_signal();

	if (signal == "location") {
		molog("[return]: Interrupted by signal '%s'\n", signal.c_str());
		return pop_task(g);
	}

	signal_handled();

	Building & location = dynamic_cast<Building &>(*get_location(g));
	if (BaseImmovable * const pos = g->map().get_immovable(get_position())) {
		if (pos == &location) {
			set_animation(g, 0);
			return pop_task(g);
		}

		if (upcast(Flag, flag, pos)) {
			// Is this "our" flag?
			if (flag->get_building() == &location) {
				if (state->ivar1 && flag->has_capacity()) {
					WareInstance* item = fetch_carried_item(g);

					if (item) {
						flag->add_item(g, item);

						set_animation(g, descr().get_animation("idle"));
						return schedule_act(g, 20); // rest a while
					}
				}

				return
					start_task_move
						(g,
						 WALK_NW,
						 &descr().get_right_walk_anims(does_carry_ware()),
						 true);
			}
		}
	}

	// Determine the building's flag and move to it

	if
		(not
		 start_task_movepath
		 	(g,
		 	 location.get_base_flag()->get_position(),
		 	 15,
		 	 descr().get_right_walk_anims(does_carry_ware())))
	{
		molog("[return]: Failed to return\n");
		return set_location(0);
	}
}



/**
 * Follow the steps of a configuration-defined program.
 * ivar1 is the next action to be performed.
 * ivar2 is used to store description indices selected by setdescription
 * objvar1 is used to store objects found by findobject
 * coords is used to store target coordinates found by findspace
 */
const Bob::Task Worker::taskProgram = {
	"program",
	static_cast<Bob::Ptr>(&Worker::program_update),
	0,
	0
};


/**
 * Start the given program.
 */
void Worker::start_task_program(Game* g, const std::string & programname)
{
	push_task(g, taskProgram);
	State & state = top_state();
	state.program = descr().get_program(programname);
	state.ivar1 = 0;
}


void Worker::program_update(Game* g, State* state)
{
	if (get_signal().size()) {
		molog("[program]: Interrupted by signal '%s'\n", get_signal().c_str());
		return pop_task(g);
	}

	const Action* action;

	for (;;) {
		const WorkerProgram* program =
			static_cast<const WorkerProgram*>(state->program);

		if (state->ivar1 >= program->get_size()) {
			molog("  End of program\n");
			return pop_task(g);
		}

		action = program->get_action(state->ivar1);

		if ((this->*(action->function))(g, state, action))
			return;
	}
}


const Bob::Task Worker::taskGowarehouse = {
	"gowarehouse",
	static_cast<Bob::Ptr>(&Worker::gowarehouse_update),
	static_cast<Bob::PtrSignal>(&Worker::gowarehouse_signalimmediate),
	static_cast<Bob::Ptr>(&Worker::gowarehouse_pop)
};


/**
 * Get the worker to move to the nearest warehouse.
 * The worker is added to the list of usable wares, so he may be reassigned to
 * a new task immediately.
 */
void Worker::start_task_gowarehouse(Game* g)
{
	assert(!m_supply);

	push_task(g, taskGowarehouse);
}


void Worker::gowarehouse_update(Game* g, State* state)
{
	PlayerImmovable *location = get_location(g);

	if (!location) {
		send_signal(g, "location");
		return pop_task(g);
	}

	// Signal handling
	std::string signal = get_signal();

	if (signal.size()) {
		// if routing has failed, try a different warehouse/route on next update()
		if (signal == "fail") {
			molog("[gowarehouse]: caught 'fail'\n");
			signal_handled();
		} else if (signal == "transfer") {
			molog("[gowarehouse]: transfer signal\n");
			signal_handled();
		} else {
			molog("[gowarehouse]: cancel for signal '%s'\n", signal.c_str());
			return pop_task(g);
		}
	}

	if (dynamic_cast<Warehouse const *>(location)) {
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
		pop_task(g);
		return start_task_transfer(g, t);
	}

	if (!get_economy()->get_nr_warehouses()) {
		molog("[gowarehouse]: No warehouse left in Economy\n");
		return pop_task(g);
	}

	// Idle until we are assigned a transfer.
	// The delay length is rather arbitrary, but we need some kind of
	// check against disappearing warehouses, or the worker will just
	// idle on a flag until the end of days (actually, until either the
	// flag is removed or a warehouse connects to the Economy).
	molog("[gowarehouse]: Idle\n");

	if (!m_supply)
		m_supply = new IdleWorkerSupply(this);

	return start_task_idle(g, get_animation("idle"), 1000);
}

void Worker::gowarehouse_signalimmediate(Game*, State*, const std::string& signal)
{
	if (signal == "transfer") {
		// We are assigned a transfer, make sure our supply disappears immediately
		// Otherwise, we might receive two transfers in a row.
		delete m_supply;
		m_supply = 0;
	}
}

void Worker::gowarehouse_pop(Game*, State*)
{
	delete m_supply;
	m_supply = 0;
}


const Bob::Task Worker::taskDropoff = {
	"dropoff",
	static_cast<Bob::Ptr>(&Worker::dropoff_update),
	0,
	0
};

/**
 * Walk to the building's flag, drop the given item, and walk back inside.
 */
void Worker::start_task_dropoff(Game* g, WareInstance* item)
{
	assert(item);

	set_carried_item(g, item);

	push_task(g, taskDropoff);
}


void Worker::dropoff_update(Game * g, State *)
{
	std::string signal = get_signal();

	if (signal.size()) {
		molog("[dropoff]: Interrupted by signal '%s'\n", signal.c_str());
		return pop_task(g);
	}

	WareInstance* item = get_carried_item(g);
	BaseImmovable * const location = g->map()[get_position()].get_immovable();
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

			return start_task_leavebuilding(g, false); //  exit throttle
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
			start_task_move
				(g, WALK_NW, &descr().get_right_walk_anims(does_carry_ware()), true);
			return;
		}

		throw wexception("MO(%u): [dropoff]: not on building or on flag - fishy", get_serial());
	}

	// We don't have the item any more, return home
	if (location->get_type() == Map_Object::FLAG)
		return
			start_task_move
				(g,
				 WALK_NW,
				 &descr().get_right_walk_anims(does_carry_ware()),
				 true);

	if (location->get_type() != Map_Object::BUILDING)
		throw wexception("MO(%u): [dropoff]: not on building on return", get_serial());

	if (dynamic_cast<Warehouse const *>(location)) {
		schedule_incorporate(g);
		return;
	}

	// Our parent task should know what to do
	molog("[dropoff]: back in building\n");
	return pop_task(g);
}


/**
 * ivar1 is set to 0 if we should move to the flag and fetch the item, and it
 * is set to 1 if we should move into the building.
 */
const Bob::Task Worker::taskFetchfromflag = {
	"fetchfromflag",
	static_cast<Bob::Ptr>(&Worker::fetchfromflag_update),
	0,
	0
};


/**
 * Walk to the building's flag, fetch an item from the flag that is destined for
 * the building, and walk back inside.
 */
void Worker::start_task_fetchfromflag(Game* g)
{
	push_task(g, taskFetchfromflag);
	top_state().ivar1 = 0;
}


void Worker::fetchfromflag_update(Game *g, State* state)
{
	PlayerImmovable & owner = *get_location(g);
	PlayerImmovable * const location =
		dynamic_cast<PlayerImmovable *>(g->map().get_immovable(get_position()));

	// If we haven't got the item yet, walk onto the flag
	if (!get_carried_item(g) && !state->ivar1) {
		if (dynamic_cast<Building const *>(location))
			return start_task_leavebuilding(g, false);

		state->ivar1 = 1; // force return to building

		// The item has decided that it doesn't want to go to us after all
		// In order to return to the warehouse, we're switching to State_DropOff
		if
			(WareInstance * const item =
			 dynamic_cast<Flag &>(*location).fetch_pending_item(g, &owner))
			set_carried_item(g, item);

		set_animation(g, descr().get_animation("idle"));
		return schedule_act(g, 20);
	}

	// Go back into the building
	if (dynamic_cast<Flag const *>(location)) {
		molog("[fetchfromflag]: return to building\n");

		return
			start_task_move
				(g,
				 WALK_NW,
				 &descr().get_right_walk_anims(does_carry_ware()), true);
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

	return pop_task(g); // assume our parent task knows what to do
}


/**
 * Wait for available capacity on a flag.
 */
const Bob::Task Worker::taskWaitforcapacity = {
	"waitforcapacity",
	static_cast<Bob::Ptr>(&Worker::waitforcapacity_update),
	0,
	static_cast<Bob::Ptr>(&Worker::waitforcapacity_pop)
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

	push_task(g, taskWaitforcapacity);

	get_state()->objvar1 = flag;

	flag->wait_for_capacity(g, this);

	return true;
}


void Worker::waitforcapacity_update(Game * g, State *)
{
	std::string signal = get_signal();

	if (signal.size()) {
		if (signal == "wakeup")
			signal_handled();
		return pop_task(g);
	}

	skip_act(); // wait indefinitely
}


void Worker::waitforcapacity_pop(Game* g, State* state)
{
	if (upcast(Flag, flag, state->objvar1.get(g)))
		flag->skip_wait_for_capacity(g, this);
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
			throw wexception
				("MO(%u): wakeup_flag_capacity: Flags don't match.", get_serial());

		send_signal(g, "wakeup");
		return true;
	}

	return false;
}


/**
 * ivar1 - 0: don't change location; 1: change location to the flag
 * objvar1 - the building we're leaving
 */
const Bob::Task Worker::taskLeavebuilding = {
	"leavebuilding",
	static_cast<Bob::Ptr>(&Worker::leavebuilding_update),
	0,
	static_cast<Bob::Ptr>(&Worker::leavebuilding_pop)
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
	push_task(g, taskLeavebuilding);

	get_state()->ivar1 = changelocation ? 1 : 0;
	get_state()->objvar1 = &building;
}


void Worker::leavebuilding_update(Game* g, State* state)
{
	std::string signal = get_signal();

	if (signal == "wakeup")
		signal_handled();
	else if (signal.size())
		return pop_task(g);

	if (upcast(Building, building, g->map().get_immovable(get_position()))) {
		assert(building == state->objvar1.get(g));

		if (!building->leave_check_and_wait(g, this))
			return skip_act();

		if (state->ivar1)
			set_location(building->get_base_flag());

		return
			start_task_move
				(g,
				 WALK_SE,
				 &descr().get_right_walk_anims(does_carry_ware()),
				 true);
	} else
		return pop_task(g);
}


void Worker::leavebuilding_pop(Game* g, State* state)
{
	// As of this writing, this is only really necessary when the task
	// is interrupted by a signal. Putting this in the pop() method is just
	// defensive programming, in case leavebuilding_update() changes
	// in the future.
	if (upcast(Building, building, state->objvar1.get(g)))
		building->leave_skip(g, this);
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
const Bob::Task Worker::taskFugitive = {
	"fugitive",
	static_cast<Bob::Ptr>(&Worker::fugitive_update),
	0,
	0
};


void Worker::start_task_fugitive(Game* g)
{
	push_task(g, taskFugitive);

	// Fugitives survive for two to four minutes
	get_state()->ivar1 = g->get_gametime() + 120000 + 200*(g->logic_rand() % 600);
}

struct FindFlagWithPlayersWarehouse {
	FindFlagWithPlayersWarehouse(Player const & owner) : m_owner(owner) {}
	bool accept(BaseImmovable * const imm) const {
		if (upcast(Flag const, flag, imm))
			if (flag->get_owner() == &m_owner)
				if (flag->economy().get_nr_warehouses())
					return true;
		return false;
	}
private:
	Player const & m_owner;
};

void Worker::fugitive_update(Game* g, State* state)
{
	if (get_signal().size()) {
		molog("[fugitive]: interrupted by signal '%s'\n", get_signal().c_str());
		return pop_task(g);
	}

	Map & map = g->map();
	PlayerImmovable *location = get_location(g);

	if (location && location->get_owner() == get_owner()) {
		molog("[fugitive]: we're on location\n");

		if (dynamic_cast<Warehouse const *>(location))
			return schedule_incorporate(g);

		set_location(0);
		location = 0;
	}

	// check whether we're on a flag and it's time to return home
	if (upcast(Flag, flag, map[get_position()].get_immovable())) {
		if
			(flag->get_owner() == get_owner()
			 and
			 flag->economy().get_nr_warehouses())
		{
			set_location(flag);
			return pop_task(g);
		}
	}

	//  try to find a flag connected to a warehouse that we can return to
	std::vector<ImmovableFound> flags;
	if
		(map.find_immovables
		 	(Area<FCoords>(map.get_fcoords(get_position()), vision_range()),
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

			int32_t const dist = map.calc_distance(get_position(), it->coords);

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
		return schedule_destroy(g);
	}

	molog("[fugitive]: wander randomly\n");

	if
		(start_task_movepath
		 	(g,
		 	 g->random_location(get_position(), vision_range()),
		 	 4,
		 	 descr().get_right_walk_anims(does_carry_ware())))
		return;

	return start_task_idle(g, descr().get_animation("idle"), 50);
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
const Bob::Task Worker::taskGeologist = {
	"geologist",
	static_cast<Bob::Ptr>(&Worker::geologist_update),
	0,
	0
};


void Worker::start_task_geologist
	(Game* g,
	 int32_t             const attempts,
	 int32_t             const radius,
	 std::string const &       subcommand)
{
	push_task(g, taskGeologist);
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
		signal_handled();
	} else if (signal.size()) {
		molog("[geologist]: Interrupted by signal '%s'\n", signal.c_str());
		return pop_task(g);
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
			return start_task_program(g, state->svar1);
		}

		// Find a suitable field and walk towards it
		std::vector<Coords> list;
		CheckStepDefault cstep(descr().movecaps());
		FindNodeAnd ffa;

		ffa.add(FindNodeImmovableSize(FindNodeImmovableSize::sizeNone), false);
		ffa.add(FindNodeImmovableAttribute(RESI), true);

		if (map.find_reachable_fields(owner_area, &list, cstep, ffa)) {
			FCoords target;

			// is center a mountain piece?
			bool is_center_mountain =
				(world.terrain_descr(owner_area.field->terrain_d()).get_is()
				 &
				 TERRAIN_MOUNTAIN)
				|
				(world.terrain_descr(owner_area.field->terrain_r()).get_is()
				 &
				 TERRAIN_MOUNTAIN);
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
					(world.terrain_descr(target.field->terrain_d()).get_is()
					 &
					 TERRAIN_MOUNTAIN)
					|
					(world.terrain_descr(target.field->terrain_r()).get_is()
					 &
					 TERRAIN_MOUNTAIN);
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
					send_signal(g, "fail");
					return pop_task(g);
				}
				return;
			}
		}

		molog("[geologist]: Found no applicable field, going home\n");
		state->ivar1 = 0;
	}

	if (get_position() == owner_area) {
		molog("[geologist]: We're home\n");
		return pop_task(g);
	}

	molog("[geologist]: Return home\n");
	if
		(not
		 start_task_movepath
		 	(g, owner_area, 0, descr().get_right_walk_anims(does_carry_ware())))
	{
		molog("[geologist]: Couldn't find path home\n");
		send_signal(g, "fail");
		return pop_task(g);
	}
}


void Worker::draw_inner
	(Editor_Game_Base const &       game,
	 RenderTarget           &       dst,
	 Point                    const drawpos)
	const
{
	dst.drawanim
		(drawpos,
		 get_current_anim(),
		 game.get_gametime() - get_animstart(),
		 get_owner());

	if (WareInstance const * const carried_item = get_carried_item(&game))
		dst.drawanim
			(drawpos - Point(0, 15),
			 carried_item->descr().get_animation("idle"),
			 0,
			 get_owner());
}


/**
 * Draw the worker, taking the carried item into account.
 */
void Worker::draw
	(Editor_Game_Base const & game, RenderTarget & dst, Point const pos) const
{
	if (get_current_anim())
		draw_inner(game, dst, calc_drawpos(game, pos));
}

};
