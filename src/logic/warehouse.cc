/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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

#include "logic/warehouse.h"

#include <algorithm>

#include <boost/format.hpp>

#include "base/log.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/fleet.h"
#include "economy/portdock.h"
#include "economy/request.h"
#include "economy/ware_instance.h"
#include "economy/warehousesupply.h"
#include "economy/wares_queue.h"
#include "logic/battle.h"
#include "logic/carrier.h"
#include "logic/editor_game_base.h"
#include "logic/expedition_bootstrap.h"
#include "logic/findbob.h"
#include "logic/findnode.h"
#include "logic/game.h"
#include "logic/message_queue.h"
#include "logic/player.h"
#include "logic/requirements.h"
#include "logic/soldier.h"
#include "logic/tribe.h"
#include "logic/worker.h"
#include "profile/profile.h"

namespace Widelands {

namespace  {

static const uint32_t WORKER_WITHOUT_COST_SPAWN_INTERVAL = 2500;

// Goes through the list and removes all workers that are no longer in the
// game.
void remove_no_longer_existing_workers(Game& game, std::vector<Worker*>* workers) {
	for (std::vector<Worker*>::iterator i = workers->begin(); i != workers->end(); ++i) {
		if (!game.objects().object_still_available(*i)) {
			workers->erase(i);
			remove_no_longer_existing_workers(game, workers);
			return;
		}
	}
}

}  // namespace

WarehouseSupply::~WarehouseSupply()
{
	if (m_economy) {
		log
			("WarehouseSupply::~WarehouseSupply: Warehouse %u still belongs to "
			 "an economy",
			 m_warehouse->serial());
		set_economy(nullptr);
	}

	// We're removed from the Economy. Therefore, the wares can simply
	// be cleared out. The global inventory will be okay.
	m_wares  .clear();
	m_workers.clear();
}

/// Inform this supply, how much wares are to be handled
void WarehouseSupply::set_nrwares(WareIndex const i) {
	assert(0 == m_wares.get_nrwareids());

	m_wares.set_nrwares(i);
}
void WarehouseSupply::set_nrworkers(WareIndex const i) {
	assert(0 == m_workers.get_nrwareids());

	m_workers.set_nrwares(i);
}


/// Add and remove our wares and the Supply to the economies as necessary.
void WarehouseSupply::set_economy(Economy * const e)
{
	if (e == m_economy)
		return;

	if (m_economy) {
		m_economy->remove_supply(*this);
		for (WareIndex i = 0; i < m_wares.get_nrwareids(); ++i)
			if (m_wares.stock(i))
				m_economy->remove_wares(i, m_wares.stock(i));
		for (WareIndex i = 0; i < m_workers.get_nrwareids(); ++i)
			if (m_workers.stock(i))
				m_economy->remove_workers(i, m_workers.stock(i));
	}

	m_economy = e;

	if (m_economy) {
		for (WareIndex i = 0; i < m_wares.get_nrwareids(); ++i)
			if (m_wares.stock(i))
				m_economy->add_wares(i, m_wares.stock(i));
		for (WareIndex i = 0; i < m_workers.get_nrwareids(); ++i)
			if (m_workers.stock(i))
				m_economy->add_workers(i, m_workers.stock(i));
		m_economy->add_supply(*this);
	}
}


/// Add wares and update the economy.
void WarehouseSupply::add_wares(WareIndex const id, uint32_t const count)
{
	if (!count)
		return;

	if (m_economy) // No economies in the editor
		m_economy->add_wares(id, count);
	m_wares.add(id, count);
}


/// Remove wares and update the economy.
void WarehouseSupply::remove_wares(WareIndex const id, uint32_t const count)
{
	if (!count)
		return;

	m_wares.remove(id, count);
	if (m_economy) // No economies in the editor
		m_economy->remove_wares(id, count);
}


/// Add workers and update the economy.
void WarehouseSupply::add_workers(WareIndex const id, uint32_t const count)
{
	if (!count)
		return;

	if (m_economy) // No economies in the editor
		m_economy->add_workers(id, count);
	m_workers.add(id, count);
}


/**
 * Remove workers and update the economy.
 * Comments see add_workers
 */
void WarehouseSupply::remove_workers(WareIndex const id, uint32_t const count)
{
	if (!count)
		return;

	m_workers.remove(id, count);
	if (m_economy) // No economies in the editor
		m_economy->remove_workers(id, count);
}

/// Return the position of the Supply, i.e. the owning Warehouse.
PlayerImmovable * WarehouseSupply::get_position(Game &) {return m_warehouse;}


/// Warehouse supplies are never active.
bool WarehouseSupply::is_active() const {return false;}

bool WarehouseSupply::has_storage() const
{
	return true;
}

void WarehouseSupply::get_ware_type(WareWorker & /* type */, WareIndex & /* ware */) const
{
	throw wexception
		("WarehouseSupply::get_ware_type: calling this is nonsensical");
}

void WarehouseSupply::send_to_storage(Game &, Warehouse * /* wh */)
{
	throw wexception("WarehouseSupply::send_to_storage: should never be called");
}

uint32_t WarehouseSupply::nr_supplies
	(const Game & game, const Request & req) const
{
	if (req.get_type() == wwWORKER)
		return
			m_warehouse->count_workers
				(game, req.get_index(), req.get_requirements());

	//  Calculate how many wares can be sent out - it might be that we need them
	// ourselves. E.g. for hiring new soldiers.
	int32_t const x = m_wares.stock(req.get_index());
	// only mark an ware of that type as available, if the priority of the
	// request + number of that wares in warehouse is > priority of request
	// of *this* warehouse + 1 (+1 is important, as else the ware would directly
	// be taken back to the warehouse as the request of the warehouse would be
	// highered and would have the same value as the original request)
	int32_t const y =
		x + (req.get_priority(0) / 100)
		- (m_warehouse->get_priority(wwWARE, req.get_index()) / 100) - 1;
	// But the number should never be higher than the number of wares available
	if (y > x)
		return x;
	return (x > 0) ? x : 0;
}


/// Launch a ware.
WareInstance & WarehouseSupply::launch_ware(Game & game, const Request & req) {
	if (req.get_type() != wwWARE)
		throw wexception("WarehouseSupply::launch_ware: called for non-ware request");
	if (!m_wares.stock(req.get_index()))
		throw wexception("WarehouseSupply::launch_ware: called for non-existing ware");

	return m_warehouse->launch_ware(game, req.get_index());
}

/// Launch a ware as worker.
Worker & WarehouseSupply::launch_worker(Game & game, const Request & req)
{
	return
		m_warehouse->launch_worker
			(game, req.get_index(), req.get_requirements());
}


/*
==============================
Warehouse Building
==============================
*/


/// Warehouse Descr
WarehouseDescr::WarehouseDescr
	(char const* const _name, char const* const _descname,
	 const std::string& directory, Profile& prof, Section& global_s, const TribeDescr& _tribe)
	: BuildingDescr(MapObjectType::WAREHOUSE, _name, _descname, directory, prof, global_s, _tribe),
	  m_conquers         (0),
	  m_heal_per_second  (0)
{
	m_heal_per_second = global_s.get_safe_int("heal_per_second");
	if ((m_conquers = prof.get_safe_section("global").get_positive("conquers", 0)))
		m_workarea_info[m_conquers].insert(descname() + " conquer");
}

/*
==============================
IMPLEMENTATION
==============================
*/

Warehouse::Warehouse(const WarehouseDescr & warehouse_descr) :
	Building(warehouse_descr),
	m_supply(new WarehouseSupply(this)),
	m_next_military_act(0),
	m_portdock(nullptr)
{
	uint8_t nr_worker_types_without_cost =
		warehouse_descr.tribe().worker_types_without_cost().size();
	m_next_worker_without_cost_spawn =
		new uint32_t[nr_worker_types_without_cost];
	for (int i = 0; i < nr_worker_types_without_cost; ++i) {
		m_next_worker_without_cost_spawn[i] = never();
	}
	m_next_stock_remove_act = 0;
	m_cleanup_in_progress = false;
}


Warehouse::~Warehouse()
{
	delete m_supply;
	delete[] m_next_worker_without_cost_spawn;
}

/**
 * Try to bring the given \ref PlannedWorkers up to date with our game data.
 * Return \c false if \p pw cannot be salvaged.
 */
bool Warehouse::_load_finish_planned_worker(PlannedWorkers & pw)
{
	if (pw.index == INVALID_INDEX || !(pw.index < m_supply->get_workers().get_nrwareids()))
		return false;

	const WorkerDescr * w_desc = descr().tribe().get_worker_descr(pw.index);
	if (!w_desc || !w_desc->is_buildable())
		return false;

	const WorkerDescr::Buildcost & cost = w_desc->buildcost();
	uint32_t idx = 0;

	for
		(WorkerDescr::Buildcost::const_iterator cost_it = cost.begin();
		 cost_it != cost.end(); ++cost_it, ++idx)
	{
		WareWorker type;
		WareIndex ware;
		ware = descr().tribe().ware_index(cost_it->first);
		if (ware != INVALID_INDEX)
			type = wwWARE;
		else if ((ware = descr().tribe().worker_index(cost_it->first)) != INVALID_INDEX)
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
			 new Request(*this, ware, &Warehouse::request_cb, type));
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

void Warehouse::load_finish(EditorGameBase & egbase) {
	Building::load_finish(egbase);

	uint32_t next_spawn = never();
	const std::vector<WareIndex> & worker_types_without_cost =
		descr().tribe().worker_types_without_cost();
	for (uint8_t i = worker_types_without_cost.size(); i;) {
		WareIndex const worker_index = worker_types_without_cost.at(--i);
		if
			(owner().is_worker_type_allowed(worker_index) &&
			 m_next_worker_without_cost_spawn[i] == static_cast<uint32_t>(never()))
		{
			if (next_spawn == static_cast<uint32_t>(never())) {
				next_spawn = schedule_act(dynamic_cast<Game&>(egbase), WORKER_WITHOUT_COST_SPAWN_INTERVAL);
			}
			m_next_worker_without_cost_spawn[i] = next_spawn;
			log
				("WARNING: player %u is allowed to create worker type %s but his "
				 "%s %u at (%i, %i) does not have a next_spawn time set for that "
				 "worker type; setting it to %u\n",
				 owner().player_number(),
				 descr().tribe().get_worker_descr(worker_index)->descname().c_str(),
				 descr().descname().c_str(), serial(), get_position().x, get_position().y,
				 next_spawn);
		}
	}

	// Ensure consistency of PlannedWorker requests
	{
		uint32_t pwidx = 0;
		while (pwidx < m_planned_workers.size()) {
			if (!_load_finish_planned_worker(m_planned_workers[pwidx])) {
				m_planned_workers[pwidx].cleanup();
				m_planned_workers.erase(m_planned_workers.begin() + pwidx);
			} else {
				pwidx++;
			}
		}
	}
}

void Warehouse::init(EditorGameBase & egbase)
{
	Building::init(egbase);

	WareIndex const nr_wares   = descr().tribe().get_nrwares  ();
	WareIndex const nr_workers = descr().tribe().get_nrworkers();
	m_supply->set_nrwares  (nr_wares);
	m_supply->set_nrworkers(nr_workers);

	m_ware_policy.resize(nr_wares, SP_Normal);
	m_worker_policy.resize(nr_workers, SP_Normal);

	// Even though technically, a warehouse might be completely empty,
	// we let warehouse see always for simplicity's sake (since there's
	// almost always going to be a carrier inside, that shouldn't hurt).
	Player & player = owner();
	if (upcast(Game, game, &egbase)) {
		player.see_area
			(Area<FCoords>
			 (egbase.map().get_fcoords(get_position()), descr().vision_range()));

		{
			uint32_t const act_time = schedule_act
					(*game, WORKER_WITHOUT_COST_SPAWN_INTERVAL);
			const std::vector<WareIndex> & worker_types_without_cost =
				descr().tribe().worker_types_without_cost();

			for (size_t i = 0; i < worker_types_without_cost.size(); ++i) {
				if (owner().is_worker_type_allowed(worker_types_without_cost.at(i))) {
					m_next_worker_without_cost_spawn[i] = act_time;
				}
			}
		}
		// m_next_military_act is not touched in the loading code. Is only needed
		// if there warehous is created in the game?  I assume it's for the
		// conquer_radius thing
		m_next_military_act = schedule_act(*game, 1000);

		m_next_stock_remove_act = schedule_act(*game, 4000);

		log("Message: adding (wh) (%s) %i \n", to_string(descr().type()).c_str(), player.player_number());

		if (descr().name() == "port") {
			send_message
				(*game,
				 Message::Type::kSeafaring,
				 descr().descname(),
				 _("A new port was added to your economy."),
				 true);
		} else if (descr().name() == "headquarters") {
			send_message
				(*game,
				 Message::Type::kEconomy,
				 descr().descname(),
				 _("A new headquarters was added to your economy."),
				 true);
		} else {
			send_message
				(*game,
				 Message::Type::kEconomy,
				 descr().descname(),
				 _("A new warehouse was added to your economy."),
				 true);
		}
	}

	if (uint32_t const conquer_radius = descr().get_conquers())
		egbase.conquer_area
			(PlayerArea<Area<FCoords> >
			 	(player.player_number(),
			 	 Area<FCoords>
			 	 	(egbase.map().get_fcoords(get_position()), conquer_radius)));

	if (descr().get_isport()) {
		init_portdock(egbase);
		PortDock* pd = m_portdock;
		// should help diagnose problems with marine
		if (!pd->get_fleet()) {
			log(" Warning: portdock without a fleet created (%3dx%3d)\n",
			get_position().x,
			get_position().y);
		}
	}

	//this is default
	m_cleanup_in_progress = false;

}

/**
 * Find a contiguous set of water fields close to the port for docking
 * and initialize the @ref PortDock instance.
 */
void Warehouse::init_portdock(EditorGameBase & egbase)
{
	molog("Setting up port dock fields\n");

	Map & map = egbase.map();
	std::vector<Coords> dock = map.find_portdock(get_position());
	if (dock.empty()) {
		log("Attempting to setup port without neighboring water (coords: %3dx%3d).\n",
		    get_position().x,
		    get_position().y);
		return;
	}

	molog("Found %" PRIuS " fields for the dock\n", dock.size());

	m_portdock = new PortDock(this);
	m_portdock->set_owner(get_owner());
	m_portdock->set_economy(get_economy());
	for (const Coords& coords : dock) {
		m_portdock->add_position(coords);
	}
	m_portdock->init(egbase);

	if (get_economy() != nullptr)
		m_portdock->set_economy(get_economy());

	// this is just to indicate something wrong is going on
	//(tiborb)
	PortDock* pd_tmp = m_portdock;
	if (!pd_tmp->get_fleet()) {
		log (" portdock for port at %3dx%3d created but without a fleet!\n",
		    get_position().x,
		    get_position().y);
	}

}

void Warehouse::destroy(EditorGameBase & egbase)
{
	Building::destroy(egbase);
}

// if the port still exists and we are in game we first try to restore the portdock
void Warehouse::restore_portdock_or_destroy(EditorGameBase& egbase) {
	Warehouse::init_portdock(egbase);
	if (!m_portdock) {
		log(" Portdock could not be restored, removing the port now (coords: %3dx%3d)\n",
		    get_position().x,
		    get_position().y);
		Building::destroy(egbase);
	} else {
		molog ("Message: portdock restored\n");
		PortDock* pd_tmp = m_portdock;
		if (!pd_tmp->get_fleet()) {
			log (" Portdock restored but without a fleet!\n");
		}
	}
}


/// Destroy the warehouse.
void Warehouse::cleanup(EditorGameBase& egbase) {

	// if this is a port, it will remove also portdock.
	// But portdock must know that it should not try to recreate itself
	m_cleanup_in_progress = true;

	if (egbase.objects().object_still_available(m_portdock)) {
		m_portdock->remove(egbase);
	}

	if (!egbase.objects().object_still_available(m_portdock)) {
		m_portdock = nullptr;
	}

	// This will empty the stock and launch all workers including incorporated
	// ones.
	if (upcast(Game, game, &egbase)) {
		const WareList& workers = get_workers();
		for (WareIndex id = 0; id < workers.get_nrwareids(); ++id) {
			const uint32_t stock = workers.stock(id);
			for (uint32_t i = 0; i < stock; ++i) {
				launch_worker(*game, id, Requirements()).start_task_leavebuilding(*game, true);
			}
			assert(!m_incorporated_workers.count(id) || m_incorporated_workers[id].empty());
		}
	}
	m_incorporated_workers.clear();

	while (!m_planned_workers.empty()) {
		m_planned_workers.back().cleanup();
		m_planned_workers.pop_back();
	}

	Map& map = egbase.map();
	if (const uint32_t conquer_radius = descr().get_conquers())
		egbase.unconquer_area
			(PlayerArea<Area<FCoords> >
			 	(owner().player_number(),
			 	 Area<FCoords>(map.get_fcoords(get_position()), conquer_radius)),
			 m_defeating_player);

	// Unsee the area that we started seeing in init()
	Player & player = owner();
	player.unsee_area
			(Area<FCoords>(map.get_fcoords(get_position()), descr().vision_range()));

	Building::cleanup(egbase);
}


/// Act regularly to create workers of buildable types without cost. According
/// to intelligence, this is some highly advanced technology. Not only do the
/// settlers have no problems with birth control, they do not even need anybody
/// to procreate. They must have built-in DNA samples in those warehouses. And
/// what the hell are they doing, killing useless tribesmen! The Borg? Or just
/// like Soylent Green? Or maybe I should just stop writing comments that late
/// at night ;-)
void Warehouse::act(Game & game, uint32_t const data)
{
	uint32_t const gametime = game.get_gametime();
	{
		const std::vector<WareIndex> & worker_types_without_cost =
			owner().tribe().worker_types_without_cost();
		for (size_t i = worker_types_without_cost.size(); i;)
			if (m_next_worker_without_cost_spawn[--i] <= gametime) {
				WareIndex const id = worker_types_without_cost.at(i);
				if (owner().is_worker_type_allowed(id)) {
					int32_t const stock = m_supply->stock_workers(id);
					int32_t tdelta = WORKER_WITHOUT_COST_SPAWN_INTERVAL;

					if (stock < 100) {
						tdelta -= 4 * (100 - stock);
						insert_workers(id, 1);
					} else if (stock > 100) {
						tdelta -= 4 * (stock - 100);
						if (tdelta < 10)
							tdelta = 10;
						remove_workers(id, 1);
					}

					m_next_worker_without_cost_spawn[i] =
						schedule_act(game, tdelta);
				} else
					m_next_worker_without_cost_spawn[i] = never();
			}
	}

	//  Military stuff: Kill the soldiers that are dead.
	if (m_next_military_act <= gametime) {
		WareIndex const ware = descr().tribe().safe_worker_index("soldier");

		if (m_incorporated_workers.count(ware)) {
			WorkerList & soldiers = m_incorporated_workers[ware];

			uint32_t total_heal = descr().get_heal_per_second();
			// Using an explicit iterator, as we plan to erase some
			// of those guys
			for
				(WorkerList::iterator it = soldiers.begin();
				 it != soldiers.end();
				 ++it)
			{
				// This is a safe cast: we know only soldiers can land in this
				// slot in the incorporated array
				Soldier * soldier = static_cast<Soldier *>(*it);

				//  Soldier dead ...
				if (!soldier || soldier->get_current_hitpoints() == 0) {
					it = soldiers.erase(it);
					m_supply->remove_workers(ware, 1);
					continue;
				}

				if (soldier->get_current_hitpoints() < soldier->get_max_hitpoints()) {
					soldier->heal(total_heal);
					continue;
				}

			}
		}
		m_next_military_act = schedule_act(game, 1000);
	}

	if (static_cast<int32_t>(m_next_stock_remove_act - gametime) <= 0) {
		check_remove_stock(game);

		m_next_stock_remove_act = schedule_act(game, 4000);
	}

	// Update planned workers; this is to update the request amounts and
	// check because whether we suddenly can produce a requested worker. This
	// is mostly previously available wares may become unavailable due to
	// secondary requests.
	_update_all_planned_workers(game);

	Building::act(game, data);
}


/// Transfer our registration to the new economy.
void Warehouse::set_economy(Economy * const e)
{
	Economy * const old = get_economy();

	if (old == e)
		return;

	if (old)
		old->remove_warehouse(*this);

	if (m_portdock)
		m_portdock->set_economy(e);
	m_supply->set_economy(e);
	Building::set_economy(e);

	for (const PlannedWorkers& pw : m_planned_workers) {
		for (Request * req : pw.requests) {
			req->set_economy(e);
		}
	}

	if (m_portdock)
		m_portdock->set_economy(e);

	if (e)
		e->add_warehouse(*this);
}


const WareList & Warehouse::get_wares() const
{
	return m_supply->get_wares();
}


const WareList & Warehouse::get_workers() const
{
	return m_supply->get_workers();
}

PlayerImmovable::Workers Warehouse::get_incorporated_workers()
{
	PlayerImmovable::Workers all_workers;

	for (const std::pair<WareIndex, WorkerList>& worker_pair : m_incorporated_workers) {
		for (Worker * worker : worker_pair.second) {
			all_workers.push_back(worker);
		}
	}
	return all_workers;
}


/// Magically create wares in this warehouse. Updates the economy accordingly.
void Warehouse::insert_wares(WareIndex const id, uint32_t const count)
{
	m_supply->add_wares(id, count);
}


/// Magically destroy wares.
void Warehouse::remove_wares(WareIndex const id, uint32_t const count)
{
	m_supply->remove_wares(id, count);
}


/// Magically create workers in this warehouse. Updates the economy accordingly.
void Warehouse::insert_workers(WareIndex const id, uint32_t const count)
{
	m_supply->add_workers(id, count);
}


/// Magically destroy workers.
void Warehouse::remove_workers(WareIndex const id, uint32_t const count)
{
	m_supply->remove_workers(id, count);
}



/// Launch a carrier to fetch an ware from our flag.
bool Warehouse::fetch_from_flag(Game & game)
{
	WareIndex const carrierid = descr().tribe().safe_worker_index("carrier");

	if (!m_supply->stock_workers(carrierid)) // XXX yep, let's cheat
		insert_workers(carrierid, 1);

	launch_worker(game, carrierid, Requirements()).start_task_fetchfromflag
		(game);

	return true;
}


/**
 * \return the number of workers that we can launch satisfying the given
 * requirements.
 */
uint32_t Warehouse::count_workers
	(const Game & /* game */, WareIndex ware, const Requirements & req)
{
	uint32_t sum = 0;

	do {
		sum += m_supply->stock_workers(ware);

		// NOTE: This code lies about the TrainingAttributes of non-instantiated workers.
		if (m_incorporated_workers.count(ware)) {
			for (Worker * worker : m_incorporated_workers[ware]) {
				if (!req.check(*worker)) {
					//  This is one of the workers in our sum.
					//  But he is too stupid for this job
					--sum;
				}
			}
		}

		ware = descr().tribe().get_worker_descr(ware)->becomes();
	} while (ware != INVALID_INDEX);

	return sum;
}

/// Start a worker of a given type. The worker will
/// be assigned a job by the caller.
Worker & Warehouse::launch_worker
	(Game & game, WareIndex ware, const Requirements & req)
{
	do {
		if (m_supply->stock_workers(ware)) {
			uint32_t unincorporated = m_supply->stock_workers(ware);

			//  look if we got one of those in stock
			if (m_incorporated_workers.count(ware)) {
				// On cleanup, it could be that the worker was deleted under
				// us, so we erase the pointer we had to it and create a new
				// one.
				remove_no_longer_existing_workers(game, &m_incorporated_workers[ware]);
				WorkerList& incorporated_workers = m_incorporated_workers[ware];

				for (std::vector<Worker *>::iterator worker_iter = incorporated_workers.begin();
					 worker_iter != incorporated_workers.end(); ++worker_iter)
				{
					Worker* worker = *worker_iter;
					--unincorporated;

					if (req.check(*worker)) {
						worker->reset_tasks(game);  //  forget everything you did
						worker->set_location(this); //  back in a economy
						incorporated_workers.erase(worker_iter);

						m_supply->remove_workers(ware, 1);
						return *worker;
					}
				}
			}

			assert(unincorporated <= m_supply->stock_workers(ware));

			if (unincorporated) {
				// Create a new one
				// NOTE: This code lies about the TrainingAttributes of the new worker
				m_supply->remove_workers(ware, 1);
				const WorkerDescr & workerdescr = *descr().tribe().get_worker_descr(ware);
				return workerdescr.create(game, owner(), this, m_position);
			}
		}

		if (can_create_worker(game, ware)) {
			// don't want to use an upgraded worker, so create new one.
			create_worker(game, ware);
		} else {
			ware = descr().tribe().get_worker_descr(ware)->becomes();
		}
	} while (ware != INVALID_INDEX);

	throw wexception
		("Warehouse::launch_worker: worker does not actually exist");
}


void Warehouse::incorporate_worker(EditorGameBase & egbase, Worker* w)
{
	assert(w != nullptr);
	assert(w->get_owner() == &owner());

	if (WareInstance* ware = w->fetch_carried_ware(egbase))
		incorporate_ware(egbase, ware);

	WareIndex worker_index = descr().tribe().worker_index(w->descr().name().c_str());

	m_supply->add_workers(worker_index, 1);

	//  We remove carriers, but we keep other workers around.
	//  TODO(unknown): Remove all workers that do not have properties such as experience.
	//  And even such workers should be removed and only a small record
	//  with the experience (and possibly other data that must survive)
	//  may be kept.
	//  When this is done, the get_incorporated_workers method above must
	//  be reworked so that workers are recreated, and rescheduled for
	//  incorporation.
	if (upcast(Carrier, carrier, w)) {
		carrier->remove(egbase);
		return;
	}

	// Incorporate the worker
	if (!m_incorporated_workers.count(worker_index))
		m_incorporated_workers[worker_index] = std::vector<Worker *>();
	m_incorporated_workers[worker_index].push_back(w);

	w->set_location(nullptr); //  no longer in an economy

	if (upcast(Game, game, &egbase)) {
		//  Bind the worker into this house, hide him on the map.
		w->reset_tasks(*game);
		w->start_task_idle(*game, 0, -1);
	}
}

/// Create an instance of a ware and make sure it gets
/// carried out of the warehouse.
WareInstance & Warehouse::launch_ware(Game & game, WareIndex const ware_index) {
	// Create the ware
	WareInstance & ware = *new WareInstance(ware_index, descr().tribe().get_ware_descr(ware_index));
	ware.init(game);
	do_launch_ware(game, ware);

	m_supply->remove_wares(ware_index, 1);

	return ware;
}


/// Get a carrier to actually move this ware out of the warehouse.
void Warehouse::do_launch_ware(Game & game, WareInstance & ware)
{
	// Create a carrier
	WareIndex const carrierid = descr().tribe().worker_index("carrier");
	const WorkerDescr & workerdescr = *descr().tribe().get_worker_descr(carrierid);

	Worker & worker = workerdescr.create(game, owner(), this, m_position);

	// Yup, this is cheating.
	if (m_supply->stock_workers(carrierid))
		m_supply->remove_workers(carrierid, 1);

	// Setup the carrier
	worker.start_task_dropoff(game, ware);
}


void Warehouse::incorporate_ware(EditorGameBase & egbase, WareInstance* ware)
{
	m_supply->add_wares(ware->descr_index(), 1);
	ware->destroy(egbase);
}


/// Called when a transfer for one of the idle Requests completes.
void Warehouse::request_cb
	(Game            &       game,
	 Request         &,
	 WareIndex        const ware,
	 Worker          * const w,
	 PlayerImmovable &       target)
{
	Warehouse & wh = dynamic_cast<Warehouse&>(target);

	if (w) {
		w->schedule_incorporate(game);
	} else {
		wh.m_supply->add_wares(ware, 1);

		// This ware may be used to build planned workers,
		// so it seems like a good idea to update the associated requests
		// and use the ware before it is sent away again.
		wh._update_all_planned_workers(game);
	}
}

/**
 * Receive a ware from a transfer that was not associated to a \ref Request.
 */
void Warehouse::receive_ware(Game & /* game */, WareIndex ware)
{
	m_supply->add_wares(ware, 1);
}

/**
 * Receive a worker from a transfer that was not associated to a \ref Request.
 */
void Warehouse::receive_worker(Game & game, Worker & worker)
{
	worker.schedule_incorporate(game);
}

Building & WarehouseDescr::create_object() const {
	return *new Warehouse(*this);
}


bool Warehouse::can_create_worker(Game &, WareIndex const worker) const {
	if (!(worker < m_supply->get_workers().get_nrwareids()))
		throw wexception
			("worker type %d does not exists (max is %d)",
			 worker, m_supply->get_workers().get_nrwareids());

	const WorkerDescr & w_desc = *descr().tribe().get_worker_descr(worker);
	assert(&w_desc);
	if (!w_desc.is_buildable())
		return false;

	//  see if we have the resources
	for (const std::pair<std::string, uint8_t>& buildcost : w_desc.buildcost()) {
		const std::string & input_name = buildcost.first;
		WareIndex id_w = descr().tribe().ware_index(input_name);
		if (id_w != INVALID_INDEX) {
			if (m_supply->stock_wares(id_w) < buildcost.second)
				return false;
		} else if ((id_w = descr().tribe().worker_index(input_name)) != INVALID_INDEX) {
			if (m_supply->stock_workers(id_w) < buildcost.second)
				return false;
		} else
			throw wexception
				("worker type %s needs \"%s\" to be built but that is neither "
				 "a ware type nor a worker type defined in the tribe %s",
				 w_desc.descname().c_str(), input_name.c_str(),
				 descr().tribe().name().c_str());
	}
	return true;
}


void Warehouse::create_worker(Game & game, WareIndex const worker) {
	assert(can_create_worker (game, worker));

	const WorkerDescr & w_desc = *descr().tribe().get_worker_descr(worker);

	for (const std::pair<std::string, uint8_t>& buildcost : w_desc.buildcost()) {
		const std::string & input = buildcost.first;
		WareIndex const id_ware = descr().tribe().ware_index(input);
		if (id_ware != INVALID_INDEX) {
			remove_wares  (id_ware,                        buildcost.second);
			//update statistic accordingly
			owner().ware_consumed(id_ware, buildcost.second);
		} else
			remove_workers(descr().tribe().safe_worker_index(input), buildcost.second);
	}

	incorporate_worker(game, &w_desc.create(game, owner(), this, m_position));

	// Update PlannedWorkers::amount here if appropriate, because this function
	// may have been called directly by the Economy.
	// Do not update anything else about PlannedWorkers here, because this
	// function is called by _update_planned_workers, so avoid recursion
	for (PlannedWorkers& planned_worker : m_planned_workers) {
		if (planned_worker.index == worker && planned_worker.amount)
			planned_worker.amount--;
	}
}

/**
 * Return the number of workers of the given type that we plan to
 * create in this warehouse.
 */
uint32_t Warehouse::get_planned_workers(Game & /* game */, WareIndex index) const
{
	for (const PlannedWorkers& pw : m_planned_workers) {
		if (pw.index == index)
			return pw.amount;
	}
	return 0;
}

/**
 * Calculate the supply of wares available to this warehouse in each of the
 * buildcost wares for the given worker.
 *
 * This is the current stock plus any incoming transfers.
 */
std::vector<uint32_t> Warehouse::calc_available_for_worker
	(Game & /* game */, WareIndex index) const
{
	const WorkerDescr & w_desc = *descr().tribe().get_worker_descr(index);
	std::vector<uint32_t> available;

	for (const std::pair<std::string, uint8_t>& buildcost : w_desc.buildcost()) {
		const std::string & input_name = buildcost.first;
		WareIndex id_w = descr().tribe().ware_index(input_name);
		if (id_w != INVALID_INDEX) {
			available.push_back(get_wares().stock(id_w));
		} else if ((id_w = descr().tribe().worker_index(input_name)) != INVALID_INDEX) {
			available.push_back(get_workers().stock(id_w));
		} else
			throw wexception
				("Economy::_create_requested_worker: buildcost inconsistency '%s'",
				 input_name.c_str());
	}

	for (const PlannedWorkers& pw : m_planned_workers) {
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
void Warehouse::plan_workers(Game & game, WareIndex index, uint32_t amount)
{
	PlannedWorkers * pw = nullptr;

	for (PlannedWorkers& planned_worker : m_planned_workers) {
		if (planned_worker.index == index) {
			pw = &planned_worker;
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

		const WorkerDescr & w_desc = *descr().tribe().get_worker_descr(pw->index);
		for (const std::pair<std::string, uint8_t>& buildcost : w_desc.buildcost()) {
			const std::string & input_name = buildcost.first;

			WareIndex id_w = descr().tribe().ware_index(input_name);
			if (id_w != INVALID_INDEX) {
				pw->requests.push_back
					(new Request
					 (*this, id_w, &Warehouse::request_cb, wwWARE));
			} else if ((id_w = descr().tribe().worker_index(input_name)) != INVALID_INDEX) {
				pw->requests.push_back
					(new Request
					 (*this, id_w, &Warehouse::request_cb, wwWORKER));
			} else
				throw wexception
					("plan_workers: bad buildcost '%s'", input_name.c_str());
		}
	}

	pw->amount = amount;
	_update_planned_workers(game, *pw);
}

/**
 * See if we can create the workers of the given plan,
 * and update requests accordingly.
 */
void Warehouse::_update_planned_workers
	(Game & game, Warehouse::PlannedWorkers & pw)
{
	const WorkerDescr & w_desc = *descr().tribe().get_worker_descr(pw.index);

	while (pw.amount && can_create_worker(game, pw.index)) {
		create_worker(game, pw.index);
	}

	uint32_t idx = 0;
	for (const std::pair<std::string, uint8_t>& buildcost : w_desc.buildcost()) {

		const std::string & input_name = buildcost.first;
		uint32_t supply;

		WareIndex id_w = descr().tribe().ware_index(input_name);
		if (id_w != INVALID_INDEX) {
			supply = m_supply->stock_wares(id_w);
		} else if ((id_w = descr().tribe().worker_index(input_name)) != INVALID_INDEX) {
			supply = m_supply->stock_workers(id_w);
		} else
			throw wexception
				("_update_planned_workers: bad buildcost '%s'", input_name.c_str());

		if (supply >= pw.amount * buildcost.second)
			pw.requests[idx]->set_count(0);
		else
			pw.requests[idx]->set_count
				(pw.amount * buildcost.second - supply);
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
void Warehouse::_update_all_planned_workers(Game & game)
{
	uint32_t idx = 0;
	while (idx < m_planned_workers.size()) {
		_update_planned_workers(game, m_planned_workers[idx]);

		if (!m_planned_workers[idx].amount) {
			m_planned_workers[idx].cleanup();
			m_planned_workers.erase(m_planned_workers.begin() + idx);
		} else {
			idx++;
		}
	}
}

void Warehouse::enable_spawn
	(Game & game, uint8_t const worker_types_without_cost_index)
{
	assert
		(m_next_worker_without_cost_spawn[worker_types_without_cost_index]
		 ==
		 static_cast<uint32_t>(never()));
	m_next_worker_without_cost_spawn[worker_types_without_cost_index] =
		schedule_act(game, WORKER_WITHOUT_COST_SPAWN_INTERVAL);
}
void Warehouse::disable_spawn(uint8_t const worker_types_without_cost_index)
{
	assert
		(m_next_worker_without_cost_spawn[worker_types_without_cost_index]
		 !=
		 static_cast<uint32_t>(never()));
	m_next_worker_without_cost_spawn[worker_types_without_cost_index] = never();
}


bool Warehouse::can_attack()
{
	return descr().get_conquers() > 0;
}

void Warehouse::aggressor(Soldier & enemy)
{
	if (!descr().get_conquers())
		return;

	Game & game = dynamic_cast<Game&>(owner().egbase());
	Map  & map  = game.map();
	if
		(enemy.get_owner() == &owner() ||
		 enemy.get_battle() ||
		 descr().get_conquers()
		 <=
		 map.calc_distance(enemy.get_position(), get_position()))
		return;

	if
		(game.map().find_bobs
		 	(Area<FCoords>(map.get_fcoords(base_flag().get_position()), 2),
		 	 nullptr,
		 	 FindBobEnemySoldier(&owner())))
		return;

	WareIndex const soldier_index = descr().tribe().worker_index("soldier");
	Requirements noreq;

	if (!count_workers(game, soldier_index, noreq))
		return;

	Soldier & defender = dynamic_cast<Soldier&>(launch_worker(game, soldier_index, noreq));
	defender.start_task_defense(game, false);
}

bool Warehouse::attack(Soldier & enemy)
{
	Game & game = dynamic_cast<Game&>(owner().egbase());
	WareIndex const soldier_index = descr().tribe().worker_index("soldier");
	Requirements noreq;

	if (count_workers(game, soldier_index, noreq)) {
		Soldier & defender = dynamic_cast<Soldier&>(launch_worker(game, soldier_index, noreq));
		defender.start_task_defense(game, true);
		enemy.send_signal(game, "sleep");
		return true;
	}

	set_defeating_player(enemy.owner().player_number());
	schedule_destroy(game);
	return false;
}

void Warehouse::PlannedWorkers::cleanup()
{
	while (!requests.empty()) {
		delete requests.back();
		requests.pop_back();
	}
}

Warehouse::StockPolicy Warehouse::get_ware_policy(WareIndex ware) const
{
	assert(ware < m_ware_policy.size());
	return m_ware_policy[ware];
}

Warehouse::StockPolicy Warehouse::get_worker_policy(WareIndex ware) const
{
	assert(ware < m_worker_policy.size());
	return m_worker_policy[ware];
}

Warehouse::StockPolicy Warehouse::get_stock_policy
	(WareWorker waretype, WareIndex wareindex) const
{
	if (waretype == wwWORKER)
		return get_worker_policy(wareindex);
	else
		return get_ware_policy(wareindex);
}


void Warehouse::set_ware_policy(WareIndex ware, Warehouse::StockPolicy policy)
{
	assert(ware < m_ware_policy.size());
	m_ware_policy[ware] = policy;
}

void Warehouse::set_worker_policy
	(WareIndex ware, Warehouse::StockPolicy policy)
{
	assert(ware < m_worker_policy.size());
	m_worker_policy[ware] = policy;
}

/**
 * Check if there are remaining wares with stock policy \ref SP_Remove,
 * and remove one of them if appropriate.
 */
void Warehouse::check_remove_stock(Game & game)
{
	if (base_flag().current_wares() < base_flag().total_capacity() / 2) {
		for (WareIndex ware = 0; ware < m_ware_policy.size(); ++ware) {
			if (get_ware_policy(ware) != SP_Remove || !get_wares().stock(ware))
				continue;

			launch_ware(game, ware);
			break;
		}
	}

	for (WareIndex widx = 0; widx < m_worker_policy.size(); ++widx) {
		if (get_worker_policy(widx) != SP_Remove || !get_workers().stock(widx))
			continue;

		Worker & worker = launch_worker(game, widx, Requirements());
		worker.start_task_leavebuilding(game, true);
		break;
	}
}

WaresQueue& Warehouse::waresqueue(WareIndex index) {
	assert(m_portdock != nullptr);
	assert(m_portdock->expedition_bootstrap() != nullptr);

	return m_portdock->expedition_bootstrap()->waresqueue(index);
}

/*
 * SoldierControl implementations
 */
std::vector<Soldier *> Warehouse::present_soldiers() const
{
	std::vector<Soldier *> rv;

	WareIndex const ware = descr().tribe().safe_worker_index("soldier");
	IncorporatedWorkers::const_iterator sidx = m_incorporated_workers.find(ware);

	if (sidx != m_incorporated_workers.end()) {
		const WorkerList & soldiers = sidx->second;

		for (Worker * temp_soldier: soldiers) {
			rv.push_back(static_cast<Soldier *>(temp_soldier));
		}
	}

	return rv;
}
int Warehouse::incorporate_soldier(EditorGameBase & egbase, Soldier & soldier) {
	incorporate_worker(egbase, &soldier);
	return 0;
}

int Warehouse::outcorporate_soldier(EditorGameBase & /* egbase */, Soldier & soldier) {

	WareIndex const ware = descr().tribe().safe_worker_index("soldier");
	if (m_incorporated_workers.count(ware)) {
		WorkerList & soldiers = m_incorporated_workers[ware];

		WorkerList::iterator i = std::find
			(soldiers.begin(), soldiers.end(), &soldier);

		soldiers.erase(i);
		m_supply->remove_workers(ware, 1);
	}
#ifndef NDEBUG
	else
		throw wexception("outcorporate_soldier: soldier not in this warehouse!");
#endif

	return 0;
}

void Warehouse::log_general_info(const EditorGameBase & egbase)
{
	Building::log_general_info(egbase);

	if (descr().get_isport()){
		PortDock* pd_tmp = m_portdock;
		if (pd_tmp){
			molog("Port dock: %u\n", pd_tmp->serial());
			molog("port needs ship: %s\n", (pd_tmp->get_need_ship())?"true":"false");
			molog("wares and workers waiting: %u\n", pd_tmp->count_waiting());
			molog("exped. in progr.: %s\n", (pd_tmp->expedition_started())?"true":"false");
			Fleet* fleet = pd_tmp->get_fleet();
			if (fleet) {
				molog("* fleet: %u\n", fleet->serial());
				molog("  ships: %u, ports: %u\n", fleet->count_ships(), fleet->count_ports());
				molog("  m_act_pending: %s\n", (fleet->get_act_pending())?"true":"false");
			} else {
				molog("No fleet?!\n");
			}
		} else {
			molog ("No port dock!?\n");
		}
	}
}


}
