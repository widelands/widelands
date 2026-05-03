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
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/mapastar.h"

namespace Widelands {

/**
 * Find a path to the dock @p pd, returns its length, and the path optionally.
 */
uint32_t Ship::calculate_sea_route(EditorGameBase& egbase, PortDock& pd, Path* finalpath) const {
	Map* map = egbase.mutable_map();
	StepEvalAStar se(pd.get_warehouse()->get_position());
	se.swim_ = true;
	se.conservative_ = false;
	se.estimator_bias_ = -5 * map->calc_cost(0);

	MapAStar<StepEvalAStar> astar(*map, se, wwWORKER);

	astar.push(get_position());

	int32_t cost;
	FCoords cur;
	while (astar.step(cur, cost)) {
		if (cur.field->get_immovable() == &pd) {
			if (finalpath != nullptr) {
				astar.pathto(cur, *finalpath);
				return finalpath->get_nsteps();
			}
			Path path;
			astar.pathto(cur, path);
			return path.get_nsteps();
		}
	}

	molog(egbase.get_gametime(), "   calculate_sea_distance: Failed to find path!\n");
	return std::numeric_limits<uint32_t>::max();
}

/**
 * Find a path to the dock @p pd and follow it without using precomputed paths.
 * Returns false if the dock is unreachable.
 */
bool Ship::start_task_movetodock(Game& game, PortDock& pd) {
	erase_warship_soldier_request_manager();

	Path path;

	uint32_t const distance = calculate_sea_route(game, pd, &path);

	// if we get a meaningfull result
	if (distance < std::numeric_limits<uint32_t>::max()) {
		start_task_movepath(game, path, descr().get_sail_anims());
		return true;
	}

	log_warn_time(
	   game.get_gametime(),
	   "start_task_movedock: Failed to find a path: ship at %3dx%3d to port at: %3dx%3d\n",
	   get_position().x, get_position().y, pd.get_positions(game)[0].x, pd.get_positions(game)[0].y);
	ShipFleet* fleet = fleet_ != nullptr ? fleet_ : pd.get_fleet();
	if (fleet != nullptr) {
		fleet->split(game);
		send_message(game,
		             /** TRANSLATORS: Ship fleets had to be split */
		             pgettext("ship", "Fleet split"), _("Ship Fleet split"),
		             _("A ship fleet had to be split, because a terrain change blocked a passage."),
		             descr().icon_filename());
	}
	return false;
}

PortDock* Ship::find_nearest_port(Game& game) {
	const bool in_fleet = fleet_ != nullptr;
	Economy* eco_ware = ware_economy_;
	Economy* eco_worker = worker_economy_;

	if (!in_fleet) {
		init_fleet(game);
		assert(fleet_ != nullptr);
	}

	PortDock* nearest = nullptr;
	int32_t dist = 0;
	for (PortDock* pd : fleet_->get_ports()) {
		Path path;
		int32_t d = -1;
		if (calculate_sea_route(game, *pd, &path) != std::numeric_limits<uint32_t>::max()) {
			game.map().calc_cost(path, &d, nullptr);
			assert(d >= 0);
			if (nearest == nullptr || d < dist) {
				dist = d;
				nearest = pd;
			}
		}
	}

	if (!in_fleet) {
		fleet_->remove_ship(game, this);
		assert(fleet_ == nullptr);
		set_economy(game, eco_ware, wwWARE);
		set_economy(game, eco_worker, wwWORKER);
	}

	return nearest;
}

}  // namespace Widelands
