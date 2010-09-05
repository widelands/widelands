/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "warehouse.h"

#include "battle.h"
#include "carrier.h"
#include "worker.h"
#include "message_queue.h"

#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/request.h"
#include "economy/ware_instance.h"
#include "economy/warehousesupply.h"
#include "editor_game_base.h"
#include "game.h"
#include "log.h"
#include "player.h"
#include "profile/profile.h"
#include "requirements.h"
#include "soldier.h"
#include "tribe.h"
#include "upcast.h"
#include "wexception.h"
#include "container_iterate.h"
#include <algorithm>

namespace Widelands {

static const uint32_t WORKER_WITHOUT_COST_SPAWN_INTERVAL = 2500;

WarehouseSupply::~WarehouseSupply()
{
	if (m_economy) {
		log
			("WarehouseSupply::~WarehouseSupply: Warehouse %u still belongs to "
			 "an economy",
			 m_warehouse->serial());
		set_economy(0);
	}

	// We're removed from the Economy. Therefore, the wares can simply
	// be cleared out. The global inventory will be okay.
	m_wares  .clear();
	m_workers.clear();
}

/// Inform this supply, how much wares are to be handled
void WarehouseSupply::set_nrwares(Ware_Index const i) {
	assert(Ware_Index::First() == m_wares.get_nrwareids());

	m_wares.set_nrwares(i);
}
void WarehouseSupply::set_nrworkers(Ware_Index const i) {
	assert(Ware_Index::First() == m_workers.get_nrwareids());

	m_workers.set_nrwares(i);
}


/// Add and remove our wares and the Supply to the economies as necessary.
void WarehouseSupply::set_economy(Economy * const e)
{
	if (e == m_economy)
		return;

	if (m_economy) {
		m_economy->remove_supply(*this);
		for
			(Ware_Index i = Ware_Index::First(); i < m_wares.get_nrwareids(); ++i)
			if (m_wares.stock(i))
				m_economy->remove_wares(i, m_wares.stock(i));
		for
			(Ware_Index i = Ware_Index::First();
			 i < m_workers.get_nrwareids();
			 ++i)
			if (m_workers.stock(i))
				m_economy->remove_workers(i, m_workers.stock(i));
	}

	m_economy = e;

	if (m_economy) {
		for
			(Ware_Index i = Ware_Index::First(); i < m_wares.get_nrwareids(); ++i)
			if (m_wares.stock(i))
				m_economy->add_wares(i, m_wares.stock(i));
		for
			(Ware_Index i = Ware_Index::First();
			 i < m_workers.get_nrwareids();
			 ++i)
			if (m_workers.stock(i))
				m_economy->add_workers(i, m_workers.stock(i));
		m_economy->add_supply(*this);
	}
}


/// Add wares and update the economy.
void WarehouseSupply::add_wares     (Ware_Index const id, uint32_t const count)
{
	if (!count)
		return;

	if (m_economy) // No economies in the editor
		m_economy->add_wares(id, count);
	m_wares.add(id, count);
}


/// Remove wares and update the economy.
void WarehouseSupply::remove_wares  (Ware_Index const id, uint32_t const count)
{
	if (!count)
		return;

	m_wares.remove(id, count);
	if (m_economy) // No economies in the editor
		m_economy->remove_wares(id, count);
}


/// Add workers and update the economy.
void WarehouseSupply::add_workers   (Ware_Index const id, uint32_t const count)
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
void WarehouseSupply::remove_workers(Ware_Index const id, uint32_t const count)
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
bool WarehouseSupply::is_active() const throw () {return false;}

bool WarehouseSupply::has_storage() const throw ()
{
	return true;
}

void WarehouseSupply::get_ware_type(bool & isworker, Ware_Index & ware) const
{
	throw wexception
		("WarehouseSupply::get_ware_type: calling this is nonsensical");
}

void WarehouseSupply::send_to_storage(Game &, Warehouse * wh)
{
	throw wexception("WarehouseSupply::send_to_storage: should never be called");
}

uint32_t WarehouseSupply::nr_supplies
	(Game const & game, Request const & req) const
{
	if (req.get_type() == Request::WORKER)
		return
			m_warehouse->count_workers
				(game, req.get_index(), req.get_requirements());

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
		- (m_warehouse->get_priority(Request::WARE, req.get_index()) / 100) - 1;
	// But the number should never be higher than the number of wares available
	if (y > x)
		return x;
	return (x > 0) ? x : 0;
}


/// Launch a ware as item.
WareInstance & WarehouseSupply::launch_item(Game & game, Request const & req) {
	if (req.get_type() != Request::WARE)
		throw wexception
			("WarehouseSupply::launch_item: called for non-ware request");
	if (!m_wares.stock(req.get_index()))
		throw wexception
			("WarehouseSupply::launch_item: called for non-existing ware");

	return m_warehouse->launch_item(game, req.get_index());
}

/// Launch a ware as worker.
Worker & WarehouseSupply::launch_worker(Game & game, Request const & req)
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
Warehouse_Descr::Warehouse_Descr
	(char const * const _name, char const * const _descname,
	 std::string const & directory, Profile & prof, Section & global_s,
	 Tribe_Descr const & _tribe)
:
	Building_Descr(_name, _descname, directory, prof, global_s, _tribe),
m_conquers    (0)
{
	if
		((m_conquers =
		  	prof.get_safe_section("global").get_positive("conquers", 0)))
		m_workarea_info[m_conquers].insert(descname() + _(" conquer"));
}


/*
==============================
IMPLEMENTATION
==============================
*/

#define SET_WORKER_WITHOUT_COST_SPAWNS(nr, value)                             \
   for                                                                        \
      (wl_index_range<uint32_t *>                                             \
       i(                                                                     \
          m_next_worker_without_cost_spawn,                                   \
          m_next_worker_without_cost_spawn + nr                               \
		);                                                                      \
       i;                                                                     \
       ++i)                                                                   \
      *i.current = value;                                                     \

Warehouse::Warehouse(const Warehouse_Descr & warehouse_descr) :
	Building(warehouse_descr),
	m_supply(new WarehouseSupply(this))
{
	uint8_t nr_worker_types_without_cost =
		warehouse_descr.tribe().worker_types_without_cost().size();
	m_next_worker_without_cost_spawn =
		new uint32_t[nr_worker_types_without_cost];
	SET_WORKER_WITHOUT_COST_SPAWNS(nr_worker_types_without_cost, Never());

	m_next_stock_remove_act = 0;
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
	if (!pw.index || !(pw.index < m_supply->get_workers().get_nrwareids()))
		return false;

	const Worker_Descr * w_desc = tribe().get_worker_descr(pw.index);
	if (!w_desc || !w_desc->is_buildable())
		return false;

	const Worker_Descr::Buildcost & cost = w_desc->buildcost();
	uint32_t idx = 0;

	for
		(Worker_Descr::Buildcost::const_iterator cost_it = cost.begin();
		 cost_it != cost.end(); ++cost_it, ++idx)
	{
		Request::Type type;
		Ware_Index ware;

		if ((ware = tribe().ware_index(cost_it->first)))
			type = Request::WARE;
		else if ((ware = tribe().worker_index(cost_it->first)))
			type = Request::WORKER;
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

void Warehouse::load_finish(Editor_Game_Base & egbase) {
	Building::load_finish(egbase);

	uint32_t next_spawn = Never();
	std::vector<Ware_Index> const & worker_types_without_cost =
		tribe().worker_types_without_cost();
	for (uint8_t i = worker_types_without_cost.size(); i;) {
		Ware_Index const worker_index = worker_types_without_cost.at(--i);
		if
			(owner().is_worker_type_allowed(worker_index) and
			 m_next_worker_without_cost_spawn[i] == Never())
		{
			if (next_spawn == Never())
				next_spawn =
					schedule_act
						(ref_cast<Game, Editor_Game_Base>(egbase),
						 WORKER_WITHOUT_COST_SPAWN_INTERVAL);
			m_next_worker_without_cost_spawn[i] = next_spawn;
			log
				("WARNING: player %u is allowed to create worker type %s but his "
				 "%s %u at (%i, %i) does not have a next_spawn time set for that "
				 "worker type; setting it to %u\n",
				 owner().player_number(),
				 tribe().get_worker_descr(worker_index)->descname().c_str(),
				 descname().c_str(), serial(), get_position().x, get_position().y,
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

void Warehouse::init(Editor_Game_Base & egbase)
{
	Building::init(egbase);

	Ware_Index const nr_wares   = tribe().get_nrwares  ();
	Ware_Index const nr_workers = tribe().get_nrworkers();
	m_supply->set_nrwares  (nr_wares);
	m_supply->set_nrworkers(nr_workers);

	m_ware_policy.resize(nr_wares.value(), SP_Normal);
	m_worker_policy.resize(nr_workers.value(), SP_Normal);

	// Even though technically, a warehouse might be completely empty,
	// we let warehouse see always for simplicity's sake (since there's
	// almost always going to be a carrier inside, that shouldn't hurt).
	Player & player = owner();
	player.see_area
		(Area<FCoords>
		 (egbase.map().get_fcoords(get_position()), vision_range()));

	{
		uint32_t const act_time =
			schedule_act
				(ref_cast<Game, Editor_Game_Base>(egbase),
				 WORKER_WITHOUT_COST_SPAWN_INTERVAL);
		std::vector<Ware_Index> const & worker_types_without_cost =
			tribe().worker_types_without_cost();

		for
			(wl_index_range<uint32_t> i
			(0, worker_types_without_cost.size());
			i; ++i)
			if
				(owner().is_worker_type_allowed
				 	(worker_types_without_cost.at(i.current)))
				m_next_worker_without_cost_spawn[i.current] = act_time;
	}
	// m_next_military_act is not touched in the loading code. Is only needed if
	// there warehous is created in the game?
	// I assume it's for the conquer_radius thing
	m_next_military_act =
		schedule_act
			(ref_cast<Game, Editor_Game_Base>(egbase), 1000);

	m_next_stock_remove_act =
		schedule_act
			(ref_cast<Game, Editor_Game_Base>(egbase), 4000);

	if (uint32_t const conquer_radius = get_conquers())
		ref_cast<Game, Editor_Game_Base>(egbase).conquer_area
			(Player_Area<Area<FCoords> >
			 	(player.player_number(),
			 	 Area<FCoords>
			 	 	(egbase.map().get_fcoords(get_position()), conquer_radius)));

	log("Message: adding (wh) (%s) %i \n", type_name(), player.player_number());
	char message[2048];
	snprintf
		(message, sizeof(message),
		 _("A new %s was added to your economy."),
		 descname().c_str());
	send_message
		(ref_cast<Game, Editor_Game_Base>(egbase),
		 "warehouse",
		 descname(),
		 message);
}



/// Destroy the warehouse.
void Warehouse::cleanup(Editor_Game_Base & egbase)
{
	while (m_planned_workers.size()) {
		m_planned_workers.back().cleanup();
		m_planned_workers.pop_back();
	}

	//  all cached workers are unbound and freed
	while (m_incorporated_workers.size()) {
		//  If the game ends and this worker has been created before this
		//  warehouse, it might already be deleted. So do not try and free him
		if (upcast(Worker, worker, m_incorporated_workers.begin()->get(egbase))) {
			if (upcast(Game, game, &egbase))
				worker->reset_tasks(ref_cast<Game, Editor_Game_Base>(egbase));
		}
		m_incorporated_workers.erase(m_incorporated_workers.begin());
	}
	Map & map = egbase.map();
	if (const uint32_t conquer_radius = get_conquers())
		egbase.unconquer_area
			(Player_Area<Area<FCoords> >
			 	(owner().player_number(),
			 	 Area<FCoords>(map.get_fcoords(get_position()), conquer_radius)),
			 m_defeating_player);

	// Unsee the area that we started seeing in init()
	Player & player = owner();
	player.unsee_area
			(Area<FCoords>(map.get_fcoords(get_position()), vision_range()));

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
		std::vector<Ware_Index> const & worker_types_without_cost =
			owner().tribe().worker_types_without_cost();
		for (size_t i = worker_types_without_cost.size(); i;)
			if (m_next_worker_without_cost_spawn[--i] <= gametime) {
				Ware_Index const id = worker_types_without_cost.at(i);
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
					m_next_worker_without_cost_spawn[i] = Never();
			}
	}

	//  Military stuff: Kill the soldiers that are dead.
	if (m_next_military_act <= gametime) {
		Ware_Index const ware = tribe().safe_worker_index("soldier");
		Worker_Descr const & workerdescr = *tribe().get_worker_descr(ware);
		const std::string & workername = workerdescr.name();
		//  Look if we got one in stock of those.
		for
			(std::vector<OPtr<Worker> >::iterator it =
			 	m_incorporated_workers.begin();
			 it != m_incorporated_workers.end();
			 ++it)
		{
			Worker const & worker = *it->get(game);
			if (worker.name() == workername) {
				upcast(Soldier const, soldier, &worker);

				//  Soldier dead ...
				if (not soldier or soldier->get_current_hitpoints() == 0) {
					it = m_incorporated_workers.erase(it);
					m_supply->remove_workers(ware, 1);
					continue;
				}
				//  If warehouse can heal, this is the place to put it.
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

	m_supply->set_economy(e);
	Building::set_economy(e);

	container_iterate_const
		(std::vector<PlannedWorkers>, m_planned_workers, pw_it)
	{
		container_iterate_const
			(std::vector<Request *>, pw_it.current->requests, req_it)
			(*req_it.current)->set_economy(e);
	}

	if (e)
		e->add_warehouse(*this);
}


WareList const & Warehouse::get_wares() const
{
	return m_supply->get_wares();
}


WareList const & Warehouse::get_workers() const
{
	return m_supply->get_workers();
}

std::vector<const Soldier *> Warehouse::get_soldiers
	(Editor_Game_Base & egbase) const
{
	std::vector<const Soldier *> rv;

	container_iterate_const
		(std::vector<OPtr<Worker> >, m_incorporated_workers, i)
		if (upcast(const Soldier, soldier, i.current->get(egbase)))
			rv.push_back(soldier);

	return rv;
}


/// Magically create wares in this warehouse. Updates the economy accordingly.
void Warehouse::insert_wares(Ware_Index const id, uint32_t const count)
{
	m_supply->add_wares(id, count);
}


/// Magically destroy wares.
void Warehouse::remove_wares(Ware_Index const id, uint32_t const count)
{
	m_supply->remove_wares(id, count);
}


/// Magically create workers in this warehouse. Updates the economy accordingly.
void Warehouse::insert_workers(Ware_Index const id, uint32_t const count)
{
	m_supply->add_workers(id, count);
}


/// Magically destroy workers.
void Warehouse::remove_workers(Ware_Index const id, uint32_t const count)
{
	m_supply->remove_workers(id, count);
}



/// Launch a carrier to fetch an item from our flag.
bool Warehouse::fetch_from_flag(Game & game)
{
	Ware_Index const carrierid = tribe().safe_worker_index("carrier");

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
	(Game const & game, Ware_Index ware, Requirements const & req)
{
	std::vector<Worker_Descr const *> subs;
	uint32_t sum = 0;

	do {
		subs.push_back(tribe().get_worker_descr(ware));
		sum += m_supply->stock_workers(ware);
		ware = tribe().get_worker_descr(ware)->becomes();
	} while (ware != Ware_Index::Null());

	// NOTE: This code lies about the tAttributes of non-instantiated workers.

	container_iterate_const
		(std::vector<OPtr<Worker> >, m_incorporated_workers, i)
		if (Worker const * const w = i.current->get(game))
			if (std::find(subs.begin(), subs.end(), &w->descr()) != subs.end())
				//  This is one of the workers in our sum.
				if (!req.check(*w))
					--sum;

	return sum;
}


/// Start a worker of a given type. The worker will
/// be assigned a job by the caller.
Worker & Warehouse::launch_worker
	(Game & game, Ware_Index ware, Requirements const & req)
{
	do {
		if (m_supply->stock_workers(ware)) {
			uint32_t unincorporated = m_supply->stock_workers(ware);
			Worker_Descr const & workerdescr = *tribe().get_worker_descr(ware);

			//  look if we got one of those in stock
			const std::string & workername = workerdescr.name();
			container_iterate
				(std::vector<OPtr<Worker> >, m_incorporated_workers, i)
				if (upcast(Worker, worker, i.current->get(game)))
					if (worker->name() == workername) {
						--unincorporated;

						if (req.check(*worker)) {
							worker->reset_tasks(game);  //  forget everything you did
							worker->set_location(this); //  back in a economy
							m_incorporated_workers.erase(i.current);

							m_supply->remove_workers(ware, 1);
							return *worker;
						}
					}

			assert(unincorporated <= m_supply->stock_workers(ware));

			if (unincorporated) {
				// Create a new one
				// NOTE: This code lies about the tAttributes of the new worker
				m_supply->remove_workers(ware, 1);
				return workerdescr.create(game, owner(), this, m_position);
			}
		}

		if (can_create_worker(game, ware)) {
			// don't want to use an upgraded worker, so create new one.
			create_worker(game, ware);
		} else {
			ware = tribe().get_worker_descr(ware)->becomes();
		}
	} while (ware != Ware_Index::Null());

	throw wexception
		("Warehouse::launch_worker: worker does not actually exist");
}


/**
 * This is the opposite of launch_worker: destroy the worker and add the
 * appropriate ware to our warelist
 */
void Warehouse::incorporate_worker(Editor_Game_Base & egbase, Worker & w)
{
	assert(w.get_owner() == &owner());

	if (WareInstance * const item = w.fetch_carried_item(egbase))
		incorporate_item(egbase, *item); //  rescue an item

	m_supply->add_workers(tribe().worker_index(w.name().c_str()), 1);

	//  We remove carriers, but we keep other workers around.
	//  FIXME Remove all workers that do not have properties such as experience.
	//  FIXME And even such workers should be removed and only a small record
	//  FIXME with the experience (and possibly other data that must survive)
	//  FIXME may be kept.
	if (dynamic_cast<Carrier const *>(&w)) {
		w.remove(egbase);
		return;
	}

	sort_worker_in(egbase, w);
	w.set_location(0); //  no longer in an economy

	if (upcast(Game, game, &egbase)) {
		//  Bind the worker into this house, hide him on the map.
		w.reset_tasks(*game);
		w.start_task_idle(*game, 0, -1);
	}
}


/// Sort the worker into the right position in m_incorporated_workers
void Warehouse::sort_worker_in(Editor_Game_Base & egbase, Worker & w)
{
	//  We insert this worker, but to keep some consistency in ordering, we tell
	//  him where to insert.
	std::string const & workername = w.name();

	std::vector<OPtr<Worker> >::iterator i = m_incorporated_workers.begin();

	while
		(i != m_incorporated_workers.end()
		 &&
		 workername <= i->get(egbase)->name())
		++i;
	if (i == m_incorporated_workers.end()) {
		m_incorporated_workers.insert(i, &w);
		return;
	}

	while
		(i != m_incorporated_workers.end()
		 &&
		 w.serial() <= i->get(egbase)->serial())
		++i;

	m_incorporated_workers.insert(i, &w);
}

/// Create an instance of a ware and make sure it gets
/// carried out of the warehouse.
WareInstance & Warehouse::launch_item(Game & game, Ware_Index const ware) {
	// Create the item
	WareInstance & item =
		*new WareInstance(ware, tribe().get_ware_descr(ware));
	item.init(game);

	m_supply->remove_wares(ware, 1);

	do_launch_item(game, item);

	return item;
}


/// Get a carrier to actually move this item out of the warehouse.
void Warehouse::do_launch_item(Game & game, WareInstance & item)
{
	// Create a carrier
	Ware_Index const carrierid = tribe().worker_index("carrier");
	Worker_Descr const & workerdescr = *tribe().get_worker_descr(carrierid);

	Worker & worker = workerdescr.create(game, owner(), this, m_position);

	// Yup, this is cheating.
	if (m_supply->stock_workers(carrierid))
		m_supply->remove_workers(carrierid, 1);

	// Setup the carrier
	worker.start_task_dropoff(game, item);
}


/// Swallow the item, adding it to out inventory.
void Warehouse::incorporate_item(Editor_Game_Base & egbase, WareInstance & item)
{
	m_supply->add_wares(item.descr_index(), 1);
	return item.destroy(egbase);
}


/// Called when a transfer for one of the idle Requests completes.
void Warehouse::request_cb
	(Game            &       game,
	 Request         &,
	 Ware_Index        const ware,
	 Worker          * const w,
	 PlayerImmovable &       target)
{
	Warehouse & wh = ref_cast<Warehouse, PlayerImmovable>(target);

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
void Warehouse::receive_ware(Game & game, Ware_Index ware)
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

Building & Warehouse_Descr::create_object() const {
	return *new Warehouse(*this);
}


bool Warehouse::can_create_worker(Game &, Ware_Index const worker) const {
	if (not (worker < m_supply->get_workers().get_nrwareids()))
		throw wexception
			("worker type %d does not exists (max is %d)",
			 worker.value(), m_supply->get_workers().get_nrwareids().value());

	Worker_Descr const & w_desc = *tribe().get_worker_descr(worker);
	assert(&w_desc);
	if (not w_desc.is_buildable())
		return false;

	//  see if we have the resources
	Worker_Descr::Buildcost const & buildcost = w_desc.buildcost();
	container_iterate_const(Worker_Descr::Buildcost, buildcost, it) {
		std::string const & input_name = it.current->first;
		if (Ware_Index id_w = tribe().ware_index(input_name)) {
			if (m_supply->stock_wares  (id_w) < it.current->second)
				return false;
		} else if ((id_w = tribe().worker_index(input_name))) {
			if (m_supply->stock_workers(id_w) < it.current->second)
				return false;
		} else
			throw wexception
				("worker type %s needs \"%s\" to be built but that is neither "
				 "a ware type nor a worker type defined in the tribe %s",
				 w_desc.descname().c_str(), input_name.c_str(),
				 tribe().name().c_str());
	}
	return true;
}


void Warehouse::create_worker(Game & game, Ware_Index const worker) {
	assert(can_create_worker (game, worker));

	Worker_Descr const & w_desc = *tribe().get_worker_descr(worker);
	Worker_Descr::Buildcost const & buildcost = w_desc.buildcost();
	container_iterate_const(Worker_Descr::Buildcost, buildcost, i) {
		std::string const & input = i.current->first;
		if (Ware_Index const id_ware = tribe().ware_index(input))
			remove_wares  (id_ware,                        i.current->second);
		else
			remove_workers(tribe().safe_worker_index(input), i.current->second);
	}

	incorporate_worker(game, w_desc.create(game, owner(), this, m_position));

	// Update PlannedWorkers::amount here if appropriate, because this function
	// may have been called directly by the Economy.
	// Do not update anything else about PlannedWorkers here, because this
	// function is called by _update_planned_workers, so avoid recursion
	container_iterate(std::vector<PlannedWorkers>, m_planned_workers, pw_it) {
		if (pw_it.current->index == worker && pw_it.current->amount)
			pw_it.current->amount--;
	}
}

/**
 * Return the number of workers of the given type that we plan to
 * create in this warehouse.
 */
uint32_t Warehouse::get_planned_workers(Game & game, Ware_Index index) const
{
	container_iterate_const(std::vector<PlannedWorkers>, m_planned_workers, i) {
		if (i.current->index == index)
			return i.current->amount;
	}

	return 0;
}

/**
 * Calculate the supply of wares available to this warehouse in each of the
 * buildcost items for the given worker.
 *
 * This is the current stock plus any incoming transfers.
 */
std::vector<uint32_t> Warehouse::calc_available_for_worker
	(Game & game, Ware_Index index) const
{
	const Worker_Descr & w_desc = *tribe().get_worker_descr(index);
	const Worker_Descr::Buildcost & cost = w_desc.buildcost();
	std::vector<uint32_t> available;

	container_iterate_const(Worker_Descr::Buildcost, cost, bc) {
		std::string const & input_name = bc.current->first;
		if (Ware_Index id_w = tribe().ware_index(input_name)) {
			available.push_back(get_wares().stock(id_w));
		} else if ((id_w = tribe().worker_index(input_name))) {
			available.push_back(get_workers().stock(id_w));
		} else
			throw wexception
				("Economy::_create_requested_worker: buildcost inconsistency '%s'",
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


/**
 * Set the amount of workers we plan to create
 * of the given \p index to \p amount.
 */
void Warehouse::plan_workers(Game & game, Ware_Index index, uint32_t amount)
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

		const Worker_Descr & w_desc = *tribe().get_worker_descr(pw->index);
		const Worker_Descr::Buildcost & cost = w_desc.buildcost();
		container_iterate_const(Worker_Descr::Buildcost, cost, cost_it) {
			std::string const & input_name = cost_it.current->first;

			if (Ware_Index id_w = tribe().ware_index(input_name)) {
				pw->requests.push_back
					(new Request
					 (*this, id_w, &Warehouse::request_cb, Request::WARE));
			} else if ((id_w = tribe().worker_index(input_name))) {
				pw->requests.push_back
					(new Request
					 (*this, id_w, &Warehouse::request_cb, Request::WORKER));
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
	const Worker_Descr & w_desc = *tribe().get_worker_descr(pw.index);
	const Worker_Descr::Buildcost & cost = w_desc.buildcost();

	while (pw.amount && can_create_worker(game, pw.index))
		create_worker(game, pw.index);

	uint32_t idx = 0;
	container_iterate_const(Worker_Descr::Buildcost, cost, cost_it) {
		std::string const & input_name = cost_it.current->first;
		uint32_t supply;

		if (Ware_Index id_w = tribe().ware_index(input_name)) {
			supply = m_supply->stock_wares(id_w);
		} else if ((id_w = tribe().worker_index(input_name))) {
			supply = m_supply->stock_workers(id_w);
		} else
			throw wexception
				("_update_planned_workers: bad buildcost '%s'", input_name.c_str());

		if (supply >= pw.amount * cost_it.current->second)
			pw.requests[idx]->set_count(0);
		else
			pw.requests[idx]->set_count
				(pw.amount * cost_it.current->second - supply);
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
		 Never());
	m_next_worker_without_cost_spawn[worker_types_without_cost_index] =
		schedule_act(game, WORKER_WITHOUT_COST_SPAWN_INTERVAL);
}
void Warehouse::disable_spawn(uint8_t const worker_types_without_cost_index)
{
	assert
		(m_next_worker_without_cost_spawn[worker_types_without_cost_index]
		 !=
		 Never());
	m_next_worker_without_cost_spawn[worker_types_without_cost_index] = Never();
}


bool Warehouse::canAttack()
{
	return get_conquers() > 0;
}

void Warehouse::aggressor(Soldier & enemy)
{
	if (!get_conquers())
		return;

	Game & game = ref_cast<Game, Editor_Game_Base>(owner().egbase());
	Map  & map  = game.map();
	if
		(enemy.get_owner() == &owner() ||
		 enemy.getBattle() ||
		 get_conquers()
		 <=
		 map.calc_distance(enemy.get_position(), get_position()))
		return;

	if
		(game.map().find_bobs
		 	(Area<FCoords>(map.get_fcoords(base_flag().get_position()), 2),
		 	 0,
		 	 FindBobEnemySoldier(&owner())))
		return;

	Ware_Index const soldier_index = tribe().worker_index("soldier");
	Requirements noreq;

	if (!count_workers(game, soldier_index, noreq))
		return;

	Soldier & defender =
		ref_cast<Soldier, Worker>(launch_worker(game, soldier_index, noreq));
	defender.start_task_defense(game, false, owner().get_retreat_percentage());
}

bool Warehouse::attack(Soldier & enemy)
{
	Game & game = ref_cast<Game, Editor_Game_Base>(owner().egbase());
	Ware_Index const soldier_index = tribe().worker_index("soldier");
	Requirements noreq;

	if (count_workers(game, soldier_index, noreq)) {
		Soldier & defender =
			ref_cast<Soldier, Worker>(launch_worker(game, soldier_index, noreq));
		defender.start_task_defense(game, true, 0);
		enemy.send_signal(game, "sleep");
		return true;
	}

	set_defeating_player(enemy.owner().player_number());
	schedule_destroy(game);
	return false;
}

void Warehouse::PlannedWorkers::cleanup()
{
	while (requests.size()) {
		delete requests.back();
		requests.pop_back();
	}
}

Warehouse::StockPolicy Warehouse::get_ware_policy(Ware_Index ware) const
{
	assert(ware.value() < m_ware_policy.size());
	return m_ware_policy[ware.value()];
}

Warehouse::StockPolicy Warehouse::get_worker_policy(Ware_Index ware) const
{
	assert(ware.value() < m_worker_policy.size());
	return m_worker_policy[ware.value()];
}

Warehouse::StockPolicy Warehouse::get_stock_policy
	(bool isworker, Ware_Index ware) const
{
	if (isworker)
		return get_worker_policy(ware);
	else
		return get_ware_policy(ware);
}


void Warehouse::set_ware_policy(Ware_Index ware, Warehouse::StockPolicy policy)
{
	assert(ware.value() < m_ware_policy.size());
	m_ware_policy[ware.value()] = policy;
}

void Warehouse::set_worker_policy
	(Ware_Index ware, Warehouse::StockPolicy policy)
{
	assert(ware.value() < m_worker_policy.size());
	m_worker_policy[ware.value()] = policy;
}

/**
 * Check if there are remaining wares with stock policy \ref SP_Remove,
 * and remove one of them if appropriate.
 */
void Warehouse::check_remove_stock(Game & game)
{
	if (base_flag().current_items() < base_flag().total_capacity() / 2) {
		for
			(Ware_Index ware = Ware_Index::First();
			 ware.value() < m_ware_policy.size(); ++ware)
		{
			if (get_ware_policy(ware) != SP_Remove || !get_wares().stock(ware))
				continue;

			launch_item(game, ware);
			break;
		}
	}

	for
		(Ware_Index widx = Ware_Index::First();
		 widx.value() < m_worker_policy.size(); ++widx)
	{
		if (get_worker_policy(widx) != SP_Remove || !get_workers().stock(widx))
			continue;

		Worker & worker = launch_worker(game, widx, Requirements());
		worker.start_task_leavebuilding(game, true);
		break;
	}
}


}
