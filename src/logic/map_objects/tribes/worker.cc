/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "logic/map_objects/tribes/worker.h"

#include <memory>
#include <tuple>

#include "base/log.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/portdock.h"
#include "economy/road.h"
#include "economy/transfer.h"
#include "graphic/rendertarget.h"
#include "graphic/text_layout.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/cmd_incorporate.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/findbob.h"
#include "logic/map_objects/findimmovable.h"
#include "logic/map_objects/findnode.h"
#include "logic/map_objects/terrain_affinity.h"
#include "logic/map_objects/tribes/carrier.h"
#include "logic/map_objects/tribes/dismantlesite.h"
#include "logic/map_objects/tribes/market.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/map_objects/tribes/worker_program.h"
#include "logic/map_objects/world/critter.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/map_objects/world/terrain_description.h"
#include "logic/mapfringeregion.h"
#include "logic/mapregion.h"
#include "logic/message_queue.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"
#include "map_io/map_packet_versions.h"
#include "sound/note_sound.h"

namespace Widelands {

/**
 * createware=\<waretype\>
 *
 * The worker will create and carry an ware of the given type.
 *
 * sparam1 = ware name
 */
bool Worker::run_createware(Game& game, State& state, const Action& action) {

	if (WareInstance* const ware = fetch_carried_ware(game)) {
		molog(game.get_gametime(), "  Still carrying a ware! Delete it.\n");
		ware->schedule_destroy(game);
	}

	Player& player = *get_owner();
	DescriptionIndex const wareid(action.iparam1);
	WareInstance& ware = *new WareInstance(wareid, player.tribe().get_ware_descr(wareid));
	ware.init(game);

	set_carried_ware(game, &ware);

	// For statistics, inform the user that a ware was produced
	player.ware_produced(wareid);

	++state.ivar1;
	schedule_act(game, Duration(10));
	return true;
}

/**
 * Mine on the current coordinates for resources decrease, go home.
 *
 * Syntax in conffile: mine=\<resource\> \<area\>
 *
 * \param g
 * \param state
 * \param action Which resource to mine (action.sparam1) and where to look for
 * it (in a radius of action.iparam1 around current location)
 */
// TODO(unknown): Lots of magic numbers in here
bool Worker::run_mine(Game& game, State& state, const Action& action) {
	Map* map = game.mutable_map();

	// Make sure that the specified resource is available in this world
	DescriptionIndex const res = game.descriptions().resource_index(action.sparam1);
	if (res == Widelands::INVALID_INDEX) {
		throw GameDataError("should mine resource %s, which does not exist", action.sparam1.c_str());
	}

	// Select one of the fields randomly
	uint32_t totalres = 0;
	uint32_t totalchance = 0;
	int32_t pick;
	MapRegion<Area<FCoords>> mr(
	   *map, Area<FCoords>(map->get_fcoords(get_position()), action.iparam1));
	do {
		DescriptionIndex fres = mr.location().field->get_resources();
		ResourceAmount amount = mr.location().field->get_resources_amount();

		// In the future, we might want to support amount = 0 for
		// fields that can produce an infinite amount of resources.
		// Rather -1 or something similar. not 0
		if (fres != res) {
			amount = 0;
		}

		totalres += amount;
		totalchance += 8 * amount;

		// Add penalty for fields that are running out
		// Except for totally depleted fields or wrong ressource fields
		// if we already know there is no ressource (left) we won't mine there
		if (amount > 0) {
			if (amount <= 2) {
				totalchance += 6;
			} else if (amount <= 4) {
				totalchance += 4;
			} else if (amount <= 6) {
				totalchance += 2;
			}
		}
	} while (mr.advance(*map));

	if (totalres == 0) {
		molog(game.get_gametime(), "  Run out of resources\n");
		send_signal(game, "fail");  //  mine empty, abort program
		pop_task(game);
		return true;
	}

	// Second pass through fields - reset mr
	pick = game.logic_rand() % totalchance;
	mr = MapRegion<Area<FCoords>>(
	   *map, Area<FCoords>(map->get_fcoords(get_position()), action.iparam1));
	do {
		DescriptionIndex fres = mr.location().field->get_resources();
		if (fres != res) {
			continue;
		}

		ResourceAmount amount = mr.location().field->get_resources_amount();

		pick -= 8 * amount;
		if (pick < 0) {
			assert(amount > 0);

			--amount;
			map->set_resources(mr.location(), amount);
			break;
		}
	} while (mr.advance(*map));

	if (pick >= 0) {
		molog(game.get_gametime(), "  Not successful this time\n");
		send_signal(game, "fail");  //  not successful, abort program
		pop_task(game);
		return true;
	}

	// Advance program state
	++state.ivar1;
	schedule_act(game, Duration(10));
	return true;
}

/**
 * Breed on the current coordinates for resource increase, go home.
 *
 * Syntax in conffile: breed=\<resource\> \<area\>
 *
 * \param g
 * \param state
 * \param action Which resource to breed (action.sparam1) and where to put
 * it (in a radius of action.iparam1 around current location)
 */

// TODO(unknown): in FindNodeResourceBreedable, the node (or neighbors) is accepted if it is
// breedable.
// In here, breeding may happen on a node emptied of resource.
// TODO(unknown): Lots of magic numbers in here
// TODO(unknown): Document parameters g and state
bool Worker::run_breed(Game& game, State& state, const Action& action) {
	molog(game.get_gametime(), " Breed(%s, %i)\n", action.sparam1.c_str(), action.iparam1);

	Map* map = game.mutable_map();

	// Make sure that the specified resource is available in this world
	DescriptionIndex const res = game.descriptions().resource_index(action.sparam1);
	if (res == Widelands::INVALID_INDEX) {
		throw GameDataError(
		   "should breed resource type %s, which does not exist", action.sparam1.c_str());
	}

	// Select one of the fields randomly
	uint32_t totalres = 0;
	uint32_t totalchance = 0;
	int32_t pick;
	MapRegion<Area<FCoords>> mr(
	   *map, Area<FCoords>(map->get_fcoords(get_position()), action.iparam1));
	do {
		DescriptionIndex fres = mr.location().field->get_resources();
		ResourceAmount amount = mr.location().field->get_initial_res_amount() -
		                        mr.location().field->get_resources_amount();

		// In the future, we might want to support amount = 0 for
		// fields that can produce an infinite amount of resources.
		// Rather -1 or something similar. not 0
		if (fres != res) {
			amount = 0;
		}

		totalres += amount;
		totalchance += 8 * amount;

		// Add penalty for fields that are running out
		if (amount == 0) {
			// we already know it's completely empty, so punish is less
			++totalchance;
		} else if (amount <= 2) {
			totalchance += 6;
		} else if (amount <= 4) {
			totalchance += 4;
		} else if (amount <= 6) {
			totalchance += 2;
		}
	} while (mr.advance(*map));

	if (totalres == 0) {
		molog(game.get_gametime(), "  All resources full\n");
		send_signal(game, "fail");  //  no space for more, abort program
		pop_task(game);
		return true;
	}

	// Second pass through fields - reset mr!
	assert(totalchance);
	pick = game.logic_rand() % totalchance;
	mr = MapRegion<Area<FCoords>>(
	   *map, Area<FCoords>(map->get_fcoords(get_position()), action.iparam1));

	do {
		DescriptionIndex fres = mr.location().field->get_resources();
		if (fres != res) {
			continue;
		}

		ResourceAmount amount = mr.location().field->get_initial_res_amount() -
		                        mr.location().field->get_resources_amount();

		pick -= 8 * amount;
		if (pick < 0) {
			assert(amount > 0);

			--amount;
			map->set_resources(mr.location(), mr.location().field->get_initial_res_amount() - amount);
			break;
		}
	} while (mr.advance(*map));

	if (pick >= 0) {
		molog(game.get_gametime(), "  Not successful this time\n");
		send_signal(game, "fail");  //  not successful, abort program
		pop_task(game);
		return true;
	}

	molog(game.get_gametime(), "  Successfully bred\n");

	// Advance program state
	++state.ivar1;
	schedule_act(game, Duration(10));
	return true;
}

/**
 * findobject=key:value key:value ...
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
bool Worker::run_findobject(Game& game, State& state, const Action& action) {
	CheckStepWalkOn cstep(descr().movecaps(), false);

	const Map& map = game.map();

	// First try to look for immovables that were marked for removal by our player
	if (action.sparam1 == "immovable") {
		std::vector<ImmovableFound> list;
		Area<FCoords> area(map.get_fcoords(get_position()), action.iparam1);
		if (action.iparam2 < 0) {
			map.find_reachable_immovables(game, area, &list, cstep);
		} else {
			map.find_reachable_immovables(
			   game, area, &list, cstep, FindImmovableAttribute(action.iparam2));
		}
		for (auto it = list.begin(); it != list.end();) {
			if (it->object->is_reserved_by_worker() ||
			    !dynamic_cast<Immovable&>(*it->object)
			        .is_marked_for_removal(owner().player_number())) {
				it = list.erase(it);
			} else {
				++it;
			}
		}
		if (!list.empty()) {
			set_program_objvar(game, state, list[game.logic_rand() % list.size()].object);
			++state.ivar1;
			schedule_act(game, Duration(10));
			return true;
		}
	}

	Area<FCoords> area(map.get_fcoords(get_position()), 0);
	bool found_reserved = false;

	for (;; ++area.radius) {
		if (action.iparam1 < area.radius) {
			send_signal(game, "fail");  //  no object found, cannot run program
			pop_task(game);
			if (upcast(ProductionSite, productionsite, get_location(game))) {
				if (found_reserved) {
					productionsite->unnotify_player();
				} else if (action.iparam3 != 0) {
					productionsite->notify_player(game, 30);
				}
			}
			return true;
		}
		if (action.sparam1 == "immovable") {
			if (upcast(ProductionSite, productionsite, get_location(game))) {
				productionsite->unnotify_player();
			}
			std::vector<ImmovableFound> list;
			if (action.iparam2 < 0) {
				map.find_reachable_immovables(game, area, &list, cstep);
			} else {
				map.find_reachable_immovables(
				   game, area, &list, cstep, FindImmovableAttribute(action.iparam2));
			}

			for (int idx = list.size() - 1; idx >= 0; idx--) {
				if (upcast(Immovable, imm, list[idx].object)) {
					if (imm->is_reserved_by_worker()) {
						found_reserved = true;
						list.erase(list.begin() + idx);
					} else {
						Coords const coord = imm->get_position();
						MapIndex mapidx = map.get_index(coord, map.get_width());
						if (owner().get_vision(mapidx) == VisibleState::kUnexplored) {
							list.erase(list.begin() + idx);
						}
					}
				}
			}

			if (!list.empty()) {
				set_program_objvar(game, state, list[game.logic_rand() % list.size()].object);
				break;
			}
		} else {
			if (upcast(ProductionSite, productionsite, get_location(game))) {
				productionsite->unnotify_player();
			}
			std::vector<Bob*> list;
			if (action.iparam2 < 0) {
				map.find_reachable_bobs(game, area, &list, cstep);
			} else {
				map.find_reachable_bobs(game, area, &list, cstep, FindBobAttribute(action.iparam2));
			}

			for (int idx = list.size() - 1; idx >= 0; idx--) {
				if (upcast(MapObject, bob, list[idx])) {
					if (bob->is_reserved_by_worker()) {
						found_reserved = true;
						list.erase(list.begin() + idx);
					}
				}
			}
			if (!list.empty()) {
				set_program_objvar(game, state, list[game.logic_rand() % list.size()]);
				break;
			}
		}
	}
	++state.ivar1;
	schedule_act(game, Duration(10));
	return true;
}

/**
 * Care about the game.forester_cache_.
 *
 * Making the run_findspace routine shorter, by putting one special case into its own function.
 * This gets called many times each time the forester searches for a place for a sapling.
 * Since this already contains three nested for-loops, I dedided to cache the values for a
 * cpu/memory tradeoff (hint: Widelands is pretty heavy on my oldish PC).
 * Since the implementation details of double could vary between platforms, I decided to
 * quantize the terrain goodness into int16_t. This lowers the footprint of the caching,
 * and also makes desyncs caused by different floats horribly unlikely.
 *
 * The forester_cache_ is sparse, but then, lookups are fast.
 *
 * At the moment of writing, map changing is really infrequent (only in two scenarios)
 * and even those do not affect this. However, since map changes are possible, this
 * checks the reliability of the cached value with a small probability (~1%), If a
 * disparency is found, the entire cache is nuked.
 *
 * If somebody in the future makes a scenario, where the land first is barren, and then
 * spots of eden show up, the foresters will not immediately notice (because of the cache).
 * They will eventually notice, and since the instance is shared between tribes,
 * all foresters notice this at the same moment, also in network play. I hope this is okay.
 *
 */
int16_t Worker::findspace_helper_for_forester(const Coords& pos, const Map& map, Game& game) {

	std::vector<int16_t>& forester_cache = game.forester_cache_;
	const unsigned vecsize = 1 + unsigned(map.max_index());
	const MapIndex mi = map.get_index(pos, map.get_width());
	const FCoords fpos = map.get_fcoords(pos);
	// This if-statement should be true only once per game.
	if (vecsize != forester_cache.size()) {
		forester_cache.resize(vecsize, kInvalidForesterEntry);
	}
	int16_t cache_entry = forester_cache[mi];
	bool x_check = false;
	assert(cache_entry >= kInvalidForesterEntry);
	if (cache_entry != kInvalidForesterEntry) {
		if (0 == ((game.logic_rand()) & 0xfe)) {
			// Cached value found, but exceptionally not trusted.
			x_check = true;
		} else {
			// Found the terrain forestability, no more work to do
			return cache_entry;
		}
	}

	// Okay, I do not know whether this terrain suits. Let's obtain the value (and then cache it)

	const DescriptionMaintainer<ImmovableDescr>& immovables = game.descriptions().immovables();

	// TODO(kxq): could the tree_sapling come from config? Currently, there is only one sparam..
	// TODO(k.halfmann): avoid fetching this vlaues every time, as it is const during runtime?.
	// This code is only executed at cache miss.
	const uint32_t attribute_id = ImmovableDescr::get_attribute_id("tree_sapling");

	const DescriptionMaintainer<TerrainDescription>& terrains = game.descriptions().terrains();
	int best = 0;
	for (DescriptionIndex i = 0; i < immovables.size(); ++i) {
		const ImmovableDescr& immovable_descr = immovables.get(i);
		if (immovable_descr.has_attribute(attribute_id) && immovable_descr.has_terrain_affinity()) {
			int probability =
			   probability_to_grow(immovable_descr.terrain_affinity(), fpos, map, terrains);
			if (probability > best) {
				best = probability;
			}
		}
	}
	// normalize value to int16 range
	const int16_t correct_val = (std::numeric_limits<int16_t>::max() - 1) *
	                            (static_cast<double>(best) / TerrainAffinity::kPrecisionFactor);

	if (x_check && (correct_val != cache_entry)) {
		forester_cache.clear();
		forester_cache.resize(vecsize, kInvalidForesterEntry);
	}
	forester_cache[mi] = correct_val;
	return correct_val;
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
 *
 *
 * iparam1 = radius
 * iparam2 = FindNodeSize::sizeXXX
 * iparam3 = 1st bit: whether the "space" flag is set; 2nd bit: whether the no_notify flag is set
 * iparam4 = whether the "breed" flag is set
 * sparam1 = Resource
 */
// TODO(unknown): This is an embarrasingly ugly hack to make bug #1796611 happen less
// often. But it gives no passability guarantee (that workers will not
// get locked in). For example one farmer may call findspace and then,
// before he plants anything, another farmer may call findspace, which
// may find a space without considering that the first farmer will plant
// something. Together they can cause a passability problem. This code
// will also allow blocking the shoreline if it is next to the worker's
// location. Also, the gap of 2 nodes between 2 farms will be blocked,
// because both are next to their farm. The only real solution that I can
// think of for this kind of bugs is to only allow unwalkable objects to
// be placed on a node if ALL neighbouring nodes are passable. This must
// of course be checked at the moment when the object is placed and not,
// as in this case, only before a worker starts walking there to place an
// object. But that would make it very difficult to find space for things
// like farm fileds. So our only option seems to be to keep all farm
// fields, trees, rocks and such on triangles and keep the nodes
// passable. See code structure issue #1096824.
//
// If landbased_ is false, the behaviour is modified to instead accept the node
// only if *at least one* adjacent triangle has MOVECAPS_SWIM.
struct FindNodeSpace {
	explicit FindNodeSpace(bool land) : landbased_(land) {
	}

	bool accept(const EditorGameBase& egbase, const FCoords& coords) const {
		if (!(coords.field->nodecaps() & MOVECAPS_WALK)) {
			return false;
		}

		for (uint8_t dir = FIRST_DIRECTION; dir <= LAST_DIRECTION; ++dir) {
			FCoords const neighb = egbase.map().get_neighbour(coords, dir);
			if (landbased_) {
				if (!(neighb.field->maxcaps() & MOVECAPS_WALK)) {
					return false;
				}
			} else {
				if (neighb.field->nodecaps() & MOVECAPS_SWIM) {
					return true;
				}
			}
		}
		return landbased_;
	}

private:
	bool landbased_;
};

bool Worker::run_findspace(Game& game, State& state, const Action& action) {
	std::vector<Coords> list;
	const Map& map = game.map();
	const Descriptions& descriptions = game.descriptions();

	CheckStepDefault cstep(descr().movecaps());

	Area<FCoords> area(map.get_fcoords(get_position()), action.iparam1);

	FindNodeAnd functor;
	const FindNodeSize::Size findnodesize = static_cast<FindNodeSize::Size>(action.iparam2);
	functor.add(FindNodeSize(findnodesize));
	if (!action.sparam1.empty()) {
		if (action.iparam4) {
			functor.add(FindNodeResourceBreedable(descriptions.resource_index(action.sparam1)));
		} else {
			functor.add(FindNodeResource(descriptions.resource_index(action.sparam1)));
		}
	}

	if (action.iparam5 > -1) {
		functor.add(FindNodeImmovableAttribute(action.iparam5), true);
	}
	if (action.iparam3 & 1) {
		functor.add(FindNodeSpace(findnodesize != FindNodeSize::Size::sizeSwim));
	}
	for (const std::string& terraform : action.sparamv) {
		functor.add(FindNodeTerraform(terraform));
	}

	if (!map.find_reachable_fields(game, area, &list, cstep, functor)) {

		// This is default note "out of resources" sent to a player
		FailNotificationType fail_notification_type = FailNotificationType::kDefault;

		// In case this is a fishbreeder, we do more checks
		if (!action.sparam1.empty() && action.iparam4) {

			// We need to create create another functor that will look for nodes full of fish
			FindNodeAnd functorAnyFull;
			functorAnyFull.add(FindNodeSize(static_cast<FindNodeSize::Size>(action.iparam2)));
			functorAnyFull.add(FindNodeResourceBreedable(
			   descriptions.resource_index(action.sparam1), AnimalBreedable::kAnimalFull));

			if (action.iparam5 > -1) {
				functorAnyFull.add(FindNodeImmovableAttribute(action.iparam5), true);
			}
			if (action.iparam3 & 1) {
				functorAnyFull.add(FindNodeSpace(findnodesize != FindNodeSize::Size::sizeSwim));
			}
			// If there are fields full of fish, we change the type of notification
			if (map.find_reachable_fields(game, area, &list, cstep, functorAnyFull)) {
				fail_notification_type = FailNotificationType::kFull;
			}
		}
		switch (fail_notification_type) {
		case FailNotificationType::kFull:
			molog(game.get_gametime(), "  all reachable nodes are full\n");
			break;
		default:
			molog(game.get_gametime(), "  no space found\n");
		}

		if ((action.iparam3 & 2) == 0) {
			if (upcast(ProductionSite, productionsite, get_location(game))) {
				productionsite->notify_player(game, 30, fail_notification_type);
			}
		}

		send_signal(game, "fail");
		pop_task(game);
		return true;
	}
	if (upcast(ProductionSite, productionsite, get_location(game))) {
		productionsite->unnotify_player();
	}

	// Pick a location at random
	state.coords = list[game.logic_rand() % list.size()];

	// Special case: forester checks multiple locations instead of one.
	if (1 < action.iparam6) {
		// In the bug comments, somebody asked for unequal quality for the foresters of various
		// tribes to find the best spot. Here stubborness is the number of slots to consider.
		int stubborness = action.iparam6;
		int16_t best = findspace_helper_for_forester(state.coords, map, game);
		while (1 < stubborness--) {
			const Coords altpos = list[game.logic_rand() % list.size()];
			const int16_t alt_pos_goodness = findspace_helper_for_forester(altpos, map, game);
			if (alt_pos_goodness > best) {
				best = alt_pos_goodness;
				state.coords = altpos;
			}
		}
	}

	++state.ivar1;
	schedule_act(game, Duration(10));
	return true;
}

/**
 * walk=\<where\>
 *
 * Walk to a previously selected destination. where can be one of:
 * object  walk to a previously found and selected object
 * coords  walk to a previously found and selected field/coordinate
 *
 * iparam1 = walkXXX
 */
bool Worker::run_walk(Game& game, State& state, const Action& action) {
	BaseImmovable const* const imm = game.map()[get_position()].get_immovable();
	Coords dest(Coords::null());
	bool forceonlast = false;
	int32_t max_steps = -1;

	// First of all, make sure we're outside
	if (upcast(Building, b, get_location(game))) {
		if (b == imm) {
			start_task_leavebuilding(game, false);
			return true;
		}
	}

	// Determine the coords we need to walk towards
	if (action.iparam1 & Action::walkObject) {
		MapObject* obj = state.objvar1.get(game);

		if (obj) {
			if (upcast(Bob const, bob, obj)) {
				dest = bob->get_position();
			} else if (upcast(Immovable const, immovable, obj)) {
				dest = immovable->get_position();
			} else if (upcast(Flag, f, obj)) {
				// Special handling for flags: Go there by road using a Transfer
				if (f == imm) {
					// already there – call next program step
					++state.ivar1;
					return false;
				}
				Transfer* t = new Transfer(game, *this);
				t->set_destination(*f);
				start_task_transfer(game, t);
				return true;  // do not advance program yet
			} else {
				throw wexception("MO(%u): [actWalk]: bad object type %s", serial(),
				                 to_string(obj->descr().type()).c_str());
			}

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
		return false;  // next instruction
	}

	// Walk towards it
	if (!start_task_movepath(game, dest, 10, descr().get_right_walk_anims(does_carry_ware(), this),
	                         forceonlast, max_steps)) {
		molog(game.get_gametime(), "  could not find path\n");
		send_signal(game, "fail");
		pop_task(game);
		return true;
	}

	return true;
}

/**
 * animate=\<name\> \<duration\>
 *
 * Play the given animation for the given amount of time.
 *
 * iparam1 = anim id
 * iparam2 = duration
 */
bool Worker::run_animate(Game& game, State& state, const Action& action) {
	set_animation(game, action.iparam1);

	++state.ivar1;
	schedule_act(game, Duration(action.iparam2));
	return true;
}

/**
 * Return home, drop any ware we're carrying onto our building's flag.
 *
 * iparam1 = 0: don't drop ware on flag, 1: do drop ware on flag
 */
bool Worker::run_return(Game& game, State& state, const Action& action) {
	++state.ivar1;
	start_task_return(game, action.iparam1);
	return true;
}

/**
 * callobject=\<command\>
 *
 * Cause the currently selected object to execute the given program.
 *
 * sparam1 = object command name
 */
bool Worker::run_callobject(Game& game, State& state, const Action& action) {
	MapObject* const obj = state.objvar1.get(game);

	if (!obj) {
		send_signal(game, "fail");
		pop_task(game);
		return true;
	}

	if (upcast(Immovable, immovable, obj)) {
		immovable->switch_program(game, action.sparam1);
	} else if (upcast(Bob, bob, obj)) {
		if (upcast(Critter, crit, bob)) {
			crit->reset_tasks(game);  //  TODO(unknown): ask the critter more nicely
			crit->start_task_program(game, action.sparam1);
		} else if (upcast(Worker, w, bob)) {
			w->reset_tasks(game);  //  TODO(unknown): ask the worker more nicely
			w->start_task_program(game, action.sparam1);
		} else {
			throw wexception("MO(%i): [actObject]: bad bob type %s", serial(),
			                 to_string(bob->descr().type()).c_str());
		}
	} else {
		throw wexception("MO(%u): [actObject]: bad object type %s", serial(),
		                 to_string(obj->descr().type()).c_str());
	}

	++state.ivar1;
	schedule_act(game, Duration(10));
	return true;
}

/**
 * Plant an immovable on the current position. The immovable type must have
 * been selected by a previous command (i.e. plant)
 */
bool Worker::run_plant(Game& game, State& state, const Action& action) {
	assert(!action.sparamv.empty());

	if (action.iparam1 == Action::plantUnlessObject) {
		if (state.objvar1.get(game)) {
			// already have an object, so don't create a new one
			++state.ivar1;
			schedule_act(game, Duration(10));
			return true;
		}
	}

	const Map& map = game.map();
	Coords pos = get_position();
	FCoords fpos = map.get_fcoords(pos);

	// Check if the map is still free here
	if (BaseImmovable const* const imm = map[pos].get_immovable()) {
		if (imm->get_size() >= BaseImmovable::SMALL) {
			molog(game.get_gametime(), "  field no longer free\n");
			send_signal(game, "fail");
			pop_task(game);
			return true;
		}
	}

	// Figure the (at most) six best fitting immovables (as judged by terrain
	// affinity). We will pick one of them at random later. The container is
	// picked to be a stable sorting one, so that no deyncs happen in
	// multiplayer.
	std::set<std::pair<int, DescriptionIndex>> best_suited_immovables_index;

	// Checks if the 'immovable_description' has a terrain_affinity, if so use it. Otherwise assume
	// it to be 1 (perfect fit). Adds it to the best_suited_immovables_index.
	const auto test_suitability = [&best_suited_immovables_index, &fpos, &map, &game](
	                                 const uint32_t attribute_id, const DescriptionIndex index,
	                                 const ImmovableDescr& immovable_description) {
		if (!immovable_description.has_attribute(attribute_id)) {
			return;
		}
		int p = TerrainAffinity::kPrecisionFactor;
		if (immovable_description.has_terrain_affinity()) {
			p = probability_to_grow(
			   immovable_description.terrain_affinity(), fpos, map, game.descriptions().terrains());
		}
		best_suited_immovables_index.insert(std::make_pair(p, index));
		if (best_suited_immovables_index.size() > 6) {
			best_suited_immovables_index.erase(best_suited_immovables_index.begin());
		}
	};

	if (action.sparamv.empty()) {
		throw GameDataError("plant needs at least one attrib:<attribute>.");
	}

	// Collect all world and tribe immovable types for all the attributes along with a suitability
	// metric
	for (const std::string& attrib : action.sparamv) {
		if (attrib.empty()) {
			throw GameDataError("plant has an empty attrib:<attribute>");
		}
		const uint32_t attribute_id = ImmovableDescr::get_attribute_id(attrib);

		// Add immovables
		const DescriptionMaintainer<ImmovableDescr>& immovables = game.descriptions().immovables();
		for (uint32_t i = 0; i < immovables.size(); ++i) {
			test_suitability(attribute_id, i, immovables.get(i));
		}
	}

	if (best_suited_immovables_index.empty()) {
		molog(game.get_gametime(), "  WARNING: No suitable immovable found!\n");
		send_signal(game, "fail");
		pop_task(game);
		return true;
	}

	// Randomly pick one of the immovables to be planted.

	// Each candidate is weighted by its probability to grow.
	int total_weight = 0;
	for (const auto& bsii : best_suited_immovables_index) {
		const int weight = bsii.first;
		total_weight += weight;
	}

	// Avoid division by 0
	int choice = game.logic_rand() % std::max(1, total_weight);
	for (const auto& bsii : best_suited_immovables_index) {
		const int weight = bsii.first;
		state.ivar2 = bsii.second;
		choice -= weight;
		if (0 > choice) {
			break;
		}
	}

	Immovable& newimm = game.create_immovable(pos, state.ivar2, get_owner());

	if (action.iparam1 == Action::plantUnlessObject) {
		state.objvar1 = &newimm;
	}

	++state.ivar1;
	schedule_act(game, Duration(10));
	return true;
}

/**
 * createbob=\<bob name\> \<bob name\> ...
 *
 * Plants a bob (critter usually, maybe also worker later on), randomly selected from one of the
 * given types.
 *
 * sparamv = possible bobs
 */
bool Worker::run_createbob(Game& game, State& state, const Action& action) {
	int32_t const idx = game.logic_rand() % action.sparamv.size();

	const std::string& bob = action.sparamv[idx];
	DescriptionIndex index = owner_.load()->tribe().worker_index(bob);
	if (owner_.load()->tribe().has_worker(index)) {
		game.create_worker(get_position(), index, owner_);
	} else {
		const DescriptionIndex critter = game.descriptions().critter_index(bob);
		if (critter == INVALID_INDEX) {
			molog(game.get_gametime(), "  WARNING: Unknown bob %s\n", bob.c_str());
			send_signal(game, "fail");
			pop_task(game);
			return true;
		}
		game.create_critter(get_position(), critter);
	}

	++state.ivar1;
	schedule_act(game, Duration(10));
	return true;
}

bool Worker::run_terraform(Game& game, State& state, const Action& a) {
	const Descriptions& descriptions = game.descriptions();
	std::map<TCoords<FCoords>, DescriptionIndex> triangles;
	const FCoords f = get_position();
	FCoords tln;
	FCoords ln;
	FCoords trn;
	game.map().get_tln(f, &tln);
	game.map().get_trn(f, &trn);
	game.map().get_ln(f, &ln);

	DescriptionIndex di = descriptions.terrain_index(
	   descriptions.get_terrain_descr(f.field->terrain_r())->enhancement(a.sparam1));
	if (di != INVALID_INDEX) {
		triangles.emplace(std::make_pair(TCoords<FCoords>(f, TriangleIndex::R), di));
	}
	di = descriptions.terrain_index(
	   descriptions.get_terrain_descr(f.field->terrain_d())->enhancement(a.sparam1));
	if (di != INVALID_INDEX) {
		triangles.emplace(std::make_pair(TCoords<FCoords>(f, TriangleIndex::D), di));
	}
	di = descriptions.terrain_index(
	   descriptions.get_terrain_descr(tln.field->terrain_r())->enhancement(a.sparam1));
	if (di != INVALID_INDEX) {
		triangles.emplace(std::make_pair(TCoords<FCoords>(tln, TriangleIndex::R), di));
	}
	di = descriptions.terrain_index(
	   descriptions.get_terrain_descr(tln.field->terrain_d())->enhancement(a.sparam1));
	if (di != INVALID_INDEX) {
		triangles.emplace(std::make_pair(TCoords<FCoords>(tln, TriangleIndex::D), di));
	}
	di = descriptions.terrain_index(
	   descriptions.get_terrain_descr(ln.field->terrain_r())->enhancement(a.sparam1));
	if (di != INVALID_INDEX) {
		triangles.emplace(std::make_pair(TCoords<FCoords>(ln, TriangleIndex::R), di));
	}
	di = descriptions.terrain_index(
	   descriptions.get_terrain_descr(trn.field->terrain_d())->enhancement(a.sparam1));
	if (di != INVALID_INDEX) {
		triangles.emplace(std::make_pair(TCoords<FCoords>(trn, TriangleIndex::D), di));
	}

	if (triangles.empty()) {
		send_signal(game, "fail");
		pop_task(game);
		return true;
	}
	assert(game.mutable_map());
	auto it = triangles.begin();
	for (size_t rand = game.logic_rand() % triangles.size(); rand > 0; --rand) {
		++it;
	}
	game.mutable_map()->change_terrain(game, it->first, it->second);
	++state.ivar1;
	schedule_act(game, Duration(10));
	return true;
}

/**
 * buildferry
 *
 * Creates a new instance of the ferry the worker's
 * tribe uses and adds it to the appropriate fleet.
 *
 */
// TODO(GunChleoc): Savegame compatibility, remove after v1.0.
bool Worker::run_buildferry(Game& game, State& state, const Action& /* action */) {
	game.create_worker(get_position(), owner_.load()->tribe().ferry(), owner_);
	++state.ivar1;
	schedule_act(game, Duration(10));
	return true;
}

/**
 * Simply remove the currently selected object - make no fuss about it.
 */
bool Worker::run_removeobject(Game& game, State& state, const Action& /* action */) {
	if (MapObject* const obj = state.objvar1.get(game)) {
		obj->remove(game);
		state.objvar1 = nullptr;
	}

	++state.ivar1;
	schedule_act(game, Duration(10));
	return true;
}

/**
 * repeatsearch=\<repeat #\> \<radius\> \<subcommand\>
 *
 * Walk around the starting point randomly within a certain radius, and
 * execute the subcommand for some of the fields.
 *
 * iparam1 = maximum repeat #
 * iparam2 = radius
 * sparam1 = subcommand
 */
bool Worker::run_repeatsearch(Game& game, State& state, const Action& action) {
	molog(game.get_gametime(), "  Start Repeat Search (%i attempts, %i radius -> %s)\n",
	      action.iparam1, action.iparam2, action.sparam1.c_str());

	++state.ivar1;
	start_task_geologist(game, action.iparam1, action.iparam2, action.sparam1);
	return true;
}

/**
 * Check resources at the current position, and plant a marker object when
 * possible.
 */
bool Worker::run_findresources(Game& game, State& state, const Action& /* action */) {
	const FCoords position = game.map().get_fcoords(get_position());
	BaseImmovable const* const imm = position.field->get_immovable();
	const Descriptions& descriptions = game.descriptions();

	if (!(imm && imm->get_size() > BaseImmovable::NONE)) {

		const ResourceDescription* rdescr =
		   descriptions.get_resource_descr(position.field->get_resources());
		const TribeDescr& t = owner().tribe();
		if (rdescr && !t.uses_resource(rdescr->name())) {
			rdescr = nullptr;
		}
		const Immovable& ri = game.create_immovable(
		   position,
		   t.get_resource_indicator(
		      rdescr, (rdescr && rdescr->detectable()) ? position.field->get_resources_amount() : 0),
		   get_owner());

		// Geologist also sends a message notifying the player
		// TODO(GunChleoc): We keep formatting this even when timeout has not elapsed
		if (rdescr && rdescr->detectable() && position.field->get_resources_amount()) {
			const std::string rt_description = as_mapobject_message(
			   ri.descr().name(), g_image_cache->get(rdescr->representative_image())->width(),
			   _("A geologist found resources."));

			//  We should add a message to the player's message queue - but only,
			//  if there is not already a similar one in list.
			get_owner()->add_message_with_timeout(
			   game,
			   std::unique_ptr<Message>(new Message(Message::Type::kGeologists, game.get_gametime(),
			                                        rdescr->descname(), rdescr->representative_image(),
			                                        ri.descr().descname(), rt_description, position,
			                                        serial_, rdescr->name())),
			   Duration(rdescr->timeout_ms()), rdescr->timeout_radius());
		}
	}

	++state.ivar1;
	return false;
}

/**
 * Demand from the g_sh to play a certain sound effect.
 * Whether the effect actually gets played is decided only by the sound server.
 */
bool Worker::run_playsound(Game& game, State& state, const Action& action) {
	Notifications::publish(NoteSound(
	   SoundType::kAmbient, action.iparam2, get_position(), action.iparam1, action.iparam3 == 1));

	++state.ivar1;
	schedule_act(game, Duration(10));
	return true;
}

/**
 * If we are currently carrying some ware ware, hand it off to the currently
 * selected immovable (\ref objvar1) for construction.
 */
bool Worker::run_construct(Game& game, State& state, const Action& /* action */) {
	Immovable* imm = dynamic_cast<Immovable*>(state.objvar1.get(game));
	if (!imm) {
		molog(game.get_gametime(), "run_construct: no objvar1 immovable set");
		send_signal(game, "fail");
		pop_task(game);
		return true;
	}

	WareInstance* ware = get_carried_ware(game);
	if (!ware) {
		molog(game.get_gametime(), "run_construct: no ware being carried");
		send_signal(game, "fail");
		pop_task(game);
		return true;
	}

	DescriptionIndex wareindex = ware->descr_index();
	if (!imm->construct_ware(game, wareindex)) {
		molog(game.get_gametime(), "run_construct: construct_ware failed");
		send_signal(game, "fail");
		pop_task(game);
		return true;
	}

	// Update consumption statistic
	get_owner()->ware_consumed(wareindex, 1);

	ware = fetch_carried_ware(game);
	ware->remove(game);

	++state.ivar1;
	schedule_act(game, Duration(10));
	return true;
}

Worker::Worker(const WorkerDescr& worker_descr)
   : Bob(worker_descr),
     worker_economy_(nullptr),
     ware_economy_(nullptr),
     supply_(nullptr),
     transfer_(nullptr),
     current_exp_(0) {
}

Worker::~Worker() {
	assert(!location_.is_set());
	assert(!transfer_);
}

/// Log basic information.
void Worker::log_general_info(const EditorGameBase& egbase) const {
	Bob::log_general_info(egbase);

	if (upcast(PlayerImmovable, loc, location_.get(egbase))) {
		FORMAT_WARNINGS_OFF
		molog(egbase.get_gametime(), "* Owner: (%p)\n", &loc->owner());
		FORMAT_WARNINGS_ON
		molog(egbase.get_gametime(), "** Owner (plrnr): %i\n", loc->owner().player_number());
		FORMAT_WARNINGS_OFF
		molog(egbase.get_gametime(), "* WorkerEconomy: %p\n", loc->get_economy(wwWORKER));
		molog(egbase.get_gametime(), "* WareEconomy: %p\n", loc->get_economy(wwWARE));
		FORMAT_WARNINGS_ON
	}

	PlayerImmovable* imm = location_.get(egbase);
	molog(egbase.get_gametime(), "location: %u\n", imm ? imm->serial() : 0);
	FORMAT_WARNINGS_OFF
	molog(egbase.get_gametime(), "WorkerEconomy: %p\n", worker_economy_);
	molog(egbase.get_gametime(), "WareEconomy: %p\n", ware_economy_);
	molog(egbase.get_gametime(), "transfer: %p\n", transfer_);
	FORMAT_WARNINGS_ON

	if (upcast(WareInstance, ware, carried_ware_.get(egbase))) {
		molog(egbase.get_gametime(), "* carried_ware->get_ware() (id): %i\n", ware->descr_index());
		FORMAT_WARNINGS_OFF
		molog(egbase.get_gametime(), "* carried_ware->get_economy() (): %p\n", ware->get_economy());
		FORMAT_WARNINGS_ON
	}

	molog(egbase.get_gametime(), "current_exp: %i / %i\n", current_exp_,
	      descr().get_needed_experience());

	FORMAT_WARNINGS_OFF
	molog(egbase.get_gametime(), "supply: %p\n", supply_);
	FORMAT_WARNINGS_ON
}

/**
 * Change the location. This should be called in the following situations:
 * \li worker creation (usually, location is a warehouse)
 * \li worker moves along a route (location is a road and finally building)
 * \li current location is destroyed (building burnt down etc...)
 */
void Worker::set_location(PlayerImmovable* const location) {
	assert(!location || ObjectPointer(location).get(owner().egbase()));

	PlayerImmovable* const old_location = get_location(owner().egbase());
	if (old_location == location) {
		return;
	}

	if (old_location) {
		// Note: even though we have an old location, economy_ may be zero
		// (old_location got deleted)
		old_location->remove_worker(*this);
	} else {
		if (!is_shipping()) {
			assert(!ware_economy_);
			assert(!worker_economy_);
		}
	}

	location_ = location;

	if (location) {
		Economy* const eco_wo = location->get_economy(wwWORKER);
		Economy* const eco_wa = location->get_economy(wwWARE);

		if (!worker_economy_ || (descr().type() == MapObjectType::SOLDIER)) {
			set_economy(eco_wo, wwWORKER);
		} else if (worker_economy_ != eco_wo) {
			throw wexception("Worker::set_location changes worker_economy, but worker is no soldier");
		}
		if (!ware_economy_ || (descr().type() == MapObjectType::SOLDIER)) {
			set_economy(eco_wa, wwWARE);
		} else if (ware_economy_ != eco_wa) {
			throw wexception("Worker::set_location changes ware_economy, but worker is no soldier");
		}
		location->add_worker(*this);
	} else {
		if (!is_shipping()) {
			// Our location has been destroyed, we are now fugitives.
			// Interrupt whatever we've been doing.
			set_economy(nullptr, wwWARE);
			set_economy(nullptr, wwWORKER);

			EditorGameBase& egbase = get_owner()->egbase();
			if (upcast(Game, game, &egbase)) {
				send_signal(*game, "location");
			}
		}
	}
}

/**
 * Change the worker's current economy. This is called:
 * \li by set_location() when appropriate
 * \li by the current location, when the location's economy changes
 */
void Worker::set_economy(Economy* const economy, WareWorker type) {
	Economy* old = get_economy(type);
	if (economy == old) {
		return;
	}

	switch (type) {
	case wwWARE: {
		ware_economy_ = economy;
		if (WareInstance* const ware = get_carried_ware(get_owner()->egbase())) {
			ware->set_economy(ware_economy_);
		}
	} break;
	case wwWORKER: {
		worker_economy_ = economy;
		if (old) {
			old->remove_wares_or_workers(owner().tribe().worker_index(descr().name()), 1);
		}
		if (supply_) {
			supply_->set_economy(worker_economy_);
		}
		if (worker_economy_) {
			worker_economy_->add_wares_or_workers(
			   owner().tribe().worker_index(descr().name()), 1, ware_economy_);
		}
	} break;
	}
}

/**
 * Initialize the worker
 */
bool Worker::init(EditorGameBase& egbase) {
	Bob::init(egbase);

	// a worker should always start out at a fixed location
	// (this assert is not longer true for save games. Where it lives
	// is unknown to this worker till he is initialized
	//  assert(get_location(egbase));

	if (upcast(Game, game, &egbase)) {
		create_needed_experience(*game);
	}
	return true;
}

/**
 * Remove the worker.
 */
void Worker::cleanup(EditorGameBase& egbase) {
	WareInstance* const ware = get_carried_ware(egbase);

	if (supply_) {
		delete supply_;
		supply_ = nullptr;
	}

	if (ware) {
		ware->destroy(egbase);
	}

	// We are destroyed, but we were maybe idling
	// or doing something else. Get Location might
	// init a gowarehouse task or something and this results
	// in a dirty stack. Nono, we do not want to end like this
	if (upcast(Game, game, &egbase)) {
		reset_tasks(*game);
	}

	if (get_location(egbase)) {
		set_location(nullptr);
	}

	set_economy(nullptr, wwWARE);
	set_economy(nullptr, wwWORKER);

	Bob::cleanup(egbase);
}

/**
 * Set the ware we carry.
 * If we carry an ware right now, it will be destroyed (see
 * fetch_carried_ware()).
 */
void Worker::set_carried_ware(EditorGameBase& egbase, WareInstance* const ware) {
	if (WareInstance* const oldware = get_carried_ware(egbase)) {
		oldware->cleanup(egbase);
		delete oldware;
	}

	carried_ware_ = ware;
	ware->set_location(egbase, this);
	if (upcast(Game, game, &egbase)) {
		ware->update(*game);
	}
}

/**
 * Stop carrying the current ware, and return a pointer to it.
 */
WareInstance* Worker::fetch_carried_ware(EditorGameBase& game) {
	WareInstance* const ware = get_carried_ware(game);

	if (ware) {
		ware->set_location(game, nullptr);
		carried_ware_ = nullptr;
	}

	return ware;
}

/**
 * Schedule an immediate CMD_INCORPORATE, which will integrate this worker into
 * the warehouse he is standing on.
 */
void Worker::schedule_incorporate(Game& game) {
	game.cmdqueue().enqueue(new CmdIncorporate(game.get_gametime(), this));
	return skip_act();
}

/**
 * Incorporate the worker into the warehouse it's standing on immediately.
 */
void Worker::incorporate(Game& game) {
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
void Worker::create_needed_experience(Game& /* game */) {
	if (descr().get_needed_experience() == INVALID_INDEX) {
		current_exp_ = INVALID_INDEX;
		return;
	}

	current_exp_ = 0;
}

void Worker::set_current_experience(const int32_t xp) {
	assert(needs_experience());
	assert(xp >= 0);
	assert(xp < descr().get_needed_experience());
	current_exp_ = xp;
}

/**
 * Gain experience
 *
 * This function increases the experience
 * of the worker by one, if he reaches
 * needed_experience he levels
 */
DescriptionIndex Worker::gain_experience(Game& game) {
	return (descr().get_needed_experience() == INVALID_INDEX ||
	        ++current_exp_ < descr().get_needed_experience()) ?
             INVALID_INDEX :
             level(game);
}

/**
 * Level this worker to the next higher level. this includes creating a
 * new worker with his propertys and removing this worker
 */
DescriptionIndex Worker::level(Game& game) {

	// We do not really remove this worker, all we do
	// is to overwrite his description with the new one and to
	// reset his needed experience. Congratulations to promotion!
	// This silently expects that the new worker is the same type as the old
	// worker and can fullfill the same jobs (which should be given in all
	// circumstances)
	assert(descr().becomes() != INVALID_INDEX);
	const TribeDescr& t = owner().tribe();
	DescriptionIndex const old_index = t.worker_index(descr().name());
	DescriptionIndex const new_index = descr().becomes();
	descr_ = t.get_worker_descr(new_index);
	assert(t.has_worker(new_index));

	// Inform the economy, that something has changed
	worker_economy_->remove_wares_or_workers(old_index, 1);
	worker_economy_->add_wares_or_workers(new_index, 1, ware_economy_);

	create_needed_experience(game);
	return old_index;  //  So that the caller knows what to replace him with.
}

/**
 * Set a fallback task.
 */
void Worker::init_auto_task(Game& game) {
	if (PlayerImmovable* location = get_location(game)) {
		if (!get_economy(wwWORKER)->warehouses().empty() ||
		    location->descr().type() >= MapObjectType::BUILDING) {
			return start_task_gowarehouse(game);
		}
		set_location(nullptr);
	}

	molog(game.get_gametime(), "init_auto_task: become fugitive\n");

	return start_task_fugitive(game);
}

/**
 * Follow the given transfer.
 *
 * Signal "cancel" to cancel the transfer.
 */
const Bob::Task Worker::taskTransfer = {"transfer", static_cast<Bob::Ptr>(&Worker::transfer_update),
                                        nullptr, static_cast<Bob::Ptr>(&Worker::transfer_pop),
                                        false};

/**
 * Tell the worker to follow the Transfer
 */
void Worker::start_task_transfer(Game& game, Transfer* t) {
	// Hackish override for receiving transfers during gowarehouse,
	// and to correctly handle the stack during loading of games
	// (in that case, the transfer task already exists on the stack
	// when this is called).
	if (get_state(taskGowarehouse) || get_state(taskTransfer)) {
		assert(!transfer_);

		transfer_ = t;
		send_signal(game, "transfer");
	} else {  //  just start a normal transfer
		push_task(game, taskTransfer);
		transfer_ = t;
	}
}

void Worker::transfer_pop(Game& /* game */, State& /* state */) {
	if (transfer_) {
		transfer_->has_failed();
		transfer_ = nullptr;
	}
}

void Worker::transfer_update(Game& game, State& /* state */) {
	const Map& map = game.map();
	PlayerImmovable* location = get_location(game);

	// We expect to always have a location at this point,
	// but this assumption may fail when loading a corrupted savegame.
	if (location == nullptr) {
		send_signal(game, "location");
		return pop_task(game);
	}

	// The request is no longer valid, the task has failed
	if (!transfer_) {
		molog(game.get_gametime(), "[transfer]: Fail (without transfer)\n");

		send_signal(game, "fail");
		return pop_task(game);
	}

	// Signal handling
	const std::string& signal = get_signal();

	if (!signal.empty()) {
		// The caller requested a route update, or the previously calculated route
		// failed.
		// We will recalculate the route on the next update().
		if (signal == "road" || signal == "fail" || signal == "transfer" || signal == "wakeup") {
			molog(game.get_gametime(), "[transfer]: Got signal '%s' -> recalculate\n", signal.c_str());

			signal_handled();
		} else if (signal == "blocked") {
			molog(game.get_gametime(), "[transfer]: Blocked by a battle\n");

			signal_handled();
			return start_task_idle(game, descr().get_animation("idle", this), 500);
		} else {
			molog(game.get_gametime(), "[transfer]: Cancel due to signal '%s'\n", signal.c_str());
			return pop_task(game);
		}
	}

	// If our location is a building, our position may be somewhere else:
	// We may be on the building's flag for a fetch_from_flag or dropoff task.
	// We may also be somewhere else entirely (e.g. lumberjack, soldier).
	// Similarly for flags.
	if (upcast(Building, building, location)) {
		if (building->get_position() != get_position()) {
			return start_task_leavebuilding(game, true);
		}
	} else if (upcast(Flag, flag, location)) {
		BaseImmovable* const position = map[get_position()].get_immovable();

		if (position != flag) {
			if (position == flag->get_building()) {
				location = flag->get_building();
				set_location(location);
			} else {
				return set_location(nullptr);
			}
		}
	}

	// Figure out where to go
	bool success;
	PlayerImmovable* const nextstep = transfer_->get_next_step(location, success);

	if (!nextstep) {
		Transfer* const t = transfer_;

		transfer_ = nullptr;

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
	if (upcast(Building, building, location)) {
		if (&building->base_flag() != nextstep) {
			if (upcast(Warehouse, warehouse, building)) {
				if (warehouse->get_portdock() == nextstep) {
					return start_task_shipping(game, warehouse->get_portdock());
				}
			}

			throw wexception(
			   "MO(%u): [transfer]: in building, nextstep is not building's flag", serial());
		}

		return start_task_leavebuilding(game, true);
	}
	if (upcast(Flag, flag, location)) {
		if (upcast(Building, nextbuild, nextstep)) {  //  Flag to Building
			if (&nextbuild->base_flag() != location) {
				throw wexception(
				   "MO(%u): [transfer]: next step is building, but we are nowhere near", serial());
			}

			return start_task_move(
			   game, WALK_NW, descr().get_right_walk_anims(does_carry_ware(), this), true);
		}
		if (upcast(Flag, nextflag, nextstep)) {  //  Flag to Flag
			Road& road = *flag->get_road(*nextflag);

			Path path(road.get_path());

			if (nextstep != &road.get_flag(RoadBase::FlagEnd)) {
				path.reverse();
			}

			molog(game.get_gametime(),
			      "[transfer]: starting task [movepath] and setting location to road %u\n",
			      road.serial());
			start_task_movepath(game, path, descr().get_right_walk_anims(does_carry_ware(), this));
			set_location(&road);
		} else if (upcast(RoadBase, road, nextstep)) {  //  Flag to Road
			if (&road->get_flag(RoadBase::FlagStart) != location &&
			    &road->get_flag(RoadBase::FlagEnd) != location) {
				throw wexception(
				   "MO(%u): [transfer]: nextstep is road, but we are nowhere near", serial());
			}
			molog(game.get_gametime(), "[transfer]: set location to road %u\n", road->serial());
			set_location(road);
			set_animation(game, descr().get_animation("idle", this));
			schedule_act(game, Duration(10));  //  wait a little
		} else {
			throw wexception(
			   "MO(%u): [transfer]: flag to bad nextstep %u", serial(), nextstep->serial());
		}
	} else if (upcast(RoadBase, road, location)) {
		// Road to Flag
		if (nextstep->descr().type() == MapObjectType::FLAG) {
			const Path& path = road->get_path();
			int32_t const index = nextstep == &road->get_flag(RoadBase::FlagStart) ? 0 :
			                      nextstep == &road->get_flag(RoadBase::FlagEnd) ? path.get_nsteps() :
                                                                                -1;

			if (index >= 0) {
				if (start_task_movepath(
				       game, path, index, descr().get_right_walk_anims(does_carry_ware(), this))) {
					molog(game.get_gametime(), "[transfer]: from road %u to flag %u\n", road->serial(),
					      nextstep->serial());
					return;
				}
			} else if (nextstep != map[get_position()].get_immovable()) {
				throw wexception(
				   "MO(%u): [transfer]: road to flag, but flag is nowhere near", serial());
			}

			set_location(dynamic_cast<Flag*>(nextstep));
			set_animation(game, descr().get_animation("idle", this));
			schedule_act(game, Duration(10));  //  wait a little
		} else {
			throw wexception(
			   "MO(%u): [transfer]: from road to bad nextstep %u", serial(), nextstep->serial());
		}
	} else {
		// Check location to make clang-tidy happy
		throw wexception(
		   "MO(%u): location %u has bad type", serial(), location ? location->serial() : 0);
	}
}

/**
 * Called by transport code when the transfer has been cancelled & destroyed.
 */
void Worker::cancel_task_transfer(Game& game) {
	transfer_ = nullptr;
	send_signal(game, "cancel");
}

/**
 * Sleep while the shipping code in @ref PortDock and @ref Ship handles us.
 */
const Bob::Task Worker::taskShipping = {"shipping", static_cast<Bob::Ptr>(&Worker::shipping_update),
                                        nullptr, static_cast<Bob::Ptr>(&Worker::shipping_pop),
                                        true};

/**
 * Start the shipping task. If pd != nullptr, add us as a shipping item. We
 * could be an expedition worker though, so we will not be a shipping item
 * though.
 *
 * ivar1 = end shipping?
 */
void Worker::start_task_shipping(Game& game, PortDock* pd) {
	push_task(game, taskShipping);
	top_state().ivar1 = 0;
	if (pd) {
		pd->add_shippingitem(game, *this);
	}
}

/**
 * Trigger the end of the shipping task.
 *
 * @note the worker must be in a @ref Warehouse location
 */
void Worker::end_shipping(Game& game) {
	if (State* state = get_state(taskShipping)) {
		state->ivar1 = 1;
		send_signal(game, "endshipping");
	}
}

/**
 * Whether we are currently being handled by the shipping code.
 */
bool Worker::is_shipping() {
	return get_state(taskShipping);
}

void Worker::shipping_pop(Game& game, State& /* state */) {
	// Defense against unorderly cleanup via reset_tasks
	if (!get_location(game)) {
		set_economy(nullptr, wwWARE);
		set_economy(nullptr, wwWORKER);
	}
}

void Worker::shipping_update(Game& game, State& state) {
	PlayerImmovable* location = get_location(game);

	// Signal handling
	const std::string& signal = get_signal();

	if (!signal.empty()) {
		if (signal == "endshipping") {
			signal_handled();
			if (!dynamic_cast<Warehouse*>(location)) {
				molog(game.get_gametime(),
				      "shipping_update: received signal 'endshipping' while not in warehouse!\n");
				pop_task(game);
				return;
			}
		}
		if (signal == "transfer" || signal == "wakeup") {
			signal_handled();
		}
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
   "buildingwork", static_cast<Bob::Ptr>(&Worker::buildingwork_update), nullptr, nullptr, true};

/**
 * Begin work at a building.
 */
void Worker::start_task_buildingwork(Game& game) {
	push_task(game, taskBuildingwork);
	State& state = top_state();
	state.ivar1 = 0;
}

void Worker::buildingwork_update(Game& game, State& state) {
	// Reset any signals that are not related to location
	std::string signal = get_signal();
	signal_handled();

	upcast(Building, building, get_location(game));

	if (signal == "evict") {
		if (building) {
			building->notify_worker_evicted(game, *this);
		}
		return pop_task(game);
	}

	if (state.ivar1 == 1) {
		state.ivar1 = (signal == "fail") * 2;
	}

	// Return to building, if necessary
	if (!building) {
		return pop_task(game);
	}

	if (game.map().get_immovable(get_position()) != building) {
		return start_task_return(game, false);  //  do not drop ware
	}

	// Get the new job
	bool const success = state.ivar1 != 2;

	// Set this *before* calling to get_building_work, because the
	// state pointer might become invalid
	state.ivar1 = 1;

	if (!building->get_building_work(game, *this, success)) {
		set_animation(game, 0);
		return skip_act();
	}
}

/**
 * Wake up the buildingwork task if it was sleeping.
 * Otherwise, the buildingwork task will update as soon as the previous task
 * is finished.
 */
void Worker::update_task_buildingwork(Game& game) {
	// After the worker is evicted and 'taskBuildingwork' is popped from the stack but before
	// `taskLeavebuilding` is started, there is a brief window of time where this function can
	// still be called, so we need to take into account that 'state' may be 'nullptr' here.
	const State* const state = get_state();
	if (state && state->task == &taskBuildingwork) {
		send_signal(game, "update");
	}
}

// The task when a worker is part of the caravan that is trading items.
const Bob::Task Worker::taskCarryTradeItem = {
   "carry_trade_item", static_cast<Bob::Ptr>(&Worker::carry_trade_item_update), nullptr, nullptr,
   true};

void Worker::start_task_carry_trade_item(Game& game,
                                         const int trade_id,
                                         ObjectPointer other_market) {
	push_task(game, taskCarryTradeItem);
	auto& state = top_state();
	state.ivar1 = 0;
	state.ivar2 = trade_id;
	state.objvar1 = other_market;
}

// This is a state machine: leave building, go to the other market, drop off
// wares, and return.
void Worker::carry_trade_item_update(Game& game, State& state) {
	// Reset any signals that are not related to location
	std::string signal = get_signal();
	signal_handled();
	if (!signal.empty()) {
		// TODO(sirver,trading): Remove once signals are correctly handled.
		log_dbg_time(
		   game.get_gametime(), "carry_trade_item_update: signal received: %s\n", signal.c_str());
	}
	if (signal == "evict") {
		return pop_task(game);
	}

	// First of all, make sure we're outside
	if (state.ivar1 == 0) {
		start_task_leavebuilding(game, false);
		++state.ivar1;
		return;
	}

	auto* other_market = dynamic_cast<Market*>(state.objvar1.get(game));
	if (state.ivar1 == 1) {
		// Arrived on site. Move to the building and advance our state.
		if (other_market->base_flag().get_position() == get_position()) {
			++state.ivar1;
			return start_task_move(
			   game, WALK_NW, descr().get_right_walk_anims(does_carry_ware(), this), true);
		}

		// Otherwise continue making progress towards the other market.
		if (!start_task_movepath(game, other_market->base_flag().get_position(), 5,
		                         descr().get_right_walk_anims(does_carry_ware(), this))) {
			molog(game.get_gametime(), "carry_trade_item_update: Could not move to other flag.\n");
			// TODO(sirver,trading): something needs to happen here.
		}
		return;
	}

	if (state.ivar1 == 2) {
		WareInstance* const ware = fetch_carried_ware(game);
		other_market->traded_ware_arrived(state.ivar2, ware->descr_index(), &game);
		ware->remove(game);
		++state.ivar1;
		start_task_move(game, WALK_SE, descr().get_right_walk_anims(does_carry_ware(), this), true);
		return;
	}

	if (state.ivar1 == 3) {
		++state.ivar1;
		start_task_return(game, false);
		return;
	}

	if (state.ivar1 == 4) {
		pop_task(game);
		start_task_idle(game, 0, -1);
		dynamic_cast<Market*>(get_location(game))->try_launching_batch(&game);
		return;
	}
	NEVER_HERE();
}

void Worker::update_task_carry_trade_item(Game& game) {
	if (top_state().task == &taskCarryTradeItem) {
		send_signal(game, "update");
	}
}

/**
 * Evict the worker from its current building.
 */
void Worker::evict(Game& game) {
	if (is_evict_allowed()) {
		send_signal(game, "evict");
	}
}

bool Worker::is_evict_allowed() {
	return true;
}

/**
 * Return to our owning building.
 * If dropware (ivar1) is true, we'll drop our carried ware (if any) on the
 * building's flag, if possible.
 * Blocks all signals except for "location".
 */
const Bob::Task Worker::taskReturn = {
   "return", static_cast<Bob::Ptr>(&Worker::return_update), nullptr, nullptr, true};

/**
 * Return to our owning building.
 */
void Worker::start_task_return(Game& game, bool const dropware) {
	PlayerImmovable* const location = get_location(game);

	if (!location || (location->descr().type() < MapObjectType::BUILDING &&
	                  location->descr().type() != MapObjectType::FLAG)) {
		throw wexception("MO(%u): start_task_return(): not owned by building or flag", serial());
	}

	push_task(game, taskReturn);
	top_state().ivar1 = dropware ? 1 : 0;
}

void Worker::return_update(Game& game, State& state) {
	std::string signal = get_signal();

	if (signal == "location") {
		molog(game.get_gametime(), "[return]: Interrupted by signal '%s'\n", signal.c_str());
		return pop_task(game);
	}

	signal_handled();

	PlayerImmovable* location = get_location(game);

	if (!location) {
		// Usually, this should be caught via the "location" signal above.
		// However, in certain cases, e.g. for a soldier during battle,
		// the location may be overwritten by a different signal while
		// walking home.
		molog(game.get_gametime(), "[return]: Our location disappeared from under us\n");
		return pop_task(game);
	}

	if (BaseImmovable* const pos = game.map().get_immovable(get_position())) {
		if (pos == location) {
			set_animation(game, 0);
			return pop_task(game);
		}

		if (upcast(Flag, flag, pos)) {
			// Is this "our" flag?
			if (flag->get_building() == location) {
				if (state.ivar1 && flag->has_capacity()) {
					if (WareInstance* const ware = fetch_carried_ware(game)) {
						flag->add_ware(game, *ware);
						set_animation(game, descr().get_animation("idle", this));
						return schedule_act(game, Duration(20));  //  rest a while
					}
				}

				// Don't try to enter building if it is a dismantle site
				// It is no problem for builders since they won't return before
				// dismantling is complete.
				if (location && location->descr().type() == MapObjectType::DISMANTLESITE) {
					set_location(nullptr);
					return pop_task(game);
				}
				return start_task_move(
				   game, WALK_NW, descr().get_right_walk_anims(does_carry_ware(), this), true);
			}
			if (location == flag) {
				return pop_task(game);
			}
		}
	}

	// Determine the building's flag and move to it

	Flag& target_flag = location->descr().type() == MapObjectType::FLAG ?
                          dynamic_cast<Flag&>(*location) :
                          dynamic_cast<Building&>(*location).base_flag();
	if (!start_task_movepath(game, target_flag.get_position(), 15,
	                         descr().get_right_walk_anims(does_carry_ware(), this))) {
		molog(game.get_gametime(), "[return]: Failed to return\n");
		const std::string message = format(
		   _("Your %s can’t find a way home and will likely die."), descr().descname().c_str());

		get_owner()->add_message(
		   game,
		   std::unique_ptr<Message>(new Message(Message::Type::kGameLogic, game.get_gametime(),
		                                        _("Worker"), "images/ui_basic/menu_help.png",
		                                        _("Worker got lost!"), message, get_position())),
		   serial_);
		set_location(nullptr);
		return pop_task(game);
	}
}

/**
 * Follow the steps of a configuration-defined program.
 * ivar1 is the next action to be performed.
 * ivar2 is used to store description indices selected by plant
 * objvar1 is used to store objects found by findobject
 * coords is used to store target coordinates found by findspace
 */
const Bob::Task Worker::taskProgram = {"program", static_cast<Bob::Ptr>(&Worker::program_update),
                                       nullptr, static_cast<Bob::Ptr>(&Worker::program_pop), false};

/**
 * Start the given program.
 */
void Worker::start_task_program(Game& game, const std::string& programname) {
	push_task(game, taskProgram);
	State& state = top_state();
	state.program = descr().get_program(programname);
	state.ivar1 = 0;
}

void Worker::program_update(Game& game, State& state) {
	if (!get_signal().empty()) {
		molog(game.get_gametime(), "[program]: Interrupted by signal '%s'\n", get_signal().c_str());
		return pop_task(game);
	}

	if (!state.program) {
		// This might happen as fallout of some save game compatibility fix
		molog(game.get_gametime(), "[program]: No program active\n");
		send_signal(game, "fail");
		return pop_task(game);
	}

	for (;;) {
		const WorkerProgram& program = dynamic_cast<const WorkerProgram&>(*state.program);

		if ((state.ivar1 >= 0) && (static_cast<uint32_t>(state.ivar1) >= program.get_size())) {
			return pop_task(game);
		}

		const Action& action = *program.get_action(state.ivar1);

		if ((this->*(action.function))(game, state, action)) {
			return;
		}
	}
}

void Worker::program_pop(Game& game, State& state) {
	set_program_objvar(game, state, nullptr);
}

void Worker::set_program_objvar(Game& game, State& state, MapObject* obj) {
	assert(state.task == &taskProgram);

	if (state.objvar1.get(game) != nullptr) {
		(state.objvar1.get(game))->set_reserved_by_worker(false);
	}

	state.objvar1 = obj;

	if (obj != nullptr) {
		obj->set_reserved_by_worker(true);
	}
}

const Bob::Task Worker::taskGowarehouse = {
   "gowarehouse", static_cast<Bob::Ptr>(&Worker::gowarehouse_update),
   static_cast<Bob::PtrSignal>(&Worker::gowarehouse_signalimmediate),
   static_cast<Bob::Ptr>(&Worker::gowarehouse_pop), true};

/**
 * Get the worker to move to the nearest warehouse.
 * The worker is added to the list of usable wares, so he may be reassigned to
 * a new task immediately.
 */
void Worker::start_task_gowarehouse(Game& game) {
	assert(!supply_);

	push_task(game, taskGowarehouse);
}

void Worker::gowarehouse_update(Game& game, State& /* state */) {
	PlayerImmovable* const location = get_location(game);

	if (!location) {
		send_signal(game, "location");
		return pop_task(game);
	}

	// Signal handling
	std::string signal = get_signal();

	if (!signal.empty()) {
		// if routing has failed, try a different warehouse/route on next update()
		if (signal == "fail" || signal == "cancel") {
			molog(game.get_gametime(), "[gowarehouse]: caught '%s'\n", signal.c_str());
			signal_handled();
		} else if (signal == "transfer") {
			signal_handled();
		} else {
			molog(game.get_gametime(), "[gowarehouse]: cancel for signal '%s'\n", signal.c_str());
			return pop_task(game);
		}
	}

	if (location->descr().type() == Widelands::MapObjectType::WAREHOUSE) {
		delete supply_;
		supply_ = nullptr;

		schedule_incorporate(game);
		return;
	}

	// If we got a transfer, use it
	if (transfer_) {
		Transfer* const t = transfer_;
		transfer_ = nullptr;

		molog(game.get_gametime(), "[gowarehouse]: Got transfer\n");

		pop_task(game);
		return start_task_transfer(game, t);
	}

	// Always leave buildings in an orderly manner,
	// even when no warehouses are left to return to
	if (location->descr().type() >= MapObjectType::BUILDING) {
		return start_task_leavebuilding(game, true);
	}

	if (get_economy(wwWORKER)->warehouses().empty()) {
		molog(game.get_gametime(), "[gowarehouse]: No warehouse left in WorkerEconomy\n");
		return pop_task(game);
	}

	// Idle until we are assigned a transfer.
	// The delay length is rather arbitrary, but we need some kind of
	// check against disappearing warehouses, or the worker will just
	// idle on a flag until the end of days (actually, until either the
	// flag is removed or a warehouse connects to the Economy).
	if (!supply_) {
		supply_ = new IdleWorkerSupply(*this);
	}

	return start_task_idle(game, descr().get_animation("idle", this), 1000);
}

void Worker::gowarehouse_signalimmediate(Game& /* game */,
                                         State& /* state */,
                                         const std::string& signal) {
	if (signal == "transfer") {
		// We are assigned a transfer, make sure our supply disappears immediately
		// Otherwise, we might receive two transfers in a row.
		delete supply_;
		supply_ = nullptr;
	}
}

void Worker::gowarehouse_pop(Game& /* game */, State& /* state */) {
	delete supply_;
	supply_ = nullptr;

	if (transfer_) {
		transfer_->has_failed();
		transfer_ = nullptr;
	}
}

const Bob::Task Worker::taskDropoff = {
   "dropoff", static_cast<Bob::Ptr>(&Worker::dropoff_update), nullptr, nullptr, true};

const Bob::Task Worker::taskReleaserecruit = {
   "releaserecruit", static_cast<Bob::Ptr>(&Worker::releaserecruit_update), nullptr, nullptr, true};

/**
 * Walk to the building's flag, drop the given ware, and walk back inside.
 */
void Worker::start_task_dropoff(Game& game, WareInstance& ware) {
	set_carried_ware(game, &ware);
	push_task(game, taskDropoff);
}

void Worker::dropoff_update(Game& game, State& /* state */) {
	std::string signal = get_signal();

	if (!signal.empty()) {
		molog(game.get_gametime(), "[dropoff]: Interrupted by signal '%s'\n", signal.c_str());
		return pop_task(game);
	}

	WareInstance* ware = get_carried_ware(game);
	BaseImmovable* const location = game.map()[get_position()].get_immovable();

	// If the building just got destroyed, pop the task
	PlayerImmovable* current_location = get_location(game);
	if (current_location == nullptr) {
		molog(game.get_gametime(),
		      "%s: Unable to dropoff ware in building at (%d,%d) - there is no building there\n",
		      descr().name().c_str(), get_position().x, get_position().y);
		return pop_task(game);
	}

#ifndef NDEBUG
	Building* ploc = dynamic_cast<Building*>(current_location);
	assert(ploc == location || &ploc->base_flag() == location);
#endif

	// Deliver the ware
	if (ware) {
		// We're in the building, walk onto the flag
		if (upcast(Building, building, location)) {
			if (start_task_waitforcapacity(game, building->base_flag())) {
				return;
			}

			return start_task_leavebuilding(game, false);  //  exit throttle
		}

		// We're on the flag, drop the ware and pause a little
		if (upcast(Flag, flag, location)) {
			if (flag->has_capacity()) {
				flag->add_ware(game, *fetch_carried_ware(game));

				set_animation(game, descr().get_animation("idle", this));
				return schedule_act(game, Duration(50));
			}

			molog(game.get_gametime(), "[dropoff]: flag is overloaded\n");
			start_task_move(
			   game, WALK_NW, descr().get_right_walk_anims(does_carry_ware(), this), true);
			return;
		}

		throw wexception("MO(%u): [dropoff]: not on building or on flag - fishy", serial());
	}

	// We don't have the ware any more, return home
	if (location->descr().type() == MapObjectType::FLAG) {
		return start_task_move(
		   game, WALK_NW, descr().get_right_walk_anims(does_carry_ware(), this), true);
	}

	if (location->descr().type() < MapObjectType::BUILDING) {
		throw wexception("MO(%u): [dropoff]: not on building on return", serial());
	}

	if (location && location->descr().type() == Widelands::MapObjectType::WAREHOUSE) {
		schedule_incorporate(game);
		return;
	}

	// Our parent task should know what to do
	return pop_task(game);
}

/// Give the recruit his diploma and say farwell to him.
void Worker::start_task_releaserecruit(Game& game, const Worker& recruit) {
	push_task(game, taskReleaserecruit);
	molog(game.get_gametime(), "Starting to release %s %u...\n", recruit.descr().name().c_str(),
	      recruit.serial());
	return schedule_act(game, Duration(5000));
}

void Worker::releaserecruit_update(Game& game, State& /* state */) {
	molog(game.get_gametime(), "\t...done releasing recruit\n");
	return pop_task(game);
}

/**
 * ivar1 is set to 0 if we should move to the flag and fetch the ware, and it
 * is set to 1 if we should move into the building.
 */
const Bob::Task Worker::taskFetchfromflag = {
   "fetchfromflag", static_cast<Bob::Ptr>(&Worker::fetchfromflag_update), nullptr, nullptr, true};

/**
 * Walk to the building's flag, fetch an ware from the flag that is destined for
 * the building, and walk back inside.
 */
void Worker::start_task_fetchfromflag(Game& game) {
	push_task(game, taskFetchfromflag);
	top_state().ivar1 = 0;
}

void Worker::fetchfromflag_update(Game& game, State& state) {
	std::string signal = get_signal();
	if (!signal.empty()) {
		if (signal == "location") {
			molog(game.get_gametime(), "[fetchfromflag]: Building disappeared, become fugitive\n");
			return pop_task(game);
		}
	}

	PlayerImmovable& employer = *get_location(game);
	PlayerImmovable* const location =
	   dynamic_cast<PlayerImmovable*>(game.map().get_immovable(get_position()));

	// If we haven't got the ware yet, walk onto the flag
	if (!get_carried_ware(game) && !state.ivar1) {
		if (location && location->descr().type() >= Widelands::MapObjectType::BUILDING) {
			return start_task_leavebuilding(game, false);
		}

		state.ivar1 = 1;  //  force return to building

		if (!location) {
			// this can happen if the flag (and the building) is destroyed while
			// the worker leaves the building.
			molog(game.get_gametime(), "[fetchfromflag]: flag dissappeared - become fugitive");
			return pop_task(game);
		}

		// The ware has decided that it doesn't want to go to us after all
		// In order to return to the warehouse, we're switching to State_DropOff
		if (WareInstance* const ware =
		       dynamic_cast<Flag&>(*location).fetch_pending_ware(game, employer)) {
			set_carried_ware(game, ware);
		}

		set_animation(game, descr().get_animation("idle", this));
		return schedule_act(game, Duration(20));
	}

	// Go back into the building
	if (location && location->descr().type() == Widelands::MapObjectType::FLAG) {
		molog(game.get_gametime(), "[fetchfromflag]: return to building\n");

		return start_task_move(
		   game, WALK_NW, descr().get_right_walk_anims(does_carry_ware(), this), true);
	}

	if (!location || location->descr().type() < Widelands::MapObjectType::BUILDING) {
		// This can happen "naturally" if the building gets destroyed, but the
		// flag is still there and the worker tries to enter from that flag.
		// E.g. the player destroyed the building, it is destroyed, through an
		// enemy player, or it got destroyed through rising water (atlantean
		// scenario)
		molog(game.get_gametime(),
		      "[fetchfromflag]: building dissappeared - searching for alternative\n");
		return pop_task(game);
	}

	assert(location == &employer);

	molog(game.get_gametime(), "[fetchfromflag]: back home\n");

	if (WareInstance* const ware = fetch_carried_ware(game)) {
		if (ware->get_next_move_step(game) == location) {
			ware->enter_building(game, *dynamic_cast<Building*>(location));
		} else {
			// The ware changed its mind and doesn't want to go to this building
			// after all, so carry it back out.
			// This can happen in the following subtle and rare race condition:
			// We start the fetchfromflag task as the worker in an enhanceable building.
			// While we walk back into the building with the ware, the player enhances
			// the building, so that we now belong to the newly created construction site.
			// Obviously the construction site no longer has any use for the ware.
			molog(game.get_gametime(),
			      "[fetchfromflag]: ware no longer wants to go into building, drop off\n");
			pop_task(game);
			start_task_dropoff(game, *ware);
			return;
		}
	}

	// We're back!
	if (location->descr().type() == Widelands::MapObjectType::WAREHOUSE) {
		schedule_incorporate(game);
		return;
	}

	return pop_task(game);  //  assume that our parent task knows what to do
}

/**
 * Wait for available capacity on a flag.
 */
const Bob::Task Worker::taskWaitforcapacity = {
   "waitforcapacity", static_cast<Bob::Ptr>(&Worker::waitforcapacity_update), nullptr,
   static_cast<Bob::Ptr>(&Worker::waitforcapacity_pop), true};

/**
 * Checks the capacity of the flag.
 *
 * If there is none, a wait task is pushed, and the worker is added to the
 * flag's wait queue. The function returns true in this case.
 * If the flag still has capacity, the function returns false and doesn't
 * act at all.
 */
bool Worker::start_task_waitforcapacity(Game& game, Flag& flag) {
	if (flag.has_capacity()) {
		return false;
	}

	push_task(game, taskWaitforcapacity);

	top_state().objvar1 = &flag;

	flag.wait_for_capacity(game, *this);

	return true;
}

void Worker::waitforcapacity_update(Game& game, State& /* state */) {
	std::string signal = get_signal();

	if (!signal.empty()) {
		if (signal == "wakeup") {
			signal_handled();
		}
		return pop_task(game);
	}

	return skip_act();  //  wait indefinitely
}

void Worker::waitforcapacity_pop(Game& game, State& state) {
	if (upcast(Flag, flag, state.objvar1.get(game))) {
		flag->skip_wait_for_capacity(game, *this);
	}
}

/**
 * Called when the flag we waited on has now got capacity left.
 * Return true if we actually woke up due to this.
 */
bool Worker::wakeup_flag_capacity(Game& game, Flag& flag) {
	if (State const* const state = get_state()) {
		if (state->task == &taskWaitforcapacity) {
			molog(game.get_gametime(), "[waitforcapacity]: Wake up: flag capacity.\n");

			if (state->objvar1.get(game) != &flag) {
				throw wexception("MO(%u): wakeup_flag_capacity: Flags do not match.", serial());
			}
			send_signal(game, "wakeup");
			return true;
		}
	}
	return false;
}

/**
 * ivar1 - 0: don't change location; 1: change location to the flag
 * objvar1 - the building we're leaving
 */
const Bob::Task Worker::taskLeavebuilding = {
   "leavebuilding", static_cast<Bob::Ptr>(&Worker::leavebuilding_update), nullptr,
   static_cast<Bob::Ptr>(&Worker::leavebuilding_pop), true};

/**
 * Leave the current building.
 * Waits on the buildings leave wait queue if necessary.
 *
 * If changelocation is true, change the location to the flag once we're
 * outside.
 */
void Worker::start_task_leavebuilding(Game& game, bool const changelocation) {
	// Set the wait task
	push_task(game, taskLeavebuilding);
	State& state = top_state();
	state.ivar1 = changelocation;
	state.objvar1 = &dynamic_cast<Building&>(*get_location(game));
}

void Worker::leavebuilding_update(Game& game, State& state) {
	const std::string& signal = get_signal();

	if (signal == "wakeup") {
		signal_handled();
	} else if (!signal.empty()) {
		return pop_task(game);
	}

	upcast(Building, building, get_location(game));
	if (!building) {
		return pop_task(game);
	}

	Flag& baseflag = building->base_flag();

	if (get_position() == building->get_position()) {
		assert(building == state.objvar1.get(game));
		if (!building->leave_check_and_wait(game, *this)) {
			return skip_act();
		}

		if (state.ivar1) {
			set_location(&baseflag);
		}

		return start_task_move(
		   game, WALK_SE, descr().get_right_walk_anims(does_carry_ware(), this), true);
	}
	const Coords& flagpos = baseflag.get_position();

	if (state.ivar1) {
		set_location(&baseflag);
	}

	if (get_position() == flagpos) {
		return pop_task(game);
	}

	if (!start_task_movepath(
	       game, flagpos, 0, descr().get_right_walk_anims(does_carry_ware(), this))) {
		molog(game.get_gametime(),
		      "[leavebuilding]: outside of building, but failed to walk back to flag");
		set_location(nullptr);
		return pop_task(game);
	}
}

void Worker::leavebuilding_pop(Game& game, State& state) {
	// As of this writing, this is only really necessary when the task
	// is interrupted by a signal. Putting this in the pop() method is just
	// defensive programming, in case leavebuilding_update() changes
	// in the future.
	//
	//  The if-statement is needed because this is (unfortunately) also called
	//  when the Worker is deallocated when shutting down the simulation. Then
	//  the building might not exist any more.
	if (MapObject* const building = state.objvar1.get(game)) {
		dynamic_cast<Building&>(*building).leave_skip(game, *this);
	}
}

/**
 * Called when the given building allows us to leave it.
 * \return true if we actually woke up due to this.
 */
bool Worker::wakeup_leave_building(Game& game, Building& building) {
	if (State const* const state = get_state()) {
		if (state->task == &taskLeavebuilding) {
			if (state->objvar1.get(game) != &building) {
				throw wexception("MO(%u): [waitleavebuilding]: buildings do not match", serial());
			}
			send_signal(game, "wakeup");
			return true;
		}
	}
	return false;
}

/**
 * Run around aimlessly until we find a warehouse.
 */
const Bob::Task Worker::taskFugitive = {
   "fugitive", static_cast<Bob::Ptr>(&Worker::fugitive_update), nullptr, nullptr, true};

void Worker::start_task_fugitive(Game& game) {
	push_task(game, taskFugitive);

	// Fugitives survive for two to four minutes
	top_state().ivar1 = game.get_gametime().get() + 120000 + 200 * (game.logic_rand() % 600);
}

struct FindFlagWithPlayersWarehouse {
	explicit FindFlagWithPlayersWarehouse(const Player& owner) : owner_(owner) {
	}
	bool accept(const BaseImmovable& imm) const {
		if (upcast(Flag const, flag, &imm)) {
			if (flag->get_owner() == &owner_) {
				if (!flag->economy(wwWORKER).warehouses().empty()) {
					return true;
				}
			}
		}
		return false;
	}

private:
	const Player& owner_;
};

void Worker::fugitive_update(Game& game, State& state) {
	if (!get_signal().empty()) {
		molog(game.get_gametime(), "[fugitive]: interrupted by signal '%s'\n", get_signal().c_str());
		return pop_task(game);
	}

	const Map& map = game.map();
	PlayerImmovable const* location = get_location(game);

	if (location && location->get_owner() == get_owner()) {
		molog(game.get_gametime(), "[fugitive]: we are on location\n");

		if (location->descr().type() == Widelands::MapObjectType::WAREHOUSE) {
			return schedule_incorporate(game);
		}

		set_location(nullptr);
		location = nullptr;
	}

	// check whether we're on a flag and it's time to return home
	if (upcast(Flag, flag, map[get_position()].get_immovable())) {
		if (flag->get_owner() == get_owner() && !flag->economy(wwWORKER).warehouses().empty()) {
			set_location(flag);
			if (does_carry_ware()) {
				if (flag->has_capacity()) {
					if (WareInstance* const ware = fetch_carried_ware(game)) {
						molog(game.get_gametime(), "[fugitive] is on flag, drop carried ware %s\n",
						      ware->descr().name().c_str());
						flag->add_ware(game, *ware);
					}
				} else {
					molog(game.get_gametime(),
					      "[fugitive] is on flag, which has no capacity for the carried ware!\n");
				}
			}
			return pop_task(game);
		}
	}

	// Try to find a flag connected to a warehouse that we can return to
	//
	// We always have a high probability to see flags within our vision range,
	// but with some luck we see flags that are even further away.
	std::vector<ImmovableFound> flags;
	uint32_t vision = descr().vision_range();
	uint32_t maxdist = 4 * vision;
	if (map.find_immovables(game, Area<FCoords>(map.get_fcoords(get_position()), maxdist), &flags,
	                        FindFlagWithPlayersWarehouse(*get_owner()))) {
		uint32_t bestdist = 0;
		Flag* best = nullptr;

		molog(game.get_gametime(), "[fugitive]: found a flag connected to warehouse(s)\n");
		for (const ImmovableFound& tmp_flag : flags) {

			Flag& flag = dynamic_cast<Flag&>(*tmp_flag.object);

			uint32_t const dist = map.calc_distance(get_position(), tmp_flag.coords);

			if (!best || bestdist > dist) {
				best = &flag;
				bestdist = dist;
			}
		}

		if (best && bestdist > vision) {
			uint32_t chance = maxdist - (bestdist - vision);
			if (game.logic_rand() % maxdist >= chance) {
				best = nullptr;
			}
		}

		if (best) {
			molog(game.get_gametime(), "[fugitive]: try to move to flag\n");

			// Warehouse could be on a different island, so check for failure
			// Also, move only a few number of steps in the right direction,
			// so that we could theoretically lose the flag again, but also
			// perhaps find a closer flag.
			if (start_task_movepath(game, best->get_position(), 0,
			                        descr().get_right_walk_anims(does_carry_ware(), this), false, 4)) {
				return;
			}
		}
	}

	if ((state.ivar1 < 0) || (Time(state.ivar1) < game.get_gametime())) {  //  time to die?
		molog(game.get_gametime(), "[fugitive]: die\n");
		return schedule_destroy(game);
	}

	molog(game.get_gametime(), "[fugitive]: wander randomly\n");

	if (start_task_movepath(game, game.random_location(get_position(), descr().vision_range()), 4,
	                        descr().get_right_walk_anims(does_carry_ware(), this))) {
		return;
	}

	return start_task_idle(game, descr().get_animation("idle", this), 50);
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
   "geologist", static_cast<Bob::Ptr>(&Worker::geologist_update), nullptr, nullptr, true};

void Worker::start_task_geologist(Game& game,
                                  uint8_t const attempts,
                                  uint8_t const radius,
                                  const std::string& subcommand) {
	push_task(game, taskGeologist);
	State& state = top_state();
	state.ivar1 = attempts;
	state.ivar2 = radius;
	state.svar1 = subcommand;
}

void Worker::geologist_update(Game& game, State& state) {
	const uint32_t resource_indicator_attribute = MapObjectDescr::get_attribute_id("resi", false);

	std::string signal = get_signal();

	if (signal == "fail") {
		molog(game.get_gametime(), "[geologist]: Caught signal '%s'\n", signal.c_str());
		signal_handled();
	} else if (!signal.empty()) {
		molog(game.get_gametime(), "[geologist]: Interrupted by signal '%s'\n", signal.c_str());
		return pop_task(game);
	}

	//
	const Map& map = game.map();
	const Descriptions& descriptions = game.descriptions();
	Area<FCoords> owner_area(
	   map.get_fcoords(dynamic_cast<Flag&>(*get_location(game)).get_position()), state.ivar2);

	// Check if it's not time to go home
	if (state.ivar1 > 0) {
		// Check to see if we're on suitable terrain
		BaseImmovable* const imm = map.get_immovable(get_position());

		if (!imm || (imm->get_size() == BaseImmovable::NONE &&
		             !imm->has_attribute(resource_indicator_attribute))) {
			--state.ivar1;
			return start_task_program(game, state.svar1);
		}

		// Find a suitable field and walk towards it
		std::vector<Coords> list;
		CheckStepDefault cstep(descr().movecaps());
		FindNodeAnd ffa;

		ffa.add(FindNodeImmovableSize(FindNodeImmovableSize::sizeNone), false);
		ffa.add(FindNodeImmovableAttribute(resource_indicator_attribute), true);

		if (map.find_reachable_fields(game, owner_area, &list, cstep, ffa)) {
			FCoords target;

			auto is_mountain = [&map, &descriptions](const FCoords& f) {
				for (const TCoords<FCoords>& t :
				     {TCoords<FCoords>(f, TriangleIndex::D), TCoords<FCoords>(f, TriangleIndex::R),
				      TCoords<FCoords>(map.tl_n(f), TriangleIndex::D),
				      TCoords<FCoords>(map.tl_n(f), TriangleIndex::R),
				      TCoords<FCoords>(map.tr_n(f), TriangleIndex::D),
				      TCoords<FCoords>(map.l_n(f), TriangleIndex::R)}) {
					if (descriptions
					       .get_terrain_descr((t.t == TriangleIndex::D ? t.node.field->terrain_d() :
                                                                    t.node.field->terrain_r()))
					       ->get_is() &
					    TerrainDescription::Is::kMineable) {
						return true;
					}
				}
				return false;
			};

			// is center a mountain piece?
			const bool is_center_mountain = is_mountain(owner_area);

			// Only run towards fields that are on a mountain (or not)
			// depending on position of center
			bool is_target_mountain;
			uint32_t n = list.size();
			assert(n);
			uint32_t i = game.logic_rand() % n;
			do {
				target = map.get_fcoords(list[game.logic_rand() % list.size()]);
				is_target_mountain = is_mountain(target);
				if (i == 0) {
					i = list.size();
				}
				--i;
				--n;
			} while ((is_center_mountain != is_target_mountain) && n);

			if (!n) {
				// no suitable field found, this is no fail, there's just
				// nothing else to do so let's go home
				// FALLTHROUGH TO RETURN HOME
			} else {
				if (!start_task_movepath(
				       game, target, 0, descr().get_right_walk_anims(does_carry_ware(), this))) {

					molog(game.get_gametime(), "[geologist]: Bug: could not find path\n");
					send_signal(game, "fail");
					return pop_task(game);
				}
				return;
			}
		}

		state.ivar1 = 0;
	}

	if (get_position() == owner_area) {
		return pop_task(game);
	}

	if (!start_task_movepath(
	       game, owner_area, 0, descr().get_right_walk_anims(does_carry_ware(), this))) {
		molog(game.get_gametime(), "[geologist]: could not find path home\n");
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
   "scout", static_cast<Bob::Ptr>(&Worker::scout_update), nullptr, nullptr, true};

/**
 * scout=\<radius\> \<time\>
 *
 * Find a spot that is in the fog of war and go there to see what's up.
 *
 * iparam1 = radius where the scout initially searches for unseen fields
 * iparam2 = maximum search time (in msecs)
 */
bool Worker::run_scout(Game& game, State& state, const Action& action) {
	molog(game.get_gametime(), "  Try scouting for %i ms with search in radius of %i\n",
	      action.iparam2, action.iparam1);
	if (upcast(ProductionSite, productionsite, get_location(game))) {
		productionsite->unnotify_player();
	}
	++state.ivar1;
	start_task_scout(game, action.iparam1, action.iparam2);
	// state reference may be invalid now
	return true;
}

/** Setup scouts_worklist at the start of its task.
 *
 * The first element of scouts_worklist vector stores the location of my hut, at the time of
 * creation.
 * If the building location changes, then pop the now-obsolete list of points of interest
 */
void Worker::prepare_scouts_worklist(const Map& map, const Coords& hutpos) {

	if (!scouts_worklist.empty()) {
		if (map.calc_distance(scouts_worklist[0].scoutme, hutpos) != 0) {
			// Hut has been relocated, I must rebuild the worklist
			scouts_worklist.clear();
		}
	}

	if (scouts_worklist.empty()) {
		// Store the position of homebase
		const PlaceToScout home(hutpos);
		scouts_worklist.push_back(home);
	} else if (1 < scouts_worklist.size()) {
		// If there was an old place to visit in queue, remove it.
		scouts_worklist.pop_back();
	}
}

/** Check if militray sites have become visible by now.
 *
 * After the pop in prepare_scouts_worklist,
 * the latest entry of scouts_worklist is the next MS to visit (if known)
 * Check whether it is still interesting (=whether it is still invisible)
 */
void Worker::check_visible_sites(const Map& map, const Player& player) {
	while (1 < scouts_worklist.size()) {
		if (scouts_worklist.back().randomwalk) {
			return;  // Random walk never goes out of fashion.
		}
		MapIndex mt = map.get_index(scouts_worklist.back().scoutme, map.get_width());
		if (player.is_seeing(mt)) {
			// The military site is now visible. Either player
			// has acquired possession of more military sites
			// of own, or own folks are nearby.
			scouts_worklist.pop_back();
		} else {
			return;
		}
	}
}

/** Make a plan which militar sites (if any) to visit.
 *
 * @param found_sites list of miliar sites to consider.
 */
void Worker::add_sites(Game& game,
                       const Map& map,
                       const Player& player,
                       const std::vector<ImmovableFound>& found_sites) {

	// If there are many enemy sites, push a random walk request into queue every third finding.
	uint32_t haveabreak = 3;

	for (const ImmovableFound& vu : found_sites) {
		upcast(Flag, aflag, vu.object);
		Building* a_building = aflag->get_building();
		// Assuming that this always succeeds.
		if (a_building->descr().type() == MapObjectType::MILITARYSITE) {
			// This would be safe even if this assert failed: Own militarysites are always visible.
			// However: There would be something wrong with FindForeignMilitarySite or associated
			// code. Hence, let's keep the assert.
			assert(a_building->get_owner() != &player);
			const Coords buildingpos = a_building->get_positions(game)[0];
			// Check the visibility: only invisible ones interest the scout.
			MapIndex mx = map.get_index(buildingpos, map.get_width());
			if (!player.is_seeing(mx)) {
				// The find_reachable_immovable sometimes returns multiple instances.
				// TODO(kxq): Is that okay? This could be a performance issue elsewhere.
				// Let's not add duplicates to my work list.
				bool unique = true;
				unsigned worklist_size = scouts_worklist.size();
				for (unsigned t = 1; t < worklist_size; t++) {
					if (buildingpos.x == scouts_worklist[t].scoutme.x &&
					    buildingpos.y == scouts_worklist[t].scoutme.y) {
						unique = false;
						break;
					}
				}
				if (unique) {
					if (1 > --haveabreak) {
						// If there are many MSs to visit, do a random walk in-between also.
						haveabreak = 3;
						const PlaceToScout randomwalk;
						scouts_worklist.push_back(randomwalk);
					}
					// if vision is zero, blacked out.
					// if vision is one, old info exists; unattackable.
					// When entering here, the place is worth scouting.
					const PlaceToScout go_there(buildingpos);
					scouts_worklist.push_back(go_there);
				}
			}
		}
	}

	// I suppose that this never triggers. Anyway. In savegame, I assume that the vector
	// length fits to eight bits. If the entire search area of the scout is full of
	// enemy military sites that are invisible to player, >254 would be possible.
	// Therefore,
	while (254 < scouts_worklist.size()) {
		scouts_worklist.pop_back();
	}
	// (the limit is 254 not 255, since one randomwalk is unconditionally pushed in later)
}

/**
 * Make scout walk random or lurking around some military site.
 *
 * Enemy military sites cannot be attacked, if those are not visible.
 * However, Widelands workers are still somewhat aware of their presence. For example,
 * Player does not acquire ownership of land, even if a militarysite blocking it is
 * invisible. Therefore, it is IMO okay for the scout to be aware of those as well.
 * Scout occasionally pays special attention to enemy military sites, to give the player
 * an opportunity to attack. This is important, if the player can only build small huts
 * and the enemy has one of the biggest ones: without scout, the player has no way of attacking.
 */
void Worker::start_task_scout(Game& game, uint16_t const radius, uint32_t const time) {
	push_task(game, taskScout);
	State& state = top_state();
	state.ivar1 = radius;
	state.ivar2 = game.get_gametime().get() + time;

	// The following code switches between two modes of operation:
	// - Random walk
	// - Lurking near an enemy military site.
	// The code keeps track of interesting military sites, so that they all are visited.
	// When the list of unvisited potential attack targets is exhausted, the list is rebuilt.
	// The first element in the vector is special: It is used to store the location of the scout's
	// hut at the moment of creation. If player dismantles the site and builds a new, the old
	// points of interest are no longer valid and the list is cleared.
	// Random remarks:
	// Some unattackable military sites are also visited (like one under construction).
	// Also, dismantled buildings may end up here. I do not consider these bugs, but if somebody
	// reports, the behavior can always be altered.

	const FCoords& bobpos = get_position();
	assert(nullptr != bobpos.field);

	// Some assumptions: When scout starts working, he is located in his hut.
	// I cannot imagine any situations where this is not the case. However,
	// such situation could trigger bugs.
	// – Now he can also be located at an arbitrary flag instead.
	// But the same code should work fine for that as well.
	const BaseImmovable* homebase = bobpos.field->get_immovable();
	assert(nullptr != homebase);

	const Coords hutpos = homebase->get_positions(game)[0];
	const Map& map = game.map();
	const Player& player = owner();

	// The prepare-routine checks that the list or places to visit is still valid,
	// plus pushes in the "first entry", which is used to x-check the validity.
	prepare_scouts_worklist(map, hutpos);

	// If an enemy military site has become visible, this removes it from the work list.
	// Note that dismantled/burnt military sites are *not* removed from work list.
	// These changes are still somewhat interesting.
	// TODO(kxq): Ideally, if the military site has been dismantled/burnt, then the
	// scout should not spend that long around, but revert to random walking after
	// first visiting the dismantlesite/ruins.
	check_visible_sites(map, player);

	// Check whether there is still undone work in the queue,
	// keeping in mind that 1st element of the vector is special
	if (2 > scouts_worklist.size()) {
		assert(!scouts_worklist.empty());
		// If there was only one entry, worklist has been exhausted. Rebuild it.
		// Time to find new places worth visiting.
		Area<FCoords> revealations(map.get_fcoords(get_position()), state.ivar1);
		std::vector<ImmovableFound> found_sites;
		CheckStepWalkOn csteb(MOVECAPS_WALK, true);
		map.find_reachable_immovables(
		   game, revealations, &found_sites, csteb, FindFlagOf(FindForeignMilitarysite(player)));

		add_sites(game, map, player, found_sites);

		// Always push a "go-anywhere" -directive into work list.
		const PlaceToScout gosomewhere;
		scouts_worklist.push_back(gosomewhere);
	}

	// first get out
	if (upcast(Building, b, get_location(game))) {
		push_task(game, taskLeavebuilding);
		State& stateLeave = top_state();
		stateLeave.ivar1 = false;
		stateLeave.objvar1 = b;
	}
}

bool Worker::scout_random_walk(Game& game, const Map& map, const State& state) {
	Coords oldest_coords = get_position();

	std::vector<Coords> list;  //< List of interesting points
	CheckStepDefault cstep(descr().movecaps());
	FindNodeAnd ffa;
	ffa.add(FindNodeImmovableSize(FindNodeImmovableSize::sizeNone), false);
	Area<FCoords> exploring_area(map.get_fcoords(get_position()), state.ivar1);
	Time oldest_time = game.get_gametime();

	// if some fields can be reached
	if (map.find_reachable_fields(game, exploring_area, &list, cstep, ffa) > 0) {
		// Parse randomly the reachable fields, maximum 50 iterations
		uint8_t iterations = list.size() % 51;
		uint8_t oldest_distance = 0;
		for (uint8_t i = 0; i < iterations; ++i) {
			const std::vector<Coords>::size_type lidx = game.logic_rand() % list.size();
			Coords const coord = list[lidx];
			list.erase(list.begin() + lidx);
			MapIndex idx = map.get_index(coord, map.get_width());
			const VisibleState visible = owner().get_vision(idx);

			// If the field is not yet discovered, go there
			if (visible == VisibleState::kUnexplored) {
				molog(game.get_gametime(), "[scout]: Go to interesting field (%i, %i)\n", coord.x,
				      coord.y);
				if (!start_task_movepath(
				       game, coord, 0, descr().get_right_walk_anims(does_carry_ware(), this))) {
					molog(game.get_gametime(), "[scout]: failed to reach destination\n");
					return false;
				}
				return true;  // start_task_movepath was successful.
			}

			// Else evaluate for second best target
			int dist = map.calc_distance(coord, get_position());
			Time time = owner().fields()[idx].time_node_last_unseen;
			// time is only valid if the field was previously seen but is no longer seen now
			if (visible != VisibleState::kPreviouslySeen) {
				time = oldest_time;
			}

			if (dist > oldest_distance || (dist == oldest_distance && time < oldest_time)) {
				oldest_distance = dist;
				oldest_time = time;
				oldest_coords = coord;
			}
		}

		// All fields discovered, go to second choice target

		if (oldest_coords != get_position()) {
			molog(game.get_gametime(), "[scout]: All fields discovered. Go to (%i, %i)\n",
			      oldest_coords.x, oldest_coords.y);
			if (!start_task_movepath(
			       game, oldest_coords, 0, descr().get_right_walk_anims(does_carry_ware(), this))) {
				molog(game.get_gametime(), "[scout]: Failed to reach destination\n");
				return false;  // If failed go home
			}
			return true;  // Start task movepath success.
		}
	}
	// No reachable fields found.
	molog(game.get_gametime(), "[scout]: nowhere to go!\n");
	return false;
}

/** Make scout hang around an enemy military site.
 *
 */
bool Worker::scout_lurk_around(Game& game, const Map& map, struct Worker::PlaceToScout& scoutat) {
	Coords oldest_coords = get_position();

	std::vector<Coords> surrounding_places;  // locations near the MS under inspection
	CheckStepDefault cstep(descr().movecaps());
	FindNodeAnd fna;
	fna.add(FindNodeImmovableSize(FindNodeImmovableSize::sizeNone), false);

	// scoutat points to the enemy military site; walk in random at vicinity.
	// First try some near-close fields. If no success then try some further off ones.
	// This code is partially copied from scout_random_walk(); I did not check why
	// start_task_movepath
	// would fail. Therefore, the looping can be a bit silly to more knowledgeable readers.
	for (unsigned vicinity = 1; vicinity < 4; vicinity++) {
		Area<FCoords> exploring_area(map.get_fcoords(scoutat.scoutme), vicinity);
		if (map.find_reachable_fields(game, exploring_area, &surrounding_places, cstep, fna) > 0) {
			unsigned formax = surrounding_places.size();
			if (3 + vicinity < formax) {
				formax = 3 + vicinity;
			}
			for (uint8_t i = 0; i < formax; ++i) {
				const std::vector<Coords>::size_type l_idx =
				   game.logic_rand() % surrounding_places.size();
				Coords const coord = surrounding_places[l_idx];
				surrounding_places.erase(surrounding_places.begin() + l_idx);
				// The variable name "oldest_coords" makes sense in the "random walk" branch.
				// Here, it simply is the current position of the scout.
				if (coord.x != oldest_coords.x || coord.y != oldest_coords.y) {
					if (!start_task_movepath(
					       game, coord, 0, descr().get_right_walk_anims(does_carry_ware(), this))) {
						molog(game.get_gametime(), "[scout]: failed to reach destination (x)\n");
						return false;
					}
					return true;  // start_task_movepath was successful.
				}
			}
		}
	}
	return false;
}

void Worker::scout_update(Game& game, State& state) {
	const std::string& signal = get_signal();
	molog(game.get_gametime(), "  Update Scout (%i time)\n", state.ivar2);

	if (!signal.empty()) {
		molog(game.get_gametime(), "[scout]: Interrupted by signal '%s'\n", signal.c_str());
		return pop_task(game);
	}

	const Map& map = game.map();

	const bool do_run = static_cast<int32_t>(state.ivar2 - game.get_gametime().get()) > 0;

	// do not pop; this function is called many times per run.
	struct PlaceToScout scoutat = scouts_worklist.back();

	// If not yet time to go home
	if (do_run) {
		if (scoutat.randomwalk) {
			if (scout_random_walk(game, map, state)) {
				return;
			}
		} else {
			if (scout_lurk_around(game, map, scoutat)) {
				return;
			}
		}
	}
	// time to go home or found nothing to go to
	pop_task(game);
	schedule_act(game, Duration(10));
}

void Worker::draw_inner(const EditorGameBase& game,
                        const Vector2f& point_on_dst,
                        const Coords& coords,
                        const float scale,
                        RenderTarget* dst) const {
	assert(get_owner() != nullptr);
	const RGBColor& player_color = get_owner()->get_playercolor();

	dst->blit_animation(point_on_dst, coords, scale, get_current_anim(),
	                    Time((game.get_gametime() - get_animstart()).get()), &player_color);

	if (WareInstance const* const carried_ware = get_carried_ware(game)) {
		const Vector2f hotspot = descr().ware_hotspot().cast<float>();
		const Vector2f location(
		   point_on_dst.x - hotspot.x * scale, point_on_dst.y - hotspot.y * scale);
		dst->blit_animation(location, Widelands::Coords::null(), scale,
		                    carried_ware->descr().get_animation("idle", this), Time(0),
		                    &player_color);
	}
}

/**
 * Draw the worker, taking the carried ware into account.
 */
void Worker::draw(const EditorGameBase& egbase,
                  const InfoToDraw& /*info_to_draw*/,
                  const Vector2f& field_on_dst,
                  const Widelands::Coords& coords,
                  const float scale,
                  RenderTarget* dst) const {
	if (!get_current_anim()) {
		return;
	}
	draw_inner(egbase, calc_drawpos(egbase, field_on_dst, scale), coords, scale, dst);
}

/*
==============================

Load/save support

==============================
*/

constexpr uint8_t kCurrentPacketVersion = 3;

Worker::Loader::Loader() : location_(0), carried_ware_(0) {
}

void Worker::Loader::load(FileRead& fr) {
	Bob::Loader::load(fr);
	try {
		const uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersion) {

			Worker& worker = get<Worker>();
			location_ = fr.unsigned_32();
			carried_ware_ = fr.unsigned_32();
			worker.current_exp_ = fr.signed_32();

			if (fr.unsigned_8()) {
				worker.transfer_ = new Transfer(dynamic_cast<Game&>(egbase()), worker);
				worker.transfer_->read(fr, transfer_);
			}
			const unsigned veclen = fr.unsigned_8();
			for (unsigned q = 0; q < veclen; q++) {
				if (fr.unsigned_8()) {
					const PlaceToScout gsw;
					worker.scouts_worklist.push_back(gsw);
				} else {
					const int16_t x = fr.signed_16();
					const int16_t y = fr.signed_16();
					Coords peekpos = Coords(x, y);
					const PlaceToScout gtt(peekpos);
					worker.scouts_worklist.push_back(gtt);
				}
			}

		} else {
			throw UnhandledVersionError("Worker", packet_version, kCurrentPacketVersion);
		}
	} catch (const std::exception& e) {
		throw wexception("loading worker: %s", e.what());
	}
}

void Worker::Loader::load_pointers() {
	Bob::Loader::load_pointers();

	Worker& worker = get<Worker>();

	if (location_) {
		worker.set_location(&mol().get<PlayerImmovable>(location_));
	}
	if (carried_ware_) {
		worker.carried_ware_ = &mol().get<WareInstance>(carried_ware_);
	}
	if (worker.transfer_) {
		worker.transfer_->read_pointers(mol(), transfer_);
	}
}

void Worker::Loader::load_finish() {
	Bob::Loader::load_finish();

	Worker& worker = get<Worker>();

	// If our economy is unclear because we have no location, it is wise to not
	// mess with it. For example ships will not be a location for Workers
	// (because they are no PlayerImmovable), but they will handle economies for
	// us and will do so on load too. To make the order at which we are loaded
	// not a factor, we do not overwrite the economy they might have set for us
	// already.
	if (PlayerImmovable* const location = worker.location_.get(egbase())) {
		worker.set_economy(location->get_economy(wwWARE), wwWARE);
		worker.set_economy(location->get_economy(wwWORKER), wwWORKER);
	}
}

const Bob::Task* Worker::Loader::get_task(const std::string& name) {
	if (name == "program") {
		return &taskProgram;
	}
	if (name == "transfer") {
		return &taskTransfer;
	}
	if (name == "shipping") {
		return &taskShipping;
	}
	if (name == "buildingwork") {
		return &taskBuildingwork;
	}
	if (name == "return") {
		return &taskReturn;
	}
	if (name == "gowarehouse") {
		return &taskGowarehouse;
	}
	if (name == "dropoff") {
		return &taskDropoff;
	}
	if (name == "releaserecruit") {
		return &taskReleaserecruit;
	}
	if (name == "fetchfromflag") {
		return &taskFetchfromflag;
	}
	if (name == "waitforcapacity") {
		return &taskWaitforcapacity;
	}
	if (name == "leavebuilding") {
		return &taskLeavebuilding;
	}
	if (name == "fugitive") {
		return &taskFugitive;
	}
	if (name == "geologist") {
		return &taskGeologist;
	}
	if (name == "scout") {
		return &taskScout;
	}
	return Bob::Loader::get_task(name);
}

const MapObjectProgram* Worker::Loader::get_program(const std::string& name) {
	const Worker& worker = get<Worker>();
	return worker.descr().get_program(name);
}

Worker::Loader* Worker::create_loader() {
	return new Loader;
}

/**
 * Load function for all classes derived from \ref Worker
 *
 * Derived classes must override \ref create_loader to make sure
 * the appropriate actual load functions are called.
 */
MapObject::Loader*
Worker::load(EditorGameBase& egbase, MapObjectLoader& mol, FileRead& fr, uint8_t packet_version) {
	if (packet_version == kCurrentPacketVersionMapObject) {
		try {
			// header has already been read by caller
			const WorkerDescr* descr = egbase.descriptions().get_worker_descr(
			   egbase.descriptions().safe_worker_index(fr.c_string()));

			Worker* worker = dynamic_cast<Worker*>(&descr->create_object());
			std::unique_ptr<Loader> loader(worker->create_loader());
			loader->init(egbase, mol, *worker);
			loader->load(fr);
			return loader.release();
		} catch (const std::exception& e) {
			throw wexception("loading worker: %s", e.what());
		}
	} else {
		throw UnhandledVersionError(
		   "MapObjectPacket::Worker", packet_version, kCurrentPacketVersionMapObject);
	}
}

/**
 * Save the \ref Worker specific header and version info.
 *
 * \warning Do not override this function, override \ref do_save instead.
 */
void Worker::save(EditorGameBase& egbase, MapObjectSaver& mos, FileWrite& fw) {
	fw.unsigned_8(HeaderWorker);
	fw.c_string(descr().name());

	do_save(egbase, mos, fw);
}

/**
 * Save the data fields of this worker.
 *
 * This is separate from \ref save because of the way data headers are treated.
 *
 * Override this function in derived classes.
 */
void Worker::do_save(EditorGameBase& egbase, MapObjectSaver& mos, FileWrite& fw) {
	Bob::save(egbase, mos, fw);

	fw.unsigned_8(kCurrentPacketVersion);
	fw.unsigned_32(mos.get_object_file_index_or_zero(location_.get(egbase)));
	fw.unsigned_32(mos.get_object_file_index_or_zero(carried_ware_.get(egbase)));
	fw.signed_32(current_exp_);

	if (transfer_) {
		fw.unsigned_8(1);
		transfer_->write(mos, fw);
	} else {
		fw.unsigned_8(0);
	}

	fw.unsigned_8(scouts_worklist.size());
	for (auto p : scouts_worklist) {
		if (p.randomwalk) {
			fw.unsigned_8(1);
		} else {
			fw.unsigned_8(0);
			// Is there a better way to save Coords? This makes
			// unnecessary assumptions of the internals of Coords
			fw.signed_16(p.scoutme.x);
			fw.signed_16(p.scoutme.y);
		}
	}
}
}  // namespace Widelands
