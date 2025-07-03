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

}  // namespace Widelands
