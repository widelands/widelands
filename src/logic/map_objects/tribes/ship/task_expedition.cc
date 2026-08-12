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

#include "economy/ship_fleet.h"
#include "logic/map_objects/checkstep.h"
#include "logic/player.h"

namespace Widelands {

struct FindBobEnemyWarship : public FindBob {
	explicit FindBobEnemyWarship(const Widelands::Ship& s) : ship_(s) {
	}
	[[nodiscard]] bool accept(Bob* bob) const override {
		return ship_.is_attackable_enemy_warship(*bob);
	}

private:
	const Widelands::Ship& ship_;
};

/// Prepare everything for the coming exploration
void Ship::start_task_expedition(Game& game) {
	// Initialize a new, yet empty expedition
	expedition_.reset(new Expedition());
	expedition_->seen_port_buildspaces.clear();
	expedition_->island_exploration = false;
	expedition_->scouting_direction = WalkingDir::IDLE;
	expedition_->exploration_start = Coords(0, 0);
	expedition_->island_explore_direction = IslandExploreDirection::kClockwise;
	expedition_->attack_targets.clear();
	expedition_->ware_economy = get_owner()->create_economy(wwWARE);
	expedition_->worker_economy = get_owner()->create_economy(wwWORKER);

	// Now we are waiting
	set_ship_state_and_notify(ShipStates::kExpeditionWaiting, NoteShip::Action::kDestinationChanged);

	// We are no longer in any other economy, but instead are an economy of our
	// own.
	if (fleet_ != nullptr) {
		fleet_->remove_ship(game, this);
		assert(fleet_ == nullptr);
	}

	set_economy(game, expedition_->ware_economy, wwWARE);
	set_economy(game, expedition_->worker_economy, wwWORKER);

	for (const ShippingItem& si : items_) {
		WareInstance* ware;
		Worker* worker;
		si.get(game, &ware, &worker);
		if (worker != nullptr) {
			worker->reset_tasks(game);
			worker->start_task_idle(game, 0, -1);
		} else {
			assert(ware);
		}
	}

	// Send a message to the player that an expedition is ready to go
	if (ship_type_ == ShipType::kWarship) {
		send_message(game,
		             /** TRANSLATORS: Warship ready */
		             pgettext("ship", "Warship"), _("Warship Ready"),
		             _("A warship is waiting for your commands."),
		             "images/wui/buildings/start_expedition.png");
	} else {
		send_message(game,
		             /** TRANSLATORS: Ship expedition ready */
		             pgettext("ship", "Expedition"), _("Expedition Ready"),
		             _("An expedition ship is waiting for your commands."),
		             "images/wui/buildings/start_expedition.png");
	}
	Notifications::publish(NoteShip(this, NoteShip::Action::kWaitingForCommand));
}

/// updates a ships tasks in expedition mode; returns whether tasks were updated
bool Ship::ship_update_expedition(Game& game, Bob::State& /* state */) {
	Map* map = game.mutable_map();

	assert(expedition_ != nullptr);
	const FCoords position = get_position();

	// Update the knowledge of the surrounding fields
	recalc_expedition_swimmable(game);

	if (get_ship_type() == ShipType::kWarship) {
		// Look for nearby enemy warships.
		Area<FCoords> area(get_position(), descr().vision_range());
		bool found_new_target = false;
		std::vector<Bob*> candidates;
		map->find_reachable_bobs(
		   game, area, &candidates, CheckStepDefault(MOVECAPS_SWIM), FindBobEnemyWarship(*this));

		// Clear outdated attack targets.
		std::set<OPtr<Ship>>& attack_targets = expedition_->attack_targets;
		for (auto it = attack_targets.begin(); it != attack_targets.end();) {
			if (std::find_if(candidates.begin(), candidates.end(), [&it](Bob* b) {
				    return b->serial() == it->serial();
			    }) != candidates.end()) {
				++it;
			} else {
				it = attack_targets.erase(it);
			}
		}

		// Add new attack targets.
		for (Bob* enemy : candidates) {
			if (attack_targets.insert(OPtr<Ship>(dynamic_cast<Ship*>(enemy))).second) {
				found_new_target = true;
				send_message(game, _("Enemy Ship"), _("Enemy Ship Spotted"),
				             _("A warship spotted an enemy ship."), enemy->descr().icon_filename());
			}
		}

		// TODO(tothxa): Implement expedition options for stop_on_report and report_known
		//               (report_known can probably always be disabled when stopping is disabled)
		if (!update_seen_portspaces(game, !has_destination(), !has_destination()) &&
		    found_new_target) {
			set_ship_state_and_notify(
			   ShipStates::kExpeditionWaiting, NoteShip::Action::kWaitingForCommand);
		}
	}

	if (destination_coords_ != nullptr) {
		erase_warship_soldier_request_manager();

		if (destination_coords_->has_dockpoint(get_position())) {  // Already there
			destination_coords_ = nullptr;
			start_task_idle(game, descr().main_animation(), 250);
			return true;
		}

		if (start_task_movepath(
		       game, destination_coords_->dockpoints.front(), 0, descr().get_sail_anims())) {
			return true;
		}

		molog(game.get_gametime(), "Could not find path to destination at %dx%d",
		      destination_coords_->dockpoints.front().x, destination_coords_->dockpoints.front().y);
		if (send_message_at_destination_) {
			send_message(game, _("Destination Unreachable"), _("Ship Destination Unreachable"),
			             _("Your ship could not find a path to its destination."),
			             descr().icon_filename());
			send_message_at_destination_ = false;
		}
		destination_coords_ = nullptr;

	} else if (MapObject* destination_object = destination_object_.get(game);
	           destination_object != nullptr) {
		switch (destination_object->descr().type()) {
		case MapObjectType::PORTDOCK: {
			PortDock* dest = dynamic_cast<PortDock*>(destination_object);

			// Sail to the destination port if we're not there yet.
			if (position.field->get_immovable() != dest) {
				erase_warship_soldier_request_manager();
				if (!start_task_movetodock(game, *dest)) {
					if (send_message_at_destination_) {
						send_message(
						   game, _("Destination Unreachable"), _("Ship Destination Unreachable"),
						   format(_("Your ship could not find a path to its destination port ‘%s’."),
						          dest->get_warehouse()->get_warehouse_name()),
						   descr().icon_filename());
					}
					send_message_at_destination_ = false;
					destination_object_ = nullptr;
					start_task_idle(game, descr().main_animation(), 250);
				}
				return true;
			}

			// We're on the destination dock. Load soldiers, heal, and wait for orders.
			constexpr Duration kHealInterval(1000);
			if (hitpoints_ < descr().max_hitpoints_ &&
			    game.get_gametime() - last_heal_time_ >= kHealInterval) {
				last_heal_time_ = game.get_gametime();
				hitpoints_ = std::min(descr().max_hitpoints_, hitpoints_ + descr().heal_per_second_);
			}

			lastdock_ = dest;
			set_ship_state_and_notify(
			   ShipStates::kExpeditionWaiting, NoteShip::Action::kWaitingForCommand);

			update_warship_soldier_request(true);

			start_task_idle(game, descr().main_animation(), 250);

			return true;
		}

		case MapObjectType::SHIP:
		case MapObjectType::PINNED_NOTE: {
			erase_warship_soldier_request_manager();
			Bob* dest = dynamic_cast<Bob*>(destination_object);

			if (map->calc_distance(position, dest->get_position()) <=
			    (dest->descr().type() == MapObjectType::SHIP ? kNearDestinationShipRadius :
			                                                   kNearDestinationNoteRadius)) {
				// Already there, idle and await further orders.
				start_task_idle(game, descr().main_animation(), 250);
				return true;
			}

			// Sail to the destination ship/note if we're not there yet.
			Path path;
			if (map->findpath(
			       position, dest->get_position(), 0, path, CheckStepDefault(MOVECAPS_SWIM)) < 0) {
				molog(game.get_gametime(), "No path to destination ship/note found!");
				set_destination(game, nullptr);
				break;
			}

			// Ships tend to move around, so we need to recompute the path after every few steps.
			constexpr unsigned kMaxSteps = 4;
			if (path.get_nsteps() <= kMaxSteps) {
				start_task_movepath(game, path, descr().get_sail_anims());
				return true;
			}

			Path truncated(path.get_start());
			for (unsigned i = 0; i < kMaxSteps; ++i) {
				truncated.append(*map, path[i]);
			}
			start_task_movepath(game, truncated, descr().get_sail_anims());

			return true;
		}

		default:
			NEVER_HERE();
		}
	}

	erase_warship_soldier_request_manager();  // Clear the request when not in port

	if (ship_state_ == ShipStates::kExpeditionScouting && get_ship_type() == ShipType::kTransport) {
		// TODO(tothxa): Implement expedition options for stop_on_report and report_known
		//               (report_known can probably always be disabled when stopping is disabled)
		update_seen_portspaces(game, !has_destination(), !has_destination());
	} else if (ship_state_ == ShipStates::kExpeditionPortspaceFound) {
		check_port_space_still_available(game);
	}

	return false;  // Continue with the regular expedition updates
}

/// Initializes / changes the direction of scouting to @arg direction
/// @note only called via player command
void Ship::exp_scouting_direction(Game& game, WalkingDir scouting_direction) {
	assert(expedition_ != nullptr);
	destination_object_ = nullptr;
	destination_coords_ = nullptr;
	if (scouting_direction == WalkingDir::IDLE) {
		if (ship_type_ == ShipType::kTransport && !expedition_->seen_port_buildspaces.empty()) {
			set_ship_state_and_notify(
			   ShipStates::kExpeditionPortspaceFound, NoteShip::Action::kWaitingForCommand);
		} else {
			set_ship_state_and_notify(
			   ShipStates::kExpeditionWaiting, NoteShip::Action::kWaitingForCommand);
		}
	} else {
		set_ship_state_and_notify(
		   ShipStates::kExpeditionScouting, NoteShip::Action::kDestinationChanged);
	}
	expedition_->scouting_direction = scouting_direction;
	expedition_->island_exploration = false;
	set_destination(game, nullptr);
}

WalkingDir Ship::get_scouting_direction() const {
	if (expedition_ != nullptr && ship_state_ == ShipStates::kExpeditionScouting &&
	    !expedition_->island_exploration) {
		return expedition_->scouting_direction;
	}
	return WalkingDir::IDLE;
}

/// Initializes the construction of a port at @arg c
/// @note only called via player command
void Ship::exp_construct_port(Game& game, const Coords& c) {
	assert(expedition_ != nullptr);
	// recheck ownership and availability before setting the csite
	if (!check_port_space_still_available(game)) {
		return;
	}
	get_owner()->force_csite(c, get_owner()->tribe().port()).set_destruction_blocked(true);

	// Make sure that we have space to squeeze in a lumberjack or a quarry
	std::vector<ImmovableFound> all_immos;
	game.map().find_immovables(game, Area<FCoords>(game.map().get_fcoords(c), 3), &all_immos,
	                           FindImmovableType(MapObjectType::IMMOVABLE));
	for (auto& immo : all_immos) {
		if (immo.object->descr().has_attribute(MapObjectDescr::get_attribute_id("rocks")) ||
		    dynamic_cast<Immovable*>(immo.object)->descr().has_terrain_affinity()) {
			immo.object->remove(game);
		}
	}
	set_ship_state_and_notify(
	   ShipStates::kExpeditionColonizing, NoteShip::Action::kDestinationChanged);
	// Update ownership
	remember_detected_portspace(c);
}

/// Initializes / changes the direction the island exploration in @arg island_explore_direction
/// direction
/// @note only called via player command
void Ship::exp_explore_island(Game& game, IslandExploreDirection island_explore_direction) {
	assert(expedition_ != nullptr);
	destination_object_ = nullptr;
	destination_coords_ = nullptr;
	set_ship_state_and_notify(
	   ShipStates::kExpeditionScouting, NoteShip::Action::kDestinationChanged);
	expedition_->island_explore_direction = island_explore_direction;
	expedition_->scouting_direction = WalkingDir::IDLE;
	expedition_->island_exploration = true;
	set_destination(game, nullptr);
}

IslandExploreDirection Ship::get_island_explore_direction() const {
	if (expedition_ != nullptr && ship_state_ == ShipStates::kExpeditionScouting &&
	    expedition_->island_exploration) {
		return expedition_->island_explore_direction;
	}
	return IslandExploreDirection::kNotSet;
}

/// Cancels a currently running expedition
/// @note only called via player command
void Ship::exp_cancel(Game& game) {
	// Running colonization has the highest priority before cancelation
	// + cancelation only works if an expedition is actually running

	if ((ship_state_ == ShipStates::kExpeditionColonizing) || !state_is_expedition() ||
	    get_ship_type() == ShipType::kWarship) {
		return;
	}

	// The workers were hold in an idle state so that they did not try
	// to become fugitive or run to the next warehouse. But now, we
	// have a proper destination, so we can just inform them that they
	// are now getting shipped there.
	// Theres nothing to be done for wares - they already changed
	// economy with us and the warehouse will make sure that they are
	// getting used.
	Worker* worker;
	for (ShippingItem& item : items_) {
		item.get(game, nullptr, &worker);
		if (worker != nullptr) {
			worker->reset_tasks(game);
			worker->start_task_shipping(game, nullptr);
		}
	}
	set_ship_state_and_notify(ShipStates::kTransport, NoteShip::Action::kDestinationChanged);

	// Bring us back into a fleet and a economy.
	set_economy(game, nullptr, wwWARE);
	set_economy(game, nullptr, wwWORKER);
	destination_object_ = nullptr;  // remove potential destination, ...
	destination_coords_ = nullptr;  // ... for assert in ShippingSchedule::ship_added()

	init_fleet(game);
	if ((get_fleet() == nullptr) || !get_fleet()->has_ports()) {
		// We lost our last reachable port, so we reset the expedition's state
		set_ship_state_and_notify(
		   ShipStates::kExpeditionWaiting, NoteShip::Action::kDestinationChanged);
		if (fleet_ != nullptr) {
			fleet_->remove_ship(game, this);
			assert(fleet_ == nullptr);
		}
		set_economy(game, expedition_->ware_economy, wwWARE);
		set_economy(game, expedition_->worker_economy, wwWORKER);

		worker = nullptr;
		for (ShippingItem& item : items_) {
			item.get(game, nullptr, &worker);
			if (worker != nullptr) {
				worker->reset_tasks(game);
				worker->start_task_idle(game, 0, -1);
			}
		}

		Notifications::publish(NoteShip(this, NoteShip::Action::kNoPortLeft));
		return;
	}
	assert(get_economy(wwWARE) && get_economy(wwWARE) != expedition_->ware_economy);
	assert(get_economy(wwWORKER) && get_economy(wwWORKER) != expedition_->worker_economy);

	send_signal(game, "cancel_expedition");

	// Delete the expedition and the economy it created.
	expedition_.reset(nullptr);
}

}  // namespace Widelands
