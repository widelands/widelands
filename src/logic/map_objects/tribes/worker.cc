/*
 * Copyright (C) 2002-2024 by the Widelands Development Team
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
 * name:\<name\>         (optional)
 * Find only objects of this name
 *
 * [no notify]         (optional)
 * Do not notify the player of the result of the search
 *
 * iparam1 = radius predicate
 * iparam2 = attribute predicate (if >= 0)
 * iparam3 = send message on failure (if != 0)
 * sparam1 = type
 * sparam2 = name
 */
bool Worker::run_findobject(Game& game, State& state, const Action& action) {
	CheckStepWalkOn cstep(descr().movecaps(), false);

	const Map& map = game.map();

	// First try to look for immovables that were marked for removal by our player
	if (action.sparam1 == "immovable") {
		std::vector<ImmovableFound> list;
		Area<FCoords> area(map.get_fcoords(get_position()), action.iparam1);
		if (action.sparam2.empty()) {
			if (action.iparam2 < 0) {
				map.find_reachable_immovables(game, area, &list, cstep);
			} else {
				map.find_reachable_immovables(
				   game, area, &list, cstep, FindImmovableAttribute(action.iparam2));
			}
		} else {
			map.find_reachable_immovables(
			   game, area, &list, cstep, FindImmovableByName(action.sparam2));
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
			if (action.sparam2.empty()) {
				if (action.iparam2 < 0) {
					map.find_reachable_immovables(game, area, &list, cstep);
				} else {
					map.find_reachable_immovables(
					   game, area, &list, cstep, FindImmovableAttribute(action.iparam2));
				}
			} else {
				map.find_reachable_immovables(
				   game, area, &list, cstep, FindImmovableByName(action.sparam2));
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
			if (action.sparam2.empty()) {
				if (action.iparam2 < 0) {
					map.find_reachable_bobs(game, area, &list, cstep);
				} else {
					map.find_reachable_bobs(game, area, &list, cstep, FindBobAttribute(action.iparam2));
				}
			} else {
				map.find_reachable_bobs(game, area, &list, cstep, FindBobByName(action.sparam2));
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

	[[nodiscard]] bool accept(const EditorGameBase& egbase, const FCoords& coords) const {
		if ((coords.field->nodecaps() & MOVECAPS_WALK) == 0) {
			return false;
		}

		for (uint8_t dir = FIRST_DIRECTION; dir <= LAST_DIRECTION; ++dir) {
			FCoords const neighb = egbase.map().get_neighbour(coords, dir);
			if (landbased_) {
				if ((neighb.field->maxcaps() & MOVECAPS_WALK) == 0) {
					return false;
				}
			} else {
				if ((neighb.field->nodecaps() & MOVECAPS_SWIM) != 0) {
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
		if (action.iparam4 != 0) {
			functor.add(FindNodeResourceBreedable(descriptions.resource_index(action.sparam1)));
		} else {
			functor.add(FindNodeResource(descriptions.resource_index(action.sparam1)));
		}
	}

	if (action.iparam5 > -1) {
		functor.add(FindNodeImmovableAttribute(action.iparam5), true);
	}
	if ((action.iparam3 & (1 << 0)) != 0) {
		functor.add(FindNodeSpace(findnodesize != FindNodeSize::Size::sizeSwim));
	}
	if ((action.iparam3 & (1 << 2)) != 0) {
		functor.add(FindNodeFerry(owner().player_number()));
	}
	for (const std::string& terraform : action.sparamv) {
		functor.add(FindNodeTerraform(terraform));
	}

	if (map.find_reachable_fields(game, area, &list, cstep, functor) == 0u) {

		// This is default note "out of resources" sent to a player
		FailNotificationType fail_notification_type = FailNotificationType::kDefault;

		// In case this is a fishbreeder, we do more checks
		if (!action.sparam1.empty() && (action.iparam4 != 0)) {

			// We need to create create another functor that will look for nodes full of fish
			FindNodeAnd functorAnyFull;
			functorAnyFull.add(FindNodeSize(static_cast<FindNodeSize::Size>(action.iparam2)));
			functorAnyFull.add(FindNodeResourceBreedable(
			   descriptions.resource_index(action.sparam1), AnimalBreedable::kAnimalFull));

			if (action.iparam5 > -1) {
				functorAnyFull.add(FindNodeImmovableAttribute(action.iparam5), true);
			}
			if ((action.iparam3 & (1 << 0)) != 0) {
				functorAnyFull.add(FindNodeSpace(findnodesize != FindNodeSize::Size::sizeSwim));
			}
			if ((action.iparam3 & (1 << 2)) != 0) {
				functorAnyFull.add(FindNodeFerry(owner().player_number()));
			}
			// If there are fields full of fish, we change the type of notification
			if (map.find_reachable_fields(game, area, &list, cstep, functorAnyFull) != 0u) {
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

		if ((action.iparam3 & (1 << 1)) == 0) {
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
 * <dir>   walk one field in a fixed direction
 *
 * iparam1 = walkXXX
 * iparam2 = direction for walkDir
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

	if ((action.iparam1 & Action::walkDir) != 0) {
		start_task_move(
		   game, action.iparam2, descr().get_right_walk_anims(does_carry_ware(), this), false);
		++state.ivar1;  // next instruction
		return true;
	}

	// Determine the coords we need to walk towards
	if ((action.iparam1 & Action::walkObject) != 0) {
		MapObject* obj = state.objvar1.get(game);

		if (obj != nullptr) {
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
	if (!dest.valid() && ((action.iparam1 & Action::walkCoords) != 0)) {
		dest = state.coords;
	}
	if (!dest.valid()) {
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
	start_task_return(game, action.iparam1 != 0);
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

	if (obj == nullptr) {
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
			throw wexception("MO(%u): [actObject]: bad bob type %s", serial(),
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
		if (state.objvar1.get(game) != nullptr) {
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
		triangles.emplace(TCoords<FCoords>(f, TriangleIndex::R), di);
	}
	di = descriptions.terrain_index(
	   descriptions.get_terrain_descr(f.field->terrain_d())->enhancement(a.sparam1));
	if (di != INVALID_INDEX) {
		triangles.emplace(TCoords<FCoords>(f, TriangleIndex::D), di);
	}
	di = descriptions.terrain_index(
	   descriptions.get_terrain_descr(tln.field->terrain_r())->enhancement(a.sparam1));
	if (di != INVALID_INDEX) {
		triangles.emplace(TCoords<FCoords>(tln, TriangleIndex::R), di);
	}
	di = descriptions.terrain_index(
	   descriptions.get_terrain_descr(tln.field->terrain_d())->enhancement(a.sparam1));
	if (di != INVALID_INDEX) {
		triangles.emplace(TCoords<FCoords>(tln, TriangleIndex::D), di);
	}
	di = descriptions.terrain_index(
	   descriptions.get_terrain_descr(ln.field->terrain_r())->enhancement(a.sparam1));
	if (di != INVALID_INDEX) {
		triangles.emplace(TCoords<FCoords>(ln, TriangleIndex::R), di);
	}
	di = descriptions.terrain_index(
	   descriptions.get_terrain_descr(trn.field->terrain_d())->enhancement(a.sparam1));
	if (di != INVALID_INDEX) {
		triangles.emplace(TCoords<FCoords>(trn, TriangleIndex::D), di);
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

	if ((imm == nullptr) || imm->get_size() <= BaseImmovable::NONE) {

		const ResourceDescription* rdescr =
		   descriptions.get_resource_descr(position.field->get_resources());
		const TribeDescr& t = owner().tribe();
		if ((rdescr != nullptr) && !t.uses_resource(rdescr->name())) {
			rdescr = nullptr;
		}
		const Immovable& ri = game.create_immovable(
		   position,
		   t.get_resource_indicator(rdescr, ((rdescr != nullptr) && rdescr->detectable()) ?
		                                       position.field->get_resources_amount() :
		                                       0),
		   get_owner());

		// Geologist also sends a message notifying the player
		// TODO(GunChleoc): We keep formatting this even when timeout has not elapsed
		if ((rdescr != nullptr) && rdescr->detectable() &&
		    (position.field->get_resources_amount() != 0u)) {
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
 * Call a Lua function.
 */
bool Worker::run_script(Game& game, State& state, const Action& action) {
	MapObjectProgram::do_run_script(game.lua(), this, action.sparam1);

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
	if (imm == nullptr) {
		molog(game.get_gametime(), "run_construct: no objvar1 immovable set");
		send_signal(game, "fail");
		pop_task(game);
		return true;
	}

	imm->set_reserved_by_worker(false);
	WareInstance* ware = get_carried_ware(game);
	if (ware == nullptr) {
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

Worker::Worker(const WorkerDescr& worker_descr) : Bob(worker_descr) {
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
	molog(egbase.get_gametime(), "location: %u\n", imm != nullptr ? imm->serial() : 0);
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

	if (old_location != nullptr) {
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

	if (location != nullptr) {
		Economy* const eco_wo = location->get_economy(wwWORKER);
		Economy* const eco_wa = location->get_economy(wwWARE);

		if ((worker_economy_ == nullptr) || (descr().type() == MapObjectType::SOLDIER)) {
			set_economy(eco_wo, wwWORKER);
		} else if (worker_economy_ != eco_wo) {
			throw wexception("Worker::set_location changes worker_economy, but worker is no soldier");
		}
		if ((ware_economy_ == nullptr) || (descr().type() == MapObjectType::SOLDIER)) {
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
		if (old != nullptr) {
			old->remove_wares_or_workers(owner().tribe().worker_index(descr().name()), 1);
		}
		if (supply_ != nullptr) {
			supply_->set_economy(worker_economy_);
		}
		if (worker_economy_ != nullptr) {
			worker_economy_->add_wares_or_workers(
			   owner().tribe().worker_index(descr().name()), 1, ware_economy_);
		}
	} break;
	default:
		NEVER_HERE();
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

	if (supply_ != nullptr) {
		delete supply_;
		supply_ = nullptr;
	}

	if (ware != nullptr) {
		ware->destroy(egbase);
	}

	// We are destroyed, but we were maybe idling
	// or doing something else. Get Location might
	// init a gowarehouse task or something and this results
	// in a dirty stack. Nono, we do not want to end like this
	if (upcast(Game, game, &egbase)) {
		reset_tasks(*game);
	}

	if (get_location(egbase) != nullptr) {
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

	if (ware != nullptr) {
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
 * Immediately evict the worker from his current building, if allowed.
 */
void Worker::evict(Game& game) {
	if (!is_evict_allowed()) {
		verb_log_warn_time(game.get_gametime(), "Worker %s %u: evict not currently allowed",
		                   descr().name().c_str(), serial());
		return;
	}
	if (!is_employed()) {
		verb_log_warn_time(game.get_gametime(), "Trying to evict worker %s %u who is not employed",
		                   descr().name().c_str(), serial());
		return;
	}

	molog(game.get_gametime(), "Evicting!");
	upcast(Building, building, get_location(game));
	building->notify_worker_evicted(game, *this);
	reset_tasks(game);
	set_location(&building->base_flag());
	start_task_return(game, true);
}

bool Worker::is_evict_allowed() {
	return true;
}

/** Check if this worker is currently employed in a building. */
bool Worker::is_employed() {
	PlayerImmovable* loc = get_location(owner().egbase());
	if (loc == nullptr || loc->descr().type() < MapObjectType::BUILDING) {
		return false;
	}

	if (get_state(taskBuildingwork) != nullptr) {
		// Main worker has task buildingwork anywhere in the stack.
		return true;
	}

	// Additional workers have idle task and no other task.
	return get_stack_size() == 1 && is_idle();
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
	if (get_current_anim() == 0u) {
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

void Worker::Loader::load(FileRead& fr) {
	Bob::Loader::load(fr);
	try {
		const uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersion) {

			Worker& worker = get<Worker>();
			location_ = fr.unsigned_32();
			carried_ware_ = fr.unsigned_32();
			worker.current_exp_ = fr.signed_32();

			if (fr.unsigned_8() != 0u) {
				worker.transfer_ = new Transfer(dynamic_cast<Game&>(egbase()), worker);
				worker.transfer_->read(fr, transfer_);
			}
			const unsigned veclen = fr.unsigned_8();
			for (unsigned q = 0; q < veclen; q++) {
				if (fr.unsigned_8() != 0u) {
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

	if (location_ != 0u) {
		worker.set_location(&mol().get<PlayerImmovable>(location_));
	}
	if (carried_ware_ != 0u) {
		worker.carried_ware_ = &mol().get<WareInstance>(carried_ware_);
	}
	if (worker.transfer_ != nullptr) {
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

	if (transfer_ != nullptr) {
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
