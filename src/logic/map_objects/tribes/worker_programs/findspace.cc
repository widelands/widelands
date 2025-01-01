/*
 * Copyright (C) 2002-2025 by the Widelands Development Team
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

#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/findnode.h"
#include "logic/map_objects/terrain_affinity.h"
#include "logic/player.h"

namespace Widelands {

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

}  // namespace Widelands
