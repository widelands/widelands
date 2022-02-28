/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

Ferry::Ferry(const FerryDescr& ferry_descr)
   : Carrier(ferry_descr), fleet_(nullptr), destination_(nullptr), unemployed_since_(0) {
}

bool Ferry::init(EditorGameBase& egbase) {
	Carrier::init(egbase);
	return init_fleet();
}

void Ferry::cleanup(EditorGameBase& e) {
	if (fleet_) {
		fleet_->remove_ferry(e, this);
	}
	Carrier::cleanup(e);
}

const Bob::Task Ferry::taskUnemployed = {
   "unemployed", static_cast<Bob::Ptr>(&Ferry::unemployed_update), nullptr, nullptr, true};

void Ferry::start_task_unemployed(Game& game) {
	assert(unemployed_since_.get() == 0);
	push_task(game, taskUnemployed);
	unemployed_since_ = game.get_gametime();
}

constexpr Duration kUnemployedLifetime(1000 * 60 * 10);  // 10 minutes

void Ferry::unemployed_update(Game& game, State& /* state */) {
	if (!get_signal().empty()) {
		molog(
		   game.get_gametime(), "[unemployed]: interrupted by signal '%s'\n", get_signal().c_str());
		if (get_signal() == "row") {
			assert(destination_);
			signal_handled();
			unemployed_since_ = Time(0);
			pop_task(game);
			push_task(game, taskRow);
			return schedule_act(game, Duration(10));
		}
	}
	if (destination_) {
		// Sometimes (e.g. when reassigned directly from waterway servicing),
		// the 'row' signal is consumed before we can receive it
		unemployed_since_ = Time(0);
		pop_task(game);
		push_task(game, taskRow);
		return schedule_act(game, Duration(10));
	}

	assert(game.get_gametime() >= unemployed_since_);
	if (game.get_gametime() - unemployed_since_ > kUnemployedLifetime) {
		return schedule_destroy(game);
	}

	const Map& map = game.map();
	const FCoords& pos = get_position();

	if (does_carry_ware()) {
		if (upcast(Flag, flag, pos.field->get_immovable())) {
			// We are on a flag
			if (flag->has_capacity()) {
				molog(game.get_gametime(), "[unemployed]: dropping ware here\n");
				flag->add_ware(game, *fetch_carried_ware(game));
				return start_task_idle(game, descr().get_animation("idle", this), 50);
			}
		}
		molog(game.get_gametime(), "[unemployed]: trying to find a flag\n");
		std::vector<ImmovableFound> flags;
		if (!map.find_reachable_immovables(game, Area<FCoords>(pos, 4), &flags, CheckStepFerry(game),
		                                   FindImmovableType(MapObjectType::FLAG))) {
			molog(game.get_gametime(), "[unemployed]: no flag found at all\n");
			// Fall through to the selection of a random nearby location
		} else {
			for (ImmovableFound& imm : flags) {
				if (upcast(Flag, flag, imm.object)) {
					if (flag->get_owner() == get_owner()) {
						if (flag->has_capacity()) {
							Path path(pos);
							if (map.findpath(pos, flag->get_position(), 0, path, CheckStepFerry(game))) {
								molog(game.get_gametime(), "[unemployed]: moving to nearby flag\n");
								return start_task_movepath(
								   game, path, descr().get_right_walk_anims(true, this));
							}
							molog(game.get_gametime(), "[unemployed]: unable to row to reachable flag!\n");
							return start_task_idle(game, descr().get_animation("idle", this), 50);
						}
					}
				}
			}
			molog(game.get_gametime(), "[unemployed]: no nearby flag has capacity\n");
			// If no flag with capacity is nearby, fall through to the selection of a random nearby
			// location
		}
	}

	bool move = true;
	if (!(pos.field->nodecaps() & MOVECAPS_SWIM)) {
		molog(game.get_gametime(), "[unemployed]: we are on shore\n");
	} else if (pos.field->get_immovable()) {
		molog(game.get_gametime(), "[unemployed]: we are on location\n");
	} else if (pos.field->get_first_bob()->get_next_bob()) {
		molog(game.get_gametime(), "[unemployed]: we are on another bob\n");
	} else {
		move = false;
	}

	if (move) {
		// 2 and 5 are arbitrary values that define how far away we'll
		// row at most and how hard we'll try to find a nice new location.
		Path path(pos);
		for (uint8_t i = 0; i < 5; i++) {
			if (map.findpath(pos, game.random_location(pos, 2), 0, path, CheckStepFerry(game))) {
				return start_task_movepath(
				   game, path, descr().get_right_walk_anims(does_carry_ware(), this));
			}
		}
		molog(game.get_gametime(), "[unemployed]: no suitable locations to row to found\n");
		return start_task_idle(game, descr().get_animation("idle", this), 50);
	}

	return start_task_idle(game, descr().get_animation("idle", this), 500);
}

bool Ferry::unemployed() {
	return get_state(taskUnemployed) && !destination_;
}

const Bob::Task Ferry::taskRow = {
   "row", static_cast<Bob::Ptr>(&Ferry::row_update), nullptr, nullptr, true};

void Ferry::start_task_row(Game& game, const Waterway& ww) {
	// Our new destination is the middle of the waterway
	destination_.reset(
	   new Coords(CoordPath(game.map(), ww.get_path()).get_coords()[ww.get_idle_index()]));
	send_signal(game, "row");
}

void Ferry::row_update(Game& game, State& /* state */) {
	if (!destination_) {
		return pop_task(game);
	}

	const Map& map = game.map();

	const std::string& signal = get_signal();
	if (!signal.empty()) {
		if (signal == "road" || signal == "fail" || signal == "row" || signal == "wakeup") {
			molog(game.get_gametime(), "[row]: Got signal '%s' -> recalculate\n", signal.c_str());
			signal_handled();
		} else if (signal == "blocked") {
			molog(game.get_gametime(), "[row]: Blocked by a battle\n");
			signal_handled();
			return start_task_idle(game, descr().get_animation("idle", this), 900);
		} else {
			molog(game.get_gametime(), "[row]: Cancel due to signal '%s'\n", signal.c_str());
			return pop_task(game);
		}
	}

	const FCoords& pos = get_position();

	if (pos == *destination_) {
		// Reached destination
		if (upcast(Waterway, ww, map.get_immovable(*destination_))) {
			destination_.reset(nullptr);
			set_location(ww);
			ww->assign_carrier(*this, 0);
			pop_task(game);
			return start_task_road(game);
		}
		// If we get here, the waterway was destroyed and we didn't notice
		molog(game.get_gametime(),
		      "[row]: Reached the destination (%3dx%3d) but it is no longer there\n",
		      get_position().x, get_position().y);
		destination_.reset(nullptr);
		return pop_task(game);
	}

	Path path(pos);
	if (!map.findpath(pos, *destination_, 0, path, CheckStepFerry(game))) {
		molog(game.get_gametime(),
		      "[row]: Can't find a path to the waterway! Ferry at %3dx%3d, Waterway at %3dx%3d\n",
		      get_position().x, get_position().y, destination_->x, destination_->y);
		// try again later
		return schedule_act(game, Duration(50));
	}
	return start_task_movepath(game, path, descr().get_right_walk_anims(does_carry_ware(), this));
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
	if (ww) {
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
			if (fr.unsigned_8()) {
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
