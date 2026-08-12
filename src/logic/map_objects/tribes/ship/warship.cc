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

#include "logic/player.h"

namespace Widelands {

void Ship::set_warship_soldier_capacity(Quantity c) {
	assert(c <= capacity_);
	warship_soldier_capacity_ = c;
}

bool Ship::is_attackable_enemy_warship(const Bob& b) const {
	if (b.descr().type() != MapObjectType::SHIP) {
		return false;
	}
	const Ship& s = dynamic_cast<const Ship&>(b);
	return s.can_be_attacked() && owner().is_hostile(s.owner());
}

bool Ship::can_be_attacked() const {
	return get_ship_type() == ShipType::kWarship;
}

bool Ship::can_attack() const {
	return get_ship_type() == ShipType::kWarship && !has_battle();
}

void Ship::warship_command(Game& game,
                           const WarshipCommand cmd,
                           const std::vector<uint32_t>& parameters) {
	switch (cmd) {
	case WarshipCommand::kSetCapacity: {
		assert(parameters.size() == 1);
		warship_soldier_capacity_ =
		   std::max(std::min(parameters.back(), get_capacity()), min_warship_soldier_capacity());
		if (get_ship_type() == ShipType::kWarship) {
			update_warship_soldier_request(false);
			kickout_superfluous_soldiers(game);
		}
		return;
	}

	case WarshipCommand::kAttack:
		assert(!parameters.empty());
		if (!can_attack()) {
			return;
		}

		if (parameters.size() == 1) {  // Attacking a ship.
			if (Ship* target = dynamic_cast<Ship*>(game.objects().get_object(parameters.front()));
			    target != nullptr) {
				start_battle(game, Battle(target, Coords::null(), {}, true), false);
			}
		} else {  // Attacking port coordinates.
			assert(parameters.size() > 2);
			Coords portspace(parameters.at(0), parameters.at(1));
			assert(game.map().is_port_space(portspace));

			std::vector<Coords> dockpoints = game.map().find_portdock(portspace, true);
			assert(!dockpoints.empty());

			start_battle(game,
			             Battle(nullptr, dockpoints.at(game.logic_rand() % dockpoints.size()),
			                    std::vector<uint32_t>(parameters.begin() + 2, parameters.end()), true),
			             false);
		}
		return;

	default:
		throw wexception("Invalid warship command %d", static_cast<int>(cmd));
	}
}

void Ship::set_soldier_preference(SoldierPreference pref) {
	soldier_preference_ = pref;
	update_warship_soldier_request(false);
}

void Ship::erase_warship_soldier_request_manager() {
	const EditorGameBase& egbase = owner().egbase();
	if (PortDock* dock = requestdock_.get(egbase); dock != nullptr) {
		molog(egbase.get_gametime(), "Erasing soldier request manager at %s",
		      dock->get_warehouse()->get_warehouse_name().c_str());
		dock->erase_warship_request_manager(serial());
	}
	requestdock_ = nullptr;
}

void Ship::update_warship_soldier_request(bool create) {
	if (ship_type_ != ShipType::kWarship || is_refitting()) {
		erase_warship_soldier_request_manager();
		return;
	}

	const EditorGameBase& egbase = owner().egbase();
	PortDock* dock = requestdock_.get(egbase);

	if (dock != nullptr) {
		// We should already have a request manager
		molog(egbase.get_gametime(), "Updating existing soldier request manager at %s",
		      dock->get_warehouse()->get_warehouse_name().c_str());
		SoldierRequestManager* srm = dock->get_warship_request_manager(serial());
		if (srm == nullptr) {
			throw wexception("Ship %s has no soldier request manager at request dock %s",
			                 get_shipname().c_str(),
			                 dock->get_warehouse()->get_warehouse_name().c_str());
		}
		srm->set_preference(soldier_preference_);
		srm->update();
		return;
	}

	if (!create) {
		// No request exists and none is desired.
		return;
	}

	// Create a new request manager at the current port.
	dock = lastdock_.get(owner().egbase());
	if (dock == nullptr) {
		throw wexception(
		   "Ship %s attempts to create warship soldier request manager while not in dock",
		   get_shipname().c_str());
	}
	molog(egbase.get_gametime(), "Creating new soldier request manager at %s",
	      dock->get_warehouse()->get_warehouse_name().c_str());
	if (get_position().field->get_immovable() != dock) {
		throw wexception(
		   "Ship %s attempts to create warship soldier request manager while not on request dock %s",
		   get_shipname().c_str(), dock->get_warehouse()->get_warehouse_name().c_str());
	}

	SoldierRequestManager* srm = dock->get_warship_request_manager(serial());
	if (srm == nullptr) {
		srm = &dock->create_warship_request_manager(this, soldier_preference_);
	} else {
		// TODO(Nordfriese): Can only happen with legacy savegames, replace this with an assert
		log_warn("Ship %s already has soldier request manager at new request dock %s",
		         get_shipname().c_str(), dock->get_warehouse()->get_warehouse_name().c_str());
	}

	requestdock_ = dock;
	srm->update();
}

}  // namespace Widelands
