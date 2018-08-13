/*
 * Copyright (C) 2002-2018 by the Widelands Development Team
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "logic/map_objects/tribes/ferry.h"

#include "economy/fleet.h"
#include "economy/waterway.h"
#include "logic/map_objects/checkstep.h"
#include "logic/path.h"
#include "logic/player.h"

namespace Widelands {

FerryDescr::FerryDescr(const std::string& init_descname,
                           const LuaTable& table,
                           const EditorGameBase& egbase)
   : CarrierDescr(init_descname, table, egbase, MapObjectType::FERRY) {
}

uint32_t FerryDescr::movecaps() const {
	return MOVECAPS_SWIM;
}

Ferry::Ferry(const FerryDescr& ferry_descr)
   : Carrier(ferry_descr), destination_(nullptr) {
}

bool Ferry::init(EditorGameBase& egbase) {
	Carrier::init(egbase);
	return init_fleet();
}

bool Ferry::unemployed(Game& game) const {
	return !destination_ && !get_location(game);
}

const Bob::Task Ferry::taskUnemployed = {
   "unemployed", static_cast<Bob::Ptr>(&Ferry::unemployed_update), nullptr, nullptr, true};

void Ferry::start_task_unemployed(Game& game) {
	push_task(game, taskUnemployed);
	top_state().ivar1 = 0;
}

void Ferry::unemployed_update(Game& game, State&) {
	if (get_signal().size()) {
		molog("[unemployed]: interrupted by signal '%s'\n", get_signal().c_str());
		pop_task(game);
		if (get_signal() == "row") {
			push_task(game, taskRow);
			top_state().ivar1 = 0;
		}
		return;
	}

	bool move = false;
	if (get_position().field->get_immovable()) {
		molog("[unemployed]: we are on location\n");
		move = true;
	}
	else if (Bob* b = get_position().field->get_first_bob()) {
		if (b->get_next_bob()) {
			molog("[unemployed]: we are on other bob\n");
			move = true;
		}
	}
	else
		throw wexception("This ferry is not on the field where it is!");

	if (move) {
		for (uint8_t i = 0; i < 4; i++)
			if (start_task_movepath(game, game.random_location(get_position(), 2), 4,
					descr().get_right_walk_anims(does_carry_ware())))
				return;
		molog("[unemployed]: no suitable locations to row to found!\n");
		return start_task_idle(game, descr().get_animation("idle"), 50);
	}

	// ferries are a bit unresponsive, long delay
	return start_task_idle(game, descr().get_animation("idle"), 900);
}

const Bob::Task Ferry::taskRow = {
   "row", static_cast<Bob::Ptr>(&Ferry::row_update), nullptr, nullptr, true};

void Ferry::start_task_row(Game& game, Waterway* ww) {
	if (destination_)
		delete destination_;
	// our new destination is the middle of the waterway
	destination_ = new Coords(CoordPath(game.map(), ww->get_path()).get_coords()[ww->get_idle_index()]);
	send_signal(game, "row");
}

void Ferry::row_update(Game& game, State&) {
	if (!destination_)
		return pop_task(game);

	const Map& map = game.map();

	const std::string& signal = get_signal();
	if (signal.size()) {
		if (signal == "road" || signal == "fail" || signal == "row" || signal == "wakeup") {
			molog("[row]: Got signal '%s' -> recalculate\n", signal.c_str());
			signal_handled();
		} else if (signal == "blocked") {
			molog("[row]: Blocked by a battle\n");
			signal_handled();
			return start_task_idle(game, descr().get_animation("idle"), 900);
		} else {
			molog("[row]: Cancel due to signal '%s'\n", signal.c_str());
			return pop_task(game);
		}
	}

	if (get_position() == *destination_) {
		// reached destination
		if (BaseImmovable* imm = map.get_immovable(*destination_)) {
			if (upcast(Waterway, ww, imm)) {
				delete destination_;
				destination_ = nullptr;
				set_location(ww);
				pop_task(game);
				return start_task_road(game);
			}
		}
		// if we get here, the waterway was destroyed and we didn't notice
		molog("[row]: Reached the destination but it is no longer there\n");
		delete destination_;
		destination_ = nullptr;
		pop_task(game);
		return;
	}
	if (start_task_movepath(game, *destination_, 0, descr().get_right_walk_anims(does_carry_ware())))
		return;
	molog("[row]: Can't find path to the waterway for some reason!\n");
	// try again later
	return schedule_act(game, 900);
}

void Ferry::init_auto_task(Game& game) {
	set_location(nullptr);
	molog("init_auto_task: wait for employment\n");
	return start_task_unemployed(game);
}

void Ferry::set_economy(Game& game, Economy* e) {
	if (WareInstance* ware = get_carried_ware(game))
		ware->set_economy(e);
}

Fleet* Ferry::get_fleet() const {
	return fleet_;
}

void Ferry::set_fleet(Fleet* fleet) {
	fleet_ = fleet;
}

bool Ferry::init_fleet() {
	assert(get_owner() != nullptr);
	Fleet* fleet = new Fleet(get_owner());
	fleet->add_ferry(this);
	// fleet calls the set_fleet function appropriately
	return fleet->init(get_owner()->egbase());
}

/**
 * Create a new ferry
 */
Bob& FerryDescr::create_object() const {
	return *new Ferry(*this);
}

const Bob::Task* Ferry::Loader::get_task(const std::string& name) {
	if (name == "unemployed")
		return &taskUnemployed;
	if (name == "row")
		return &taskRow;
	return Worker::Loader::get_task(name);
}
}
