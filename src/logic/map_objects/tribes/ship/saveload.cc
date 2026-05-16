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

#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/player.h"
#include "logic/widelands_geometry_io.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

/*
==============================

Load / Save implementation

==============================
*/

/* Changelog:
 * 12 - v1.1
 * 13 - Added warships and naval warfare.
 * 14 - Another naval warfare change (coords as destination and soldier preference).
 * 15 (v1.2.1) - Another naval warfare change (remember request dock).
 * 16 - Another naval warfare change (added battle_position to Battle).
 */
constexpr uint8_t kCurrentPacketVersion = 16;

const Bob::Task* Ship::Loader::get_task(const std::string& name) {
	if (name == "shipidle" || name == "ship") {
		return &taskShip;
	}
	return Bob::Loader::get_task(name);
}

void Ship::Loader::load(FileRead& fr, uint8_t packet_version) {
	// TODO(Nordfriese): Savegame compatibility v1.2
	if (packet_version >= 15 && packet_version <= kCurrentPacketVersion) {
		Bob::Loader::load(fr);
		// Economy
		ware_economy_serial_ = fr.unsigned_32();
		worker_economy_serial_ = fr.unsigned_32();

		// The state the ship is in
		ship_state_ = static_cast<ShipStates>(fr.unsigned_8());
		ship_type_ = static_cast<ShipType>(fr.unsigned_8());
		pending_refit_ = static_cast<ShipType>(fr.unsigned_8());

		// Expedition specific data
		switch (ship_state_) {
		case ShipStates::kExpeditionScouting:
		case ShipStates::kExpeditionWaiting:
		case ShipStates::kExpeditionPortspaceFound:
		case ShipStates::kExpeditionColonizing: {
			expedition_.reset(new Expedition());
			// Currently seen port build spaces
			expedition_->seen_port_buildspaces.clear();
			uint8_t numofports = fr.unsigned_8();
			for (uint8_t i = 0; i < numofports; ++i) {
				expedition_->seen_port_buildspaces.push_back(read_coords_32(&fr));
			}
			// Swimability of the directions
			for (bool& swimmable : expedition_->swimmable) {
				swimmable = (fr.unsigned_8() != 0u);
			}
			// whether scouting or exploring
			expedition_->island_exploration = (fr.unsigned_8() != 0u);
			// current direction
			expedition_->scouting_direction = static_cast<WalkingDir>(fr.unsigned_8());
			// Start coordinates of an island exploration
			expedition_->exploration_start = read_coords_32(&fr);
			// Whether the exploration is done clockwise or counter clockwise
			expedition_->island_explore_direction =
			   static_cast<IslandExploreDirection>(fr.unsigned_8());
			for (unsigned i = fr.unsigned_32(); i > 0; --i) {
				expedition_attack_target_serials_.insert(fr.unsigned_32());
			}
		} break;

		default:
			ship_state_ = ShipStates::kTransport;
			break;
		}

		for (uint8_t i = fr.unsigned_8(); i != 0U; --i) {
			const bool first = fr.unsigned_8() != 0U;
			battle_serials_.push_back(fr.unsigned_32());
			battles_.emplace_back(nullptr, Coords::null(), std::vector<uint32_t>(), first);
			battles_.back().phase = static_cast<Battle::Phase>(fr.unsigned_8());
			battles_.back().attack_coords.x = fr.signed_16();
			battles_.back().attack_coords.y = fr.signed_16();
			// TODO(Nordfriese): Savegame compatibility v1.2
			if (packet_version >= 16) {
				battles_.back().battle_position.x = fr.signed_16();
				battles_.back().battle_position.y = fr.signed_16();
			} else {
				battles_.back().battle_position = Coords::null();
				if (battles_.back().phase == Battle::Phase::kMovingToBattlePositions) {
					battles_.back().phase = Battle::Phase::kNotYetStarted;
				}
			}
			battles_.back().pending_damage = fr.unsigned_32();
			for (size_t j = fr.unsigned_32(); j > 0U; --j) {
				battles_.back().attack_soldier_serials.push_back(fr.unsigned_32());
			}
			battles_.back().time_of_last_action = Time(fr);
		}
		hitpoints_ = fr.unsigned_32();
		last_heal_time_ = Time(fr);
		send_message_at_destination_ = fr.unsigned_8() != 0;

		shipname_ = fr.c_string();
		capacity_ = fr.unsigned_32();
		warship_soldier_capacity_ = fr.unsigned_32();
		soldier_preference_ = static_cast<SoldierPreference>(fr.unsigned_8());
		lastdock_ = fr.unsigned_32();
		requestdock_ = fr.unsigned_32();
		destination_object_ = fr.unsigned_32();
		destination_coords_ = fr.unsigned_32();

		items_.resize(fr.unsigned_32());
		for (ShippingItem::Loader& item_loader : items_) {
			item_loader.load(fr);
		}
	} else {
		throw UnhandledVersionError("MapObjectPacket::Ship", packet_version, kCurrentPacketVersion);
	}
}

