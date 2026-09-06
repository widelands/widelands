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
#include "logic/map_objects/pinned_note.h"
#include "logic/player.h"

namespace Widelands {

bool Ship::has_destination() const {
	return destination_coords_ != nullptr || destination_object_.get(owner().egbase()) != nullptr;
}
PortDock* Ship::get_destination_port(EditorGameBase& e) const {
	if (MapObject* mo = destination_object_.get(e);
	    mo != nullptr && mo->descr().type() == MapObjectType::PORTDOCK) {
		return dynamic_cast<PortDock*>(mo);
	}
	return nullptr;
}
Ship* Ship::get_destination_ship(EditorGameBase& e) const {
	if (MapObject* mo = destination_object_.get(e);
	    mo != nullptr && mo->descr().type() == MapObjectType::SHIP) {
		return dynamic_cast<Ship*>(mo);
	}
	return nullptr;
}
PinnedNote* Ship::get_destination_note(EditorGameBase& e) const {
	if (MapObject* mo = destination_object_.get(e);
	    mo != nullptr && mo->descr().type() == MapObjectType::PINNED_NOTE) {
		return dynamic_cast<PinnedNote*>(mo);
	}
	return nullptr;
}

const PortDock* Ship::get_destination_port(const EditorGameBase& e) const {
	if (const MapObject* mo = destination_object_.get(e);
	    mo != nullptr && mo->descr().type() == MapObjectType::PORTDOCK) {
		return dynamic_cast<const PortDock*>(mo);
	}
	return nullptr;
}
const Ship* Ship::get_destination_ship(const EditorGameBase& e) const {
	if (const MapObject* mo = destination_object_.get(e);
	    mo != nullptr && mo->descr().type() == MapObjectType::SHIP) {
		return dynamic_cast<const Ship*>(mo);
	}
	return nullptr;
}
const PinnedNote* Ship::get_destination_note(const EditorGameBase& e) const {
	if (const MapObject* mo = destination_object_.get(e);
	    mo != nullptr && mo->descr().type() == MapObjectType::PINNED_NOTE) {
		return dynamic_cast<const PinnedNote*>(mo);
	}
	return nullptr;
}

bool Ship::is_on_destination_dock() const {
	const MapObject* dest = destination_object_.get(owner().egbase());
	return dest != nullptr && dest->descr().type() == MapObjectType::PORTDOCK &&
	       get_position().field->get_immovable() == dest;
}

void Ship::set_destination(EditorGameBase& egbase, MapObject* dest, bool is_playercommand) {
	assert(dest == nullptr || dest->descr().type() == MapObjectType::PORTDOCK ||
	       dest->descr().type() == MapObjectType::SHIP ||
	       dest->descr().type() == MapObjectType::PINNED_NOTE);

	destination_object_ = dest;
	destination_coords_ = nullptr;
	send_message_at_destination_ = is_playercommand;

	if (upcast(Game, g, &egbase)) {
		send_signal(*g, "wakeup");
	}

	if (is_playercommand) {
		assert(ship_state_ != ShipStates::kTransport);
		assert(expedition_ != nullptr);

		expedition_->scouting_direction = WalkingDir::IDLE;
		expedition_->island_exploration = false;

		set_ship_state_and_notify(
		   dest == nullptr ? ShipStates::kExpeditionWaiting : ShipStates::kExpeditionScouting,
		   NoteShip::Action::kDestinationChanged);
	} else {
		Notifications::publish(NoteShip(this, NoteShip::Action::kDestinationChanged));
	}
}
void Ship::set_destination(EditorGameBase& egbase,
                           const DetectedPortSpace& dest,
                           bool is_playercommand) {
	destination_object_ = nullptr;
	destination_coords_ = &dest;
	send_message_at_destination_ = is_playercommand;

	if (upcast(Game, g, &egbase)) {
		send_signal(*g, "wakeup");
	}

	if (is_playercommand) {
		assert(ship_state_ != ShipStates::kTransport);
		assert(expedition_ != nullptr);

		expedition_->scouting_direction = WalkingDir::IDLE;
		expedition_->island_exploration = false;

		set_ship_state_and_notify(
		   ShipStates::kExpeditionScouting, NoteShip::Action::kDestinationChanged);
	} else {
		Notifications::publish(NoteShip(this, NoteShip::Action::kDestinationChanged));
	}
}

}  // namespace Widelands
