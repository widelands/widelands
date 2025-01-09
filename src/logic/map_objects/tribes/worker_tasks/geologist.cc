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
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/findnode.h"
#include "logic/map_objects/world/terrain_description.h"

namespace Widelands {

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

		if ((imm == nullptr) || (imm->get_size() == BaseImmovable::NONE &&
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

		if (map.find_reachable_fields(game, owner_area, &list, cstep, ffa) != 0u) {
			FCoords target;

			auto is_mountain = [&map, &descriptions](const FCoords& f) {
				auto array = {TCoords<FCoords>(f, TriangleIndex::D),
				              TCoords<FCoords>(f, TriangleIndex::R),
				              TCoords<FCoords>(map.tl_n(f), TriangleIndex::D),
				              TCoords<FCoords>(map.tl_n(f), TriangleIndex::R),
				              TCoords<FCoords>(map.tr_n(f), TriangleIndex::D),
				              TCoords<FCoords>(map.l_n(f), TriangleIndex::R)};
				return std::any_of(
				   array.begin(), array.end(), [&descriptions](const TCoords<FCoords>& t) {
					   return (descriptions
					              .get_terrain_descr((t.t == TriangleIndex::D ?
					                                     t.node.field->terrain_d() :
					                                     t.node.field->terrain_r()))
					              ->get_is() &
					           TerrainDescription::Is::kMineable) != 0;
				   });
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
			} while ((is_center_mountain != is_target_mountain) && (n != 0u));

			if (n == 0u) {
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

}  // namespace Widelands