void Ship::Loader::load_pointers() {
	Bob::Loader::load_pointers();

	Ship& ship = get<Ship>();

	if (lastdock_ != 0u) {
		ship.lastdock_ = &mol().get<PortDock>(lastdock_);
	}
	if (requestdock_ != 0u) {
		ship.requestdock_ = &mol().get<PortDock>(requestdock_);
	}
	if (destination_object_ != 0u) {
		MapObject& mo = mol().get<MapObject>(destination_object_);
		assert(mo.descr().type() == MapObjectType::PORTDOCK ||
		       mo.descr().type() == MapObjectType::SHIP ||
		       mo.descr().type() == MapObjectType::PINNED_NOTE);
		ship.destination_object_ = &mo;
	} else {
		ship.destination_object_ = nullptr;
	}
	ship.destination_coords_ = destination_coords_ == 0U ?
	                              nullptr :
	                              &ship.owner().get_detected_port_space(destination_coords_);

	for (Serial serial : expedition_attack_target_serials_) {
		if (serial != 0) {
			expedition_->attack_targets.insert(&mol().get<Ship>(serial));
		}
	}

	for (uint32_t i = 0; i < battle_serials_.size(); ++i) {
		if (battle_serials_[i] != 0U) {
			battles_[i].opponent = &mol().get<Ship>(battle_serials_[i]);
		}
	}

	for (Battle& battle : battles_) {
		for (uint32_t& serial : battle.attack_soldier_serials) {
			// Convert file indices to actual serials
			Soldier& soldier = mol().get<Soldier>(serial);
			serial = soldier.serial();
		}
	}

	ship.items_.resize(items_.size());
	for (uint32_t i = 0; i < items_.size(); ++i) {
		ship.items_[i] = items_[i].get(mol());
	}
}

void Ship::Loader::load_finish() {
	Bob::Loader::load_finish();

	Ship& ship = get<Ship>();

	// The economy can sometimes be nullptr (e.g. when there are no ports).
	if (ware_economy_serial_ != kInvalidSerial) {
		ship.ware_economy_ = ship.get_owner()->get_economy(ware_economy_serial_);
		if (ship.ware_economy_ == nullptr) {
			ship.ware_economy_ = ship.get_owner()->create_economy(ware_economy_serial_, wwWARE);
		}
	}
	if (worker_economy_serial_ != kInvalidSerial) {
		ship.worker_economy_ = ship.get_owner()->get_economy(worker_economy_serial_);
		if (ship.worker_economy_ == nullptr) {
			ship.worker_economy_ = ship.get_owner()->create_economy(worker_economy_serial_, wwWORKER);
		}
	}

	// restore the state the ship is in
	ship.ship_state_ = ship_state_;
	ship.ship_type_ = ship_type_;
	ship.pending_refit_ = pending_refit_;
	ship.hitpoints_ = (hitpoints_ < 0) ? ship.descr().max_hitpoints_ : hitpoints_;
	ship.last_heal_time_ = last_heal_time_;
	ship.send_message_at_destination_ = send_message_at_destination_;

	// restore the  ship id and name
	ship.shipname_ = shipname_;

	ship.capacity_ = capacity_;
	ship.warship_soldier_capacity_ = warship_soldier_capacity_;
	ship.soldier_preference_ = soldier_preference_;

	// if the ship is on an expedition, restore the expedition specific data
	if (expedition_) {
		ship.expedition_.swap(expedition_);
		ship.expedition_->ware_economy = ship.ware_economy_;
		ship.expedition_->worker_economy = ship.worker_economy_;
	} else {
		assert(ship_state_ == ShipStates::kTransport);
	}
	ship.battles_ = battles_;

	// Workers load code set their economy to the economy of their location
	// (which is a PlayerImmovable), that means that workers on ships do not get
	// a correct economy assigned. We, as ship therefore have to reset the
	// economy of all workers we're transporting so that they are in the correct
	// economy. Also, we might are on an expedition which means that we just now
	// created the economy of this ship and must inform all wares.
	ship.set_economy(dynamic_cast<Game&>(egbase()), ship.ware_economy_, wwWARE);
	ship.set_economy(dynamic_cast<Game&>(egbase()), ship.worker_economy_, wwWORKER);
	ship.get_owner()->add_ship(ship.serial());

	// The ship's serial may have changed, inform onboard workers
	for (uint32_t i = 0; i < ship.items_.size(); ++i) {
		Worker* worker;
		ship.items_[i].get(ship.owner().egbase(), nullptr, &worker);
		if (worker != nullptr) {
			worker->set_ship_serial(ship.serial());
		}
	}
}

