/*
 * Copyright (C) 2010-2026 by the Widelands Development Team
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

#include "logic/map_objects/tribes/ship/ship.h"

#include "economy/portdock.h"
#include "economy/ship_fleet.h"

namespace Widelands {

/// updates a ships tasks in transport mode \returns false if failed to update tasks
bool Ship::ship_update_transport(Game& game, Bob::State& state) {
	const Map& map = game.map();

	MapObject* destination_object = destination_object_.get(game);
	assert(destination_object == nullptr ||
	       destination_object->descr().type() == MapObjectType::PORTDOCK);
	PortDock* destination = dynamic_cast<PortDock*>(destination_object);

	if (destination == nullptr) {
		// The ship has no destination, so let it sleep
		ship_update_idle(game, state);
		return true;
	}

	FCoords position = map.get_fcoords(get_position());
	if (position.field->get_immovable() == destination) {
		if (lastdock_ != destination) {
			molog(game.get_gametime(), "ship_update: Arrived at dock %u\n", destination->serial());
			lastdock_ = destination;
		}
		while (withdraw_item(game, *destination)) {
		}

		destination->ship_arrived(game, *this);  // This will also set the destination

		if (destination != nullptr) {
			start_task_movetodock(game, *destination);
		} else {
			start_task_idle(game, descr().main_animation(), 250);
		}
		return true;
	}

	molog(game.get_gametime(), "ship_update: Go to dock %u\n", destination->serial());

	PortDock* lastdock = lastdock_.get(game);
	if (lastdock != nullptr && lastdock != destination) {
		molog(game.get_gametime(), "ship_update: Have lastdock %u\n", lastdock->serial());

		Path path;
		if (fleet_->get_path(*lastdock, *destination, path)) {
			uint32_t closest_idx = std::numeric_limits<uint32_t>::max();
			uint32_t closest_dist = std::numeric_limits<uint32_t>::max();
			Coords closest_target(Coords::null());

			Coords cur(path.get_start());
			for (uint32_t idx = 0; idx <= path.get_nsteps(); ++idx) {
				uint32_t dist = map.calc_distance(get_position(), cur);

				if (dist == 0) {
					molog(game.get_gametime(), "Follow pre-computed path from (%i,%i)  [idx = %u]\n",
					      cur.x, cur.y, idx);

					Path subpath(cur);

					while (idx < path.get_nsteps()) {
						subpath.append(map, path[idx]);
						map.get_neighbour(cur, path[idx], &cur);
						idx++;
						if ((map[cur].nodecaps() & MOVECAPS_SWIM) == 0) {
							molog(game.get_gametime(),
							      "Non swimmable terrain at (%i,%i) recalculate path to port %u\n", cur.x,
							      cur.y, destination->serial());
							fleet_->remove_port(game, destination);
							fleet_->add_port(game, destination);
							if (!fleet_->get_path(*lastdock, *destination, path)) {
								fleet_->split(game);
								send_message(game,
								             /** TRANSLATORS: Ship fleets had to be split */
								             pgettext("ship", "Fleet split"), _("Ship Fleet split"),
								             _("A ship fleet had to be split, because a terrain change "
								               "blocked a passage."),
								             descr().icon_filename());
							}
							return true;
						}
					}

					start_task_movepath(game, subpath, descr().get_sail_anims());
					return true;
				}

				if (dist < closest_dist) {
					closest_dist = dist;
					closest_idx = idx;
				}

				if (idx == closest_idx + closest_dist) {
					closest_target = cur;
				}

				if (idx < path.get_nsteps()) {
					map.get_neighbour(cur, path[idx], &cur);
				}
			}

			if (closest_target.valid()) {
				molog(game.get_gametime(), "Closest target en route is (%i,%i)\n", closest_target.x,
				      closest_target.y);
				if (start_task_movepath(game, closest_target, 0, descr().get_sail_anims())) {
					return true;
				}

				molog(game.get_gametime(), "  Failed to find path!!! Retry full search\n");
			}
		}

		lastdock_ = nullptr;
	}

	start_task_movetodock(game, *destination);
	return true;
}

}  // namespace Widelands
