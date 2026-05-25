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

#include <memory>

#include "economy/ship_fleet.h"
#include "graphic/text_layout.h"
#include "logic/map_objects/findbob.h"
#include "logic/map_objects/pinned_note.h"
#include "logic/player.h"

namespace Widelands {

/**
 * The contents of 'table' are documented in
 * /data/tribes/ships/atlanteans/init.lua
 */
ShipDescr::ShipDescr(const std::string& init_descname,
                     const LuaTable& table,
                     const std::vector<std::string>& attribs)
   : BobDescr(
        init_descname, MapObjectType::SHIP, MapObjectDescr::OwnerType::kTribe, table, attribs),
     max_hitpoints_(table.get_int("hitpoints")),
     min_attack_(table.get_int("min_attack")),
     max_attack_(table.get_int("max_attack")),
     defense_(table.get_int("defense")),
     attack_accuracy_(table.get_int("attack_accuracy")),
     heal_per_second_(table.get_int("heal_per_second")),
     default_capacity_(table.has_key("capacity") ? table.get_int("capacity") : 20),
     ship_names_(table.get_table("names")->array_entries<std::string>()) {
	// Read the sailing animations
	assign_directional_animation(&sail_anims_, "sail");
}

uint32_t ShipDescr::movecaps() const {
	return MOVECAPS_SWIM;
}

Bob& ShipDescr::create_object() const {
	return *new Ship(*this);
}

Ship::Ship(const ShipDescr& gdescr)
   : Bob(gdescr),
     hitpoints_(gdescr.max_hitpoints_),
     capacity_(gdescr.get_default_capacity()),
     warship_soldier_capacity_(capacity_) {
}

PortDock* Ship::get_lastdock(EditorGameBase& egbase) const {
	return lastdock_.get(egbase);
}

ShipFleet* Ship::get_fleet() const {
	return fleet_;
}

void Ship::init_auto_task(Game& game) {
	start_task_ship(game);
}

bool Ship::init(EditorGameBase& egbase) {
	Bob::init(egbase);
	init_fleet(egbase);
	assert(get_owner());
	get_owner()->add_ship(serial());

	// Assigning a ship name
	shipname_ = get_owner()->pick_shipname();
	molog(egbase.get_gametime(), "New ship: %s\n", shipname_.c_str());
	Notifications::publish(NoteShip(this, NoteShip::Action::kGained));
	return true;
}

void Ship::set_shipname(const std::string& name) {
	shipname_ = name;
	get_owner()->reserve_shipname(name);
}

/**
 * Create the initial singleton @ref ShipFleet to which we belong.
 * The fleet code will automatically merge us into a larger
 * fleet, if one is reachable.
 */
bool Ship::init_fleet(EditorGameBase& egbase) {
	assert(get_owner() != nullptr);
	assert(!fleet_);
	ShipFleet* fleet = new ShipFleet(get_owner());
	fleet->add_ship(egbase, this);
	const bool result = fleet->init(egbase);
	// fleet calls the set_fleet function appropriately
	assert(fleet_);
	fleet_->update(egbase);
	return result;
}

void Ship::cleanup(EditorGameBase& egbase) {
	erase_warship_soldier_request_manager();

	if (fleet_ != nullptr) {
		fleet_->remove_ship(egbase, this);
	}

	Player* o = get_owner();
	if (o != nullptr) {
		o->remove_ship(serial());
	}

	while (!items_.empty()) {
		items_.back().remove(egbase);
		items_.pop_back();
	}

	Notifications::publish(NoteShip(this, NoteShip::Action::kLost));

	Bob::cleanup(egbase);
}

/**
 * This function is to be called only by @ref ShipFleet.
 */
void Ship::set_fleet(ShipFleet* fleet) {
	fleet_ = fleet;
}

void Ship::wakeup_neighbours(Game& game) {
	FCoords position = get_position();
	Area<FCoords> area(position, 1);
	std::vector<Bob*> ships;
	game.map().find_bobs(game, area, &ships, FindBobShip());

	for (Bob* it : ships) {
		if (it == this) {
			continue;
		}

		dynamic_cast<Ship&>(*it).ship_wakeup(game);
	}
}

void Ship::set_capacity(Quantity c) {
	capacity_ = c;
	warship_soldier_capacity_ = std::min(warship_soldier_capacity_, capacity_);
}

void Ship::set_position(EditorGameBase& egbase, const Coords& coords) {
	Bob::set_position(egbase, coords);

	if (expedition_ != nullptr) {
		recalc_expedition_swimmable(egbase);
	}
}

bool Ship::can_refit(const ShipType type) const {
	return !is_refitting() && !has_battle() && type != ship_type_;
}

void Ship::set_ship_type(EditorGameBase& egbase, ShipType t) {
	items_.clear();

	ship_type_ = t;
	pending_refit_ = ship_type_;

	erase_warship_soldier_request_manager();

	if (upcast(Game, game, &egbase)) {
		if (ship_type_ == ShipType::kWarship) {
			start_task_expedition(*game);
		} else {
			exp_cancel(*game);
		}
	}
}

void Ship::refit(Game& game, const ShipType type) {
	if (!can_refit(type)) {
		molog(game.get_gametime(), "Requested refit to %d not possible", static_cast<int>(type));
		return;
	}

	if (get_destination_port(game) != nullptr) {
		send_signal(game, "wakeup");
	} else if (PortDock* dest = find_nearest_port(game); dest != nullptr) {
		set_destination(game, dest);
	} else {
		molog(game.get_gametime(), "Attempted refit to %d but no ports in fleet",
		      static_cast<int>(type));
		return;
	}

	pending_refit_ = type;

	// Already remove the ship from the fleet
	if (fleet_ != nullptr) {
		fleet_->remove_ship(game, this);
		assert(fleet_ == nullptr);
	}
}

void Ship::set_ship_state_and_notify(ShipStates state, NoteShip::Action action) {
	if (ship_state_ != state) {
		ship_state_ = state;
		Notifications::publish(NoteShip(this, action));
	}
}

void Ship::set_economy(const Game& game, Economy* e, WareWorker type) {
	// Do not check here that the economy actually changed, because on loading
	// we rely that wares really get reassigned our economy.

	(type == wwWARE ? ware_economy_ : worker_economy_) = e;
	for (ShippingItem& shipping_item : items_) {
		shipping_item.set_economy(game, e, type);
	}
}

void Ship::add_item(Game& game, const ShippingItem& item) {
	assert(items_.size() < get_capacity());

	items_.push_back(item);
	items_.back().set_location(game, this);
}

/**
 * Unload one item designated for given dock or for no dock.
 * \return true if item unloaded.
 */
bool Ship::withdraw_item(Game& game, PortDock& pd) {
	bool unloaded = false;
	size_t dst = 0;
	for (ShippingItem& si : items_) {
		if (!unloaded) {
			const PortDock* itemdest = si.get_destination(game);
			if ((itemdest == nullptr) || itemdest == &pd) {
				pd.shipping_item_arrived(game, si);
				unloaded = true;
				continue;
			}
		}
		items_[dst++] = si;
	}
	items_.resize(dst);
	return unloaded;
}

/**
 * Remove the given shipping item from the game.
 */
void Ship::remove_item_by_serial(Game& game, const Serial serial) {
	for (auto it = items_.begin(); it != items_.end(); ++it) {
		if (serial == it->get_object_serial()) {
			it->remove(game);
			it = items_.erase(it);
			return;
		}
	}
	log_warn_time(
	   game.get_gametime(), "Item %u is not on %s, cannot remove.", serial, shipname_.c_str());
}

/// Sinks the ship
/// @note only called via player command
void Ship::sink_ship(Game& game) {
	// Running colonization has the highest priority + a sink request is only valid once
	if (!state_is_sinkable()) {
		return;
	}
	ship_state_ = ShipStates::kSinkRequest;
	// Make sure the ship is active and close possible open windows
	ship_wakeup(game);
}

void Ship::log_general_info(const EditorGameBase& egbase) const {
	Bob::log_general_info(egbase);

	molog(egbase.get_gametime(), "Name: %s", get_shipname().c_str());
	molog(egbase.get_gametime(), "Ship belongs to fleet %u\nlastdock: %s\nrequestdock: %s\n",
	      fleet_ != nullptr ? fleet_->serial() : 0,
	      (lastdock_.is_set() ?
	          format("%u (%s at %3dx%3d)", lastdock_.serial(),
	                 lastdock_.get(egbase)->get_warehouse()->get_warehouse_name().c_str(),
	                 lastdock_.get(egbase)->get_positions(egbase)[0].x,
	                 lastdock_.get(egbase)->get_positions(egbase)[0].y)
	             .c_str() :
	          "-"),
	      (requestdock_.is_set() ?
	          format("%u (%s at %3dx%3d)", requestdock_.serial(),
	                 requestdock_.get(egbase)->get_warehouse()->get_warehouse_name().c_str(),
	                 requestdock_.get(egbase)->get_positions(egbase)[0].x,
	                 requestdock_.get(egbase)->get_positions(egbase)[0].y)
	             .c_str() :
	          "-"));
	if (const PortDock* dock = get_destination_port(egbase); dock != nullptr) {
		molog(egbase.get_gametime(), "Has destination port %u (%3dx%3d) %s\n", dock->serial(),
		      dock->get_positions(egbase)[0].x, dock->get_positions(egbase)[0].y,
		      dock->get_warehouse()->get_warehouse_name().c_str());
	} else if (const Ship* ship = get_destination_ship(egbase); ship != nullptr) {
		molog(egbase.get_gametime(), "Has destination ship %u (%3dx%3d) %s\n", ship->serial(),
		      ship->get_position().x, ship->get_position().y, ship->get_shipname().c_str());
	} else if (const PinnedNote* note = get_destination_note(egbase); note != nullptr) {
		molog(egbase.get_gametime(), "Has destination note %u (%3dx%3d) %s\n", note->serial(),
		      note->get_position().x, note->get_position().y, note->get_text().c_str());
	} else if (destination_coords_ != nullptr) {
		molog(egbase.get_gametime(), "Has destination detected port space %u at %3dx%3d\n",
		      destination_coords_->serial, destination_coords_->coords.x,
		      destination_coords_->coords.y);
	} else {
		molog(egbase.get_gametime(), "No destination\n");
	}

	molog(egbase.get_gametime(), "In state: %u (%s)\n", static_cast<unsigned int>(ship_state_),
	      (expedition_) ? "expedition" : "transportation");

	if (is_on_destination_dock()) {
		molog(egbase.get_gametime(), "Currently in destination portdock\n");
	}

	molog(egbase.get_gametime(), "Carrying %" PRIuS " items%s\n", items_.size(),
	      (items_.empty()) ? "." : ":");

	for (const ShippingItem& shipping_item : items_) {
		molog(egbase.get_gametime(), "  * %u (%s), destination: %s\n", shipping_item.object_.serial(),
		      shipping_item.object_.get(egbase)->descr().name().c_str(),
		      (shipping_item.destination_dock_.is_set()) ?
		         format("%u (%d x %d)", shipping_item.destination_dock_.serial(),
		                shipping_item.destination_dock_.get(egbase)->get_positions(egbase)[0].x,
		                shipping_item.destination_dock_.get(egbase)->get_positions(egbase)[0].y)

		            .c_str() :
		         "-");
	}
}

/**
 * Send a message to the owning player.
 *
 * It will have the ship's coordinates, and display a picture in its description.
 *
 * \param msgsender a computer-readable description of why the message was sent
 * \param title short title to be displayed in message listings
 * \param heading long title to be displayed within the message
 * \param description user-visible message body, will be placed in an appropriate rich-text
 *paragraph
 * \param picture the filename to be used for the icon in message listings
 */
void Ship::send_message(Game& game,
                        const std::string& title,
                        const std::string& heading,
                        const std::string& description,
                        const std::string& picture) {
	const std::string rt_description =
	   as_mapobject_message(picture, g_image_cache->get(picture)->width(), description);

	get_owner()->add_message(game, std::unique_ptr<Message>(new Message(
	                                  Message::Type::kSeafaring, game.get_gametime(), title, picture,
	                                  heading, rt_description, get_position(), serial_)));
}

}  // namespace Widelands
