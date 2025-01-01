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

#include "economy/flag.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/findnode.h"
#include "logic/player.h"

namespace Widelands {

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
void Worker::scout_add_sites(Game& game,
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

		scout_add_sites(game, map, player, found_sites);

		// Always push a "go-anywhere" -directive into work list.
		const PlaceToScout gosomewhere;
		scouts_worklist.push_back(gosomewhere);
	}

	// first get out
	if (upcast(Building, b, get_location(game))) {
		push_task(game, taskLeavebuilding);
		State& stateLeave = top_state();
		stateLeave.ivar1 = 0;
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

}  // namespace Widelands