MapObject::Loader* Ship::load(EditorGameBase& egbase, MapObjectLoader& mol, FileRead& fr) {
	std::unique_ptr<Loader> loader(new Loader);
	try {
		// The header has been peeled away by the caller
		uint8_t const packet_version = fr.unsigned_8();
		if (packet_version >= 15 && packet_version <= kCurrentPacketVersion) {
			try {
				const ShipDescr* descr = nullptr;
				// Removing this will break the test suite
				std::string name = fr.c_string();
				const DescriptionIndex& ship_index = egbase.descriptions().safe_ship_index(name);
				descr = egbase.descriptions().get_ship_descr(ship_index);
				loader->init(egbase, mol, descr->create_object());
				loader->load(fr, packet_version);
			} catch (const WException& e) {
				throw GameDataError("Failed to load ship: %s", e.what());
			}
		} else {
			throw UnhandledVersionError("Ship", packet_version, kCurrentPacketVersion);
		}
	} catch (const std::exception& e) {
		throw wexception("loading ship: %s", e.what());
	}

	return loader.release();
}

void Ship::save(EditorGameBase& egbase, MapObjectSaver& mos, FileWrite& fw) {
	fw.unsigned_8(HeaderShip);
	fw.unsigned_8(kCurrentPacketVersion);
	fw.c_string(descr().name());

	Bob::save(egbase, mos, fw);

	// The economy can sometimes be nullptr (e.g. when there are no ports).
	fw.unsigned_32(ware_economy_ != nullptr ? ware_economy_->serial() : kInvalidSerial);
	fw.unsigned_32(worker_economy_ != nullptr ? worker_economy_->serial() : kInvalidSerial);

	// state the ship is in
	fw.unsigned_8(static_cast<uint8_t>(ship_state_));
	fw.unsigned_8(static_cast<uint8_t>(ship_type_));
	fw.unsigned_8(static_cast<uint8_t>(pending_refit_));

	// expedition specific data
	if (state_is_expedition()) {
		// currently seen port buildspaces
		fw.unsigned_8(expedition_->seen_port_buildspaces.size());
		for (const Coords& coords : expedition_->seen_port_buildspaces) {
			write_coords_32(&fw, coords);
		}
		// swimmability of the directions
		for (const bool& swim : expedition_->swimmable) {
			fw.unsigned_8(swim ? 1 : 0);
		}
		// whether scouting or exploring
		fw.unsigned_8(expedition_->island_exploration ? 1 : 0);
		// current direction
		fw.unsigned_8(static_cast<uint8_t>(expedition_->scouting_direction));
		// Start coordinates of an island exploration
		write_coords_32(&fw, expedition_->exploration_start);
		// Whether the exploration is done clockwise or counter clockwise
		fw.unsigned_8(static_cast<uint8_t>(expedition_->island_explore_direction));
		fw.unsigned_32(expedition_->attack_targets.size());
		for (const auto& ptr : expedition_->attack_targets) {
			fw.unsigned_32(mos.get_object_file_index_or_zero(ptr.get(egbase)));
		}
	}
	fw.unsigned_8(battles_.size());
	for (const Battle& b : battles_) {
		fw.unsigned_8(b.is_first ? 1 : 0);
		fw.unsigned_32(mos.get_object_file_index_or_zero(b.opponent.get(egbase)));
		fw.unsigned_8(static_cast<uint8_t>(b.phase));
		fw.signed_16(b.attack_coords.x);
		fw.signed_16(b.attack_coords.y);
		fw.signed_16(b.battle_position.x);
		fw.signed_16(b.battle_position.y);
		fw.unsigned_32(b.pending_damage);
		fw.unsigned_32(b.attack_soldier_serials.size());
		for (Serial s : b.attack_soldier_serials) {
			fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(s)));
		}
		b.time_of_last_action.save(fw);
	}
	fw.unsigned_32(hitpoints_);
	last_heal_time_.save(fw);
	fw.unsigned_8(send_message_at_destination_ ? 1 : 0);

	fw.string(shipname_);
	fw.unsigned_32(capacity_);
	fw.unsigned_32(warship_soldier_capacity_);
	fw.unsigned_8(static_cast<uint8_t>(soldier_preference_));
	fw.unsigned_32(mos.get_object_file_index_or_zero(lastdock_.get(egbase)));
	fw.unsigned_32(mos.get_object_file_index_or_zero(requestdock_.get(egbase)));
	fw.unsigned_32(mos.get_object_file_index_or_zero(destination_object_.get(egbase)));
	fw.unsigned_32(destination_coords_ == nullptr ? 0 : destination_coords_->serial);

	fw.unsigned_32(items_.size());
	for (ShippingItem& shipping_item : items_) {
		shipping_item.save(egbase, mos, fw);
	}
}

}  // namespace Widelands
