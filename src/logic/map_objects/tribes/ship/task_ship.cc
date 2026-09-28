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
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/pinned_note.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/map_objects/tribes/worker.h"

namespace Widelands {

/**
 * Standard behaviour of ships.
 *
 * ivar1 = helper flag for coordination of mutual evasion of ships
 */
// TODO(Nordfriese): Having just 1 global task and those numerous ship_update_x
// functions is ugly. Refactor to use a stack of multiple tasks like every
// other bob. But not while I'm still working on the naval warfare please ;)
const Bob::Task Ship::taskShip = {
   "ship", static_cast<Bob::Ptr>(&Ship::ship_update), nullptr, nullptr,
   true  // unique task
};

void Ship::start_task_ship(Game& game) {
	push_task(game, taskShip);
	top_state().ivar1 = 0;
}

void Ship::ship_wakeup(Game& game) {
	if (get_state(taskShip) != nullptr) {
		send_signal(game, "wakeup");
	}
}

void Ship::ship_update(Game& game, Bob::State& state) {
	// Handle signals
	std::string signal = get_signal();
	if (!signal.empty()) {
		if (signal == "wakeup") {
			signal_handled();
		} else if (signal == "cancel_expedition") {
			pop_task(game);
			PortDock* dst = fleet_->get_arbitrary_dock();
			// TODO(sirver): What happens if there is no port anymore?
			if (dst != nullptr) {
				start_task_movetodock(game, *dst);
			}

			signal_handled();
			return;
		} else {
			send_signal(game, "fail");
			pop_task(game);
			return;
		}
	}

	if (send_message_at_destination_) {
		const MapObject* mo = destination_object_.get(game);
		if (mo == nullptr && destination_coords_ == nullptr) {  // Destination vanished.
			send_message_at_destination_ = false;
			send_message(game, _("Destination Gone"), _("Ship Destination Vanished"),
			             _("Your ship’s destination has disappeared."), descr().icon_filename());
			destination_object_ = nullptr;
		} else {
			bool arrived;
			if (destination_coords_ != nullptr) {
				arrived = destination_coords_->has_dockpoint(get_position());
			} else {
				switch (mo->descr().type()) {
				case MapObjectType::PORTDOCK:
					arrived = get_position().field->get_immovable() == mo;
					break;
				case MapObjectType::SHIP:
					arrived = game.map().calc_distance(
					             get_position(), dynamic_cast<const Ship&>(*mo).get_position()) <=
					          kNearDestinationShipRadius;
					break;
				case MapObjectType::PINNED_NOTE:
					arrived = game.map().calc_distance(
					             get_position(), dynamic_cast<const PinnedNote&>(*mo).get_position()) <=
					          kNearDestinationNoteRadius;
					break;
				default:
					NEVER_HERE();
				}
			}
			if (arrived) {
				send_message_at_destination_ = false;
				send_message(game, _("Ship Arrived"), _("Ship Reached Destination"),
				             _("Your ship has arrived at its destination."), descr().icon_filename());
			}
		}
	}

	if (has_battle()) {
		return battle_update(game);
	}

	if (is_refitting()) {
		assert(fleet_ == nullptr);
		if (PortDock* dest = get_destination_port(game); dest != nullptr) {
			const Map& map = game.map();
			FCoords position = map.get_fcoords(get_position());

			if (position.field->get_immovable() != dest) {
				molog(game.get_gametime(), "Move to dock %u for refit\n", dest->serial());
				start_task_movetodock(game, *dest);
				return;
			}

			// Arrived at destination, now unload and refit
			set_destination(game, nullptr);
			Warehouse* wh = dest->get_warehouse();
			for (ShippingItem& si : items_) {
				/* Since the items may not have been in transit properly,
				 * force their reception instead of doing it the normal way.
				 */
				WareInstance* ware;
				Worker* worker;
				si.get(game, &ware, &worker);
				if (worker == nullptr) {
					assert(ware != nullptr);
					wh->receive_ware(game, game.descriptions().safe_ware_index(ware->descr().name()));
					ware->remove(game);
				} else {
					assert(ware == nullptr);
					worker->set_economy(nullptr, wwWARE);
					worker->set_economy(nullptr, wwWORKER);
					worker->set_position(game, wh->get_position());
					wh->incorporate_worker(game, worker);
				}
			}

			set_ship_type(game, pending_refit_);
			if (ship_type_ == ShipType::kWarship) {
				set_destination(game, dest);
			}

		} else {
			// Destination vanished, try to find a new one
			molog(game.get_gametime(), "Refit failed, retry\n");
			const ShipType t = pending_refit_;
			pending_refit_ = ship_type_;
			refit(game, t);
		}
		return;
	}

	switch (ship_state_) {
	case ShipStates::kTransport:
		if (ship_update_transport(game, state)) {
			return;
		}
		break;
	case ShipStates::kExpeditionPortspaceFound:
	case ShipStates::kExpeditionScouting:
	case ShipStates::kExpeditionWaiting:
		if (ship_update_expedition(game, state)) {
			return;
		}
		break;
	case ShipStates::kExpeditionColonizing:
		break;
	case ShipStates::kSinkRequest:
		if (descr().is_animation_known("sinking")) {
			ship_state_ = ShipStates::kSinkAnimation;
			start_task_idle(game, descr().get_animation("sinking", this), kSinkAnimationDuration);
			return;
		}
		log_warn_time(game.get_gametime(), "Oh no... this ship has no sinking animation :(!\n");
		FALLS_THROUGH;
	case ShipStates::kSinkAnimation:
		// The sink animation has been played, so finally remove the ship from the map
		pop_task(game);
		schedule_destroy(game);
		return;
	default:
		NEVER_HERE();
	}
	// if the real update function failed (e.g. nothing to transport), the ship goes idle
	ship_update_idle(game, state);
}

}  // namespace Widelands
