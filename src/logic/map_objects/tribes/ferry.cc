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

#include "logic/map_objects/tribes/ferry.h"

#include "economy/ferry_fleet.h"
#include "economy/flag.h"
#include "economy/waterway.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/checkstep.h"
#include "logic/path.h"
#include "logic/player.h"

namespace Widelands {

FerryDescr::FerryDescr(const std::string& init_descname,
                       const LuaTable& table,
                       Descriptions& descriptions)
   : CarrierDescr(init_descname, table, descriptions, MapObjectType::FERRY) {
}

// When pathfinding, we _always_ use a CheckStepFerry to account for our very special movement
// rules. This function result ensures that bob code won't complain about our sometimes strange
// paths.
uint32_t FerryDescr::movecaps() const {
	return MOVECAPS_SWIM | MOVECAPS_WALK;
}

Ferry::Ferry(const FerryDescr& ferry_descr) : Carrier(ferry_descr) {
}

bool Ferry::init(EditorGameBase& egbase) {
	Carrier::init(egbase);
	return init_fleet();
}

void Ferry::cleanup(EditorGameBase& e) {
	if (fleet_ != nullptr) {
		fleet_->remove_ferry(e, this);
	}
	Carrier::cleanup(e);
}

void Ferry::init_auto_task(Game& game) {
	set_location(nullptr);
	molog(game.get_gametime(), "init_auto_task: wait for employment\n");
	return start_task_unemployed(game);
}

void Ferry::set_economy(Game& game, Economy* e, WareWorker type) {
	if (type == wwWARE) {
		if (WareInstance* ware = get_carried_ware(game)) {
			ware->set_economy(e);
		}
	}
	// Since ferries are distributed to waterways by fleets instead of economies,
	// we do not need to maintain our worker economy
}

FerryFleet* Ferry::get_fleet() const {
	return fleet_;
}

void Ferry::set_fleet(FerryFleet* fleet) {
	fleet_ = fleet;
}

bool Ferry::init_fleet() {
	assert(get_owner());
	assert(fleet_ == nullptr);
	EditorGameBase& egbase = get_owner()->egbase();
	FerryFleet* fleet = new FerryFleet(get_owner());
	fleet->add_ferry(this);
	// fleet calls the set_fleet function appropriately
	return fleet->init(egbase);
}

Waterway* Ferry::get_destination(const Game& game) const {
	if (!destination_) {
		return nullptr;
	}
	return dynamic_cast<Waterway*>(game.map().get_immovable(*destination_));
}

void Ferry::set_destination(Game& game, Waterway* ww) {
	destination_.reset(nullptr);
	set_location(nullptr);
	if (ww != nullptr) {
		start_task_row(game, *ww);
	} else {
		send_signal(game, "cancel");
	}
}

/**
 * Create a new ferry
 */
Bob& FerryDescr::create_object() const {
	return *new Ferry(*this);
}

/*
 * Load/save support
 */

constexpr uint8_t kCurrentPacketVersion = 1;

const Bob::Task* Ferry::Loader::get_task(const std::string& name) {
	if (name == "unemployed") {
		return &taskUnemployed;
	}
	if (name == "row") {
		return &taskRow;
	}
	return Carrier::Loader::get_task(name);
}

void Ferry::Loader::load(FileRead& fr) {
	Carrier::Loader::load(fr);
	try {
		uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersion) {
			Ferry& ferry = get<Ferry>();
			if (fr.unsigned_8() != 0u) {
				int16_t dest_x = fr.signed_16();
				int16_t dest_y = fr.signed_16();
				ferry.destination_.reset(new Coords(dest_x, dest_y));
			} else {
				ferry.destination_.reset(nullptr);
			}
			ferry.unemployed_since_ = Time(fr);
			ferry.fleet_ = nullptr;
		} else {
			throw UnhandledVersionError("Ferry", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw wexception("loading ferry: %s", e.what());
	}
}

void Ferry::do_save(EditorGameBase& egbase, MapObjectSaver& mos, FileWrite& fw) {
	Carrier::do_save(egbase, mos, fw);

	fw.unsigned_8(kCurrentPacketVersion);
	fw.unsigned_8(destination_ ? 1 : 0);
	if (destination_) {
		fw.signed_16(destination_->x);
		fw.signed_16(destination_->y);
	}
	unemployed_since_.save(fw);
}

Ferry::Loader* Ferry::create_loader() {
	return new Loader;
}

}  // namespace Widelands
