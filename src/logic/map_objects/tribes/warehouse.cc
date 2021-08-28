/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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

#include "logic/map_objects/tribes/warehouse.h"

#include <memory>

#include "base/log.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "economy/economy.h"
#include "economy/expedition_bootstrap.h"
#include "economy/flag.h"
#include "economy/portdock.h"
#include "economy/request.h"
#include "economy/ship_fleet.h"
#include "economy/warehousesupply.h"
#include "economy/wares_queue.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/map_objects/findbob.h"
#include "logic/map_objects/findnode.h"
#include "logic/map_objects/tribes/battle.h"
#include "logic/map_objects/tribes/carrier.h"
#include "logic/map_objects/tribes/requirements.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/message_queue.h"
#include "logic/player.h"

namespace Widelands {

namespace {

constexpr Duration kCostlessWorkerSpawnInterval = Duration(2500);
constexpr int kFleeingUnitsCap = 500;

// Goes through the list and removes all workers that are no longer in the
// game.
void remove_no_longer_existing_workers(Game& game, std::vector<OPtr<Worker>>* workers) {
	for (std::vector<OPtr<Worker>>::iterator i = workers->begin(); i != workers->end(); ++i) {
		if (!i->get(game)) {
			workers->erase(i);
			remove_no_longer_existing_workers(game, workers);
			return;
		}
	}
}

}  // namespace

bool Warehouse::AttackTarget::can_be_attacked() const {
	return warehouse_->descr().get_conquers() > 0;
}

void Warehouse::AttackTarget::enemy_soldier_approaches(const Soldier& enemy) const {
	if (!warehouse_->descr().get_conquers()) {
		return;
	}

	Player* owner = warehouse_->get_owner();
	Game& game = dynamic_cast<Game&>(owner->egbase());
	const Map& map = game.map();
	if (enemy.get_owner() == owner || enemy.get_battle() ||
	    warehouse_->descr().get_conquers() <=
	       map.calc_distance(enemy.get_position(), warehouse_->get_position())) {
		return;
	}

	if (map.find_bobs(game,
	                  Area<FCoords>(map.get_fcoords(warehouse_->base_flag().get_position()), 2),
	                  nullptr, FindBobEnemySoldier(owner))) {
		return;
	}

	DescriptionIndex const soldier_index = owner->tribe().soldier();
	Requirements noreq;

	if (!warehouse_->count_workers(game, soldier_index, noreq, Match::kCompatible)) {
		return;
	}

	Soldier& defender =
	   dynamic_cast<Soldier&>(warehouse_->launch_worker(game, soldier_index, noreq));
	defender.start_task_defense(game, false);
}

AttackTarget::AttackResult Warehouse::AttackTarget::attack(Soldier* enemy) const {
	Player* owner = warehouse_->get_owner();
	Game& game = dynamic_cast<Game&>(owner->egbase());
	DescriptionIndex const soldier_index = owner->tribe().soldier();
	Requirements noreq;

	if (warehouse_->count_workers(game, soldier_index, noreq, Match::kCompatible)) {
		Soldier& defender =
		   dynamic_cast<Soldier&>(warehouse_->launch_worker(game, soldier_index, noreq));
		defender.start_task_defense(game, true);
		enemy->send_signal(game, "sleep");
		return AttackTarget::AttackResult::DefenderLaunched;
	}

	warehouse_->set_defeating_player(enemy->owner().player_number());
	warehouse_->schedule_destroy(game);
	return AttackTarget::AttackResult::Defenseless;
}

WarehouseSupply::~WarehouseSupply() {
	if (ware_economy_) {
		log_warn("WarehouseSupply::~WarehouseSupply: Warehouse %u still belongs to "
		         "a ware_economy",
		         warehouse_->serial());
		set_economy(nullptr, wwWARE);
	}
	if (worker_economy_) {
		log_warn("WarehouseSupply::~WarehouseSupply: Warehouse %u still belongs to "
		         "a worker_economy",
		         warehouse_->serial());
		set_economy(nullptr, wwWORKER);
	}

	// We're removed from the Economy. Therefore, the wares can simply
	// be cleared out. The global inventory will be okay.
	wares_.clear();
	workers_.clear();
}

/// Inform this supply, how much wares are to be handled
void WarehouseSupply::set_nrwares(DescriptionIndex const i) {
	assert(0 == wares_.get_nrwareids());

	wares_.set_nrwares(i);
}
void WarehouseSupply::set_nrworkers(DescriptionIndex const i) {
	assert(0 == workers_.get_nrwareids());

	workers_.set_nrwares(i);
}

/// Add and remove our wares and the Supply to the economies as necessary.
void WarehouseSupply::set_economy(Economy* const e, WareWorker type) {
	if (e == (type == wwWARE ? ware_economy_ : worker_economy_)) {
		return;
	}

	if (Economy* ec = (type == wwWARE ? ware_economy_ : worker_economy_)) {
		ec->remove_supply(*this);
		switch (type) {
		case wwWARE:
			for (DescriptionIndex i = 0; i < wares_.get_nrwareids(); ++i) {
				if (wares_.stock(i)) {
					ec->remove_wares_or_workers(i, wares_.stock(i));
				}
			}
			break;
		case wwWORKER:
			for (DescriptionIndex i = 0; i < workers_.get_nrwareids(); ++i) {
				if (workers_.stock(i)) {
					ec->remove_wares_or_workers(i, workers_.stock(i));
				}
			}
			break;
		}
	}

	(type == wwWARE ? ware_economy_ : worker_economy_) = e;

	if (Economy* ec = (type == wwWARE ? ware_economy_ : worker_economy_)) {
		switch (type) {
		case wwWARE:
			for (DescriptionIndex i = 0; i < wares_.get_nrwareids(); ++i) {
				if (wares_.stock(i)) {
					ec->add_wares_or_workers(i, wares_.stock(i), worker_economy_);
				}
			}
			break;
		case wwWORKER:
			for (DescriptionIndex i = 0; i < workers_.get_nrwareids(); ++i) {
				if (workers_.stock(i)) {
					e->add_wares_or_workers(i, workers_.stock(i), ware_economy_);
				}
			}
			break;
		}
		ec->add_supply(*this);
	}
}

/// Add wares and update the economy.
void WarehouseSupply::add_wares(DescriptionIndex const id, Quantity const count) {
	if (!count) {
		return;
	}

	if (ware_economy_) {  // No economies in the editor
		ware_economy_->add_wares_or_workers(id, count, worker_economy_);
	}
	wares_.add(id, count);
}

/// Remove wares and update the economy.
void WarehouseSupply::remove_wares(DescriptionIndex const id, uint32_t const count) {
	if (!count) {
		return;
	}

	wares_.remove(id, count);
	if (ware_economy_) {  // No economies in the editor
		ware_economy_->remove_wares_or_workers(id, count);
	}
}

/// Add workers and update the economy.
void WarehouseSupply::add_workers(DescriptionIndex const id, uint32_t const count) {
	if (!count) {
		return;
	}

	if (worker_economy_) {  // No economies in the editor
		worker_economy_->add_wares_or_workers(id, count, ware_economy_);
	}
	workers_.add(id, count);
}

/**
 * Remove workers and update the economy.
 * Comments see add_workers
 */
void WarehouseSupply::remove_workers(DescriptionIndex const id, uint32_t const count) {
	if (!count) {
		return;
	}

	workers_.remove(id, count);
	if (worker_economy_) {  // No economies in the editor
		worker_economy_->remove_wares_or_workers(id, count);
	}
}

/// Return the position of the Supply, i.e. the owning Warehouse.
PlayerImmovable* WarehouseSupply::get_position(Game&) {
	return warehouse_;
}

/// Warehouse supplies are never active.
bool WarehouseSupply::is_active() const {
	return false;
}
SupplyProviders WarehouseSupply::provider_type(Game*) const {
	return SupplyProviders::kWarehouse;
}

bool WarehouseSupply::has_storage() const {
	return true;
}

void WarehouseSupply::get_ware_type(WareWorker& /* type */, DescriptionIndex& /* ware */) const {
	throw wexception("WarehouseSupply::get_ware_type: calling this is nonsensical");
}

void WarehouseSupply::send_to_storage(Game&, Warehouse* /* wh */) {
	throw wexception("WarehouseSupply::send_to_storage: should never be called");
}

uint32_t WarehouseSupply::nr_supplies(const Game& game, const Request& req) const {
	return req.get_type() == wwWORKER ?
             warehouse_->count_workers(game, req.get_index(), req.get_requirements(),
	                                    (req.get_exact_match() ? Warehouse::Match::kExact :
                                                                Warehouse::Match::kCompatible)) :
             wares_.stock(req.get_index());
}

/// Launch a ware.
WareInstance& WarehouseSupply::launch_ware(Game& game, const Request& req) {
	if (req.get_type() != wwWARE) {
		throw wexception("WarehouseSupply::launch_ware: called for non-ware request");
	}
	if (!wares_.stock(req.get_index())) {
		throw wexception("WarehouseSupply::launch_ware: called for non-existing ware");
	}

	return warehouse_->launch_ware(game, req.get_index());
}

/// Launch a ware as worker.
Worker& WarehouseSupply::launch_worker(Game& game, const Request& req) {
	return warehouse_->launch_worker(game, req.get_index(), req.get_requirements());
}

/*
==============================
Warehouse Building
==============================
*/

/**
 * The contents of 'table' are documented in
 * /data/tribes/buildings/warehouses/atlanteans/headquarters/init.lua
 */
WarehouseDescr::WarehouseDescr(const std::string& init_descname,
                               const LuaTable& table,
                               Descriptions& descriptions)
   : BuildingDescr(init_descname, MapObjectType::WAREHOUSE, table, descriptions),
     conquers_(0),
     heal_per_second_(0) {
	heal_per_second_ = table.get_int("heal_per_second");
	if (table.has_key("conquers")) {
		conquers_ = table.get_int("conquers");
		workarea_info_[conquers_].insert(name() + " conquer");
	}
}

std::vector<Soldier*> Warehouse::SoldierControl::present_soldiers() const {
	std::vector<Soldier*> rv;
	DescriptionIndex const soldier_index = warehouse_->owner().tribe().soldier();
	IncorporatedWorkers::const_iterator sidx = warehouse_->incorporated_workers_.find(soldier_index);

	if (sidx != warehouse_->incorporated_workers_.end()) {
		const WorkerList& soldiers = sidx->second;
		for (OPtr<Worker> temp_soldier : soldiers) {
			rv.push_back(dynamic_cast<Soldier*>(temp_soldier.get(warehouse_->get_owner()->egbase())));
		}
	}
	return rv;
}

std::vector<Soldier*> Warehouse::SoldierControl::stationed_soldiers() const {
	return present_soldiers();
}

Quantity Warehouse::SoldierControl::min_soldier_capacity() const {
	return 0;
}

Quantity Warehouse::SoldierControl::max_soldier_capacity() const {
	return std::numeric_limits<Quantity>::max();
}

Quantity Warehouse::SoldierControl::soldier_capacity() const {
	return max_soldier_capacity();
}

void Warehouse::SoldierControl::set_soldier_capacity(Quantity /* capacity */) {
	throw wexception("Not implemented for a Warehouse!");
}

void Warehouse::SoldierControl::drop_soldier(Soldier&) {
	throw wexception("Not implemented for a Warehouse!");
}

int Warehouse::SoldierControl::outcorporate_soldier(Soldier& soldier) {
	DescriptionIndex const soldier_index = warehouse_->owner().tribe().soldier();
	if (warehouse_->incorporated_workers_.count(soldier_index)) {
		WorkerList& soldiers = warehouse_->incorporated_workers_[soldier_index];

		WorkerList::iterator i = std::find(soldiers.begin(), soldiers.end(), &soldier);

		soldiers.erase(i);
		warehouse_->supply_->remove_workers(soldier_index, 1);
	}
#ifndef NDEBUG
	else {
		throw wexception("outcorporate_soldier: soldier not in this warehouse!");
	}
#endif
	return 0;
}

int Warehouse::SoldierControl::incorporate_soldier(EditorGameBase& egbase, Soldier& soldier) {
	warehouse_->incorporate_worker(egbase, &soldier);
	return 0;
}

Warehouse::Warehouse(const WarehouseDescr& warehouse_descr)
   : Building(warehouse_descr),
     attack_target_(this),
     soldier_control_(this),
     supply_(new WarehouseSupply(this)),
     next_military_act_(Time(0)),
     next_stock_remove_act_(Time(0)),
     portdock_(nullptr) {
	cleanup_in_progress_ = false;
	set_attack_target(&attack_target_);
	set_soldier_control(&soldier_control_);
}

Warehouse::~Warehouse() {
	delete supply_;
}

/**
 * Try to bring the given \ref PlannedWorkers up to date with our game data.
 * Return \c false if \p pw cannot be salvaged.
 */
bool Warehouse::load_finish_planned_worker(PlannedWorkers& pw) {
	const TribeDescr& tribe = owner().tribe();

	if (pw.index == INVALID_INDEX || !(pw.index < supply_->get_workers().get_nrwareids())) {
		return false;
	}

	const WorkerDescr* w_desc = tribe.get_worker_descr(pw.index);

	if (!(tribe.has_worker(pw.index) && w_desc->is_buildable())) {
		return false;
	}

	if (!(pw.index < supply_->get_workers().get_nrwareids())) {
		return false;
	}

	const WorkerDescr::Buildcost& cost = w_desc->buildcost();
	uint32_t idx = 0;

	for (WorkerDescr::Buildcost::const_iterator cost_it = cost.begin(); cost_it != cost.end();
	     ++cost_it, ++idx) {
		WareWorker type;
		DescriptionIndex wareindex = owner().tribe().ware_index(cost_it->first);
		if (owner().tribe().has_ware(wareindex)) {
			type = wwWARE;
		} else {
			wareindex = owner().tribe().worker_index(cost_it->first);
			if (owner().tribe().has_worker(wareindex)) {
				type = wwWORKER;
			} else {
				return false;
			}
		}

		if (idx < pw.requests.size()) {
			if (pw.requests[idx]->get_type() == type && pw.requests[idx]->get_index() == wareindex) {
				continue;
			}

			std::vector<Request*>::iterator req_it = pw.requests.begin() + idx + 1;
			while (req_it != pw.requests.end()) {
				if ((*req_it)->get_type() == type && (*req_it)->get_index() == wareindex) {
					break;
				}
				++req_it;
			}

			if (req_it != pw.requests.end()) {
				std::swap(*req_it, pw.requests[idx]);
				continue;
			}
		}

		log_warn("load_finish_planned_worker: old savegame: "
		         "need to create new request for '%s'\n",
		         cost_it->first.c_str());
		pw.requests.insert(
		   pw.requests.begin() + idx, new Request(*this, wareindex, &Warehouse::request_cb, type));
	}

	while (pw.requests.size() > idx) {
		log_warn("load_finish_planned_worker: old savegame: "
		         "removing outdated request.\n");
		delete pw.requests.back();
		pw.requests.pop_back();
	}

	return true;
}

void Warehouse::load_finish(EditorGameBase& egbase) {
	Building::load_finish(egbase);

	Time next_spawn;
	const std::vector<DescriptionIndex>& worker_types_without_cost =
	   owner().tribe().worker_types_without_cost();
	for (uint8_t i = worker_types_without_cost.size(); i;) {
		DescriptionIndex const worker_index = worker_types_without_cost.at(--i);
		if (owner().is_worker_type_allowed(worker_index) &&
		    next_worker_without_cost_spawn_[i].is_invalid()) {
			if (next_spawn.is_invalid()) {
				next_spawn = schedule_act(dynamic_cast<Game&>(egbase), kCostlessWorkerSpawnInterval);
			}
			next_worker_without_cost_spawn_[i] = next_spawn;
			log_warn(
			   "player %u is allowed to create worker type %s but his "
			   "%s %u at (%i, %i) does not have a next_spawn time set for that "
			   "worker type; setting it to %u\n",
			   owner().player_number(), owner().tribe().get_worker_descr(worker_index)->name().c_str(),
			   descr().name().c_str(), serial(), get_position().x, get_position().y, next_spawn.get());
		}
	}

	// Ensure consistency of PlannedWorker requests
	{
		uint32_t pwidx = 0;
		while (pwidx < planned_workers_.size()) {
			if (!load_finish_planned_worker(planned_workers_[pwidx])) {
				planned_workers_[pwidx].cleanup();
				planned_workers_.erase(planned_workers_.begin() + pwidx);
			} else {
				pwidx++;
			}
		}
	}
}

bool Warehouse::init(EditorGameBase& egbase) {
	Building::init(egbase);

	Player* player = get_owner();

	init_containers(*player);

	set_seeing(true);

	// Even though technically, a warehouse might be completely empty,
	// we let warehouse see always for simplicity's sake (since there's
	// almost always going to be a carrier inside, that shouldn't hurt).
	if (upcast(Game, game, &egbase)) {

		{
			const Time act_time = schedule_act(*game, kCostlessWorkerSpawnInterval);
			const std::vector<DescriptionIndex>& worker_types_without_cost =
			   player->tribe().worker_types_without_cost();

			for (size_t i = 0; i < worker_types_without_cost.size(); ++i) {
				if (player->is_worker_type_allowed(worker_types_without_cost.at(i))) {
					next_worker_without_cost_spawn_[i] = act_time;
				}
			}
		}
		// next_military_act_ is not touched in the loading code. Is only needed
		// if the warehouse is created in the game?  I assume it's for the
		// conquer_radius thing
		next_military_act_ = schedule_act(*game, Duration(1000));

		next_stock_remove_act_ = schedule_act(*game, Duration(4000));

		verb_log_info_time(egbase.get_gametime(), "Message: adding %s for player %i at (%d, %d)\n",
		                   to_string(descr().type()).c_str(), player->player_number(), position_.x,
		                   position_.y);

		if (descr().get_isport()) {
			send_message(*game, Message::Type::kSeafaring, descr().descname(), descr().icon_filename(),
			             descr().descname(), _("A new port was added to your economy."), true);
		} else if (!descr().is_buildable()) {
			send_message(*game, Message::Type::kEconomy, descr().descname(), descr().icon_filename(),
			             descr().descname(), _("A new headquarters was added to your economy."), true);
		} else {
			send_message(*game, Message::Type::kEconomy, descr().descname(), descr().icon_filename(),
			             descr().descname(), _("A new warehouse was added to your economy."), true);
		}
	}

	if (uint32_t const conquer_radius = descr().get_conquers()) {
		egbase.conquer_area(
		   PlayerArea<Area<FCoords>>(
		      player->player_number(),
		      Area<FCoords>(egbase.map().get_fcoords(get_position()), conquer_radius)),
		   true);
	}

	if (descr().get_isport()) {
		init_portdock(egbase);
		PortDock* pd = portdock_;
		// should help diagnose problems with marine
		if (!pd->get_fleet()) {
			log_warn_time(egbase.get_gametime(), " Portdock without a fleet created (%3dx%3d)\n",
			              get_position().x, get_position().y);
		}
	}
	cleanup_in_progress_ = false;
	return true;
}

void Warehouse::init_containers(const Player& player) {
	DescriptionIndex const nr_wares = player.egbase().descriptions().nr_wares();
	DescriptionIndex const nr_workers = player.egbase().descriptions().nr_workers();
	supply_->set_nrwares(nr_wares);
	supply_->set_nrworkers(nr_workers);

	ware_policy_.resize(nr_wares, StockPolicy::kNormal);
	worker_policy_.resize(nr_workers, StockPolicy::kNormal);

	uint8_t nr_worker_types_without_cost = player.tribe().worker_types_without_cost().size();
	next_worker_without_cost_spawn_.resize(nr_worker_types_without_cost, Time());
}

/**
 * Find a contiguous set of water fields close to the port for docking
 * and initialize the @ref PortDock instance.
 */
void Warehouse::init_portdock(EditorGameBase& egbase) {
	molog(egbase.get_gametime(), "Setting up port dock fields\n");

	std::vector<Coords> dock = egbase.map().find_portdock(get_position(), false);
	if (dock.empty()) {
		verb_log_warn_time(
		   egbase.get_gametime(),
		   "No suitable portdock space around %3dx%3d found! Attempting to force the portdock...\n",
		   get_position().x, get_position().y);

		dock = egbase.map().find_portdock(get_position(), true);
		if (dock.empty()) {
			// Better to throw an exception than to risk a segfault…
			throw wexception(
			   "Attempting to create a port dock around %3dx%3d without any water nearby",
			   get_position().x, get_position().y);
		}

		assert(dock.size() == 1);
		Field& field = egbase.map()[dock.back()];

		if (field.get_owned_by() != owner().player_number()) {
			verb_log_info_time(egbase.get_gametime(), "Conquering territory at %3dx%3d for portdock\n",
			                   dock.back().x, dock.back().y);
			egbase.conquer_area(
			   PlayerArea<Area<FCoords>>(
			      owner().player_number(), Area<FCoords>(egbase.map().get_fcoords(dock.back()), 1)),
			   true);
		}

		if (field.get_immovable()) {
			verb_log_info_time(
			   egbase.get_gametime(), "Clearing immovable '%s' at %3dx%3d for portdock\n",
			   field.get_immovable()->descr().name().c_str(), dock.back().x, dock.back().y);
			// currently only waterways and portdocks can be built on water
			assert(field.get_immovable()->descr().type() == MapObjectType::WATERWAY);
			if (upcast(Game, game, &egbase)) {
				/** TRANSLATORS: Message header, short for "Waterway Destroyed" (not much space
				 * available) */
				send_message(*game, Message::Type::kSeafaring, _("Waterway"), descr().icon_filename(),
				             _("Waterway destroyed"),
				             _("A waterway had to be destroyed to make room for your new port dock."),
				             false);
			}
			field.get_immovable()->remove(egbase);
		}
	}

	molog(egbase.get_gametime(), "Found %" PRIuS " fields for the dock\n", dock.size());

	portdock_ = new PortDock(this);
	portdock_->set_owner(get_owner());
	portdock_->set_economy(get_economy(wwWARE), wwWARE);
	portdock_->set_economy(get_economy(wwWORKER), wwWORKER);
	for (const Coords& coords : dock) {
		portdock_->add_position(coords);
	}
	portdock_->init(egbase);

	if (get_economy(wwWARE) != nullptr) {
		portdock_->set_economy(get_economy(wwWARE), wwWARE);
	}
	if (get_economy(wwWORKER) != nullptr) {
		portdock_->set_economy(get_economy(wwWORKER), wwWORKER);
	}

	// this is just to indicate something wrong is going on
	PortDock* pd_tmp = portdock_;
	if (!pd_tmp->get_fleet()) {
		log_warn_time(egbase.get_gametime(),
		              " portdock for port at %3dx%3d created but without a fleet!\n",
		              get_position().x, get_position().y);
	}
}

void Warehouse::destroy(EditorGameBase& egbase) {
	Building::destroy(egbase);
}

// if the port still exists and we are in game we first try to restore the portdock
void Warehouse::restore_portdock_or_destroy(EditorGameBase& egbase) {
	Warehouse::init_portdock(egbase);
	if (!portdock_) {
		log_warn_time(egbase.get_gametime(),
		              " Portdock could not be restored, removing the port now (coords: %3dx%3d)\n",
		              get_position().x, get_position().y);
		Building::destroy(egbase);
	} else {
		molog(egbase.get_gametime(), "Message: portdock restored\n");
		PortDock* pd_tmp = portdock_;
		if (!pd_tmp->get_fleet()) {
			log_warn_time(egbase.get_gametime(), " Portdock restored but without a fleet!\n");
		}
	}
}

/// Destroy the warehouse.
void Warehouse::cleanup(EditorGameBase& egbase) {
	// if this is a port, it will remove also portdock.
	// But portdock must know that it should not try to recreate itself
	cleanup_in_progress_ = true;

	if (portdock_) {
		portdock_->remove(egbase);
	}
	portdock_ = nullptr;

	// This will launch all workers including incorporated ones up to kFleeingUnitsCap and then empty
	// the stock.
	if (upcast(Game, game, &egbase)) {
		const WareList& workers = get_workers();
		for (DescriptionIndex id = 0; id < workers.get_nrwareids(); ++id) {
			// If the game is running, have the workers flee the warehouse.
			if (game->is_loaded()) {
				// We have kFleeingUnitsCap to make sure that we won't flood the map with carriers etc.
				Quantity stock = workers.stock(id);
				for (Quantity i = 0; i < stock && i < kFleeingUnitsCap; ++i) {
					launch_worker(*game, id, Requirements()).start_task_leavebuilding(*game, true);
				}
			}
			// Make sure that all workers are gone
			remove_workers(id, workers.stock(id));
			assert(!game->is_loaded() ||
			       (!incorporated_workers_.count(id) || incorporated_workers_[id].empty()));
		}
	}
	incorporated_workers_.clear();

	while (!planned_workers_.empty()) {
		planned_workers_.back().cleanup();
		planned_workers_.pop_back();
	}

	const Map& map = egbase.map();
	if (const uint32_t conquer_radius = descr().get_conquers()) {
		egbase.unconquer_area(
		   PlayerArea<Area<FCoords>>(owner().player_number(),
		                             Area<FCoords>(map.get_fcoords(get_position()), conquer_radius)),
		   defeating_player_);
	}

	Building::cleanup(egbase);
}

/// Act regularly to create workers of buildable types without cost. According
/// to intelligence, this is some highly advanced technology. Not only do the
/// settlers have no problems with birth control, they do not even need anybody
/// to procreate. They must have built-in DNA samples in those warehouses. And
/// what the hell are they doing, killing useless tribesmen! The Borg? Or just
/// like Soylent Green? Or maybe I should just stop writing comments that late
/// at night ;-)
void Warehouse::act(Game& game, uint32_t const data) {
	const Time& gametime = game.get_gametime();
	{
		const std::vector<DescriptionIndex>& worker_types_without_cost =
		   owner().tribe().worker_types_without_cost();
		for (size_t i = worker_types_without_cost.size(); i;) {
			if (next_worker_without_cost_spawn_[--i] <= gametime) {
				DescriptionIndex const id = worker_types_without_cost.at(i);
				if (owner().is_worker_type_allowed(id)) {
					int32_t const stock = supply_->stock_workers(id);
					Duration tdelta = kCostlessWorkerSpawnInterval;

					if (stock < 100) {
						tdelta -= Duration(4 * (100 - stock));
						insert_workers(id, 1);
					} else if (stock > 100) {
						tdelta -= Duration(4 * (stock - 100));
						if (tdelta.get() < 10) {
							tdelta = Duration(10);
						}
						remove_workers(id, 1);
					}

					next_worker_without_cost_spawn_[i] = schedule_act(game, tdelta);
				} else {
					next_worker_without_cost_spawn_[i] = Time();
				}
			}
		}
	}

	//  Military stuff: Kill the soldiers that are dead.
	if (next_military_act_ <= gametime) {
		DescriptionIndex const soldier_index = owner().tribe().soldier();

		if (incorporated_workers_.count(soldier_index)) {
			WorkerList& soldiers = incorporated_workers_[soldier_index];

			unsigned total_heal = descr().get_heal_per_second();
			// Using an explicit iterator, as we plan to erase some
			// of those guys
			for (WorkerList::iterator it = soldiers.begin(); it != soldiers.end(); ++it) {
				// This is a safe cast: we know only soldiers can land in this
				// slot in the incorporated array
				Soldier* soldier = dynamic_cast<Soldier*>(it->get(game));

				//  Soldier dead ...
				if (!soldier || soldier->get_current_health() == 0) {
					it = soldiers.erase(it);
					supply_->remove_workers(soldier_index, 1);
					continue;
				}

				if (soldier->get_current_health() < soldier->get_max_health()) {
					soldier->heal(total_heal);
					continue;
				}
			}
		}
		next_military_act_ = schedule_act(game, Duration(1000));
	}

	if (next_stock_remove_act_ <= gametime) {
		check_remove_stock(game);

		next_stock_remove_act_ = schedule_act(game, Duration(4000));
	}

	// Update planned workers; this is to update the request amounts and
	// check because whether we suddenly can produce a requested worker. This
	// is mostly previously available wares may become unavailable due to
	// secondary requests.
	update_all_planned_workers(game);

	Building::act(game, data);
}

/// Transfer our registration to the new economy.
void Warehouse::set_economy(Economy* const e, WareWorker type) {
	Economy* const old = get_economy(type);

	if (old == e) {
		return;
	}

	if (old) {
		old->remove_warehouse(*this);
	}

	if (portdock_) {
		portdock_->set_economy(e, type);
	}
	supply_->set_economy(e, type);
	Building::set_economy(e, type);

	for (const PlannedWorkers& pw : planned_workers_) {
		for (Request* req : pw.requests) {
			if (req->get_type() == type) {
				req->set_economy(e);
			}
		}
	}

	if (e) {
		e->add_warehouse(*this);
	}
}

const WareList& Warehouse::get_wares() const {
	return supply_->get_wares();
}

const WareList& Warehouse::get_workers() const {
	return supply_->get_workers();
}

PlayerImmovable::Workers Warehouse::get_incorporated_workers() {
	PlayerImmovable::Workers all_workers;

	for (const auto& worker_pair : incorporated_workers_) {
		for (OPtr<Worker> worker : worker_pair.second) {
			all_workers.push_back(worker.get(get_owner()->egbase()));
		}
	}
	return all_workers;
}

/// Magically create wares in this warehouse. Updates the economy accordingly.
void Warehouse::insert_wares(DescriptionIndex const id, Quantity const count) {
	supply_->add_wares(id, count);
}

/// Magically destroy wares.
void Warehouse::remove_wares(DescriptionIndex const id, Quantity const count) {
	supply_->remove_wares(id, count);
}

/// Magically create workers in this warehouse. Updates the economy accordingly.
void Warehouse::insert_workers(DescriptionIndex const id, uint32_t const count) {
	supply_->add_workers(id, count);
}

/// Magically destroy workers.
void Warehouse::remove_workers(DescriptionIndex const id, uint32_t const count) {
	supply_->remove_workers(id, count);
}

/// Launch a carrier to fetch an ware from our flag.
bool Warehouse::fetch_from_flag(Game& game) {
	DescriptionIndex const carrierid = owner().tribe().carriers()[0];

	if (!supply_->stock_workers(carrierid)) {
		if (can_create_worker(game, carrierid)) {
			create_worker(game, carrierid);
		}
	}
	if (supply_->stock_workers(carrierid)) {
		launch_worker(game, carrierid, Requirements()).start_task_fetchfromflag(game);
	}

	return true;
}

/**
 * \return the number of workers that we can launch satisfying the given
 * requirements.
 */
Quantity Warehouse::count_workers(const Game& game,
                                  DescriptionIndex worker_id,
                                  const Requirements& req,
                                  Match exact) {
	Quantity sum = 0;

	do {
		sum += supply_->stock_workers(worker_id);

		// NOTE: This code lies about the TrainingAttributes of non-instantiated workers.
		if (incorporated_workers_.count(worker_id)) {
			for (OPtr<Worker> worker : incorporated_workers_[worker_id]) {
				if (!req.check(*worker.get(game))) {
					//  This is one of the workers in our sum.
					//  But he is too stupid for this job
					--sum;
				}
			}
		}
		if (exact == Match::kCompatible) {
			worker_id = owner().tribe().get_worker_descr(worker_id)->becomes();
		} else {
			worker_id = INVALID_INDEX;
		}
	} while (owner().tribe().has_worker(worker_id));

	return sum;
}

/// Start a worker of a given type. The worker will
/// be assigned a job by the caller.
Worker& Warehouse::launch_worker(Game& game, DescriptionIndex worker_id, const Requirements& req) {
	do {
		if (supply_->stock_workers(worker_id)) {
			uint32_t unincorporated = supply_->stock_workers(worker_id);

			//  look if we got one of those in stock
			if (incorporated_workers_.count(worker_id)) {
				// On cleanup, it could be that the worker was deleted under
				// us, so we erase the pointer we had to it and create a new
				// one.
				remove_no_longer_existing_workers(game, &incorporated_workers_[worker_id]);
				WorkerList& incorporated_workers = incorporated_workers_[worker_id];

				for (WorkerList::iterator worker_iter = incorporated_workers.begin();
				     worker_iter != incorporated_workers.end(); ++worker_iter) {
					Worker* worker = worker_iter->get(game);
					--unincorporated;

					if (req.check(*worker)) {
						worker->reset_tasks(game);   //  forget everything you did
						worker->set_location(this);  //  back in a economy
						incorporated_workers.erase(worker_iter);

						supply_->remove_workers(worker_id, 1);
						return *worker;
					}
				}
			}

			assert(unincorporated <= supply_->stock_workers(worker_id));

			if (unincorporated) {
				// Create a new one
				// NOTE: This code lies about the TrainingAttributes of the new worker
				supply_->remove_workers(worker_id, 1);
				const WorkerDescr& workerdescr = *game.descriptions().get_worker_descr(worker_id);
				return workerdescr.create(game, get_owner(), this, position_);
			}
		}

		if (can_create_worker(game, worker_id)) {
			// don't want to use an upgraded worker, so create new one.
			create_worker(game, worker_id);
		} else {
			worker_id = game.descriptions().get_worker_descr(worker_id)->becomes();
		}
	} while (owner().tribe().has_worker(worker_id));

	throw wexception("Warehouse::launch_worker: worker does not actually exist");
}

void Warehouse::incorporate_worker(EditorGameBase& egbase, Worker* w) {
	assert(w != nullptr);
	assert(w->get_owner() == get_owner());

	if (WareInstance* ware = w->fetch_carried_ware(egbase)) {
		incorporate_ware(egbase, ware);
	}

	DescriptionIndex worker_index = owner().tribe().worker_index(w->descr().name());

	supply_->add_workers(worker_index, 1);

	//  We remove free workers, but we keep other workers around.
	//  TODO(unknown): Remove all workers that do not have properties such as experience.
	//  And even such workers should be removed and only a small record
	//  with the experience (and possibly other data that must survive)
	//  may be kept.
	//  When this is done, the get_incorporated_workers method above must
	//  be reworked so that workers are recreated, and rescheduled for
	//  incorporation.
	if (w->descr().is_buildable() && w->descr().buildcost().empty()) {
		w->remove(egbase);
		return;
	}

	// Incorporate the worker
	if (!incorporated_workers_.count(worker_index)) {
		incorporated_workers_[worker_index] = WorkerList();
	}
	incorporated_workers_[worker_index].push_back(w);

	w->set_location(nullptr);  //  no longer in an economy

	if (upcast(Game, game, &egbase)) {
		//  Bind the worker into this house, hide him on the map.
		w->reset_tasks(*game);
		w->start_task_idle(*game, 0, -1);
	}
}

/// Create an instance of a ware and make sure it gets
/// carried out of the warehouse.
WareInstance& Warehouse::launch_ware(Game& game, DescriptionIndex const ware_index) {
	// Create the ware
	WareInstance& ware = *new WareInstance(ware_index, owner().tribe().get_ware_descr(ware_index));
	ware.init(game);
	if (do_launch_ware(game, ware)) {
		supply_->remove_wares(ware_index, 1);
	}
	return ware;
}

/// Get a carrier to actually move this ware out of the warehouse.
bool Warehouse::do_launch_ware(Game& game, WareInstance& ware) {
	// Create a carrier
	const DescriptionIndex carrierid = owner().tribe().carriers()[0];

	if (!supply_->stock_workers(carrierid)) {
		if (can_create_worker(game, carrierid)) {
			create_worker(game, carrierid);
		}
	}
	if (supply_->stock_workers(carrierid)) {
		Widelands::Worker& worker = launch_worker(game, carrierid, Requirements());
		// Setup the carrier
		worker.start_task_dropoff(game, ware);
		return true;
	}

	// We did not launch the ware...
	return false;
}

void Warehouse::incorporate_ware(EditorGameBase& egbase, WareInstance* ware) {
	supply_->add_wares(ware->descr_index(), 1);
	ware->destroy(egbase);
}

/// Called when a transfer for one of the idle Requests completes.
void Warehouse::request_cb(
   Game& game, Request&, DescriptionIndex const ware, Worker* const w, PlayerImmovable& target) {
	Warehouse& wh = dynamic_cast<Warehouse&>(target);

	if (w) {
		w->schedule_incorporate(game);
	} else {
		wh.supply_->add_wares(ware, 1);

		// This ware may be used to build planned workers,
		// so it seems like a good idea to update the associated requests
		// and use the ware before it is sent away again.
		wh.update_all_planned_workers(game);
	}
}

/**
 * Receive a ware from a transfer that was not associated to a \ref Request.
 */
void Warehouse::receive_ware(Game& /* game */, DescriptionIndex ware) {
	supply_->add_wares(ware, 1);
}

/**
 * Receive a worker from a transfer that was not associated to a \ref Request.
 */
void Warehouse::receive_worker(Game& game, Worker& worker) {
	worker.schedule_incorporate(game);
}

Building& WarehouseDescr::create_object() const {
	return *new Warehouse(*this);
}

bool Warehouse::can_create_worker(Game&, DescriptionIndex const worker) const {
	assert(owner().tribe().has_worker(worker));

	if (!(worker < supply_->get_workers().get_nrwareids())) {
		throw wexception("worker type %d does not exists (max is %d)", worker,
		                 supply_->get_workers().get_nrwareids());
	}

	const WorkerDescr& w_desc = *owner().tribe().get_worker_descr(worker);
	if (!w_desc.is_buildable()) {
		return false;
	}

	//  see if we have the resources
	for (const auto& buildcost : w_desc.buildcost()) {
		const std::string& input_name = buildcost.first;
		DescriptionIndex id_w = owner().tribe().ware_index(input_name);
		if (owner().tribe().has_ware(id_w)) {
			if (supply_->stock_wares(id_w) < buildcost.second) {
				return false;
			}
		} else {
			id_w = owner().tribe().worker_index(input_name);
			if (owner().tribe().has_worker(id_w)) {
				if (supply_->stock_workers(id_w) < buildcost.second) {
					return false;
				}
			} else {
				throw wexception("worker type %s needs \"%s\" to be built but that is neither "
				                 "a ware type nor a worker type defined in the tribe %s",
				                 w_desc.name().c_str(), input_name.c_str(),
				                 owner().tribe().name().c_str());
			}
		}
	}
	return true;
}

void Warehouse::create_worker(Game& game, DescriptionIndex const worker) {
	assert(can_create_worker(game, worker));

	const WorkerDescr& w_desc = *owner().tribe().get_worker_descr(worker);

	for (const auto& buildcost : w_desc.buildcost()) {
		const std::string& input = buildcost.first;
		DescriptionIndex const id_ware = owner().tribe().ware_index(input);
		if (owner().tribe().has_ware(id_ware)) {
			remove_wares(id_ware, buildcost.second);
			// Update statistics accordingly
			get_owner()->ware_consumed(id_ware, buildcost.second);
		} else {
			remove_workers(owner().tribe().safe_worker_index(input), buildcost.second);
		}
	}

	incorporate_worker(game, &w_desc.create(game, get_owner(), this, position_));

	// Update PlannedWorkers::amount here if appropriate, because this function
	// may have been called directly by the Economy.
	// Do not update anything else about PlannedWorkers here, because this
	// function is called by update_planned_workers, so avoid recursion
	for (PlannedWorkers& planned_worker : planned_workers_) {
		if (planned_worker.index == worker && planned_worker.amount) {
			planned_worker.amount--;
		}
	}
}

/**
 * Return the number of workers of the given type that we plan to
 * create in this warehouse.
 */
Quantity Warehouse::get_planned_workers(Game& /* game */, DescriptionIndex index) const {
	for (const PlannedWorkers& pw : planned_workers_) {
		if (pw.index == index) {
			return pw.amount;
		}
	}
	return 0;
}

/**
 * Calculate the supply of wares available to this warehouse in each of the
 * buildcost wares for the given worker.
 *
 * This is the current stock plus any incoming transfers.
 */
std::vector<Quantity> Warehouse::calc_available_for_worker(Game& /* game */,
                                                           DescriptionIndex index) const {
	const WorkerDescr& w_desc = *owner().tribe().get_worker_descr(index);
	std::vector<uint32_t> available;

	for (const auto& buildcost : w_desc.buildcost()) {
		const std::string& input_name = buildcost.first;
		DescriptionIndex id_w = owner().tribe().ware_index(input_name);
		if (owner().tribe().has_ware(id_w)) {
			available.push_back(get_wares().stock(id_w));
		} else {
			id_w = owner().tribe().worker_index(input_name);
			if (owner().tribe().has_worker(id_w)) {
				available.push_back(get_workers().stock(id_w));
			} else {
				throw wexception("Economy::create_requested_worker: buildcost inconsistency '%s'",
				                 input_name.c_str());
			}
		}
	}

	for (const PlannedWorkers& pw : planned_workers_) {
		if (pw.index == index) {
			assert(available.size() == pw.requests.size());

			for (uint32_t idx = 0; idx < available.size(); ++idx) {
				available[idx] += pw.requests[idx]->get_num_transfers();
			}
		}
	}

	return available;
}

/**
 * Set the amount of workers we plan to create
 * of the given \p index to \p amount.
 */
void Warehouse::plan_workers(Game& game, DescriptionIndex index, Quantity amount) {
	PlannedWorkers* pw = nullptr;

	for (PlannedWorkers& planned_worker : planned_workers_) {
		if (planned_worker.index == index) {
			pw = &planned_worker;
			break;
		}
	}

	if (!pw) {
		if (!amount) {
			return;
		}

		planned_workers_.push_back(PlannedWorkers());
		pw = &planned_workers_.back();
		pw->index = index;
		pw->amount = 0;

		const WorkerDescr& w_desc = *owner().tribe().get_worker_descr(pw->index);
		for (const auto& buildcost : w_desc.buildcost()) {
			const std::string& input_name = buildcost.first;

			DescriptionIndex id_w = owner().tribe().ware_index(input_name);
			if (owner().tribe().has_ware(id_w)) {
				pw->requests.push_back(new Request(*this, id_w, &Warehouse::request_cb, wwWARE));
			} else {
				id_w = owner().tribe().worker_index(input_name);
				if (owner().tribe().has_worker(id_w)) {
					pw->requests.push_back(new Request(*this, id_w, &Warehouse::request_cb, wwWORKER));
				} else {
					throw wexception("plan_workers: bad buildcost '%s'", input_name.c_str());
				}
			}
		}
	}

	pw->amount = amount;
	update_planned_workers(game, *pw);
}

/**
 * See if we can create the workers of the given plan,
 * and update requests accordingly.
 */
void Warehouse::update_planned_workers(Game& game, Warehouse::PlannedWorkers& pw) {
	const WorkerDescr& w_desc = *owner().tribe().get_worker_descr(pw.index);

	while (pw.amount && can_create_worker(game, pw.index)) {
		create_worker(game, pw.index);
	}

	uint32_t idx = 0;
	for (const auto& buildcost : w_desc.buildcost()) {

		const std::string& input_name = buildcost.first;
		Quantity supply;

		DescriptionIndex id_w = owner().tribe().ware_index(input_name);
		if (owner().tribe().has_ware(id_w)) {
			supply = supply_->stock_wares(id_w);
		} else {
			id_w = owner().tribe().worker_index(input_name);
			if (owner().tribe().has_worker(id_w)) {
				supply = supply_->stock_workers(id_w);
			} else {
				throw wexception("update_planned_workers: bad buildcost '%s'", input_name.c_str());
			}
		}
		if (supply >= pw.amount * buildcost.second) {
			pw.requests[idx]->set_count(0);
		} else {
			pw.requests[idx]->set_count(pw.amount * buildcost.second - supply);
		}
		++idx;
	}

	while (pw.requests.size() > idx) {
		delete pw.requests.back();
		pw.requests.pop_back();
	}
}

/**
 * Check all planned worker creations.
 *
 * Needs to be called periodically, because some necessary supplies might arrive
 * due to idle transfers instead of by explicit request.
 */
void Warehouse::update_all_planned_workers(Game& game) {
	uint32_t idx = 0;
	while (idx < planned_workers_.size()) {
		update_planned_workers(game, planned_workers_[idx]);

		if (!planned_workers_[idx].amount) {
			planned_workers_[idx].cleanup();
			planned_workers_.erase(planned_workers_.begin() + idx);
		} else {
			idx++;
		}
	}
}

void Warehouse::enable_spawn(Game& game, uint8_t const worker_types_without_cost_index) {
	assert(next_worker_without_cost_spawn_[worker_types_without_cost_index].is_invalid());
	next_worker_without_cost_spawn_[worker_types_without_cost_index] =
	   schedule_act(game, kCostlessWorkerSpawnInterval);
}

void Warehouse::PlannedWorkers::cleanup() {
	while (!requests.empty()) {
		delete requests.back();
		requests.pop_back();
	}
}

StockPolicy Warehouse::get_ware_policy(DescriptionIndex ware) const {
	assert(ware < static_cast<DescriptionIndex>(ware_policy_.size()));
	return ware_policy_[ware];
}

StockPolicy Warehouse::get_worker_policy(DescriptionIndex ware) const {
	assert(ware < static_cast<DescriptionIndex>(worker_policy_.size()));
	return worker_policy_[ware];
}

StockPolicy Warehouse::get_stock_policy(WareWorker waretype, DescriptionIndex wareindex) const {
	return waretype == wwWORKER ? get_worker_policy(wareindex) : get_ware_policy(wareindex);
}

void Warehouse::set_ware_policy(DescriptionIndex ware, StockPolicy policy) {
	assert(ware < static_cast<DescriptionIndex>(ware_policy_.size()));
	ware_policy_[ware] = policy;
	if (Economy* eco = get_economy(wwWARE)) {
		eco->rebalance_supply();
	}
}

void Warehouse::set_worker_policy(DescriptionIndex ware, StockPolicy policy) {
	assert(ware < static_cast<DescriptionIndex>(worker_policy_.size()));
	worker_policy_[ware] = policy;
	if (Economy* eco = get_economy(wwWORKER)) {
		eco->rebalance_supply();
	}
}

/**
 * Check if there are remaining wares with \ref StockPolicy::kRemove,
 * and remove one of them if appropriate.
 */
void Warehouse::check_remove_stock(Game& game) {
	if (portdock_ && portdock_->expedition_bootstrap()) {
		for (InputQueue* q : portdock_->expedition_bootstrap()->queues(true)) {
			if (q->get_type() == wwWARE && q->get_filled() > q->get_max_fill()) {
				// TODO(Nordfriese): We directly add the ware to the warehouse
				// here. This is inconsistent, see issues #1444/3880/4311
				q->set_filled(q->get_filled() - 1);
				supply_->add_wares(q->get_index(), 1);
				break;
			}
		}
	}

	if (base_flag().current_wares() < base_flag().total_capacity() / 2) {
		for (DescriptionIndex ware = 0; ware < static_cast<DescriptionIndex>(ware_policy_.size());
		     ++ware) {
			if (get_ware_policy(ware) != StockPolicy::kRemove || !get_wares().stock(ware)) {
				continue;
			}

			launch_ware(game, ware);
			break;
		}
	}

	for (DescriptionIndex widx = 0; widx < static_cast<DescriptionIndex>(worker_policy_.size());
	     ++widx) {
		if (get_worker_policy(widx) != StockPolicy::kRemove || !get_workers().stock(widx)) {
			continue;
		}

		Worker& worker = launch_worker(game, widx, Requirements());
		worker.start_task_leavebuilding(game, true);
		break;
	}
}

InputQueue& Warehouse::inputqueue(DescriptionIndex index, WareWorker type, const Request* r) {
	assert(portdock_ != nullptr);
	assert(portdock_->expedition_bootstrap() != nullptr);
	return r ? portdock_->expedition_bootstrap()->inputqueue(*r) :
              portdock_->expedition_bootstrap()->inputqueue(index, type, false);
}

std::unique_ptr<const BuildingSettings> Warehouse::create_building_settings() const {
	std::unique_ptr<WarehouseSettings> settings(new WarehouseSettings(descr(), owner().tribe()));
	for (auto& pair : settings->ware_preferences) {
		pair.second = get_ware_policy(pair.first);
	}
	for (auto& pair : settings->worker_preferences) {
		pair.second = get_worker_policy(pair.first);
	}
	settings->launch_expedition = portdock_ && portdock_->expedition_started();
	// Prior to the resolution of a defect report against ISO C++11, local variable 'settings' would
	// have been copied despite being returned by name, due to its not matching the function return
	// type. Call 'std::move' explicitly to avoid copying on older compilers.
	// On modern compilers a simple 'return settings;' would've been fine.
	return std::unique_ptr<const BuildingSettings>(std::move(settings));
}

void Warehouse::log_general_info(const EditorGameBase& egbase) const {
	Building::log_general_info(egbase);

	if (descr().get_isport()) {
		if (portdock_) {
			molog(egbase.get_gametime(), "Port dock: %u\n", portdock_->serial());
			molog(
			   egbase.get_gametime(), "wares and workers waiting: %u\n", portdock_->count_waiting());
			molog(egbase.get_gametime(), "exped. in progr.: %s\n",
			      (portdock_->expedition_started()) ? "true" : "false");
			ShipFleet* fleet = portdock_->get_fleet();
			if (fleet) {
				molog(egbase.get_gametime(), "* fleet: %u\n", fleet->serial());
				molog(egbase.get_gametime(), "  ships: %u, ports: %u\n", fleet->count_ships(),
				      fleet->count_ports());
				molog(egbase.get_gametime(), "  act_pending: %s\n",
				      (fleet->get_act_pending()) ? "true" : "false");
			} else {
				molog(egbase.get_gametime(), "No fleet?!\n");
			}
		} else {
			molog(egbase.get_gametime(), "No port dock!?\n");
		}
	}
}
}  // namespace Widelands
