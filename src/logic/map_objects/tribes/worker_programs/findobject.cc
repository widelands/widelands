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
#include "logic/map_objects/findbob.h"
#include "logic/player.h"

namespace Widelands {

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

}  // namespace Widelands
