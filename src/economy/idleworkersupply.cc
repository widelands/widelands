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

#include "economy/idleworkersupply.h"

#include "base/wexception.h"
#include "economy/economy.h"
#include "economy/request.h"
#include "logic/game.h"
#include "logic/map_objects/tribes/requirements.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/player.h"

namespace Widelands {

/**
 * Automatically register with the worker's economy.
 */
IdleWorkerSupply::IdleWorkerSupply(Worker& w) : worker_(w), economy_(nullptr) {
	set_economy(w.get_economy(wwWORKER));
}

/**
 * Automatically unregister from economy.
 */
IdleWorkerSupply::~IdleWorkerSupply() {
	set_economy(nullptr);
}

/**
 * Add/remove this supply from the Economy as appropriate.
 */
void IdleWorkerSupply::set_economy(Economy* const e) {
	if (economy_ != e) {
		if (economy_) {
			economy_->remove_supply(*this);
		}
		if ((economy_ = e)) {
			economy_->add_supply(*this);
		}
	}
}

/**
 * Worker is walking around the road network, so active by definition.
 */
bool IdleWorkerSupply::is_active() const {
	return true;
}

SupplyProviders IdleWorkerSupply::provider_type(Game*) const {
	return SupplyProviders::kFlagOrRoad;
}

bool IdleWorkerSupply::has_storage() const {
	return worker_.get_transfer();
}

void IdleWorkerSupply::get_ware_type(WareWorker& type, DescriptionIndex& ware) const {
	type = wwWORKER;
	ware = worker_.descr().worker_index();
}

/**
 * Return the worker's position.
 */
PlayerImmovable* IdleWorkerSupply::get_position(Game& game) {
	return worker_.get_location(game);
}

uint32_t IdleWorkerSupply::nr_supplies(const Game& game, const Request& req) const {
	assert(req.get_type() != wwWORKER || worker_.owner().tribe().has_worker(req.get_index()));
	if (req.get_type() == wwWORKER &&
	    (req.get_index() == worker_.descr().worker_index() ||
	     (!req.get_exact_match() && worker_.descr().can_act_as(req.get_index()))) &&
	    !worker_.get_carried_ware(game) && req.get_requirements().check(worker_)) {
		return 1;
	}
	return 0;
}

WareInstance& IdleWorkerSupply::launch_ware(Game&, const Request&) {
	throw wexception("IdleWorkerSupply::launch_ware() makes no sense.");
}

/**
 * No need to explicitly launch the worker.
 */
Worker& IdleWorkerSupply::launch_worker(Game&, const Request& req) {
	if (req.get_type() != wwWORKER) {
		throw wexception("IdleWorkerSupply: not a worker request");
	}
	if (!worker_.descr().can_act_as(req.get_index()) || !req.get_requirements().check(worker_)) {
		throw wexception("IdleWorkerSupply: worker type mismatch");
	}

	return worker_;
}

void IdleWorkerSupply::send_to_storage(Game& game, Warehouse* wh) {
	assert(!has_storage());

	Transfer* t = new Transfer(game, worker_);
	t->set_destination(*wh);
	worker_.start_task_transfer(game, t);
}
}  // namespace Widelands
