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

#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/player.h"

namespace Widelands {

// static
void Ship::warship_soldier_callback(Game& game,
                                    Request& req,
                                    DescriptionIndex /* di */,
                                    Worker* worker,
                                    PlayerImmovable& immovable) {
	Warehouse& warehouse = dynamic_cast<Warehouse&>(immovable);
	PortDock* dock = warehouse.get_portdock();
	assert(dock != nullptr);
	Ship* ship = dock->find_ship_for_warship_request(game, req);

	if (ship == nullptr || ship->get_ship_type() != ShipType::kWarship ||
	    ship->get_destination_port(game) != dock ||
	    ship->get_position().field->get_immovable() != dock) {
		verb_log_info_time(game.get_gametime(), "%s %u missed his assigned warship at dock %s",
		                   worker->descr().name().c_str(), worker->serial(),
		                   warehouse.get_warehouse_name().c_str());
		// The soldier is in the port now, ready to get some new assignment by the economy.
		return;
	}

	assert(ship->get_owner() == warehouse.get_owner());
	assert(ship->requestdock_.serial() == dock->serial());
	ship->molog(game.get_gametime(), "%s %u embarked on warship %s", worker->descr().name().c_str(),
	            worker->serial(), ship->get_shipname().c_str());

	worker->set_location(nullptr);
	worker->start_task_shipping(game, nullptr);

	// We may temporarily exceed the ship's capacity while swapping heroes and rookies
	const Quantity old_capacity = ship->capacity_;
	ship->capacity_ = std::max<Quantity>(old_capacity, ship->items_.size() + 1);

	ship->add_item(game, ShippingItem(*worker));
	ship->update_warship_soldier_request(false);
	ship->kickout_superfluous_soldiers(game);

	assert(ship->items_.size() <= old_capacity);
	ship->capacity_ = old_capacity;
}

uint32_t Ship::min_warship_soldier_capacity() const {
	return (ship_type_ != ShipType::kWarship || is_on_destination_dock()) ? 0U : get_nritems();
}

std::vector<Soldier*> Ship::onboard_soldiers() const {
	std::vector<Soldier*> result;
	for (const ShippingItem& si : items_) {
		Worker* worker;
		si.get(owner().egbase(), nullptr, &worker);
		if (worker != nullptr && worker->descr().type() == MapObjectType::SOLDIER) {
			result.push_back(dynamic_cast<Soldier*>(worker));
		}
	}
	return result;
}

std::vector<Soldier*> Ship::associated_soldiers() const {
	std::vector<Soldier*> result = onboard_soldiers();

	if (PortDock* dock = requestdock_.get(owner().egbase()); dock != nullptr) {
		if (const SoldierRequestManager* srm = dock->get_warship_request_manager(serial());
		    srm != nullptr) {
			if (const Request* request = srm->get_request(); request != nullptr) {
				for (const Transfer* t : request->get_transfers()) {
					Soldier& s = dynamic_cast<Soldier&>(*t->get_worker());
					result.push_back(&s);
				}
			}
		}
	}

	return result;
}

void Ship::drop_soldier(Game& game, Serial soldier) {
	PortDock* dest = get_destination_port(game);
	if (dest == nullptr) {
		verb_log_warn_time(game.get_gametime(), "Ship not in dock, cannot drop soldier");
		return;
	}

	for (size_t i = 0; i < items_.size(); ++i) {
		Worker* worker;
		items_[i].get(game, nullptr, &worker);
		if (worker != nullptr && worker->serial() == soldier) {
			dest->shipping_item_arrived(game, items_[i]);

			items_[i] = items_.back();
			items_.pop_back();
			return;
		}
	}
	verb_log_warn_time(game.get_gametime(), "Ship::drop_soldier: %u is not on board", soldier);
}

/** If we have too many soldiers on board, unload the extras. */
void Ship::kickout_superfluous_soldiers(Game& game) {
	PortDock* dest = get_destination_port(game);
	if (dest == nullptr) {
		return;  // Not in port
	}

	while (get_nritems() > warship_soldier_capacity_) {
		// Always kick out a rookie, unless rookies are preferred.
		ShippingItem* worst_fit = nullptr;
		unsigned worst_fit_level = 0;
		for (ShippingItem& si : items_) {
			Worker* worker;
			si.get(game, nullptr, &worker);
			Soldier* soldier = dynamic_cast<Soldier*>(worker);
			if (soldier == nullptr) {
				continue;
			}
			unsigned soldier_level = soldier->get_total_level();
			if (worst_fit == nullptr || (get_soldier_preference() == SoldierPreference::kRookies ?
			                                soldier_level >= worst_fit_level :
			                                soldier_level <= worst_fit_level)) {
				worst_fit = &si;
				worst_fit_level = soldier_level;
			}
		}

		assert(worst_fit != nullptr);
		molog(game.get_gametime(), "Kicking out soldier with total level %u", worst_fit_level);
		dest->shipping_item_arrived(game, *worst_fit);
		*worst_fit = items_.back();
		items_.pop_back();
	}
}

}  // namespace Widelands
