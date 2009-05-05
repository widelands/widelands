/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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
#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "economy/transfer.h"
#include "economy/ware_instance.h"
#include "findimmovable.h"
#include "findnode.h"
#include "game.h"
#include "graphic/graphic.h"
#include "helper.h"
#include "wui/interactive_player.h"
#include "player.h"
#include "profile/profile.h"
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
bool Worker::run_createitem(Game & game, State & state, Action const & action)
{
	if (WareInstance * const item = fetch_carried_item(game)) {
		molog("  Still carrying an item! Delete it.\n");
		item->schedule_destroy(game);
	}

	Player & player = *get_owner();
	Ware_Index const wareid = player.tribe().ware_index(action.sparam1.c_str());
	WareInstance & item =
		*new WareInstance(wareid, tribe().get_ware_descr(wareid));
	item.init(game);

	set_carried_item(game, &item);

	// For statistics, inform the user that a ware was produced
	player.ware_produced(wareid);

	++state.ivar1;
	schedule_act(game, 10);
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
 */
bool Worker::run_mine(Game & game, State & state, Action const & action)
{
	Map & map = game.map();

	//Make sure that the specified resource is available in this world
	Resource_Index const res =
		map.get_world()->get_resource(action.sparam1.c_str());
	if (static_cast<int8_t>(res) == -1) //  FIXME ARGH!!
		throw wexception
			("should mine resource %s, which does not exist in world. Tribe is "
			 "not compatible with world",
			 action.sparam1.c_str());

	// Select one of the fields randomly
	uint32_t totalres = 0;
	uint32_t totalchance = 0;
	int32_t pick;
	MapRegion<Area<FCoords> > mr
		(map, Area<FCoords>(map.get_fcoords(get_position()), action.iparam1));
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
		send_signal(game, "fail"); //  mine empty, abort program
		pop_task(game);
		return true;
	}

	// Second pass through fields
	pick = game.logic_rand() % totalchance;

	do {
		uint8_t fres  = mr.location().field->get_resources();
		uint32_t amount = mr.location().field->get_resources_amount();

		if (fres != res)
			amount = 0;

		pick -= 8 * amount;
		if (pick < 0) {
			assert(amount > 0);

			--amount;

			mr.location().field->set_resources(res, amount);
			break;
		}
	} while (mr.advance(map));

	if (pick >= 0) {
		molog("  Not successful this time\n");
		send_signal(game, "fail"); //  not successful, abort program
		pop_task(game);
		return true;
	}

	// Advance program state
	++state.ivar1;
	schedule_act(game, 10);
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
bool Worker::run_breed(Game & game, State & state, Action const & action)
{
	molog(" Breed(%s, %i)\n", action.sparam1.c_str(), action.iparam1);

	Map & map = game.map();

	//Make sure that the specified resource is available in this world
	Resource_Index const res =
		map.get_world()->get_resource(action.sparam1.c_str());
	if (static_cast<int8_t>(res) == -1) //  FIXME ARGH!!
		throw wexception
			("should breed resource type %s, which does not exist in world, "
			 "tribe is not compatible with world",
			 action.sparam1.c_str());

	// Select one of the fields randomly
	uint32_t totalres = 0;
	uint32_t totalchance = 0;
	int32_t pick;
	MapRegion<Area<FCoords> > mr
		(map, Area<FCoords>(map.get_fcoords(get_position()), action.iparam1));
	do {
		uint8_t fres  = mr.location().field->get_resources();
		uint32_t amount =
			mr.location().field->get_starting_res_amount() -
			mr.location().field->get_resources_amount   ();

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
		send_signal(game, "fail"); //  no space for more, abort program
		pop_task(game);
		return true;
	}

	// Second pass through fields
	pick = game.logic_rand() % totalchance;

	do {
		uint8_t fres  = mr.location().field->get_resources();
		uint32_t amount =
			mr.location().field->get_starting_res_amount() -
			mr.location().field->get_resources_amount   ();

		if (fres != res)
			amount = 0;

		pick -= 8 * amount;
		if (pick < 0) {
			assert(amount > 0);

			--amount;

			mr.location().field->set_resources
				(res, mr.location().field->get_starting_res_amount() - amount);
			break;
		}
	} while (mr.advance(map));

	if (pick >= 0) {
		molog("  Not successful this time\n");
		send_signal(game, "fail"); //  not successful, abort program
		pop_task(game);
		return true;
	}

	molog("  Bred one item\n");

	// Advance program state
	++state.ivar1;
	schedule_act(game, 10);
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
bool Worker::run_setdescription
	(Game & game, State & state, Action const & action)
{
	uint32_t const idx = game.logic_rand() % action.sparamv.size();

	std::vector<std::string> const list(split_string(action.sparamv[idx], ":"));
	std::string bob;
	if (list.size() == 1) {
		state.svar1 = "world";
		bob = list[0];
	} else {
		state.svar1 = "tribe";
		bob = list[1];
	}

	state.ivar2 =
		state.svar1 == "world" ?
		game.map().world().get_immovable_index(bob.c_str())
		:
		descr ().tribe().get_immovable_index(bob.c_str());

	if (state.ivar2 < 0) {
		molog("  WARNING: Unknown immovable %s\n", action.sparamv[idx].c_str());
		send_signal(game, "fail");
		pop_task(game);
		return true;
	}

	++state.ivar1;
	schedule_act(game, 10);
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
bool Worker::run_setbobdescription
	(Game & game, State & state, Action const & action)
{
	int32_t const idx = game.logic_rand() % action.sparamv.size();

	std::vector<std::string> const list(split_string(action.sparamv[idx], ":"));
	std::string bob;
	if (list.size() == 1) {
		state.svar1 = "world";
		bob = list[0];
	} else {
		state.svar1 = "tribe";
		bob = list[1];
	}

	state.ivar2 =
		state.svar1 == "world" ?
		game.map().world().get_bob(bob.c_str())
		:
		descr ().tribe().get_bob(bob.c_str());

	if (state.ivar2 < 0) {
		molog("  WARNING: Unknown bob %s\n", action.sparamv[idx].c_str());
		send_signal(game, "fail");
		pop_task(game);
		return true;
	}

	++state.ivar1;
	schedule_act(game, 10);
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
bool Worker::run_findobject(Game & game, State & state, Action const & action)
{
	CheckStepWalkOn cstep(descr().movecaps(), false);

	Map & map = game.map();
	Area<FCoords> area (map.get_fcoords(get_position()), 0);
	if (action.sparam1 == "immovable")
		for (;; ++area.radius) {
			if (action.iparam1 < area.radius) {
				send_signal(game, "fail"); //  no object found, cannot run program
				pop_task(game);
				return true;
			}
			std::vector<ImmovableFound> list;
			if (action.iparam2 < 0)
				map.find_reachable_immovables
					(area, &list, cstep);
			else
				map.find_reachable_immovables
					(area, &list, cstep, FindImmovableAttribute(action.iparam2));

			if (list.size()) {
				state.objvar1 = list[game.logic_rand() % list.size()].object;
				break;
			}
		}
	else
		for (;; ++area.radius) {
			if (action.iparam1 < area.radius) {
				send_signal(game, "fail"); //  no object found, cannot run program
				pop_task(game);
				return true;
			}
			std::vector<Bob *> list;
			if (action.iparam2 < 0)
				map.find_reachable_bobs
					(area, &list, cstep);
			else
				map.find_reachable_bobs
					(area, &list, cstep, FindBobAttribute(action.iparam2));

			if (list.size()) {
				state.objvar1 = list[game.logic_rand() % list.size()];
				break;
			}
		}

	++state.ivar1;
	schedule_act(game, 10);
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
	FindNodeSpace(BaseImmovable * const ignoreimm)
		: ignoreimmovable(ignoreimm) {}

	bool accept(Map const & map, FCoords const & coords) const {
		if (!(coords.field->get_caps() & MOVECAPS_WALK))
			return false;

		for
			(uint8_t dir = Map_Object::FIRST_DIRECTION;
			 dir <= Map_Object::LAST_DIRECTION;
			 ++dir)
		{
			FCoords neighb = map.get_neighbour(coords, dir);

			if
				(!(neighb.field->get_caps() & MOVECAPS_WALK) &&
				 neighb.field->get_immovable() != ignoreimmovable)
				return false;
		}

		return true;
	}

private:
	BaseImmovable * ignoreimmovable;
};

bool Worker::run_findspace(Game & game, State & state, Action const & action)
{
	std::vector<Coords> list;
	Map & map = game.map();
	World * const w = &map.world();

	CheckStepDefault cstep(descr().movecaps());

	Area<FCoords> area(map.get_fcoords(get_position()), action.iparam1);

	FindNodeAnd functor;
	functor.add(FindNodeSize(static_cast<FindNodeSize::Size>(action.iparam2)));
	if (action.sparam1.size()) {
		if (action.iparam4)
			functor.add
				(FindNodeResourceBreedable
				 	(w->get_resource(action.sparam1.c_str())));
		else
			functor.add
				(FindNodeResource(w->get_resource(action.sparam1.c_str())));
	}

	if (action.iparam3)
		functor.add(FindNodeSpace(get_location(game)));

	if (!map.find_reachable_fields(area, &list, cstep, functor))
	{
		molog("  no space found\n");
		send_signal(game, "fail");
		pop_task(game);
		return true;
	}

	// Pick a location at random
	state.coords = list[game.logic_rand() % list.size()];

	++state.ivar1;
	schedule_act(game, 10);
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
bool Worker::run_walk(Game & game, State & state, Action const & action)
{
	BaseImmovable const * const imm = game.map()[get_position()].get_immovable();
	Coords dest;
	bool forceonlast = false;
	int32_t max_steps = -1;

	// First of all, make sure we're outside
	if (imm == &dynamic_cast<Building &>(*get_location(game))) {
		start_task_leavebuilding(game, false);
		return true;
	}

	// Determine the coords we need to walk towards
	switch (action.iparam1) {
	case Action::walkObject: {
		Map_Object * const obj = state.objvar1.get(game);

		if (!obj) {
			send_signal(game, "fail");
			pop_task(game);
			return true;
		}

		if      (upcast(Bob       const, bob,       obj))
			dest = bob      ->get_position();
		else if (upcast(Immovable const, immovable, obj))
			dest = immovable->get_position();
		else
			throw wexception
				("MO(%u): [actWalk]: bad object type = %i",
				 serial(), obj->get_type());

		//  Only take one step, then rethink (object may have moved)
		max_steps = 1;

		forceonlast = true;
		break;
	}
	case Action::walkCoords: {
		dest = state.coords;
		break;
	}
	default:
		throw wexception
			("MO(%u): [actWalk]: bad action.iparam1 = %i",
			 serial(), action.iparam1);
	}

	// If we've already reached our destination, that's cool
	if (get_position() == dest) {
		++state.ivar1;
		return false; // next instruction
	}

	// Walk towards it
	if
		(not
		 start_task_movepath
		 	(game,
		 	 dest,
		 	 10,
		 	 descr().get_right_walk_anims(does_carry_ware()),
		 	 forceonlast, max_steps))
	{
		molog("  could not find path\n");
		send_signal(game, "fail");
		pop_task(game);
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
bool Worker::run_animation(Game & game, State & state, Action const & action)
{
	set_animation(game, action.iparam1);

	++state.ivar1;
	schedule_act(game, action.iparam2);
	return true;
}



/**
 * Return home, drop any item we're carrying onto our building's flag.
 *
 * iparam1 = 0: don't drop item on flag, 1: do drop item on flag
 */
bool Worker::run_return(Game & game, State & state, Action const & action)
{
	++state.ivar1;
	start_task_return(game, action.iparam1);
	return true;
}


/**
 * object \<command\>
 *
 * Cause the currently selected object to execute the given program.
 *
 * sparam1 = object command name
 */
bool Worker::run_object(Game & game, State & state, Action const & action)
{
	Map_Object * const obj = state.objvar1.get(game);

	if (!obj) {
		send_signal(game, "fail");
		pop_task(game);
		return true;
	}

	if      (upcast(Immovable, immovable, obj))
		immovable->switch_program(game, action.sparam1);
	else if (upcast(Bob,       bob,       obj)) {
		if        (upcast(Critter_Bob, crit, bob)) {
			crit->reset_tasks(game); //  TODO ask the critter more nicely
			crit->start_task_program(game, action.sparam1);
		} else if (upcast(Worker,      w,    bob)) {
			w   ->reset_tasks(game); //  TODO  ask the worker more nicely
			w   ->start_task_program(game, action.sparam1);
		} else
			throw wexception
				("MO(%i): [actObject]: bab bob type = %i",
				 serial(), bob->get_bob_type());
	} else
		throw wexception
			("MO(%u): [actObject]: bad object type = %i",
			 serial(), obj->get_type());

	++state.ivar1;
	schedule_act(game, 10);
	return true;
}


/**
 * Plant an immovable on the current position. The immovable type must have
 * been selected by a previous command (i.e. setdescription)
 */
bool Worker::run_plant(Game & game, State & state, Action const &)
{
	Coords pos = get_position();

	// Check if the map is still free here
	if (BaseImmovable const * const imm = game.map()[pos].get_immovable())
		if (imm->get_size() >= BaseImmovable::SMALL) {
			molog("  field no longer free\n");
			send_signal(game, "fail");
			pop_task(game);
			return true;
		}

	game.create_immovable
		(pos, state.ivar2, state.svar1 == "world" ? 0 : &descr().tribe());

	++state.ivar1;
	schedule_act(game, 10);
	return true;
}


/**
 * Plants a bob (critter usually, maybe also worker later on). The immovable
 * type must have been selected by a previous command (i.e. setbobdescription).
 */
bool Worker::run_create_bob(Game & game, State & state, Action const &)
{
	game.create_bob
		(get_position(), state.ivar2, state.svar1 == "world" ? 0 : &tribe());
	++state.ivar1;
	schedule_act(game, 10);
	return true;
}


/**
 * Simply remove the currently selected object - make no fuss about it.
 */
bool Worker::run_removeobject(Game & game, State & state, Action const &)
{
	if (Map_Object * const obj = state.objvar1.get(game)) {
		obj->remove(game);
		state.objvar1 = 0;
	}

	++state.ivar1;
	schedule_act(game, 10);
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
bool Worker::run_geologist(Game & game, State & state, Action const & action)
{
	dynamic_cast<Flag const &>(*get_location(game));

	molog
		("  Start Geologist (%i attempts, %i radius -> %s)\n",
		 action.iparam1, action.iparam2, action.sparam1.c_str());

	++state.ivar1;
	start_task_geologist(game, action.iparam1, action.iparam2, action.sparam1);
	return true;
}


/**
 * Check resources at the current position, and plant a marker object when
 * possible.
 */
bool Worker::run_geologist_find(Game & game, State & state, Action const &)
{
	Map const & map = game.map();
	const FCoords position = map.get_fcoords(get_position());
	BaseImmovable const * const imm = position.field->get_immovable();

	if (imm && imm->get_size() > BaseImmovable::NONE) {
		//NoLog("  Field is no longer empty\n");
	} else if
		(const Resource_Descr * const rdescr =
		 map.world().get_resource(position.field->get_resources()))
	{
		Tribe_Descr const & t = tribe();
		game.create_immovable
			(position,
			 t.get_resource_indicator
			 	(rdescr,
			 	 rdescr->is_detectable() ?
			 	 position.field->get_resources_amount() : 0),
			 &t);
	}

	++state.ivar1;
	return false;
}


/**
 * Demand from the g_sound_handler to play a certain sound effect.
 * Whether the effect actually gets played is decided only by the sound server.
 */
bool Worker::run_playFX(Game & game, State & state, Action const & action)
{
	g_sound_handler.play_fx(action.sparam1, get_position(), action.iparam1);

	++state.ivar1;
	schedule_act(game, 10);
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
void Worker::log_general_info(Editor_Game_Base const & egbase)
{
	Bob::log_general_info(egbase);

	if (upcast(PlayerImmovable, loc, m_location.get(egbase))) {
		molog("* Owner: (%p)\n", &loc->owner());
		molog("** Owner (plrnr): %i\n", loc->owner().get_player_number());
		molog("* Economy: %p\n", loc->get_economy());
	}

	molog("Economy: %p\n", m_economy);

	if (upcast(WareInstance, ware, m_carried_item.get(egbase))) {
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
	assert(not location or Object_Ptr(location).get(owner().egbase()));
	PlayerImmovable * const oldlocation = get_location(owner().egbase());
	if (oldlocation == location)
		return;

	if (oldlocation)
		// Note: even though we have an oldlocation, m_economy may be zero
		// (oldlocation got deleted)
		oldlocation->remove_worker(*this);
	else
		assert(!m_economy);

	m_location = location;

	if (location) {
		Economy *eco = location->get_economy();

		if (!m_economy)
			set_economy(eco);
		else if (m_economy != eco)
			throw wexception("Worker::set_location changes economy");

		location->add_worker(*this);
	} else {
		// Our location has been destroyed, we are now fugitives.
		// Interrupt whatever we've been doing.
		set_economy(0);

		send_signal(dynamic_cast<Game &>(owner().egbase()), "location");
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
		m_economy->remove_workers
			(descr().tribe().worker_index(name().c_str()), 1);

	m_economy = economy;

	if (WareInstance * const item = get_carried_item(owner().egbase()))
		item->set_economy(m_economy);
	if (m_supply)
		m_supply->set_economy(m_economy);

	if (m_economy)
		m_economy->add_workers(descr().tribe().worker_index(name().c_str()), 1);
}


/**
 * Initialize the worker
 */
void Worker::init(Editor_Game_Base & egbase)
{
	Bob::init(egbase);

	// a worker should always start out at a fixed location
	// (this assert is not longer true for save games. Where it lives
	// is unknown to this worker till he is initialized
	//  assert(get_location(egbase));

	if (upcast(Game, game, &egbase))
		create_needed_experience(*game); //  set his experience
}


/**
 * Remove the worker.
 */
void Worker::cleanup(Editor_Game_Base & egbase)
{
	WareInstance * const item = get_carried_item(egbase);

	delete m_supply;
	m_supply = 0;

	if (item)
		if (egbase.objects().object_still_available(item))
			item->destroy(egbase);

	// We are destroyed, but we were maybe idling
	// or doing something else. Get Location might
	// init a gowarehouse task or something and this results
	// in a dirty stack. Nono, we do not want to end like this
	if (upcast(Game, game, &egbase))
		reset_tasks(*game);

	if (get_location(egbase))
		set_location(0);

	assert(!get_economy());

	Bob::cleanup(egbase);
}


/**
 * Set the item we carry.
 * If we carry an item right now, it will be destroyed (see
 * fetch_carried_item()).
 */
void Worker::set_carried_item(Game & game, WareInstance * const item)
{
	if (WareInstance * const olditem = get_carried_item(game)) {
		olditem->cleanup(game);
		delete olditem;
	}

	m_carried_item = item;
	item->set_location(game, this);
	item->update(game);
}


/**
 * Stop carrying the current item, and return a pointer to it.
 */
WareInstance * Worker::fetch_carried_item(Game & game)
{
	WareInstance * const item = get_carried_item(game);

	if (item) {
		item->set_location(game, 0);
		m_carried_item = 0;
	}

	return item;
}


/**
 * Schedule an immediate CMD_INCORPORATE, which will integrate this worker into
 * the warehouse he is standing on.
 */
void Worker::schedule_incorporate(Game & game)
{
	game.cmdqueue().enqueue (new Cmd_Incorporate(game.get_gametime(), this));
	return skip_act();
}


/**
 * Incorporate the worker into the warehouse it's standing on immediately.
 * This will delete the worker.
 */
void Worker::incorporate(Game & game)
{
	if (upcast(Warehouse, wh, get_location(game))) {
		wh->incorporate_worker(game, *this);
		return;
	}

	// our location has been deleted from under us
	send_signal(game, "fail");
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
		m_needed_exp == -1 or ++m_current_exp < m_needed_exp ?
		Ware_Index::Null() : level(game);
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
void Worker::init_auto_task(Game & game) {
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
void Worker::start_task_transfer(Game & game, Transfer * t)
{
	// hackish override for gowarehouse
	if (State * const state = get_state(taskGowarehouse)) {
		assert(!state->transfer);

		state->transfer = t;
		send_signal(game, "transfer");
	} else { //  just start a normal transfer
		push_task(game, taskTransfer);
		top_state().transfer = t;
	}
}


void Worker::transfer_update(Game & game, State & state) {
	Map & map = game.map();
	PlayerImmovable * location = get_location(game);

	// We expect to always have a location at this point,
	// but this assumption may fail when loading a corrupted savegame.
	if (!location) {
		send_signal(game, "location");
		return pop_task(game);
	}

	// The request is no longer valid, the task has failed
	if (!state.transfer) {
		molog("[transfer]: Fail (without transfer)\n");

		send_signal(game, "fail");
		return pop_task(game);
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
			return pop_task(game);
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
				upcast(Building, building, position);
				set_location(building);
				location = building;
			} else
				return set_location(0);
		}
	}

	// Figure out where to go
	bool success;
	PlayerImmovable * const nextstep =
		state.transfer->get_next_step(location, success);

	if (!nextstep) {
		Transfer * const t = state.transfer;

		state.transfer = 0;

		if (success) {
			pop_task(game);

			t->has_finished();
		} else {
			send_signal(game, "fail");
			pop_task(game);

			t->has_failed();
		}
		return;
	}

	// Initiate the next step
	if        (upcast(Building, building, location)) {
		if (&building->base_flag() != nextstep)
			throw wexception
				("MO(%u): [transfer]: in building, nextstep is not building's "
				 "flag",
				 serial());

		return start_task_leavebuilding(game, true);
	} else if (upcast(Flag,     flag,     location)) {
		if        (upcast(Building, nextbuild, nextstep)) { //  Flag to Building
			if (&nextbuild->base_flag() != location)
				throw wexception
					("MO(%u): [transfer]: next step is building, but we are "
					 "nowhere near",
					 serial());

			return
				start_task_move
					(game,
					 WALK_NW, &descr().get_right_walk_anims(does_carry_ware()),
					 true);
		} else if (upcast(Flag,     nextflag,  nextstep)) { //  Flag to Flag
			Road & road = *flag->get_road(nextflag);

			Path path(road.get_path());

			if (nextstep != &road.get_flag(Road::FlagEnd))
				path.reverse();

			start_task_movepath
				(game, path, descr().get_right_walk_anims(does_carry_ware()));
			set_location(&road);
		} else if (upcast(Road,    road,      nextstep)) { //  Flag to Road
			if
				(&road->get_flag(Road::FlagStart) != location
				 and
				 &road->get_flag(Road::FlagEnd)   != location)
				throw wexception
					("MO(%u): [transfer]: nextstep is road, but we are nowhere near",
					 serial());

			molog("[transfer]: set location to road %u\n", road->serial());
			set_location(road);
			set_animation(game, descr().get_animation("idle"));
			schedule_act(game, 10); //  wait a little
		} else
			throw wexception
				("MO(%u): [transfer]: flag to bad nextstep %u",
				 serial(), nextstep->serial());
	} else if (upcast(Road,     road,     location)) {
		// Road to Flag
		if (nextstep->get_type() == FLAG) {
			Path const & path = road->get_path();
			int32_t const index =
				nextstep == &road->get_flag(Road::FlagStart) ? 0                 :
				nextstep == &road->get_flag(Road::FlagEnd)   ? path.get_nsteps() :
				-1;

			if (index >= 0) {
				if
					(start_task_movepath
					 	(game,
					 	 path,
					 	 index,
					 	 descr().get_right_walk_anims(does_carry_ware())))
				{
					molog
						("[transfer]: from road %u to flag %u nextstep %u\n",
						 serial(), road->serial(), nextstep->serial());
					return;
				}
			} else if (nextstep != map[get_position()].get_immovable())
				throw wexception
					("MO(%u): [transfer]: road to flag, but flag is nowhere near",
					 serial());

			set_location(dynamic_cast<Flag *>(nextstep));
			set_animation(game, descr().get_animation("idle"));
			schedule_act(game, 10); //  wait a little
		} else
			throw wexception
				("MO(%u): [transfer]: from road to bad nextstep %u",
				 serial(), nextstep->serial());
	} else
		throw wexception
			("MO(%u): location %u has bad type",
			 serial(), location->serial());
}


void Worker::transfer_signalimmediate
	(Game &, State & state, std::string const & signal)
{
	if (signal == "cancel")
		state.transfer = 0; //  do not call transfer_fail/finish when cancelled
}


/**
 * Called by transport code when the transfer has been cancelled & destroyed.
 */
void Worker::cancel_task_transfer(Game & game)
{
	send_signal(game, "cancel");
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
void Worker::start_task_buildingwork(Game & game)
{
	push_task(game, taskBuildingwork);
	top_state().ivar1 = 0;
}


void Worker::buildingwork_update(Game & game, State & state)
{
	// Reset any signals that are not related to location
	std::string signal = get_signal();
	signal_handled();

	if (state.ivar1 == 1)
		state.ivar1 = (signal == "fail") * 2;

	// Return to building, if necessary
	upcast(Building, building, get_location(game));
	if (!building)
		return pop_task(game);

	if (game.map().get_immovable(get_position()) != building)
		return start_task_return(game, false); //  do not drop item

	// Get the new job
	bool const success = state.ivar1 != 2;

	// Set this *before* calling to get_building_work, because the
	// state pointer might become invalid
	state.ivar1 = 1;

	if (not building->get_building_work(game, *this, success)) {
		set_animation(game, 0);
		return skip_act();
	}
}


/**
 * Wake up the buildingwork task if it was sleeping.
 * Otherwise, the buildingwork task will update as soon as the previous task
 * is finished.
 */
void Worker::update_task_buildingwork(Game & game)
{
	if (top_state().task == &taskBuildingwork)
		send_signal(game, "update");
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
void Worker::start_task_return(Game & game, bool const dropitem)
{
	PlayerImmovable * const location = get_location(game);

	if (!location || location->get_type() != BUILDING)
		throw wexception
			("MO(%u): start_task_return(): not owned by building", serial());

	push_task(game, taskReturn);
	top_state().ivar1 = dropitem ? 1 : 0;
}


void Worker::return_update(Game & game, State & state)
{
	std::string signal = get_signal();

	if (signal == "location") {
		molog("[return]: Interrupted by signal '%s'\n", signal.c_str());
		return pop_task(game);
	}

	signal_handled();

	Building & location = dynamic_cast<Building &>(*get_location(game));
	if (BaseImmovable * const pos = game.map().get_immovable(get_position())) {
		if (pos == &location) {
			set_animation(game, 0);
			return pop_task(game);
		}

		if (upcast(Flag, flag, pos)) {
			// Is this "our" flag?
			if (flag->get_building() == &location) {
				if (state.ivar1 && flag->has_capacity())
					if (WareInstance * const item = fetch_carried_item(game)) {
						flag->add_item(game, *item);
						set_animation(game, descr().get_animation("idle"));
						return schedule_act(game, 20); //  rest a while
					}

				return
					start_task_move
						(game,
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
		 	(game,
		 	 location.base_flag().get_position(),
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
void Worker::start_task_program(Game & game, std::string const & programname)
{
	push_task(game, taskProgram);
	State & state = top_state();
	state.program = descr().get_program(programname);
	state.ivar1 = 0;
}


void Worker::program_update(Game & game, State & state)
{
	if (get_signal().size()) {
		molog("[program]: Interrupted by signal '%s'\n", get_signal().c_str());
		return pop_task(game);
	}

	for (;;) {
		WorkerProgram const * program =
			static_cast<WorkerProgram const *>(state.program);

		if (static_cast<uint32_t>(state.ivar1) >= program->get_size())
			return pop_task(game);

		Action const & action = *program->get_action(state.ivar1);

		if ((this->*(action.function))(game, state, action))
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
void Worker::start_task_gowarehouse(Game & game)
{
	assert(!m_supply);

	push_task(game, taskGowarehouse);
}


void Worker::gowarehouse_update(Game & game, State & state)
{
	PlayerImmovable * const location = get_location(game);

	if (!location) {
		send_signal(game, "location");
		return pop_task(game);
	}

	// Signal handling
	std::string signal = get_signal();

	if (signal.size()) {
		// if routing has failed, try a different warehouse/route on next update()
		if (signal == "fail") {
			molog("[gowarehouse]: caught 'fail'\n");
			signal_handled();
		} else if (signal == "transfer") {
			signal_handled();
		} else {
			molog("[gowarehouse]: cancel for signal '%s'\n", signal.c_str());
			return pop_task(game);
		}
	}

	if (dynamic_cast<Warehouse const *>(location)) {
		delete m_supply;
		m_supply = 0;

		schedule_incorporate(game);
		return;
	}

	// If we got a transfer, use it
	if (state.transfer) {
		Transfer * const t = state.transfer;

		state.transfer = 0;
		pop_task(game);
		return start_task_transfer(game, t);
	}

	if (!get_economy()->get_nr_warehouses()) {
		molog("[gowarehouse]: No warehouse left in Economy\n");
		return pop_task(game);
	}

	// Idle until we are assigned a transfer.
	// The delay length is rather arbitrary, but we need some kind of
	// check against disappearing warehouses, or the worker will just
	// idle on a flag until the end of days (actually, until either the
	// flag is removed or a warehouse connects to the Economy).
	if (!m_supply)
		m_supply = new IdleWorkerSupply(*this);
	return start_task_idle(game, get_animation("idle"), 1000);
}

void Worker::gowarehouse_signalimmediate
	(Game &, State &, std::string const & signal)
{
	if (signal == "transfer") {
		// We are assigned a transfer, make sure our supply disappears immediately
		// Otherwise, we might receive two transfers in a row.
		delete m_supply;
		m_supply = 0;
	}
}

void Worker::gowarehouse_pop(Game &, State &)
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
void Worker::start_task_dropoff(Game & game, WareInstance & item)
{
	set_carried_item(game, &item);
	push_task(game, taskDropoff);
}


void Worker::dropoff_update(Game & game, State &)
{
	std::string signal = get_signal();

	if (signal.size()) {
		molog("[dropoff]: Interrupted by signal '%s'\n", signal.c_str());
		return pop_task(game);
	}

	WareInstance * item = get_carried_item(game);
	BaseImmovable * const location = game.map()[get_position()].get_immovable();
#ifndef NDEBUG
	Building & ploc = dynamic_cast<Building &>(*get_location(game));
	assert(&ploc == location || &ploc.base_flag() == location);
#endif

	// Deliver the item
	if (item) {
		// We're in the building, walk onto the flag
		if (upcast(Building, building, location)) {
			if (start_task_waitforcapacity(game, building->base_flag()))
				return;

			return start_task_leavebuilding(game, false); //  exit throttle
		}

		// We're on the flag, drop the item and pause a little
		if (upcast(Flag, flag, location)) {
			if (flag->has_capacity()) {
				flag->add_item(game, *fetch_carried_item(game));

				set_animation(game, descr().get_animation("idle"));
				schedule_act(game, 50);
				return;
			}

			molog("[dropoff]: flag is overloaded\n");
			start_task_move
				(game,
				 WALK_NW,
				 &descr().get_right_walk_anims(does_carry_ware()),
				 true);
			return;
		}

		throw wexception
			("MO(%u): [dropoff]: not on building or on flag - fishy", serial());
	}

	// We don't have the item any more, return home
	if (location->get_type() == Map_Object::FLAG)
		return
			start_task_move
				(game,
				 WALK_NW,
				 &descr().get_right_walk_anims(does_carry_ware()),
				 true);

	if (location->get_type() != Map_Object::BUILDING)
		throw wexception
			("MO(%u): [dropoff]: not on building on return", serial());

	if (dynamic_cast<Warehouse const *>(location)) {
		schedule_incorporate(game);
		return;
	}

	// Our parent task should know what to do
	return pop_task(game);
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
void Worker::start_task_fetchfromflag(Game & game)
{
	push_task(game, taskFetchfromflag);
	top_state().ivar1 = 0;
}


void Worker::fetchfromflag_update(Game & game, State & state)
{
	PlayerImmovable & employer = *get_location(game);
	PlayerImmovable * const location =
		dynamic_cast<PlayerImmovable *>(game.map().get_immovable(get_position()));

	// If we haven't got the item yet, walk onto the flag
	if (!get_carried_item(game) && !state.ivar1) {
		if (dynamic_cast<Building const *>(location))
			return start_task_leavebuilding(game, false);

		state.ivar1 = 1; //  force return to building

		// The item has decided that it doesn't want to go to us after all
		// In order to return to the warehouse, we're switching to State_DropOff
		if
			(WareInstance * const item =
			 	dynamic_cast<Flag &>(*location).fetch_pending_item(game, employer))
			set_carried_item(game, item);

		set_animation(game, descr().get_animation("idle"));
		return schedule_act(game, 20);
	}

	// Go back into the building
	if (dynamic_cast<Flag const *>(location)) {
		molog("[fetchfromflag]: return to building\n");

		return
			start_task_move
				(game,
				 WALK_NW,
				 &descr().get_right_walk_anims(does_carry_ware()), true);
	}

	if (not dynamic_cast<Building const *>(location))
		throw wexception
			("MO(%u): [fetchfromflag]: building disappeared", serial());

	assert(location == &employer);

	molog("[fetchfromflag]: back home\n");

	if (WareInstance * const item = fetch_carried_item(game)) {
		item->set_location(game, location);
		item->update(game); //  this might remove the item and ack any requests
	}

	// We're back!
	if (dynamic_cast<Warehouse const *>(location)) {
		schedule_incorporate(game);
		return;
	}

	return pop_task(game); //  assume that our parent task knows what to do
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
bool Worker::start_task_waitforcapacity(Game & game, Flag & flag)
{
	if (flag.has_capacity())
		return false;

	push_task(game, taskWaitforcapacity);

	top_state().objvar1 = &flag;

	flag.wait_for_capacity(game, *this);

	return true;
}


void Worker::waitforcapacity_update(Game & game, State &)
{
	std::string signal = get_signal();

	if (signal.size()) {
		if (signal == "wakeup")
			signal_handled();
		return pop_task(game);
	}

	return skip_act(); //  wait indefinitely
}


void Worker::waitforcapacity_pop(Game & game, State & state)
{
	if (upcast(Flag, flag, state.objvar1.get(game)))
		flag->skip_wait_for_capacity(game, *this);
}


/**
 * Called when the flag we waited on has now got capacity left.
 * Return true if we actually woke up due to this.
 */
bool Worker::wakeup_flag_capacity(Game & game, Flag & flag)
{
	if (State const * const state = get_state())
		if (state->task == &taskWaitforcapacity) {
			molog("[waitforcapacity]: Wake up: flag capacity.\n");

			if (state->objvar1.get(game) != &flag)
				throw wexception
					("MO(%u): wakeup_flag_capacity: Flags do not match.", serial());

			send_signal(game, "wakeup");
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
void Worker::start_task_leavebuilding(Game & game, bool const changelocation)
{
	Building & building = dynamic_cast<Building &>(*get_location(game));

	// Set the wait task
	push_task(game, taskLeavebuilding);
	State & state = top_state();
	state.ivar1   = changelocation;
	state.objvar1 = &building;
}


void Worker::leavebuilding_update(Game & game, State & state)
{
	std::string signal = get_signal();

	if (signal == "wakeup")
		signal_handled();
	else if (signal.size())
		return pop_task(game);

	if (upcast(Building, building, game.map().get_immovable(get_position()))) {
		assert(building == state.objvar1.get(game));

		if (!building->leave_check_and_wait(game, *this))
			return skip_act();

		if (state.ivar1)
			set_location(&building->base_flag());

		return
			start_task_move
				(game,
				 WALK_SE,
				 &descr().get_right_walk_anims(does_carry_ware()),
				 true);
	} else
		return pop_task(game);
}


void Worker::leavebuilding_pop(Game & game, State & state)
{
	// As of this writing, this is only really necessary when the task
	// is interrupted by a signal. Putting this in the pop() method is just
	// defensive programming, in case leavebuilding_update() changes
	// in the future.
	if (upcast(Building, building, state.objvar1.get(game)))
		building->leave_skip(game, *this);
}


/**
 * Called when the given building allows us to leave it.
 * \return true if we actually woke up due to this.
 */
bool Worker::wakeup_leave_building(Game & game, Building & building)
{
	if (State const * const state = get_state())
		if (state->task == &taskLeavebuilding) {
			if (state->objvar1.get(game) != &building)
				throw wexception
					("MO(%u): [waitleavebuilding]: buildings do not match",
					 serial());

			send_signal(game, "wakeup");
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


void Worker::start_task_fugitive(Game & game)
{
	push_task(game, taskFugitive);

	// Fugitives survive for two to four minutes
	top_state().ivar1 =
		game.get_gametime() + 120000 + 200 * (game.logic_rand() % 600);
}

struct FindFlagWithPlayersWarehouse {
	FindFlagWithPlayersWarehouse(Player const & owner) : m_owner(owner) {}
	bool accept(BaseImmovable * const imm) const {
		if (upcast(Flag const, flag, imm))
			if (&flag->owner() == &m_owner)
				if (flag->economy().get_nr_warehouses())
					return true;
		return false;
	}
private:
	Player const & m_owner;
};

void Worker::fugitive_update(Game & game, State & state)
{
	if (get_signal().size()) {
		molog("[fugitive]: interrupted by signal '%s'\n", get_signal().c_str());
		return pop_task(game);
	}

	Map & map = game.map();
	PlayerImmovable *location = get_location(game);

	if (location && &location->owner() == &owner()) {
		molog("[fugitive]: we are on location\n");

		if (dynamic_cast<Warehouse const *>(location))
			return schedule_incorporate(game);

		set_location(0);
		location = 0;
	}

	// check whether we're on a flag and it's time to return home
	if (upcast(Flag, flag, map[get_position()].get_immovable())) {
		if (&flag->owner() == &owner() and flag->economy().get_nr_warehouses()) {
			set_location(flag);
			return pop_task(game);
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

		container_iterate_const(std::vector<ImmovableFound>, flags, i) {
			Flag & flag = dynamic_cast<Flag &>(*i.current->object);

			int32_t const dist =
				map.calc_distance(get_position(), i.current->coords);

			if (!best || dist < bestdist) {
				best = &flag;
				bestdist = dist;
			}
		}

		if
			(best and
			 static_cast<int32_t>(game.logic_rand() % 30) <= 30 - bestdist)
		{
			molog("[fugitive]: try to move to flag\n");

			//  \todo FIXME ??? \todo
			//  warehouse could be on a different island, so check for failure
			if
				(start_task_movepath
				 	(game,
				 	 best->get_position(),
				 	 0,
				 	 descr().get_right_walk_anims(does_carry_ware())))
				return;
		}
	}

	if (state.ivar1 < game.get_gametime()) { //  time to die?
		molog("[fugitive]: die\n");
		return schedule_destroy(game);
	}

	molog("[fugitive]: wander randomly\n");

	if
		(start_task_movepath
		 	(game,
		 	 game.random_location(get_position(), vision_range()),
		 	 4,
		 	 descr().get_right_walk_anims(does_carry_ware())))
		return;

	return start_task_idle(game, descr().get_animation("idle"), 50);
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
	(Game & game,
	 uint8_t const attempts, uint8_t const radius,
	 std::string const & subcommand)
{
	push_task(game, taskGeologist);
	State & state = top_state();
	state.ivar1   = attempts;
	state.ivar2   = radius;
	state.svar1   = subcommand;
}


void Worker::geologist_update(Game & game, State & state)
{
	std::string signal = get_signal();

	if (signal == "fail") {
		molog("[geologist]: Caught signal '%s'\n", signal.c_str());
		signal_handled();
	} else if (signal.size()) {
		molog("[geologist]: Interrupted by signal '%s'\n", signal.c_str());
		return pop_task(game);
	}

	//
	Map & map = game.map();
	const World & world = map.world();
	Area<FCoords> owner_area
		(map.get_fcoords
		 	(dynamic_cast<Flag &>(*get_location(game)).get_position()),
		 state.ivar2);

	// Check if it's time to go home
	if (state.ivar1 > 0) {
		// Check to see if we're on suitable terrain
		BaseImmovable * const imm = map.get_immovable(get_position());

		if
			(not imm
			 or
			 (imm->get_size() == BaseImmovable::NONE
			  and
			  not imm->has_attribute(RESI)))
		{
			--state.ivar1;
			return start_task_program(game, state.svar1);
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
			uint32_t n = list.size();
			uint32_t i = game.logic_rand() % list.size();
			do {
				target =
					map.get_fcoords(list[game.logic_rand() % list.size()]);
				is_target_mountain =
					(world.terrain_descr(target.field->terrain_d()).get_is()
					 &
					 TERRAIN_MOUNTAIN)
					|
					(world.terrain_descr(target.field->terrain_r()).get_is()
					 &
					 TERRAIN_MOUNTAIN);
				if (i == 0)
					i = list.size();
				--i;
				--n;
			} while ((is_center_mountain != is_target_mountain) && n);

			if (!n) {
				// no suitable field found, this is no fail, there's just
				// nothing else to do so let's go home
				// FALLTHROUGH TO RETURN HOME
			} else {
				if
					(!
					 start_task_movepath
					 	(game,
					 	 target,
					 	 0,
					 	 descr().get_right_walk_anims(does_carry_ware())))
				{

					molog("[geologist]: BUG: could not find path\n");
					send_signal(game, "fail");
					return pop_task(game);
				}
				return;
			}
		}

		state.ivar1 = 0;
	}

	if (get_position() == owner_area)
		return pop_task(game);

	if
		(not
		 start_task_movepath
		 	(game, owner_area, 0, descr().get_right_walk_anims(does_carry_ware())))
	{
		molog("[geologist]: could not find path home\n");
		send_signal(game, "fail");
		return pop_task(game);
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

	if (WareInstance const * const carried_item = get_carried_item(game))
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
