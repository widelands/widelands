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

#include "logic/map_objects/tribes/soldier.h"

namespace Widelands {

Bob::Task const Soldier::taskMoveInBattle = {
   "moveInBattle", static_cast<Bob::Ptr>(&Soldier::move_in_battle_update), nullptr, nullptr, true};

void Soldier::start_task_move_in_battle(Game& game, CombatWalkingDir dir) {
	int32_t mapdir = IDLE;

	switch (dir) {
	case CD_WALK_W:
	case CD_RETURN_E:
		mapdir = WALK_W;
		break;
	case CD_WALK_E:
	case CD_RETURN_W:
		mapdir = WALK_E;
		break;
	case CD_NONE:
	case CD_COMBAT_E:
	case CD_COMBAT_W:
	default:
		throw wexception("bad direction '%u'", dir);
	}

	const Map& map = game.map();
	const Duration tdelta = Duration(map.calc_cost(get_position(), mapdir)) / 2;
	molog(game.get_gametime(), "[move_in_battle] dir: (%u) tdelta: (%u)\n", dir, tdelta.get());
	combat_walking_ = dir;
	combat_walkstart_ = game.get_gametime();
	combat_walkend_ = combat_walkstart_ + tdelta;

	push_task(game, taskMoveInBattle);
	State& state = top_state();
	state.ivar1 = dir;
	set_animation(game, descr().get_animation(mapdir == WALK_E ? "walk_e" : "walk_w", this));
}

void Soldier::move_in_battle_update(Game& game, State& /* state */) {
	if (game.get_gametime() >= combat_walkend_) {
		switch (combat_walking_) {
		case CD_NONE:
			break;
		case CD_WALK_W:
			combat_walking_ = CD_COMBAT_W;
			break;
		case CD_WALK_E:
			combat_walking_ = CD_COMBAT_E;
			break;
		case CD_RETURN_W:
		case CD_RETURN_E:
		case CD_COMBAT_W:
		case CD_COMBAT_E:
			combat_walking_ = CD_NONE;
			break;
		default:
			NEVER_HERE();
		}
		return pop_task(game);
	}
	//  Only end the task once we've actually completed the step
	// Ignore signals until then
	return schedule_act(game, combat_walkend_ - game.get_gametime());
}

}  // namespace Widelands
