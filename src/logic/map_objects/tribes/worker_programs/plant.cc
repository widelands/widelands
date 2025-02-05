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

#include "logic/game_data_error.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/terrain_affinity.h"

namespace Widelands {

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

}  // namespace Widelands
