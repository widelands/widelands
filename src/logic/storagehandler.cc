/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#include "logic/storagehandler.h"

#include <utility>

#include <boost/foreach.hpp>

#include "economy/economy.h"
#include "economy/flag.h"
#include "logic/player.h"
#include "logic/soldier.h"
#include "upcast.h"
#include <helper.h>

namespace Widelands {

//************************************************************
// CLASS Storage Handler
//************************************************************

StorageHandler::StorageHandler(Building& building)
:  m_building(building),
	m_supply(new StorageSupply(this))
{
}

StorageHandler::~StorageHandler()
{
}


void StorageHandler::init(Editor_Game_Base& )
{
	Ware_Index const nr_wares   = owner().tribe().get_nrwares  ();
	Ware_Index const nr_workers = owner().tribe().get_nrworkers();
	m_supply->set_nrwares  (nr_wares);
	m_supply->set_nrworkers(nr_workers);
	m_ware_policy.resize(nr_wares.value(), Storage::StockPolicy::Normal);
	m_worker_policy.resize(nr_workers.value(), Storage::StockPolicy::Normal);

	// Init autospawn
	const std::vector <Widelands::Ware_Index> worker_types_without_cost =
		owner().tribe().worker_types_without_cost();
	BOOST_FOREACH(Ware_Index idx, worker_types_without_cost) {
		const Worker_Descr & worker_descr = *owner().tribe().get_worker_descr(idx);
		if (not worker_descr.is_buildable()) {
			continue;
		}
		if (!owner().is_worker_type_allowed(idx)) {
			continue;
		}
		if (worker_descr.buildcost().empty()) {
			//  Workers of this type can be spawned in warehouses. Start it.
			add_worker_spawn(idx);
		}
	}
}

void StorageHandler::load_finish(Editor_Game_Base& )
{
	// Ensure consistency of PlannedWorker requests
	uint32_t pwidx = 0;
	while (pwidx < m_planned_workers.size()) {
		if (!load_finish_planned_workers(m_planned_workers[pwidx])) {
			m_planned_workers[pwidx].cleanup();
			m_planned_workers.erase(m_planned_workers.begin() + pwidx);
		} else {
			pwidx++;
		}
	}
	// Verify autospawning for free workers
	const std::vector <Widelands::Ware_Index> worker_types_without_cost =
		owner().tribe().worker_types_without_cost();
	BOOST_FOREACH(Ware_Index idx, worker_types_without_cost) {
		if (!owner().is_worker_type_allowed(idx)) {
			continue;
		}
		if (is_worker_spawn_set(idx)) {
			continue;
		}
		add_worker_spawn(idx);
		log
			("WARNING: player %u is allowed to create worker type %s but his "
				"%s %u at (%i, %i) does not have spawning set for that "
				"worker type; setting it to default values\n",
				owner().player_number(),
				owner().tribe().get_worker_descr(idx)->descname().c_str(),
				m_building.descname().c_str(), m_building.serial(),
			   m_building.get_position().x, m_building.get_position().y);
	}
}


void StorageHandler::cleanup(Editor_Game_Base&)
{
	while (!m_planned_workers.empty()) {
		m_planned_workers.back().cleanup();
		m_planned_workers.pop_back();
	}
}

//TODO return a value so the owner can schedule_act
uint32_t StorageHandler::act(Game& game)
{
	uint32_t gametime = static_cast<uint32_t>(game.get_gametime());
	uint32_t spawn_act = update_spawns(game, gametime);
	uint32_t removal_act = update_removals(game, gametime);
	// Update planned workers; this is to update the request amounts and
	// check because whether we suddenly can produce a requested worker. This
	// is mostly previously available wares may become unavailable due to
	// secondary requests.
	update_all_planned_workers(game);
	// Act in 5 sec, or before if timers will run down
	uint32_t min_act = 5000;
	if (spawn_act > 0 && spawn_act < min_act) {
		min_act = spawn_act;
	}
	if (removal_act > 0 && removal_act < min_act) {
		min_act = removal_act;
	}
	return min_act;
}

void StorageHandler::set_economy(Economy* economy)
{
	Economy * const old = m_building.get_economy();

	if (old == economy) {
		return;
	}

	if (old) {
		old->remove_storage(*this);
	}

	m_supply->set_economy(economy);

	container_iterate_const(std::vector<PlannedWorkers>, m_planned_workers, pw_it) {
		container_iterate_const(std::vector<Request *>, pw_it.current->requests, req_it) {
				(*req_it.current)->set_economy(economy);
			}
	}

	if (economy) {
		economy->add_storage(*this);
	}
}

uint32_t StorageHandler::count_workers(Game& game, Ware_Index idx, const Requirements& req)
{
	uint32_t stock = m_supply->stock_workers(idx);
	if (stock <= 0) {
		return stock;
	}
	uint32_t matching = 0;
	// Get list of those for which we stored the attributes
	// This is similar logic than in launch_worker
	std::vector<const StockedWorkerAtr*> workers_with_atr;
	BOOST_FOREACH(StockedWorkerAtr& atr, m_stocked_workers_atr) {
		if (atr.index == idx) {
			workers_with_atr.push_back(&atr);
		}
	}
	// First check those without atr
	Worker* w;
	bool checkpass = false;
	const Worker_Descr & workerdescr = *owner().tribe().get_worker_descr(idx);
	if (stock > workers_with_atr.size()) {
		w = &workerdescr.create(game, owner(), &m_building, m_building.get_position());
		checkpass = req.check(*w);
		if (checkpass) {
			matching += stock - workers_with_atr.size();
		}
		w->remove(game);
	}
	// Check with our atr
	// TODO CGH use smarter way of storing atr, maybe with map
	BOOST_FOREACH(const StockedWorkerAtr* atr, workers_with_atr) {
		w = create_with_atr(game, *atr);
		checkpass = req.check(*w);
		if (checkpass) {
			matching++;
		}
		w->remove(game);
	}
	return matching;
}


void StorageHandler::launch_all_workers(Game & game, bool exp_only, const WareList& excluded)
{
	if (!exp_only) {
		for (Ware_Index id = Ware_Index::First(); id < m_supply->get_workers().get_nrwareids(); ++id) {
			if (excluded.stock(id)) {
				continue;
			}
			const uint32_t stock = m_supply->get_workers().stock(id);
			if (stock > 0) {
				for (uint32_t i = 0; i < stock; ++i) {
					launch_worker(game, id).start_task_leavebuilding(game, true);
				}
			}
		}
		return;
	}
	BOOST_FOREACH(StockedWorkerAtr atr, m_stocked_workers_atr) {
		if (excluded.stock(atr.index)) {
			continue;
		}
		if (m_supply->get_workers().stock(atr.index) <= 0) {
			continue;
		}
		Worker* w = create_with_atr(game, atr);
		w->start_task_leavebuilding(game, true);
		m_supply->remove_workers(atr.index, 1);
	}
}

void StorageHandler::add_ware_spawn
	(const Ware_Index idx, uint32_t interval, uint32_t stock_max, uint32_t counter, bool dont_exceed)
{
	if (m_spawn_wares.count(idx)) {
		remove_ware_spawn(idx);
	}
	if (dont_exceed) {
		std::vector<uint32_t> values = {interval, interval, stock_max};
		m_spawn_wares.insert(std::make_pair(idx, values));
	} else {
		std::vector<uint32_t> values = {interval, interval, stock_max, counter};
		m_spawn_wares.insert(std::make_pair(idx, values));
	}
}

void StorageHandler::remove_ware_spawn(const Ware_Index idx)
{
	m_spawn_wares.erase(idx);
}
bool StorageHandler::is_ware_spawn_set(Ware_Index idx)
{
	return m_spawn_wares.count(idx);
}


void StorageHandler::add_worker_spawn
	(const Ware_Index idx, uint32_t interval, uint32_t stock_max, uint32_t counter, bool dont_exceed)
{
	if (m_spawn_workers.count(idx)) {
		remove_worker_spawn(idx);
	}
	if (dont_exceed) {
		std::vector<uint32_t> values = {interval, interval, stock_max};
		m_spawn_workers.insert(std::make_pair(idx, values));
	} else {
		std::vector<uint32_t> values = {interval, interval, stock_max, counter};
		m_spawn_workers.insert(std::make_pair(idx, values));
	}
}

void StorageHandler::remove_worker_spawn(const Ware_Index idx)
{
	m_spawn_workers.erase(idx);
}
bool StorageHandler::is_worker_spawn_set(Ware_Index idx)
{
	return m_spawn_workers.count(idx);
}

//
//
//  Storage implementation
//
//

Player& StorageHandler::owner() const
{
	return m_building.owner();
}

const WareList& StorageHandler::get_wares() const
{
	return m_supply->get_wares();
}

const WareList& StorageHandler::get_workers() const
{
	return m_supply->get_workers();
}

void StorageHandler::insert_wares(Ware_Index idx, uint32_t count)
{
	m_supply->add_wares(idx, count);
}

void StorageHandler::remove_wares(Ware_Index idx, uint32_t count)
{
	m_supply->remove_wares(idx, count);
}

void StorageHandler::insert_workers(Ware_Index idx, uint32_t count)
{
	m_supply->add_workers(idx, count);
}

void StorageHandler::remove_workers(Ware_Index idx, uint32_t count)
{
	m_supply->remove_workers(idx, count);
}

WareInstance& StorageHandler::launch_ware(Game& game, Ware_Index idx)
{
	WareInstance & item = *new WareInstance(idx, owner().tribe().get_ware_descr(idx));
	item.init(game);
	do_launch_ware(game, item);

	m_supply->remove_wares(idx, 1);

	return item;
}


void StorageHandler::do_launch_ware(Game& game, WareInstance& ware_inst)
{
	// Create a carrier
	Ware_Index const carrierid = owner().tribe().worker_index("carrier");
	const Worker_Descr & workerdescr = *owner().tribe().get_worker_descr(carrierid);

	Worker & worker = workerdescr.create(game, owner(), &m_building, m_building.get_position());

	// Yup, this is cheating.
	if (m_supply->stock_workers(carrierid))
		m_supply->remove_workers(carrierid, 1);

	// Setup the carrier
	worker.start_task_dropoff(game, ware_inst);
}


void StorageHandler::incorporate_ware(Editor_Game_Base& egbase, WareInstance& item)
{
	m_supply->add_wares(item.descr_index(), 1);
	return item.destroy(egbase);
}

Worker& StorageHandler::launch_worker(Game& game, Ware_Index idx, const Requirements& req)
{
	do {
		uint32_t stock = m_supply->stock_workers(idx);
		if (stock > 0) {
			// Get list of those for which we stored the attributes, as pointers to actual objects
			// TODO smarter storage for atrs
			std::vector<const StockedWorkerAtr*> workers_with_atr;
			for
				(std::vector<StockedWorkerAtr>::iterator it = m_stocked_workers_atr.begin();
					it != m_stocked_workers_atr.end(); ++it)
			{
				if ((*it).index == idx) {
					workers_with_atr.push_back(&(*it));
				}
			}
			// First check if one without atr is enough
			Worker* w;
			if (stock > workers_with_atr.size()) {
				const Worker_Descr & workerdescr = *owner().tribe().get_worker_descr(idx);
				w = &workerdescr.create(game, owner(), &m_building, m_building.get_position());
				if (req.check(*w)) {
					m_supply->remove_workers(idx, 1);
					return *w;
				}
			}
			// Check with our atr
			BOOST_FOREACH(const StockedWorkerAtr* atr, workers_with_atr) {
				w = create_with_atr(game, *atr);
				if (req.check(*w)) {
					// We need to find back our atr in original vector to erase it
					bool removed = false;
					for
						(std::vector<StockedWorkerAtr>::iterator it = m_stocked_workers_atr.begin();
						 it != m_stocked_workers_atr.end(); ++it)
					{
						if (&(*it) == atr) {
							m_stocked_workers_atr.erase(it);
							removed = true;
							break;
						}
					}
					assert(removed);
					m_supply->remove_workers(idx, 1);
					return *w;
				}
			}
		}
		// Check if we got the tools to create a new one
		if (can_create_worker(game, idx)) {
			// don't want to use an upgraded worker, so create new one.
			create_worker(game, idx);
		} else {
			idx = owner().tribe().get_worker_descr(idx)->becomes();
		}
	} while (idx != Ware_Index::Null());

	throw wexception
		("StorageHandler::launch_worker: worker does not actually exist");
}

void StorageHandler::incorporate_worker(Editor_Game_Base& egbase, Worker& w)
{
	assert(w.get_owner() == &owner());
	// Rescue carried ware
	if (WareInstance * const item = w.fetch_carried_item(egbase)) {
		incorporate_ware(egbase, *item);
	}
	// Add to supply
	Ware_Index worker_index = owner().tribe().worker_index(w.name().c_str());
	m_supply->add_workers(worker_index, 1);

	// Store the attributes.
	// FIXME CGH handle soldiers : hp & healing
	StockedWorkerAtr* atr = store_worker_atr(w);
	if (atr != nullptr) {
		m_stocked_workers_atr.push_back(*atr);
	}
	// Destroy the instance
	w.remove(egbase);
}

Storage::StockPolicy StorageHandler::get_ware_policy(Ware_Index ware) const
{
	assert(ware.value() < m_ware_policy.size());
	return m_ware_policy[ware.value()];
}

Storage::StockPolicy StorageHandler::get_worker_policy(Ware_Index ware) const
{
	assert(ware.value() < m_worker_policy.size());
	return m_worker_policy[ware.value()];
}

Storage::StockPolicy StorageHandler::get_stock_policy(WareWorker waretype, Ware_Index wareindex) const
{
	if (waretype == wwWORKER)
		return get_worker_policy(wareindex);
	else
		return get_ware_policy(wareindex);
}

void StorageHandler::set_ware_policy(Ware_Index ware, Storage::StockPolicy policy)
{
	assert(ware.value() < m_ware_policy.size());
	m_ware_policy[ware.value()] = policy;
}

void StorageHandler::set_worker_policy(Ware_Index ware, Storage::StockPolicy policy)
{
	assert(ware.value() < m_worker_policy.size());
	m_worker_policy[ware.value()] = policy;
}

bool StorageHandler::can_create_worker(Game&, Ware_Index worker_idx)
{
	if (not (worker_idx < m_supply->get_workers().get_nrwareids())) {
		throw wexception
			("worker type %d does not exists (max is %d)",
			 worker_idx.value(), m_supply->get_workers().get_nrwareids().value());
	}

	const Worker_Descr & w_desc = *owner().tribe().get_worker_descr(worker_idx);
	assert(&w_desc);
	if (not w_desc.is_buildable()) {
		return false;
	}

	//  see if we have the resources
	const Worker_Descr::Buildcost & buildcost = w_desc.buildcost();
	container_iterate_const(Worker_Descr::Buildcost, buildcost, it) {
		const std::string & input_name = it.current->first;
		if (Ware_Index id_w = owner().tribe().ware_index(input_name)) {
			if (m_supply->stock_wares  (id_w) < it.current->second)
				return false;
		} else if ((id_w = owner().tribe().worker_index(input_name))) {
			if (m_supply->stock_workers(id_w) < it.current->second)
				return false;
		} else
			throw wexception
				("worker type %s needs \"%s\" to be built but that is neither "
				 "a ware type nor a worker type defined in the tribe %s",
				 w_desc.descname().c_str(), input_name.c_str(),
				 owner().tribe().name().c_str());
	}
	return true;
}

void StorageHandler::create_worker(Game& game, Ware_Index worker_idx)
{
	assert(can_create_worker (game, worker_idx));

	const Worker_Descr & w_desc = *owner().tribe().get_worker_descr(worker_idx);
	const Worker_Descr::Buildcost & buildcost = w_desc.buildcost();
	container_iterate_const(Worker_Descr::Buildcost, buildcost, i) {
		const std::string & input = i.current->first;
		if (Ware_Index const id_ware = owner().tribe().ware_index(input)) {
			remove_wares  (id_ware,                        i.current->second);
			//update statistic accordingly
			owner().ware_consumed(id_ware, i.current->second);
		} else
			remove_workers(owner().tribe().safe_worker_index(input), i.current->second);
	}

	incorporate_worker(game, w_desc.create(game, owner(), &m_building, m_building.get_position()));

	// Update PlannedWorkers::amount here if appropriate, because this function
	// may have been called directly by the Economy.
	// Do not update anything else about PlannedWorkers here, because this
	// function is called by _update_planned_workers, so avoid recursion
	container_iterate(std::vector<PlannedWorkers>, m_planned_workers, pw_it) {
		if (pw_it.current->index == worker_idx && pw_it.current->amount) {
			pw_it.current->amount--;
		}
	}
}

uint32_t StorageHandler::get_planned_workers(Game&, Ware_Index index) const
{
	container_iterate_const(std::vector<PlannedWorkers>, m_planned_workers, i) {
		if (i.current->index == index)
			return i.current->amount;
	}

	return 0;
}

void StorageHandler::plan_workers(Game& game, Ware_Index index, uint32_t amount)
{
	PlannedWorkers * pw = 0;

	container_iterate(std::vector<PlannedWorkers>, m_planned_workers, i) {
		if (i.current->index == index) {
			pw = &*i.current;
			break;
		}
	}

	if (!pw) {
		if (!amount)
			return;

		m_planned_workers.push_back(PlannedWorkers());
		pw = &m_planned_workers.back();
		pw->index = index;
		pw->amount = 0;

		const Worker_Descr & w_desc = *owner().tribe().get_worker_descr(pw->index);
		const Worker_Descr::Buildcost & cost = w_desc.buildcost();
		container_iterate_const(Worker_Descr::Buildcost, cost, cost_it) {
			const std::string & input_name = cost_it.current->first;

			if (Ware_Index id_w = owner().tribe().ware_index(input_name)) {
				pw->requests.push_back
					(new Request
					 (m_building, id_w, &StorageHandler::planned_worker_callback, wwWARE));
			} else if ((id_w = owner().tribe().worker_index(input_name))) {
				pw->requests.push_back
					(new Request
					 (m_building, id_w, &StorageHandler::planned_worker_callback, wwWORKER));
			} else
				throw wexception
					("plan_workers: bad buildcost '%s'", input_name.c_str());
		}
	}

	pw->amount = amount;
	update_planned_workers(game, *pw);
}

std::vector< uint32_t > StorageHandler::calc_available_for_worker(Game&, Ware_Index index) const
{
	const Worker_Descr & w_desc = *owner().tribe().get_worker_descr(index);
	const Worker_Descr::Buildcost & cost = w_desc.buildcost();
	std::vector<uint32_t> available;

	container_iterate_const(Worker_Descr::Buildcost, cost, bc) {
		const std::string & input_name = bc.current->first;
		if (Ware_Index id_w = owner().tribe().ware_index(input_name)) {
			available.push_back(get_wares().stock(id_w));
		} else if ((id_w = owner().tribe().worker_index(input_name))) {
			available.push_back(get_workers().stock(id_w));
		} else
			throw wexception
				("StorageHandler::calc_available_for_worker: buildcost inconsistency '%s'",
				 input_name.c_str());
	}

	container_iterate_const(std::vector<PlannedWorkers>, m_planned_workers, i) {
		if (i.current->index == index) {
			assert(available.size() == i.current->requests.size());

			for (uint32_t idx = 0; idx < available.size(); ++idx)
				available[idx] += i.current->requests[idx]->get_num_transfers();
		}
	}

	return available;
}


//
//
//
// Private methods
//
//

uint32_t StorageHandler::update_spawns(Game&, uint32_t gametime)
{
	uint32_t min_act = 0;
	// Update spawnings
	SpawnMapType::iterator it;
	for (it = m_spawn_wares.begin(); it != m_spawn_wares.end();) {
		Ware_Index idx = (*it).first;
		std::vector<uint32_t> values = (*it).second;
		bool dont_exceed = values.size() < 4;
		bool infinite = dont_exceed || values[3] == 0;
		uint32_t stock = m_supply->stock_wares(idx);
		uint32_t stock_max = values[2];
		// Skip if max stock excessed for non infinite spawns
		if (stock_max > 0 && stock >= stock_max && !dont_exceed) {
			++it;
			continue;
		}
		// update timer
		uint32_t* next_act = &((*it).second[0]);
		if (*next_act > gametime) {
			++it;
			continue;
		}
		uint32_t interval = values[1];
		while (*next_act <= gametime) {
			*next_act += interval;
		}
		if (!infinite) {
			// decrease counter
			uint32_t* counter_p = &((*it).second[3]);
			(*counter_p)--;
			m_supply->add_wares(idx, 1);
			if ((*counter_p) == 0) {
				m_spawn_wares.erase(it++);
				continue;
			}
		} else {
			// infinte spawns
			if (stock < stock_max) {
				m_supply->add_wares(idx, 1);
			} else if (stock > stock_max && dont_exceed) {
				m_supply->remove_wares(idx, 1);
			}
		}
		++it;
		uint32_t time_till_next_act = *next_act - gametime;
		min_act = (min_act == 0 ? time_till_next_act :
			min_act > time_till_next_act ? time_till_next_act : min_act);
	}
	for (it = m_spawn_workers.begin(); it != m_spawn_workers.end();) {
		Ware_Index idx = (*it).first;
		std::vector<uint32_t> values = (*it).second;
		bool dont_exceed = values.size() < 4;
		bool infinite = dont_exceed || values[3] == 0;
		uint32_t stock = m_supply->stock_workers(idx);
		uint32_t stock_max = values[2];
		// Skip if max stock excessed for non infinite spawns
		if (stock_max > 0 && stock >= stock_max && !dont_exceed) {
			++it;
			continue;
		}
		// update timer
		uint32_t* next_act = &((*it).second[0]);
		if (*next_act > gametime) {
			++it;
			continue;
		}
		uint32_t interval = values[1];
		while (*next_act <= gametime) {
			*next_act += interval;
		}
		if (!infinite) {
			// decrease counter
			uint32_t* counter_p = &((*it).second[3]);
			(*counter_p)--;
			m_supply->add_workers(idx, 1);
			if ((*counter_p) == 0) {
				m_spawn_workers.erase(it++);
				continue;
			}
		} else {
			// infinte spawns
			if (stock < stock_max) {
				m_supply->add_workers(idx, 1);
			} else if (stock > stock_max && dont_exceed) {
				m_supply->remove_workers(idx, 1);
			}
		}
		++it;
		uint32_t time_till_next_act = *next_act - gametime;
		min_act = (min_act == 0 ? time_till_next_act :
			min_act > time_till_next_act ? time_till_next_act : min_act);
	}

	return min_act;
}

uint32_t StorageHandler::update_removals(Game& game, uint32_t gametime)
{
	if (m_removal_next_act > gametime) {
		return gametime - m_removal_next_act;
	}
	while (m_removal_next_act <= gametime) {
		m_removal_next_act += STORAGE_REMOVAL_INTERVAL;
	}
	if (m_building.base_flag().current_items() < m_building.base_flag().total_capacity() / 2) {
		for (Ware_Index ware = Ware_Index::First(); ware.value() < m_ware_policy.size(); ++ware) {
			if (get_ware_policy(ware) != StockPolicy::Remove || !get_wares().stock(ware)) {
				continue;
			}
			launch_ware(game, ware);
			break;
		}
	}

	for (Ware_Index widx = Ware_Index::First(); widx.value() < m_worker_policy.size(); ++widx) {
		if (get_worker_policy(widx) != StockPolicy::Remove || !get_workers().stock(widx)) {
			continue;
		}
		Worker & worker = launch_worker(game, widx);
		worker.start_task_leavebuilding(game, true);
		break;
	}
	return gametime - m_removal_next_act;
}

void StorageHandler::update_all_planned_workers(Game& game)
{
	uint32_t idx = 0;
	while (idx < m_planned_workers.size()) {
		update_planned_workers(game, m_planned_workers[idx]);

		if (!m_planned_workers[idx].amount) {
			m_planned_workers[idx].cleanup();
			m_planned_workers.erase(m_planned_workers.begin() + idx);
		} else {
			idx++;
		}
	}
}

void StorageHandler::update_planned_workers
	(Game& game, StorageHandler::PlannedWorkers& planned_workers)
{
	const Tribe_Descr& tribe_descr = owner().tribe();
	const Worker_Descr & w_desc = *tribe_descr.get_worker_descr(planned_workers.index);
	const Worker_Descr::Buildcost & cost = w_desc.buildcost();

	while (planned_workers.amount && can_create_worker(game, planned_workers.index)) {
		create_worker(game, planned_workers.index);
	}

	uint32_t idx = 0;
	container_iterate_const(Worker_Descr::Buildcost, cost, cost_it) {
		const std::string & input_name = cost_it.current->first;
		uint32_t supply;

		if (Ware_Index id_w = tribe_descr.ware_index(input_name)) {
			supply = m_supply->stock_wares(id_w);
		} else if ((id_w = tribe_descr.worker_index(input_name))) {
			supply = m_supply->stock_workers(id_w);
		} else {
			throw wexception
				("_update_planned_workers: bad buildcost '%s'", input_name.c_str());
		}

		if (supply >= planned_workers.amount * cost_it.current->second) {
			planned_workers.requests[idx]->set_count(0);
		} else {
			planned_workers.requests[idx]->set_count
				(planned_workers.amount * cost_it.current->second - supply);
		}
		++idx;
	}

	while (planned_workers.requests.size() > idx) {
		delete planned_workers.requests.back();
		planned_workers.requests.pop_back();
	}
}

StorageHandler::StockedWorkerAtr* StorageHandler::store_worker_atr(Worker& w)
{
	if (upcast(Soldier, s, &w)) {
		if
			(!s->get_attack_level()
			&& !s->get_defense_level()
			&& !s->get_evade_level()
			&& !s->get_hp_level())
		{
			return nullptr;
		}
		StockedWorkerAtr* atr = new StockedWorkerAtr();
		atr->atck_lvl = s->get_attack_level();
		atr->defense_lvl = s->get_defense_level();
		atr->evade_lvl = s->get_evade_level();
		atr->exp_or_hp_lvl = s->get_hp_level();
		atr->index = w.descr().worker_index();
		atr->descr = &w.descr();
		return atr;
	}
	// Normal worker - only exp
	if (w.get_current_experience() <= 0) {
		return nullptr;
	}
	StockedWorkerAtr* atr = new StockedWorkerAtr();
	atr->exp_or_hp_lvl = w.get_current_experience();
	atr->index = w.descr().worker_index();
	atr->descr = &w.descr();
	return atr;
}

Worker* StorageHandler::create_with_atr(Game& game, StockedWorkerAtr atr)
{
	Worker* w = &atr.descr->create(game, owner(), &m_building, m_building.get_position());
	if (upcast(Soldier, s, w)) {
		s->set_attack_level(atr.atck_lvl);
		s->set_defense_level(atr.defense_lvl);
		s->set_evade_level(atr.evade_lvl);
		s->set_hp_level(atr.exp_or_hp_lvl);
		return w;
	}
	w->set_experience(atr.exp_or_hp_lvl);
	return w;
}


void StorageHandler::PlannedWorkers::cleanup()
{
	while (!requests.empty()) {
		delete requests.back();
		requests.pop_back();
	}
}

void StorageHandler::planned_worker_callback
	(Game& game, Request&, const Ware_Index ware, Worker* const w, PlayerImmovable& target)
{
	upcast(StorageOwner, owner, &target);
	upcast(StorageHandler, sh, owner->get_storage());
	if (w) {
		w->schedule_incorporate(game);
	} else {
		sh->m_supply->add_wares(ware, 1);

		// This ware may be used to build planned workers,
		// so it seems like a good idea to update the associated requests
		// and use the ware before it is sent away again.
		sh->update_all_planned_workers(game);
	}
}

/**
 * Try to bring the given \ref PlannedWorkers up to date with our game data.
 * Return \c false if \p pw cannot be salvaged.
 */
bool StorageHandler::load_finish_planned_workers(PlannedWorkers & pw)
{
	if (!pw.index || !(pw.index < m_supply->get_workers().get_nrwareids()))
		return false;

	const Worker_Descr * w_desc = owner().tribe().get_worker_descr(pw.index);
	if (!w_desc || !w_desc->is_buildable())
		return false;

	const Worker_Descr::Buildcost & cost = w_desc->buildcost();
	uint32_t idx = 0;

	for
		(Worker_Descr::Buildcost::const_iterator cost_it = cost.begin();
		 cost_it != cost.end(); ++cost_it, ++idx)
	{
		WareWorker type;
		Ware_Index ware;

		if ((ware = owner().tribe().ware_index(cost_it->first)))
			type = wwWARE;
		else if ((ware = owner().tribe().worker_index(cost_it->first)))
			type = wwWORKER;
		else
			return false;

		if (idx < pw.requests.size()) {
			if
				(pw.requests[idx]->get_type() == type &&
				 pw.requests[idx]->get_index() == ware)
				continue;

			std::vector<Request *>::iterator req_it =
				pw.requests.begin() + idx + 1;
			while (req_it != pw.requests.end()) {
				if ((*req_it)->get_type() == type && (*req_it)->get_index() == ware)
					break;
				++req_it;
			}

			if (req_it != pw.requests.end()) {
				std::swap(*req_it, pw.requests[idx]);
				continue;
			}
		}

		log
			("_load_finish_planned_worker: old savegame: "
			 "need to create new request for '%s'\n",
			 cost_it->first.c_str());
		pw.requests.insert
			(pw.requests.begin() + idx,
			 new Request(m_building, ware, &StorageHandler::planned_worker_callback, type));
	}

	while (pw.requests.size() > idx) {
		log
			("_load_finish_planned_worker: old savegame: "
			 "removing outdated request.\n");
		delete pw.requests.back();
		pw.requests.pop_back();
	}

	return true;
}


// *************************************************************
//
// Class StorageSupply
//
// *************************************************************

StorageSupply::StorageSupply(StorageHandler* handler)
:  m_handler(*handler),
	m_economy(nullptr)
{
}

StorageSupply::~StorageSupply()
{
	if (m_economy) {
		log
			("StorageSupply::~StorageSupply: %s %u still belongs to "
			 "an economy", m_handler.get_building().descname().c_str(),
			 m_handler.get_building().serial());
		set_economy(nullptr);
	}

	// We're removed from the Economy. Therefore, the wares can simply
	// be cleared out. The global inventory will be okay.
	m_wares  .clear();
	m_workers.clear();
}

void StorageSupply::set_economy(Economy* const e)
{
	if (e == m_economy)
		return;

	if (m_economy) {
		m_economy->remove_supply(*this);
		for (Ware_Index i = Ware_Index::First(); i < m_wares.get_nrwareids(); ++i)
			if (m_wares.stock(i))
				m_economy->remove_wares(i, m_wares.stock(i));
		for (Ware_Index i = Ware_Index::First(); i < m_workers.get_nrwareids(); ++i)
			if (m_workers.stock(i))
				m_economy->remove_workers(i, m_workers.stock(i));
	}

	m_economy = e;

	if (m_economy) {
		for (Ware_Index i = Ware_Index::First(); i < m_wares.get_nrwareids(); ++i)
			if (m_wares.stock(i))
				m_economy->add_wares(i, m_wares.stock(i));
		for (Ware_Index i = Ware_Index::First(); i < m_workers.get_nrwareids(); ++i)
			if (m_workers.stock(i))
				m_economy->add_workers(i, m_workers.stock(i));
		m_economy->add_supply(*this);
	}
}

void StorageSupply::set_nrwares(Ware_Index i)
{
	assert(Ware_Index::First() == m_wares.get_nrwareids());
	m_wares.set_nrwares(i);
}
void StorageSupply::set_nrworkers(Ware_Index i)
{
	assert(Ware_Index::First() == m_workers.get_nrwareids());
	m_workers.set_nrwares(i);
}

void StorageSupply::add_wares(Ware_Index id, uint32_t count)
{
	if (!count)
		return;

	if (m_economy) // No economies in the editor
		m_economy->add_wares(id, count);
	m_wares.add(id, count);
}
void StorageSupply::remove_wares(Ware_Index id, uint32_t count)
{
	if (!count)
		return;

	m_wares.remove(id, count);
	if (m_economy) // No economies in the editor
		m_economy->remove_wares(id, count);
}
void StorageSupply::add_workers(Ware_Index id, uint32_t count)
{
	if (!count)
		return;

	if (m_economy) // No economies in the editor
		m_economy->add_workers(id, count);
	m_workers.add(id, count);
}
void StorageSupply::remove_workers(Ware_Index id, uint32_t count)
{
	if (!count)
		return;

	m_workers.remove(id, count);
	if (m_economy) // No economies in the editor
		m_economy->remove_workers(id, count);
}

PlayerImmovable* StorageSupply::get_position(Game&)
{
	return &m_handler.get_building();
}

bool StorageSupply::is_active() const throw ()
{
	return false;
}

bool StorageSupply::has_storage() const throw ()
{
	return true;
}

void StorageSupply::get_ware_type(WareWorker&, Ware_Index&) const
{
	throw wexception
		("StorageSupply::get_ware_type: calling this is nonsensical");
}

void StorageSupply::send_to_storage(Game&, StorageOwner*)
{
	throw wexception("StorageSupply::send_to_storage: should never be called");
}

uint32_t StorageSupply::nr_supplies(const Game&, const Request& req) const
{
	// out count_worker method create a worker instance, so we need non-const game
	Game& non_const_game = ref_cast<Game, Editor_Game_Base>
		(m_handler.get_building().get_owner()->egbase());
	if (req.get_type() == wwWORKER)
		return
			m_handler.count_workers
				(non_const_game, req.get_index(), req.get_requirements());

	//  Calculate how many wares can be sent out - it might be that we need them
	// ourselves. E.g. for hiring new soldiers.
	int32_t const x = m_wares.stock(req.get_index());
	// only mark an item of that type as available, if the priority of the
	// request + number of that wares in warehouse is > priority of request
	// of *this* warehouse + 1 (+1 is important, as else the ware would directly
	// be taken back to the warehouse as the request of the warehouse would be
	// highered and would have the same value as the original request)
	int32_t const y =
		x + (req.get_priority(0) / 100)
		- (m_handler.get_building().get_priority(wwWARE, req.get_index()) / 100) - 1;
	// But the number should never be higher than the number of wares available
	if (y > x)
		return x;
	return (x > 0) ? x : 0;
}

WareInstance& StorageSupply::launch_item(Game& game, const Request& req)
{
	if (req.get_type() != wwWARE)
		throw wexception("WarehouseSupply::launch_item: called for non-ware request");
	if (!m_wares.stock(req.get_index()))
		throw wexception("WarehouseSupply::launch_item: called for non-existing ware");

	return m_handler.launch_ware(game, req.get_index());
}

Worker& StorageSupply::launch_worker(Game& game, const Request& req)
{
	return m_handler.launch_worker(game, req.get_index(), req.get_requirements());
}

}
