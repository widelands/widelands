/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "logic/worker.h"

#include <boost/format.hpp>

#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/portdock.h"
#include "economy/road.h"
#include "economy/transfer.h"
#include "gamecontroller.h"
#include "graphic/rendertarget.h"
#include "helper.h"
#include "logic/carrier.h"
#include "logic/checkstep.h"
#include "logic/cmd_incorporate.h"
#include "logic/critter_bob.h"
#include "logic/dismantlesite.h"
#include "logic/findbob.h"
#include "logic/findimmovable.h"
#include "logic/findnode.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/mapfringeregion.h"
#include "logic/message_queue.h"
#include "logic/player.h"
#include "logic/soldier.h"
#include "logic/tribe.h"
#include "logic/warehouse.h"
#include "logic/worker_program.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "map_io/widelands_map_map_object_saver.h"
#include "profile/profile.h"
#include "sound/sound_handler.h"
#include "upcast.h"
#include "wexception.h"

namespace Widelands {

/**
 * createware \<waretype\>
 *
 * The worker will create and carry an ware of the given type.
 *
 * sparam1 = ware name
 */
bool Worker::run_createware(Game & game, State & state, const Action & action)
{

	if (WareInstance * const ware = fetch_carried_ware(game)) {
		molog("  Still carrying a ware! Delete it.\n");
		ware->schedule_destroy(game);
	}

	Player & player = *get_owner();
	Ware_Index const wareid(static_cast<Ware_Index::value_t>(action.iparam1));
	WareInstance & ware =
		*new WareInstance(wareid, tribe().get_ware_descr(wareid));
	ware.init(game);

	set_carried_ware(game, &ware);

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
bool Worker::run_mine(Game & game, State & state, const Action & action)
{
	Map & map = game.map();

	//Make sure that the specified resource is available in this world
	Resource_Index const res =
		map.get_world()->get_resource(action.sparam1.c_str());
	if (static_cast<int8_t>(res) == -1) //  FIXME ARGH!!
		throw game_data_error
			(_
			 	("should mine resource %s, which does not exist in world; tribe "
			 	 "is not compatible with world"),
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

	// Second pass through fields - reset mr
	pick = game.logic_rand() % totalchance;
	mr = MapRegion<Area<FCoords> >
		(map, Area<FCoords>(map.get_fcoords(get_position()), action.iparam1));
	do {
		uint8_t fres  = mr.location().field->get_resources();
		if (fres != res) {
			continue;
		}

		uint32_t amount = mr.location().field->get_resources_amount();

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
 * FIXME: in FindNodeResourceBreedable, the node (or neighbors) is accepted if it is breedable.
 * In here, breeding may happen on a node emptied of resource.
 * \todo Lots of magic numbers in here
 * \todo Document parameters g and state
 */
bool Worker::run_breed(Game & game, State & state, const Action & action)
{
	molog(" Breed(%s, %i)\n", action.sparam1.c_str(), action.iparam1);

	Map & map = game.map();

	//Make sure that the specified resource is available in this world
	Resource_Index const res =
		map.get_world()->get_resource(action.sparam1.c_str());
	if (static_cast<int8_t>(res) == -1) //  FIXME ARGH!!
		throw game_data_error
			(_
			 	("should breed resource type %s, which does not exist in world; "
			 	 "tribe is not compatible with world"),
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

	// Second pass through fields - reset mr!
	assert(totalchance);
	pick = game.logic_rand() % totalchance;
	mr = MapRegion<Area<FCoords> >
		(map, Area<FCoords>(map.get_fcoords(get_position()), action.iparam1));

	do {
		uint8_t fres  = mr.location().field->get_resources();
		if (fres != res)
			continue;

		uint32_t amount =
			mr.location().field->get_starting_res_amount() -
			mr.location().field->get_resources_amount   ();

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

	molog("  Successfully bred\n");

	// Advance program state
	++state.ivar1;
	schedule_act(game, 10);
	return true;
}


/**
 * OUTDATED - SHOULD NOT BE USED ANYMORE AND DOES NOT DO ANYTHING VALUEABLE
 *    just kept here for savegame compatibility for Build15 and earlier
 *
 * setdescription \<immovable name\> \<immovable name\> ...
 *
 * sparamv = possible bobs
 */
bool Worker::run_setdescription
	(Game & game, State & state, const Action &)
{
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
	(Game & game, State & state, const Action & action)
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
bool Worker::run_findobject(Game & game, State & state, const Action & action)
{
	CheckStepWalkOn cstep(descr().movecaps(), false);

	Map & map = game.map();
	Area<FCoords> area (map.get_fcoords(get_position()), 0);
	if (action.sparam1 == "immovable") {
		bool found_reserved = false;

		for (;; ++area.radius) {
			if (action.iparam1 < area.radius) {
				send_signal(game, "fail"); //  no object found, cannot run program
				pop_task(game);
				if (!found_reserved)
					informPlayer
						(game,
						 ref_cast<Building, PlayerImmovable>(*get_location(game)),
						 Map_Object_Descr::get_attribute_name(action.iparam2));
				return true;
			}
			std::vector<ImmovableFound> list;
			if (action.iparam2 < 0)
				map.find_reachable_immovables
					(area, &list, cstep);
			else
				map.find_reachable_immovables
					(area, &list, cstep, FindImmovableAttribute(action.iparam2));

			for (int idx = list.size() - 1; idx >= 0; idx--) {
				if (upcast(Immovable, imm, list[idx].object)) {
					if (imm->is_reserved_by_worker()) {
						found_reserved = true;
						list.erase(list.begin() + idx);
					}
					else
					{
						Coords const coord = imm->get_position();
						Map_Index mapidx = map.get_index(coord, map.get_width());
						Vision const visible = owner().vision(mapidx);
						if (!visible) {
							list.erase(list.begin() + idx);
						}
					}
				}
			}

			if (!list.empty()) {
				set_program_objvar
					(game, state, list[game.logic_rand() % list.size()].object);
				break;
			}
		}
	} else {
		for (;; ++area.radius) {
			if (action.iparam1 < area.radius) {
				send_signal(game, "fail"); //  no object found, cannot run program
				pop_task(game);
				informPlayer
					(game,
					 ref_cast<Building, PlayerImmovable>(*get_location(game)),
					 Map_Object_Descr::get_attribute_name(action.iparam2));
				return true;
			}
			std::vector<Bob *> list;
			if (action.iparam2 < 0)
				map.find_reachable_bobs
					(area, &list, cstep);
			else
				map.find_reachable_bobs
					(area, &list, cstep, FindBobAttribute(action.iparam2));

			if (!list.empty()) {
				set_program_objvar
					(game, state, list[game.logic_rand() % list.size()]);
				break;
			}
		}
	}

	++state.ivar1;
	schedule_act(game, 10);
	return true;
}



/**
 * findspace key:value key:value ...
 *
 * Find a node based on a number of predicates.
 * The node can later be used in other commands, e.g. walk.
 *
 * Predicates:
 * radius:\<dist\>
 * Search for nodes within the given radius around the worker.
 *
 * size:[any|build|small|medium|big|mine|port]
 * Search for fields with the given amount of space.
 *
 * resource:\<resname\>
 * Resource to search for. This is mainly intended for fisher and
 * therelike (non detectable Resources and default resources)
 *
 * space
 * Find only nodes that are walkable such that all neighbours
 * are also walkable (an exception is made if one of the neighbouring
 * fields is owned by this worker's location).
 * FIXME This is an embarrasingly ugly hack to make bug #1796611 happen less
 * FIXME often. But it gives no passability guarantee (that workers will not
 * FIXME get locked in). For example one farmer may call findspace and then,
 * FIXME before he plants anything, another farmer may call findspace, which
 * FIXME may find a space without considering that the first farmer will plant
 * FIXME something. Together they can cause a passability problem. This code
 * FIXME will also allow blocking the shoreline if it is next to the worker's
 * FIXME location. Also, the gap of 2 nodes between 2 farms will be blocked,
 * FIXME because both are next to their farm. The only real solution that I can
 * FIXME think of for this kind of bugs is to only allow unpassable objects to
 * FIXME be placed on a node if ALL neighbouring nodes are passable. This must
 * FIXME of course be checked at the moment when the object is placed and not,
 * FIXME as in this case, only before a worker starts walking there to place an
 * FIXME object. But that would make it very difficult to find space for things
 * FIXME like farm fileds. So our only option seems to be to keep all farm
 * FIXME fields, trees, stones and such on triangles and keep the nodes
 * FIXME passable. See code structure issue #1096824.
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

	bool accept(const Map & map, const FCoords & coords) const {
		if (!(coords.field->nodecaps() & MOVECAPS_WALK))
			return false;

		for (uint8_t dir = FIRST_DIRECTION; dir <= LAST_DIRECTION; ++dir) {
			FCoords const neighb = map.get_neighbour(coords, dir);

			if
				(!(neighb.field->nodecaps() & MOVECAPS_WALK) &&
				 neighb.field->get_immovable() != ignoreimmovable)
				return false;
		}

		return true;
	}

private:
	BaseImmovable * ignoreimmovable;
};

bool Worker::run_findspace(Game & game, State & state, const Action & action)
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

	if (action.iparam5 > -1)
		functor.add(FindNodeImmovableAttribute(action.iparam5), true);

	if (action.iparam3)
		functor.add(FindNodeSpace(get_location(game)));

	if (!map.find_reachable_fields(area, &list, cstep, functor)) {
		molog("  no space found\n");

		informPlayer
			(game,
			 ref_cast<Building, PlayerImmovable>(*get_location(game)),
			 action.sparam1);

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

// Informs the player about a building that cannot find resources any more,
void Worker::informPlayer
	(Game & game, Building & building, std::string res_type) const
{
	// NOTE this is just an ugly hack for now, to avoid getting messages
	// NOTE for farms, reed yards, vineyards, etc.
	if ((res_type != "fish") && (res_type != "stone"))
		return;
	// NOTE  AND fish_breeders
	if (building.name() == "fish_breeders_house")
		return;

	// TODO "stone" is defined as "granit" in the worlds
	if (res_type == "stone") res_type = "granit";

	// Translate the Resource name (if it is defined by the world)
	const World & world = game.map().world();
	int32_t residx = world.get_resource(res_type.c_str());
	if (residx != -1)
		res_type = world.get_resource(residx)->descname();

	building.send_message
		(game,
		 "mine",
		 (boost::format(_("Out of %s")) % res_type).str(),
		 (boost::format(_("The worker of this building cannot find any more %s.")) % res_type).str(),
		 true,
		 1800000, 0);
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
bool Worker::run_walk(Game & game, State & state, const Action & action)
{
	BaseImmovable const * const imm = game.map()[get_position()].get_immovable();
	Coords dest(Coords::Null());
	bool forceonlast = false;
	int32_t max_steps = -1;

	// First of all, make sure we're outside
	if (imm == &ref_cast<Building, PlayerImmovable>(*get_location(game))) {
		start_task_leavebuilding(game, false);
		return true;
	}

	// Determine the coords we need to walk towards
	if (action.iparam1 & Action::walkObject) {
		Map_Object * const obj = state.objvar1.get(game);

		if (obj) {
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
		}
	}
	if (!dest && (action.iparam1 & Action::walkCoords)) {
		dest = state.coords;
	}
	if (!dest) {
		send_signal(game, "fail");
		pop_task(game);
		return true;
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
bool Worker::run_animation(Game & game, State & state, const Action & action)
{
	set_animation(game, action.iparam1);

	++state.ivar1;
	schedule_act(game, action.iparam2);
	return true;
}



/**
 * Return home, drop any ware we're carrying onto our building's flag.
 *
 * iparam1 = 0: don't drop ware on flag, 1: do drop ware on flag
 */
bool Worker::run_return(Game & game, State & state, const Action & action)
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
bool Worker::run_object(Game & game, State & state, const Action & action)
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
bool Worker::run_plant(Game & game, State & state, const Action & action)
{
	assert(action.sparamv.size());

	if (action.iparam1 == Action::plantUnlessObject) {
		if (state.objvar1.get(game)) {
			// already have an object, so don't create a new one
			++state.ivar1;
			schedule_act(game, 10);
			return true;
		}
	}

	Map & map = game.map();
	Coords pos = get_position();
	FCoords fpos = map.get_fcoords(pos);

	// Check if the map is still free here
	if (BaseImmovable const * const imm = map[pos].get_immovable())
		if (imm->get_size() >= BaseImmovable::SMALL) {
			molog("  field no longer free\n");
			send_signal(game, "fail");
			pop_task(game);
			return true;
		}

	std::vector<int32_t> best_fitting;
	std::vector<bool> is_tribe_specific;
	uint32_t terrain_suitability = 0;
	for (uint8_t i = 0; i < action.sparamv.size(); ++i) {
		std::vector<std::string> const list(split_string(action.sparamv[i], ":"));
		std::string immovable;

		if (list.size() == 1) {
			state.svar1 = "world";
			immovable = list[0];
			state.ivar2 = map.world().get_immovable_index(immovable.c_str());
			if (state.ivar2 > 0) {
				Immovable_Descr const * imm =
					map.world().get_immovable_descr(state.ivar2);
				uint32_t suits = imm->terrain_suitability(fpos, map);
				// Remove existing, if this immovable suits better
				if (suits > terrain_suitability) {
					best_fitting.clear();
					is_tribe_specific.clear();
				}
				if (suits >= terrain_suitability) {
					terrain_suitability = suits;
					best_fitting.push_back(state.ivar2);
					is_tribe_specific.push_back(false);
				}
				continue;
			}
		} else {
			state.svar1 = "tribe";
			immovable = list[1];
			state.ivar2 = descr().tribe().get_immovable_index(immovable.c_str());
			if (state.ivar2 > 0) {
				Immovable_Descr const * imm =
					descr().tribe().get_immovable_descr(state.ivar2);
				uint32_t suits = imm->terrain_suitability(fpos, map);
				// Remove existing, if this immovable suits better
				if (suits > terrain_suitability) {
					best_fitting.clear();
					is_tribe_specific.clear();
				}
				if (suits >= terrain_suitability) {
					terrain_suitability = suits;
					best_fitting.push_back(state.ivar2);
					is_tribe_specific.push_back(true);
				}
				continue;
			}
		}

		// Only here if immovable was not found
		molog("  WARNING: Unknown immovable %s\n", action.sparamv[i].c_str());
		send_signal(game, "fail");
		pop_task(game);
		return true;
	}

	assert(best_fitting.size() == is_tribe_specific.size());
	if (best_fitting.empty()) {
		molog("  WARNING: No suitable immovable found!");
		send_signal(game, "fail");
		pop_task(game);
		return true;
	}
	uint32_t const idx = game.logic_rand() % best_fitting.size();

	Immovable & newimm = game.create_immovable
		(pos, best_fitting[idx], is_tribe_specific[idx] ? &descr().tribe() : nullptr);
	newimm.set_owner(get_owner());

	if (action.iparam1 == Action::plantUnlessObject)
		state.objvar1 = &newimm;

	++state.ivar1;
	schedule_act(game, 10);
	return true;
}


/**
 * Plants a bob (critter usually, maybe also worker later on). The immovable
 * type must have been selected by a previous command (i.e. setbobdescription).
 */
bool Worker::run_create_bob(Game & game, State & state, const Action &)
{
	game.create_bob
		(get_position(), state.ivar2, state.svar1 == "world" ? nullptr : &tribe());
	++state.ivar1;
	schedule_act(game, 10);
	return true;
}


/**
 * Simply remove the currently selected object - make no fuss about it.
 */
bool Worker::run_removeobject(Game & game, State & state, const Action &)
{
	if (Map_Object * const obj = state.objvar1.get(game)) {
		obj->remove(game);
		state.objvar1 = nullptr;
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
bool Worker::run_geologist(Game & game, State & state, const Action & action)
{
	// assert that location is of the right type.
	ref_cast<Flag const, PlayerImmovable const>(*get_location(game));

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
bool Worker::run_geologist_find(Game & game, State & state, const Action &)
{
	const Map & map = game.map();
	const FCoords position = map.get_fcoords(get_position());
	BaseImmovable const * const imm = position.field->get_immovable();
	const World & world = map.world();

	if (imm && imm->get_size() > BaseImmovable::NONE) {
		//NoLog("  Field is no longer empty\n");
	} else if
		(const Resource_Descr * const rdescr =
		 	world.get_resource(position.field->get_resources()))
	{
		// Geologist also sends a message notifying the player
		if (rdescr->is_detectable() && position.field->get_resources_amount()) {
			char message[1024];
			snprintf
				(message, sizeof(message),
				 "<rt image=%sresources/%s_1f.png>"
				 "<p font-size=14 font-face=DejaVuSerif>%s</p></rt>",
				 world.basedir().c_str(), rdescr->name().c_str(),
				 _("A geologist found resources."));

			//  We should add a message to the player's message queue - but only,
			//  if there is not already a similar one in list.
			owner().add_message_with_timeout
				(game,
				 *new Message
				 	("geologist " + rdescr->name(), // e.g. "geologist gold"
				 	 game.get_gametime(), 60 * 60 * 1000,
				 	 rdescr->descname(),
				 	 message,
				 	 position,
					 m_serial
					),
				 300000, 8);
		}

		const Tribe_Descr & t = tribe();
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
bool Worker::run_playFX(Game & game, State & state, const Action & action)
{
	g_sound_handler.play_fx(action.sparam1, get_position(), action.iparam1);

	++state.ivar1;
	schedule_act(game, 10);
	return true;
}

/**
 * If we are currently carrying some ware ware, hand it off to the currently
 * selected immovable (\ref objvar1) for construction.
 */
bool Worker::run_construct(Game & game, State & state, const Action & /* action */)
{
	Immovable * imm = dynamic_cast<Immovable *>(state.objvar1.get(game));
	if (!imm) {
		molog("run_construct: no objvar1 immovable set");
		send_signal(game, "fail");
		pop_task(game);
		return true;
	}

	WareInstance * ware = get_carried_ware(game);
	if (!ware) {
		molog("run_construct: no ware being carried");
		send_signal(game, "fail");
		pop_task(game);
		return true;
	}

	Ware_Index wareindex = ware->descr_index();
	if (!imm->construct_ware(game, wareindex)) {
		molog("run_construct: construct_ware failed");
		send_signal(game, "fail");
		pop_task(game);
		return true;
	}

	//update consumption statistic
	owner().ware_consumed(wareindex, 1);

	ware = fetch_carried_ware(game);
	ware->remove(game);

	++state.ivar1;
	schedule_act(game, 10);
	return true;
}


Worker::Worker(const Worker_Descr & worker_descr)
	:
	Bob          (worker_descr),
	m_economy    (nullptr),
	m_supply     (nullptr),
	m_transfer   (nullptr),
	m_current_exp(0)
{
}

Worker::~Worker()
{
	assert(!m_location.is_set());
	assert(!m_transfer);
}


/// Log basic information.
void Worker::log_general_info(const Editor_Game_Base & egbase)
{
	Bob::log_general_info(egbase);

	if (upcast(PlayerImmovable, loc, m_location.get(egbase))) {
		molog("* Owner: (%p)\n", &loc->owner());
		molog("** Owner (plrnr): %i\n", loc->owner().player_number());
		molog("* Economy: %p\n", loc->get_economy());
	}

	PlayerImmovable * imm = m_location.get(egbase);
	molog("location: %u\n", imm ? imm->serial() : 0);
	molog("Economy: %p\n", m_economy);
	molog("transfer: %p\n",  m_transfer);

	if (upcast(WareInstance, ware, m_carried_ware.get(egbase))) {
		molog
			("* m_carried_ware->get_ware() (id): %i\n",
			 ware->descr_index().value());
		molog("* m_carried_ware->get_economy() (): %p\n", ware->get_economy());
	}

	molog
		("m_current_exp: %i / %i\n",
		 m_current_exp, descr().get_level_experience());

	molog("m_supply: %p\n", m_supply);
}


/**
 * Change the location. This should be called in the following situations:
 * \li worker creation (usually, location is a warehouse)
 * \li worker moves along a route (location is a road and finally building)
 * \li current location is destroyed (building burnt down etc...)
 */
void Worker::set_location(PlayerImmovable * const location)
{
	assert(not location or Object_Ptr(location).get(owner().egbase()));

	PlayerImmovable * const oldlocation = get_location(owner().egbase());
	if (oldlocation == location)
		return;

	if (oldlocation) {
		// Note: even though we have an oldlocation, m_economy may be zero
		// (oldlocation got deleted)
		oldlocation->remove_worker(*this);
	} else {
		if (!is_shipping()) {
			assert(!m_economy);
		}
	}

	m_location = location;

	if (location) {
		Economy * const eco = location->get_economy();

		// NOTE we have to explicitly check Worker_Descr::SOLDIER, as SOLDIER is
		// NOTE as well defined in an enum in instances.h
		if (!m_economy || (get_worker_type() == Worker_Descr::SOLDIER)) {
			set_economy(eco);
		} else if (m_economy != eco) {
			throw wexception
				("Worker::set_location changes economy, but worker is no soldier");
		}
		location->add_worker(*this);
	} else {
		if (!is_shipping()) {
			// Our location has been destroyed, we are now fugitives.
			// Interrupt whatever we've been doing.
			set_economy(nullptr);

			Editor_Game_Base & egbase = owner().egbase();
			if (upcast(Game, game, &egbase)) {
				send_signal (*game, "location");
			}
		}
	}
}


/**
 * Change the worker's current economy. This is called:
 * \li by set_location() when appropriate
 * \li by the current location, when the location's economy changes
 */
void Worker::set_economy(Economy * const economy)
{
	if (economy == m_economy)
		return;

	if (m_economy)
		m_economy->remove_workers
			(descr().tribe().worker_index(name().c_str()), 1);

	m_economy = economy;

	if (WareInstance * const ware = get_carried_ware(owner().egbase()))
		ware->set_economy(m_economy);
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
		create_needed_experience(*game);
}


/**
 * Remove the worker.
 */
void Worker::cleanup(Editor_Game_Base & egbase)
{
	WareInstance * const ware = get_carried_ware(egbase);

	if (m_supply) {
		delete m_supply;
		m_supply = nullptr;
	}

	if (ware)
		if (egbase.objects().object_still_available(ware))
			ware->destroy(egbase);

	// We are destroyed, but we were maybe idling
	// or doing something else. Get Location might
	// init a gowarehouse task or something and this results
	// in a dirty stack. Nono, we do not want to end like this
	if (upcast(Game, game, &egbase))
		reset_tasks(*game);

	if (get_location(egbase))
		set_location(nullptr);

	set_economy(nullptr);

	Bob::cleanup(egbase);
}

/**
 * Set the ware we carry.
 * If we carry an ware right now, it will be destroyed (see
 * fetch_carried_ware()).
 */
void Worker::set_carried_ware
	(Editor_Game_Base & egbase, WareInstance * const ware)
{
	if (WareInstance * const oldware = get_carried_ware(egbase)) {
		oldware->cleanup(egbase);
		delete oldware;
	}

	m_carried_ware = ware;
	ware->set_location(egbase, this);
	if (upcast(Game, game, &egbase))
		ware->update(*game);
}


/**
 * Stop carrying the current ware, and return a pointer to it.
 */
WareInstance * Worker::fetch_carried_ware(Editor_Game_Base & game)
{
	WareInstance * const ware = get_carried_ware(game);

	if (ware) {
		ware->set_location(game, nullptr);
		m_carried_ware = nullptr;
	}

	return ware;
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
 */
void Worker::incorporate(Game & game)
{
	if (upcast(Warehouse, wh, get_location(game))) {
		wh->incorporate_worker(game, this);
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
void Worker::create_needed_experience(Game & /* game */)
{
	if (descr().get_level_experience() == -1) {
		m_current_exp = -1;
		return;
	}

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
		descr().get_level_experience() == -1 ||
		++m_current_exp < descr().get_level_experience() ?
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
	if (PlayerImmovable * location = get_location(game)) {
		if
			(get_economy()->warehouses().size() ||
			 location->get_type() == BUILDING)
			return start_task_gowarehouse(game);

		set_location(nullptr);
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
	nullptr,
	static_cast<Bob::Ptr>(&Worker::transfer_pop),
	false
};


/**
 * Tell the worker to follow the Transfer
 */
void Worker::start_task_transfer(Game & game, Transfer * t)
{
	// Hackish override for receiving transfers during gowarehouse,
	// and to correctly handle the stack during loading of games
	// (in that case, the transfer task already exists on the stack
	// when this is called).
	if (get_state(taskGowarehouse) || get_state(taskTransfer)) {
		assert(!m_transfer);

		m_transfer = t;
		send_signal(game, "transfer");
	} else { //  just start a normal transfer
		push_task(game, taskTransfer);
		m_transfer = t;
	}
}

void Worker::transfer_pop(Game & /* game */, State & /* state */)
{
	if (m_transfer) {
		m_transfer->has_failed();
		m_transfer = nullptr;
	}
}

void Worker::transfer_update(Game & game, State & /* state */) {
	Map & map = game.map();
	PlayerImmovable * location = get_location(game);

	// We expect to always have a location at this point,
	// but this assumption may fail when loading a corrupted savegame.
	if (!location) {
		send_signal(game, "location");
		return pop_task(game);
	}

	// The request is no longer valid, the task has failed
	if (!m_transfer) {
		molog("[transfer]: Fail (without transfer)\n");

		send_signal(game, "fail");
		return pop_task(game);
	}

	// Signal handling
	std::string const signal = get_signal();

	if (signal.size()) {
		// The caller requested a route update, or the previously calculated route
		// failed.
		// We will recalculate the route on the next update().
		if (signal == "road" || signal == "fail" || signal == "transfer" || signal == "wakeup") {
			molog("[transfer]: Got signal '%s' -> recalculate\n", signal.c_str());

			signal_handled();
		} else if (signal == "blocked") {
			molog("[transfer]: Blocked by a battle\n");

			signal_handled();
			return start_task_idle(game, get_animation("idle"), 500);
		} else {
			molog("[transfer]: Cancel due to signal '%s'\n", signal.c_str());
			return pop_task(game);
		}
	}

	// If our location is a building, our position may be somewhere else:
	// We may be on the building's flag for a fetch_from_flag or dropoff task.
	// We may also be somewhere else entirely (e.g. lumberjack, soldier).
	// Similarly for flags.
	if (upcast(Building, building, location)) {
		if (building->get_position() != get_position())
			return start_task_leavebuilding(game, true);
	} else if (upcast(Flag, flag, location)) {
		BaseImmovable * const position = map[get_position()].get_immovable();

		if (position != flag) {
			if (position == flag->get_building()) {
				location = flag->get_building();
				set_location(location);
			} else
				return set_location(nullptr);
		}
	}

	// Figure out where to go
	bool success;
	PlayerImmovable * const nextstep =
		m_transfer->get_next_step(location, success);

	if (!nextstep) {
		Transfer * const t = m_transfer;

		m_transfer = nullptr;

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
		if (&building->base_flag() != nextstep) {
			if (upcast(Warehouse, warehouse, building)) {
				if (warehouse->get_portdock() == nextstep)
					return start_task_shipping(game, warehouse->get_portdock());
			}

			throw wexception("MO(%u): [transfer]: in building, nextstep is not building's flag", serial());
		}

		return start_task_leavebuilding(game, true);
	} else if (upcast(Flag,     flag,     location)) {
		if        (upcast(Building, nextbuild, nextstep)) { //  Flag to Building
			if (&nextbuild->base_flag() != location)
				throw wexception("MO(%u): [transfer]: next step is building, but we are nowhere near", serial());

			return start_task_move(game, WALK_NW, descr().get_right_walk_anims(does_carry_ware()), true);
		} else if (upcast(Flag,     nextflag,  nextstep)) { //  Flag to Flag
			Road & road = *flag->get_road(*nextflag);

			Path path(road.get_path());

			if (nextstep != &road.get_flag(Road::FlagEnd))
				path.reverse();

			molog("[transfer]: starting task [movepath] and setting location to road %u\n", road.serial());
			start_task_movepath(game, path, descr().get_right_walk_anims(does_carry_ware()));
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
			const Path & path = road->get_path();
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
						("[transfer]: from road %u to flag %u\n",
						 road->serial(), nextstep->serial());
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


/**
 * Called by transport code when the transfer has been cancelled & destroyed.
 */
void Worker::cancel_task_transfer(Game & game)
{
	m_transfer = nullptr;
	send_signal(game, "cancel");
}


/**
 * Sleep while the shipping code in @ref PortDock and @ref Ship handles us.
 */
const Bob::Task Worker::taskShipping = {
	"shipping",
	static_cast<Bob::Ptr>(&Worker::shipping_update),
	nullptr,
	static_cast<Bob::Ptr>(&Worker::shipping_pop),
	true
};

/**
 * Start the shipping task. If pd != nullptr, add us as a shipping item. We
 * could be an expedition worker though, so we will not be a shipping item
 * though.
 *
 * ivar1 = end shipping?
 */
void Worker::start_task_shipping(Game & game, PortDock* pd)
{
	push_task(game, taskShipping);
	top_state().ivar1 = 0;
	if (pd)
		pd->add_shippingitem(game, *this);
}

/**
 * Trigger the end of the shipping task.
 *
 * @note the worker must be in a @ref Warehouse location
 */
void Worker::end_shipping(Game & game)
{
	if (State * state = get_state(taskShipping)) {
		state->ivar1 = 1;
		send_signal(game, "endshipping");
	}
}

/**
 * Whether we are currently being handled by the shipping code.
 */
bool Worker::is_shipping()
{
	return get_state(taskShipping);
}

void Worker::shipping_pop(Game & game, State & /* state */)
{
	// Defense against unorderly cleanup via reset_tasks
	if (!get_location(game)) {
		set_economy(nullptr);
	}
}


void Worker::shipping_update(Game & game, State & state)
{
	PlayerImmovable * location = get_location(game);

	// Signal handling
	const std::string signal = get_signal();

	if (signal.size()) {
		if (signal == "endshipping") {
			signal_handled();
			if (!dynamic_cast<Warehouse *>(location)) {
				molog("shipping_update: received signal 'endshipping' while not in warehouse!\n");
				pop_task(game);
				return;
			}
		}
		if (signal == "transfer" || signal == "wakeup")
			signal_handled();
	}

	if (location || state.ivar1) {
		if (upcast(PortDock, pd, location)) {
			pd->update_shippingitem(game, *this);
		} else {
			return pop_task(game);
		}
	}

	start_task_idle(game, 0, -1);
}


/**
 * Endless loop, in which the worker calls the owning building's
 * get_building_work() function to initiate subtasks.
 * The signal "update" is used to wake the worker up after a sleeping time
 * (initiated by a false return value from get_building_work()).
 *
 * ivar1 - 0: no task has failed; 1: currently in buildingwork;
 *         2: signal failure of buildingwork
 * ivar2 - whether the worker is to be evicted
 */
const Bob::Task Worker::taskBuildingwork = {
	"buildingwork",
	static_cast<Bob::Ptr>(&Worker::buildingwork_update),
	nullptr,
	nullptr,
	true
};


/**
 * Begin work at a building.
 */
void Worker::start_task_buildingwork(Game & game)
{
	push_task(game, taskBuildingwork);
	State & state = top_state();
	state.ivar1 = 0;
}


void Worker::buildingwork_update(Game & game, State & state)
{
	// Reset any signals that are not related to location
	std::string signal = get_signal();
	signal_handled();
	if (signal == "evict") {
		return pop_task(game);
	}

	if (state.ivar1 == 1)
		state.ivar1 = (signal == "fail") * 2;

	// Return to building, if necessary
	upcast(Building, building, get_location(game));
	if (!building)
		return pop_task(game);

	if (game.map().get_immovable(get_position()) != building)
		return start_task_return(game, false); //  do not drop ware

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
 * Evict the worker from its current building.
 */
void Worker::evict(Game & game)
{
	if (is_evict_allowed()) {
		send_signal(game, "evict");
	}
}

bool Worker::is_evict_allowed()
{
	return true;
}

/**
 * Return to our owning building.
 * If dropware (ivar1) is true, we'll drop our carried ware (if any) on the
 * building's flag, if possible.
 * Blocks all signals except for "location".
 */
const Bob::Task Worker::taskReturn = {
	"return",
	static_cast<Bob::Ptr>(&Worker::return_update),
	nullptr,
	nullptr,
	true
};


/**
 * Return to our owning building.
 */
void Worker::start_task_return(Game & game, bool const dropware)
{
	PlayerImmovable * const location = get_location(game);

	if (!location || location->get_type() != BUILDING)
		throw wexception
			("MO(%u): start_task_return(): not owned by building", serial());

	push_task(game, taskReturn);
	top_state().ivar1 = dropware ? 1 : 0;
}


void Worker::return_update(Game & game, State & state)
{
	std::string signal = get_signal();

	if (signal == "location") {
		molog("[return]: Interrupted by signal '%s'\n", signal.c_str());
		return pop_task(game);
	}

	signal_handled();

	Building * location = dynamic_cast<Building *>(get_location(game));

	if (!location) {
		// Usually, this should be caught via the "location" signal above.
		// However, in certain cases, e.g. for a soldier during battle,
		// the location may be overwritten by a different signal while
		// walking home.
		molog("[return]: Our location disappeared from under us\n");
		return pop_task(game);
	}

	if (BaseImmovable * const pos = game.map().get_immovable(get_position())) {
		if (pos == location) {
			set_animation(game, 0);
			return pop_task(game);
		}

		if (upcast(Flag, flag, pos)) {
			// Is this "our" flag?
			if (flag->get_building() == location) {
				if (state.ivar1 && flag->has_capacity()) {
					if (WareInstance * const ware = fetch_carried_ware(game)) {
						flag->add_ware(game, *ware);
						set_animation(game, descr().get_animation("idle"));
						return schedule_act(game, 20); //  rest a while
					}
				}

				// Don't try to enter building if it is a dismantle site
				// It is no problem for builders since they won't return before
				// dismantling is complete.
				if (is_a(DismantleSite, location)) {
					set_location(nullptr);
					return pop_task(game);
				} else {
					return
						start_task_move
							(game,
							WALK_NW,
							descr().get_right_walk_anims(does_carry_ware()),
							true);
				}
			}
		}
	}

	// Determine the building's flag and move to it

	if
		(not
		 start_task_movepath
		 	(game,
		 	 location->base_flag().get_position(),
		 	 15,
		 	 descr().get_right_walk_anims(does_carry_ware())))
	{
		molog("[return]: Failed to return\n");
		char buffer[2048];
		snprintf
			(buffer, sizeof(buffer),
			 _ ("Your %s can’t find a way home and will likely die."),
			 descname().c_str());
		owner().add_message
			(game,
			 *new Message
			 	("game engine",
			 	 game.get_gametime(), Forever(),
			 	 _("Worker got lost!"),
			 	 buffer,
			 	 get_position()),
				 m_serial);
		set_location(nullptr);
		return pop_task(game);
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
	nullptr,
	static_cast<Bob::Ptr>(&Worker::program_pop),
	false
};


/**
 * Start the given program.
 */
void Worker::start_task_program(Game & game, const std::string & programname)
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

	if (!state.program) {
		// This might happen as fallout of some save game compatibility fix
		molog("[program]: No program active\n");
		send_signal(game, "fail");
		return pop_task(game);
	}

	for (;;) {
		const WorkerProgram & program =
			ref_cast<WorkerProgram const, BobProgramBase const>(*state.program);

		if (static_cast<uint32_t>(state.ivar1) >= program.get_size())
			return pop_task(game);

		const Action & action = *program.get_action(state.ivar1);

		if ((this->*(action.function))(game, state, action))
			return;
	}
}

void Worker::program_pop(Game & game, State & state)
{
	set_program_objvar(game, state, nullptr);
}

void Worker::set_program_objvar(Game & game, State & state, Map_Object * obj)
{
	assert(state.task == &taskProgram);

	if (upcast(Immovable, imm, state.objvar1.get(game))) {
		imm->set_reserved_by_worker(false);
	}

	state.objvar1 = obj;

	if (upcast(Immovable, imm, obj)) {
		imm->set_reserved_by_worker(true);
	}
}

const Bob::Task Worker::taskGowarehouse = {
	"gowarehouse",
	static_cast<Bob::Ptr>(&Worker::gowarehouse_update),
	static_cast<Bob::PtrSignal>(&Worker::gowarehouse_signalimmediate),
	static_cast<Bob::Ptr>(&Worker::gowarehouse_pop),
	true
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


void Worker::gowarehouse_update(Game & game, State & /* state */)
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
		if (signal == "fail" || signal == "cancel") {
			molog("[gowarehouse]: caught '%s'\n", signal.c_str());
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
		m_supply = nullptr;

		schedule_incorporate(game);
		return;
	}

	// If we got a transfer, use it
	if (m_transfer) {
		Transfer * const t = m_transfer;
		m_transfer = nullptr;

		molog("[gowarehouse]: Got transfer\n");

		pop_task(game);
		return start_task_transfer(game, t);
	}

	// Always leave buildings in an orderly manner,
	// even when no warehouses are left to return to
	if (location->get_type() == BUILDING)
		return start_task_leavebuilding(game, true);

	if (!get_economy()->warehouses().size()) {
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
	(Game &, State & /* state */, const std::string & signal)
{
	if (signal == "transfer") {
		// We are assigned a transfer, make sure our supply disappears immediately
		// Otherwise, we might receive two transfers in a row.
		delete m_supply;
		m_supply = nullptr;
	}
}

void Worker::gowarehouse_pop(Game &, State &)
{
	delete m_supply;
	m_supply = nullptr;

	if (m_transfer) {
		m_transfer->has_failed();
		m_transfer = nullptr;
	}
}


const Bob::Task Worker::taskDropoff = {
	"dropoff",
	static_cast<Bob::Ptr>(&Worker::dropoff_update),
	nullptr,
	nullptr,
	true
};

const Bob::Task Worker::taskReleaserecruit = {
	"releaserecruit",
	static_cast<Bob::Ptr>(&Worker::releaserecruit_update),
	nullptr,
	nullptr,
	true
};

/**
 * Walk to the building's flag, drop the given ware, and walk back inside.
 */
void Worker::start_task_dropoff(Game & game, WareInstance & ware)
{
	set_carried_ware(game, &ware);
	push_task(game, taskDropoff);
}


void Worker::dropoff_update(Game & game, State &)
{
	std::string signal = get_signal();

	if (signal.size()) {
		molog("[dropoff]: Interrupted by signal '%s'\n", signal.c_str());
		return pop_task(game);
	}

	WareInstance * ware = get_carried_ware(game);
	BaseImmovable * const location = game.map()[get_position()].get_immovable();
#ifndef NDEBUG
	Building & ploc = ref_cast<Building, PlayerImmovable>(*get_location(game));
	assert(&ploc == location || &ploc.base_flag() == location);
#endif

	// Deliver the ware
	if (ware) {
		// We're in the building, walk onto the flag
		if (upcast(Building, building, location)) {
			if (start_task_waitforcapacity(game, building->base_flag()))
				return;

			return start_task_leavebuilding(game, false); //  exit throttle
		}

		// We're on the flag, drop the ware and pause a little
		if (upcast(Flag, flag, location)) {
			if (flag->has_capacity()) {
				flag->add_ware(game, *fetch_carried_ware(game));

				set_animation(game, descr().get_animation("idle"));
				return schedule_act(game, 50);
			}

			molog("[dropoff]: flag is overloaded\n");
			start_task_move
				(game,
				 WALK_NW,
				 descr().get_right_walk_anims(does_carry_ware()),
				 true);
			return;
		}

		throw wexception
			("MO(%u): [dropoff]: not on building or on flag - fishy", serial());
	}

	// We don't have the ware any more, return home
	if (location->get_type() == Map_Object::FLAG)
		return
			start_task_move
				(game,
				 WALK_NW,
				 descr().get_right_walk_anims(does_carry_ware()),
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


/// Give the recruit his diploma and say farwell to him.
void Worker::start_task_releaserecruit(Game & game, Worker & recruit)
{
	push_task(game, taskReleaserecruit);
	molog
		("Starting to release %s %u...\n",
		 recruit.descname().c_str(), recruit.serial());
	return schedule_act(game, 5000);
}

void Worker::releaserecruit_update(Game & game, State &)
{
	molog("\t...done releasing recruit\n");
	return pop_task(game);
}

/**
 * ivar1 is set to 0 if we should move to the flag and fetch the ware, and it
 * is set to 1 if we should move into the building.
 */
const Bob::Task Worker::taskFetchfromflag = {
	"fetchfromflag",
	static_cast<Bob::Ptr>(&Worker::fetchfromflag_update),
	nullptr,
	nullptr,
	true
};


/**
 * Walk to the building's flag, fetch an ware from the flag that is destined for
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

	// If we haven't got the ware yet, walk onto the flag
	if (!get_carried_ware(game) && !state.ivar1) {
		if (dynamic_cast<Building const *>(location))
			return start_task_leavebuilding(game, false);

		state.ivar1 = 1; //  force return to building

		if (not location) {
			// this can happen if the flag (and the building) is destroyed while
			// the worker leaves the building.
			molog
				("[fetchfromflag]: flag dissappeared - become fugitive");
			return pop_task(game);
		}

		// The ware has decided that it doesn't want to go to us after all
		// In order to return to the warehouse, we're switching to State_DropOff
		if
			(WareInstance * const ware =
			 	ref_cast<Flag, PlayerImmovable>(*location).fetch_pending_ware
			 		(game, employer))
			set_carried_ware(game, ware);

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
				 descr().get_right_walk_anims(does_carry_ware()), true);
	}

	if (not dynamic_cast<Building const *>(location)) {
		// This can happen "naturally" if the building gets destroyed, but the
		// flag is still there and the worker tries to enter from that flag.
		// E.g. the player destroyed the building, it is destroyed, through an
		// enemy player, or it got destroyed through rising water (atlantean
		// scenario)
		molog("[fetchfromflag]: building dissappeared - searching for alternative\n");
		return pop_task(game);
	}

	assert(location == &employer);

	molog("[fetchfromflag]: back home\n");

	if (WareInstance * const ware = fetch_carried_ware(game)) {
		if (ware->get_next_move_step(game) == location) {
			ware->enter_building(game, *dynamic_cast<Building *>(location));
		} else {
			// The ware changed its mind and doesn't want to go to this building
			// after all, so carry it back out.
			// This can happen in the following subtle and rare race condition:
			// We start the fetchfromflag task as the worker in an enhanceable building.
			// While we walk back into the building with the ware, the player enhances
			// the building, so that we now belong to the newly created construction site.
			// Obviously the construction site no longer has any use for the ware.
			molog("[fetchfromflag]: ware no longer wants to go into building, drop off\n");
			pop_task(game);
			start_task_dropoff(game, *ware);
			return;
		}
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
	nullptr,
	static_cast<Bob::Ptr>(&Worker::waitforcapacity_pop),
	true
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
	nullptr,
	static_cast<Bob::Ptr>(&Worker::leavebuilding_pop),
	true
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
	Building & building =
		ref_cast<Building, PlayerImmovable>(*get_location(game));

	// Set the wait task
	push_task(game, taskLeavebuilding);
	State & state = top_state();
	state.ivar1   = changelocation;
	state.objvar1 = &building;
}


void Worker::leavebuilding_update(Game & game, State & state)
{
	std::string const signal = get_signal();

	if (signal == "wakeup")
		signal_handled();
	else if (signal.size())
		return pop_task(game);

	upcast(Building, building, get_location(game));
	if (!building) {
		return pop_task(game);
	}

	Flag & baseflag = building->base_flag();

	if (get_position() == building->get_position()) {
		assert(building == state.objvar1.get(game));
		if (!building->leave_check_and_wait(game, *this))
			return skip_act();

		if (state.ivar1)
			set_location(&baseflag);

		return
			start_task_move
				(game,
				 WALK_SE,
				 descr().get_right_walk_anims(does_carry_ware()),
				 true);
	} else {
		const Coords & flagpos = baseflag.get_position();

		if (state.ivar1)
			set_location(&baseflag);

		if (get_position() == flagpos)
			return pop_task(game);

		if (!start_task_movepath(game, flagpos, 0, descr().get_right_walk_anims(does_carry_ware()))) {
			molog("[leavebuilding]: outside of building, but failed to walk back to flag");
			set_location(nullptr);
			return pop_task(game);
		}
		return;
	}
}


void Worker::leavebuilding_pop(Game & game, State & state)
{
	// As of this writing, this is only really necessary when the task
	// is interrupted by a signal. Putting this in the pop() method is just
	// defensive programming, in case leavebuilding_update() changes
	// in the future.
	//
	//  The if-statement is needed because this is (unfortunately) also called
	//  when the Worker is deallocated when shutting down the simulation. Then
	//  the building might not exist any more.
	if (Map_Object * const building = state.objvar1.get(game))
		ref_cast<Building, Map_Object>(*building).leave_skip(game, *this);
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
	nullptr,
	nullptr,
	true
};


void Worker::start_task_fugitive(Game & game)
{
	push_task(game, taskFugitive);

	// Fugitives survive for two to four minutes
	top_state().ivar1 =
		game.get_gametime() + 120000 + 200 * (game.logic_rand() % 600);
}

struct FindFlagWithPlayersWarehouse {
	FindFlagWithPlayersWarehouse(const Player & owner) : m_owner(owner) {}
	bool accept(const BaseImmovable & imm) const {
		if (upcast(Flag const, flag, &imm))
			if (&flag->owner() == &m_owner)
				if (flag->economy().warehouses().size())
					return true;
		return false;
	}
private:
	const Player & m_owner;
};

void Worker::fugitive_update(Game & game, State & state)
{
	if (get_signal().size()) {
		molog("[fugitive]: interrupted by signal '%s'\n", get_signal().c_str());
		return pop_task(game);
	}

	Map & map = game.map();
	PlayerImmovable const * location = get_location(game);

	if (location && &location->owner() == &owner()) {
		molog("[fugitive]: we are on location\n");

		if (dynamic_cast<Warehouse const *>(location))
			return schedule_incorporate(game);

		set_location(nullptr);
		location = nullptr;
	}

	// check whether we're on a flag and it's time to return home
	if (upcast(Flag, flag, map[get_position()].get_immovable())) {
		if (&flag->owner() == &owner() and flag->economy().warehouses().size()) {
			set_location(flag);
			return pop_task(game);
		}
	}

	// Try to find a flag connected to a warehouse that we can return to
	//
	// We always have a high probability to see flags within our vision range,
	// but with some luck we see flags that are even further away.
	std::vector<ImmovableFound> flags;
	int32_t vision = vision_range();
	int32_t maxdist = 4 * vision;
	if
		(map.find_immovables
		 	(Area<FCoords>(map.get_fcoords(get_position()), maxdist),
		 	 &flags, FindFlagWithPlayersWarehouse(*get_owner())))
	{
		int32_t bestdist = -1;
		Flag *  best     =  nullptr;

		molog("[fugitive]: found a flag connected to warehouse(s)\n");

		container_iterate_const(std::vector<ImmovableFound>, flags, i) {
			Flag & flag = ref_cast<Flag, BaseImmovable>(*i.current->object);

			if (game.logic_rand() % 2 == 0)
				continue;

			int32_t const dist =
				map.calc_distance(get_position(), i.current->coords);

			if (!best || bestdist > dist) {
				best = &flag;
				bestdist = dist;
			}
		}

		if (best && bestdist > vision) {
			uint32_t chance = maxdist - (bestdist - vision);
			if (game.logic_rand() % maxdist >= chance)
				best = nullptr;
		}

		if (best) {
			molog("[fugitive]: try to move to flag\n");

			// Warehouse could be on a different island, so check for failure
			// Also, move only a few number of steps in the right direction,
			// so that we could theoretically lose the flag again, but also
			// perhaps find a closer flag.
			if
				(start_task_movepath
				 	(game,
				 	 best->get_position(),
				 	 0,
				 	 descr().get_right_walk_anims(does_carry_ware()),
				 	 false,
				 	 4))
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
	nullptr,
	nullptr,
	true
};


void Worker::start_task_geologist
	(Game & game,
	 uint8_t const attempts, uint8_t const radius,
	 const std::string & subcommand)
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
		 	(ref_cast<Flag, PlayerImmovable>(*get_location(game)).get_position()),
		 state.ivar2);

	// Check if it's not time to go home
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
			assert(n);
			uint32_t i = game.logic_rand() % n;
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

/**
 * Look at fields that are in the fog of war around our owner.
 *
 * ivar1 - radius to start searching
 * ivar2 - time to spend
 *
 * Failure of path movement is caught, all other signals terminate this task.
 */
const Bob::Task Worker::taskScout = {
	"scout",
	static_cast<Bob::Ptr>(&Worker::scout_update),
	nullptr,
	nullptr,
	true
};


/**
 * scout \<radius\> \<time\>
 *
 * Find a spot that is in the fog of war and go there to see what's up.
 *
 * iparam1 = radius where the scout initially searches for unseen fields
 * iparam2 = maximum search time (in msecs)
 */
bool Worker::run_scout(Game & game, State & state, const Action & action)
{
	molog
		("  Try scouting for %i ms with search in radius of %i\n",
		 action.iparam2, action.iparam1);

	++state.ivar1;
	start_task_scout(game, action.iparam1, action.iparam2);
	// state reference may be invalid now
	return true;
}

void Worker::start_task_scout
	(Game & game, uint16_t const radius, uint32_t const time)
{
	push_task(game, taskScout);
	State & state = top_state();
	state.ivar1   = radius;
	state.ivar2   = game.get_gametime() + time;

	// first get out
	Building & building =
		ref_cast<Building, PlayerImmovable>(*get_location(game));
	push_task(game, taskLeavebuilding);
	State & stateLeave = top_state();
	stateLeave.ivar1 = false;
	stateLeave.objvar1 = &building;
}


void Worker::scout_update(Game & game, State & state)
{
	std::string signal = get_signal();
	molog("  Update Scout (%i time)\n", state.ivar2);

	if (signal.size()) {
		molog("[scout]: Interrupted by signal '%s'\n", signal.c_str());
		return pop_task(game);
	}

	Map & map = game.map();

	// If not yet time to go home
	if (static_cast<int32_t>(state.ivar2 - game.get_gametime()) > 0) {
		std::vector<Coords> list; //< List of interesting points
		CheckStepDefault cstep(descr().movecaps());
		FindNodeAnd ffa;
		ffa.add(FindNodeImmovableSize(FindNodeImmovableSize::sizeNone), false);
		Area<FCoords> exploring_area
			(map.get_fcoords(get_position()), state.ivar1);
		Coords oldest_coords = get_position();
		Time oldest_time = game.get_gametime();
		uint8_t oldest_distance = 0;

		// if some fields can be reached
		if (map.find_reachable_fields(exploring_area, &list, cstep, ffa) > 0) {
			// Parse randomly the reachable fields, maximum 50 iterations
			uint8_t iterations = list.size() % 51;
			for (uint8_t i = 0; i < iterations; ++i) {
				const std::vector<Coords>::size_type lidx = game.logic_rand() % list.size();
				Coords const coord = list[lidx];
				list.erase(list.begin() + lidx);
				Map_Index idx = map.get_index(coord, map.get_width());
				Vision const visible = owner().vision(idx);

				// If the field is not yet discovered, go there
				if (!visible) {
					molog
						("[scout]: Go to interesting field (%i, %i)\n",
						 coord.x, coord.y);
					if
						(!start_task_movepath(game, coord, 0,
						 descr().get_right_walk_anims(does_carry_ware())))
							molog("[scout]: failed to reach destination\n");
					else
						return; //start_task_movepath was successfull.
				}

				// Else evaluate for best second target
				int dist = map.calc_distance(coord, get_position());
				Time time = owner().fields()[idx].time_node_last_unseen;
				// time is only valid if visible is 1
				if (visible != 1)
					time = oldest_time;

				if
					(dist > oldest_distance
					 || (dist == oldest_distance and time < oldest_time))
				{
					oldest_distance = dist;
					oldest_time = time;
					oldest_coords = coord;
				}
			}
			// All fields discovered, go to second choice target


			if (oldest_coords != get_position()) {
				molog
				("[scout]: All fields discovered. Go to (%i, %i)\n",
				 oldest_coords.x, oldest_coords.y);

				if
					(!start_task_movepath
					 (game, oldest_coords, 0,
						  descr().get_right_walk_anims(does_carry_ware())))
						molog("[scout]: Failed to reach destination\n");
				else
					return; //Start task movepath success.
				//if failed go home
			}
		}
		// No reachable fields found.
		molog("[scout]: nowhere to go!\n");
	}
	// time to go home or found nothing to go to
	pop_task(game);
	schedule_act(game, 10);
	return;
}

void Worker::draw_inner
	(const Editor_Game_Base& game, RenderTarget& dst, const Point& drawpos)
	const
{
	dst.drawanim
		(drawpos,
		 get_current_anim(),
		 game.get_gametime() - get_animstart(),
		 get_owner());

	if (WareInstance const * const carried_ware = get_carried_ware(game))
		dst.drawanim
			(drawpos - descr().get_ware_hotspot(),
			 carried_ware->descr().get_animation("idle"),
			 0,
			 get_owner());
}


/**
 * Draw the worker, taking the carried ware into account.
 */
void Worker::draw
	(const Editor_Game_Base & game, RenderTarget & dst, const Point& pos) const
{
	if (get_current_anim())
		draw_inner(game, dst, calc_drawpos(game, pos));
}

/*
==============================

Load/save support

==============================
*/

#define WORKER_SAVEGAME_VERSION 2

Worker::Loader::Loader() :
	m_location(0),
	m_carried_ware(0)
{
}

void Worker::Loader::load(FileRead & fr)
{
	Bob::Loader::load(fr);

	uint8_t version = fr.Unsigned8();
	if (!(1 <= version && version <= WORKER_SAVEGAME_VERSION))
		throw game_data_error("unknown/unhandled version %u", version);

	Worker & worker = get<Worker>();
	m_location = fr.Unsigned32();
	m_carried_ware = fr.Unsigned32();
	worker.m_current_exp = fr.Signed32();

	if (version >= 2) {
		if (fr.Unsigned8()) {
			worker.m_transfer =
				new Transfer(ref_cast<Game, Editor_Game_Base>(egbase()), worker);
			worker.m_transfer->read(fr, m_transfer);
		}
	}
}

void Worker::Loader::load_pointers()
{
	Bob::Loader::load_pointers();

	Worker & worker = get<Worker>();

	if (m_location)
		worker.set_location(&mol().get<PlayerImmovable>(m_location));
	if (m_carried_ware)
		worker.m_carried_ware = &mol().get<WareInstance>(m_carried_ware);
	if (worker.m_transfer)
		worker.m_transfer->read_pointers(mol(), m_transfer);
}

void Worker::Loader::load_finish()
{
	Bob::Loader::load_finish();

	Worker & worker = get<Worker>();

	// If our economy is unclear because we have no location, it is wise to not
	// mess with it. For example ships will not be a location for Workers
	// (because they are no PlayerImmovable), but they will handle economies for
	// us and will do so on load too. To make the order at which we are loaded
	// not a factor, we do not overwrite the economy they might have set for us
	// already.
	if (PlayerImmovable * const location = worker.m_location.get(egbase())) {
		worker.set_economy(location->get_economy());
	}
}

const Bob::Task * Worker::Loader::get_task(const std::string & name)
{
	if (name == "program") return &taskProgram;
	if (name == "transfer") return &taskTransfer;
	if (name == "shipping") return &taskShipping;
	if (name == "buildingwork") return &taskBuildingwork;
	if (name == "return") return &taskReturn;
	if (name == "gowarehouse") return &taskGowarehouse;
	if (name == "dropoff") return &taskDropoff;
	if (name == "releaserecruit") return &taskReleaserecruit;
	if (name == "fetchfromflag") return &taskFetchfromflag;
	if (name == "waitforcapacity") return &taskWaitforcapacity;
	if (name == "leavebuilding") return &taskLeavebuilding;
	if (name == "fugitive") return &taskFugitive;
	if (name == "geologist") return &taskGeologist;
	if (name == "scout") return &taskScout;
	return Bob::Loader::get_task(name);
}

const BobProgramBase * Worker::Loader::get_program(const std::string & name)
{
	Worker & worker = get<Worker>();
	return worker.descr().get_program(name);
}

Worker::Loader * Worker::create_loader()
{
	return new Loader;
}

/**
 * Load function for all classes derived from \ref Worker
 *
 * Derived classes must override \ref create_loader to make sure
 * the appropriate actual load functions are called.
 */
Map_Object::Loader * Worker::load
	(Editor_Game_Base & egbase, Map_Map_Object_Loader & mol, FileRead & fr)
{
	try {
		// header has already been read by caller
		std::string tribename = fr.CString();
		std::string name = fr.CString();

		egbase.manually_load_tribe(tribename);

		const Tribe_Descr * tribe = egbase.get_tribe(tribename);
		if (!tribe)
			throw game_data_error("unknown tribe '%s'", tribename.c_str());

		const Worker_Descr * descr =
			tribe->get_worker_descr(tribe->safe_worker_index(name));

		Worker * worker = static_cast<Worker *>(&descr->create_object());
		std::unique_ptr<Loader> loader(worker->create_loader());
		loader->init(egbase, mol, *worker);
		loader->load(fr);
		return loader.release();
	} catch (const std::exception & e) {
		throw wexception("loading worker: %s", e.what());
	}

	return nullptr; // Should not be reached
}

/**
 * Save the \ref Worker specific header and version info.
 *
 * \warning Do not override this function, override \ref do_save instead.
 */
void Worker::save
	(Editor_Game_Base & egbase, Map_Map_Object_Saver & mos, FileWrite & fw)
{
	fw.Unsigned8(header_Worker);
	fw.CString(tribe().name());
	fw.CString(descr().name());

	do_save(egbase, mos, fw);
}

/**
 * Save the data fields of this worker.
 *
 * This is separate from \ref save because of the way data headers are treated.
 *
 * Override this function in derived classes.
 */
void Worker::do_save
	(Editor_Game_Base & egbase, Map_Map_Object_Saver & mos, FileWrite & fw)
{
	Bob::save(egbase, mos, fw);

	fw.Unsigned8(WORKER_SAVEGAME_VERSION);
	fw.Unsigned32(mos.get_object_file_index_or_zero(m_location.get(egbase)));
	fw.Unsigned32(mos.get_object_file_index_or_zero(m_carried_ware.get(egbase)));
	fw.Signed32(m_current_exp);

	if (m_transfer) {
		fw.Unsigned8(1);
		m_transfer->write(mos, fw);
	} else {
		fw.Unsigned8(0);
	}
}

}
