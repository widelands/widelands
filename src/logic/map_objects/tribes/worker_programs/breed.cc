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
#include "logic/mapregion.h"

namespace Widelands {

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

}  // namespace Widelands
