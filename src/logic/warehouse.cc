/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#include <algorithm>

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

namespace Widelands {

static const int32_t CARRIER_SPAWN_INTERVAL = 2500;

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

/*
 * Inform this supply, how much wares
 * are to be handled
 */
void WarehouseSupply::set_nrwares(Ware_Index const i) {
	assert(Ware_Index::First() == m_wares.get_nrwareids());

	m_wares.set_nrwares(i);
}
void WarehouseSupply::set_nrworkers(Ware_Index const i) {
	assert(Ware_Index::First() == m_workers.get_nrwareids());

	m_workers.set_nrwares(i);
}



/*
===============
Add and remove our wares and the Supply to the economies as necessary.
===============
*/
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


/*
===============
Add wares and update the economy.
===============
*/
void WarehouseSupply::add_wares     (Ware_Index const id, uint32_t const count)
{
	if (!count)
		return;

	m_economy->add_wares(id, count);
	m_wares.add(id, count);
}


/*
===============
Remove wares and update the economy.
===============
*/
void WarehouseSupply::remove_wares  (Ware_Index const id, uint32_t const count)
{
	if (!count)
		return;

	m_wares.remove(id, count);
	m_economy->remove_wares(id, count);
}

/*
===============
Add workers and update the economy.

===============
*/
void WarehouseSupply::add_workers   (Ware_Index const id, uint32_t const count)
{
	if (!count)
		return;

	m_economy->add_workers(id, count);
	m_workers.add(id, count);
}


/*
===============
Remove workers and update the economy.

Comments see add_workers
===============
*/
void WarehouseSupply::remove_workers(Ware_Index const id, uint32_t const count)
{
	if (!count)
		return;

	m_workers.remove(id, count);
	m_economy->remove_workers(id, count);
}

/*
===============
Return the position of the Supply, i.e. the owning Warehouse.
===============
*/
PlayerImmovable * WarehouseSupply::get_position(Game &) {return m_warehouse;}


/*
===============
Warehouse supplies are never active.
===============
*/
bool WarehouseSupply::is_active() const throw () {return false;}


uint32_t WarehouseSupply::nr_supplies
	(Game const & game, Request const & req) const
{
	return
		req.get_type() == Request::WARE ? m_wares.stock(req.get_index()) :
		m_warehouse->count_workers
			(game, req.get_index(), req.get_requirements());
}


/*
===============
Launch a ware as item.
===============
*/
WareInstance & WarehouseSupply::launch_item(Game & game, Request const & req) {
	if (req.get_type() != Request::WARE)
		throw wexception
			("WarehouseSupply::launch_item: called for non-ware request");
	if (!m_wares.stock(req.get_index()))
		throw wexception
			("WarehouseSupply::launch_item: called for non-existing ware");

	return m_warehouse->launch_item(game, req.get_index());
}


/*
===============
Launch a ware as worker.
===============
*/
Worker & WarehouseSupply::launch_worker(Game & game, Request const & req)
{
	return
		m_warehouse->launch_worker
			(game, req.get_index(), req.get_requirements());
}


/*
==============================================================================

Warehouse building

==============================================================================
*/

Warehouse_Descr::Warehouse_Descr
	(char const * const _name, char const * const _descname,
	 std::string const & directory, Profile & prof, Section & global_s,
	 Tribe_Descr const & _tribe, EncodeData const * const encdata)
:
	Building_Descr(_name, _descname, directory, prof, global_s, _tribe, encdata),
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

Warehouse::Warehouse(const Warehouse_Descr & warehouse_descr) :
Building            (warehouse_descr),
m_supply            (new WarehouseSupply(this)),
m_next_carrier_spawn(0)
{}


Warehouse::~Warehouse()
{
	delete m_supply;
}


void Warehouse::prefill
	(Game &, uint32_t const *, uint32_t const *, Soldier_Counts const *)
{}
void Warehouse::postfill
	(Game                &       game,
	 uint32_t      const *       ware_types,
	 uint32_t      const *       worker_types,
	 Soldier_Counts const * const soldier_counts)
{
	Building::postfill(game, ware_types, worker_types, soldier_counts);
	if (ware_types)
		for
			(struct {Ware_Index i; Ware_Index const nr_ware_types;} i =
			 	{Ware_Index::First(), tribe().get_nrwares  ()};
			 i.i < i.nr_ware_types;
			 ++i.i, ++ware_types)
			if (uint32_t const count = *ware_types)
				insert_wares  (i.i, count);
	if (worker_types)
		for
			(struct {Ware_Index i; Ware_Index const nr_worker_types;} i =
			 	{Ware_Index::First(), tribe().get_nrworkers()};
			 i.i < i.nr_worker_types;
			 ++i.i, ++worker_types)
			if (uint32_t const count = *worker_types)
				insert_workers(i.i, count);
	if (soldier_counts) {
		Soldier_Descr const & soldier_descr =
			dynamic_cast<Soldier_Descr const &>
				(*tribe().get_worker_descr(tribe().worker_index("soldier")));
		container_iterate_const(Soldier_Counts, *soldier_counts, i) {
			Soldier_Strength const ss = i.current->first;
			for (uint32_t j = i.current->second; j; --j) {
				Soldier & soldier =
					static_cast<Soldier &>
						(soldier_descr.create(game, owner(), this, get_position()));
				soldier.set_level(ss.hp, ss.attack, ss.defense, ss.evade);
				incorporate_worker(game, soldier);
			}
		}
	}
}


/*
warehouses determine how badly they want a certain ware
*/
int32_t Warehouse::get_priority
	(int32_t const type, Ware_Index const ware_index, bool) const
{
	return
		type == Request::WARE and m_target_supply[ware_index] > 0 ?
		(m_target_supply[ware_index] + 1) * 100 : 100;
}

void Warehouse::set_needed(Ware_Index const ware_index, uint32_t const value) {
	//assert (value >= m_supply->stock_wares(ware_index));
	if (value > m_supply->stock_wares(ware_index))
		m_target_supply[ware_index] = value - m_supply->stock_wares(ware_index);
}


void Warehouse::init(Editor_Game_Base & egbase)
{
	Building::init(egbase);

	Ware_Index const nr_wares   = tribe().get_nrwares  ();
	Ware_Index const nr_workers = tribe().get_nrworkers();
	m_supply->set_nrwares  (nr_wares);
	m_supply->set_nrworkers(nr_workers);

	if (upcast(Game, game, &egbase)) {
		for (Ware_Index i = Ware_Index::First(); i < nr_wares;   ++i) {
			Request & req =
				*new Request
					(*this, i, Warehouse::idle_request_cb, Request::WARE);

			req.set_idle(true);

			m_requests.push_back(&req);
		}
		for (Ware_Index i = Ware_Index::First(); i < nr_workers; ++i) {
			Request & req =
				*new Request
					(*this, i, &Warehouse::idle_request_cb, Request::WORKER);

			req.set_idle(true);

			m_requests.push_back(&req);
		}
		m_next_carrier_spawn = schedule_act(*game, CARRIER_SPAWN_INTERVAL);
		m_next_military_act  = schedule_act(*game, 1000);
	}
	m_target_supply.resize(m_requests.size());

	// Even though technically, a warehouse might be completely empty,
	// we let warehouse see always for simplicity's sake (since there's
	// almost always going to be a carrier inside, that shouldn't hurt).
	Player & player = owner();
	player.see_area
		(Area<FCoords>
		 	(egbase.map().get_fcoords(get_position()), vision_range()));

	if (uint32_t const conquer_radius = get_conquers())
		egbase.conquer_area
			(Player_Area<Area<FCoords> >
			 	(owner().player_number(),
			 	 Area<FCoords>
			 	 	(egbase.map().get_fcoords(get_position()), conquer_radius)));

	//  Fill the message queue with a message when a warehouse is created.
	log
		("Message: adding (wh) (%s) %i \n",
		 type_name(), owner().player_number());
	std::string sender = type_name();
	MessageQueue::add
		(owner(),
		 Message
		 	(sender,
		 	 egbase.get_gametime(),
		 	 "warehouse",
		 	 Widelands::Coords(get_position()),
		 	 _("A new warehouse is created")));
}





/*
===============
Destroy the warehouse.
===============
*/
void Warehouse::cleanup(Editor_Game_Base & egbase)
{
	if (upcast(Game, game, &egbase)) {

		while (m_requests.size()) {
			delete m_requests.back();
			m_requests.pop_back();
		}

		//  all cached workers are unbound and freed
		while (m_incorporated_workers.size()) {
			//  If the game ends and this worker has been created before this
			//  warehouse, it might already be deleted. So do not try and free him
			if
				(upcast
				 	(Worker, worker, m_incorporated_workers.begin()->get(egbase)))
				worker->reset_tasks(*game);
			m_incorporated_workers.erase(m_incorporated_workers.begin());
		}
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


/// Act regularly to create carriers. According to intelligence, this is some
/// highly advanced technology. Not only do the settlers have no problems with
/// birth control, they don't even need anybody to procreate. They must have
/// built-in DNA samples in those warehouses. And what the hell are they doing,
/// killing useless tribesmen! The Borg? Or just like Soylent Green?
/// Or maybe I should just stop writing comments that late at night ;-)
void Warehouse::act(Game & game, uint32_t const data)
{
	if (game.get_gametime() - m_next_carrier_spawn >= 0) {
		Ware_Index const id = tribe().safe_worker_index("carrier");
		int32_t stock = m_supply->stock_workers(id);
		int32_t tdelta = CARRIER_SPAWN_INTERVAL;

		if (stock < 100) {
			tdelta -= 4 * (100 - stock);
			insert_workers(id, 1);
		} else if (stock > 100) {
			tdelta -= 4 * (stock - 100);
			if (tdelta < 10)
				tdelta = 10;
			remove_workers(id, 1);
		}

		m_next_carrier_spawn = schedule_act(game, tdelta);
	}

	//  Military stuff: Kill the soldiers that are dead.
	if (game.get_gametime() - m_next_military_act >= 0) {
		Ware_Index const ware = tribe().safe_worker_index("soldier");
		Worker_Descr const & workerdescr = *tribe().get_worker_descr(ware);
		const std::string & workername = workerdescr.name();
		//  Look if we got one in stock of those.
		for
			(std::vector<Object_Ptr>::iterator it = m_incorporated_workers.begin();
			 it != m_incorporated_workers.end();
			 ++it)
		{
			Worker const & worker = dynamic_cast<Worker const &>(*it->get(game));
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
		m_next_military_act = schedule_act (game, 1000);
	}
	Building::act(game, data);
}


/*
===============
Transfer our registration to the new economy.
===============
*/
void Warehouse::set_economy(Economy * const e)
{
	Economy * const old = get_economy();

	if (old == e)
		return;

	if (old)
		old->remove_warehouse(this);

	m_supply->set_economy(e);
	Building::set_economy(e);

	for (uint32_t i = 0; i < m_requests.size(); ++i)
		m_requests[i]->set_economy(e);

	if (e)
		e->add_warehouse(this);
}


WareList const & Warehouse::get_wares() const
{
	return m_supply->get_wares();
}


WareList const & Warehouse::get_workers() const
{
	return m_supply->get_workers();
}



/*
===============
Magically create wares in this warehouse. Updates the economy accordingly.
===============
*/
void Warehouse::insert_wares(Ware_Index const id, uint32_t const count)
{
	assert(get_economy());
	if (m_target_supply[id] > count)
		m_target_supply[id] -= count;
	else
		m_target_supply[id] = 0;
	m_supply->add_wares(id, count);
}


/*
===============
Magically destroy wares.
===============
*/
void Warehouse::remove_wares(Ware_Index const id, uint32_t const count)
{
	assert(get_economy());
	if (m_target_supply[id] > 0)
		m_target_supply[id] += 1;
	m_supply->remove_wares(id, count);
}

/*
===============
Magically create workers in this warehouse. Updates the economy accordingly.
===============
*/
void Warehouse::insert_workers(Ware_Index const id, uint32_t const count)
{
	assert(get_economy());

	m_supply->add_workers(id, count);
}


/*
===============
Magically destroy workers.
===============
*/
void Warehouse::remove_workers(Ware_Index const id, uint32_t const count)
{
	assert(get_economy());

	m_supply->remove_workers(id, count);
}


/*
===============
Launch a carrier to fetch an item from our flag.
===============
*/
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
	std::vector<Map_Object_Descr *> subs;
	uint32_t sum = 0;

	do {
		subs.push_back(tribe().get_worker_descr(ware));
		sum += m_supply->stock_workers(ware);
		ware = tribe().get_worker_descr(ware)->becomes();
	} while (ware != Ware_Index::Null());

	// NOTE: This code lies about the tAttributes of non-instantiated workers.

	container_iterate_const(std::vector<Object_Ptr>, m_incorporated_workers, i)
		if (Map_Object const * const w = i.current->get(game))
			if (std::find(subs.begin(), subs.end(), &w->descr()) != subs.end())
				//  This is one of the workers in our sum.
				if (!req.check(*w))
					--sum;

	return sum;
}


/*
===============
Start a worker of a given type. The worker will be assigned a job by the caller.
===============
*/
Worker & Warehouse::launch_worker
	(Game & game, Ware_Index ware, Requirements const & req)
{
	do {
		if (m_supply->stock_workers(ware)) {
			uint32_t unincorporated = m_supply->stock_workers(ware);
			Worker_Descr const & workerdescr = *tribe().get_worker_descr(ware);

			//  look if we got one of those in stock
			const std::string & workername = workerdescr.name();
			container_iterate(std::vector<Object_Ptr>, m_incorporated_workers, i)
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


/*
===============
This is the opposite of launch_worker: destroy the worker and add the
appropriate ware to our warelist
===============
*/
void Warehouse::incorporate_worker(Game & game, Worker & w)
{
	assert(w.get_owner() == &owner());

	if (WareInstance * const item = w.fetch_carried_item(game))
		incorporate_item(game, *item); //  rescue an item

	m_supply->add_workers(tribe().worker_index(w.name().c_str()), 1);

	//  We remove carriers, but we keep other workers around.
	//  FIXME Remove all workers that do not have properties such as experience.
	//  FIXME And even such workers should be removed and only a small record
	//  FIXME with the experience (and possibly other data that must survive)
	//  FIXME may be kept.
	if (dynamic_cast<Carrier const *>(&w))
		return w.remove(game);

	sort_worker_in(game, w.name(), w);
	w.set_location(0); //  no longer in an economy
	w.reset_tasks(game);

	//  Bind the worker into this house, hide him on the map.
	w.start_task_idle(game, 0, -1);
}


/*
 * Sort the worker into the right position in m_incorporated_workers
 */
void Warehouse::sort_worker_in
	(Editor_Game_Base & egbase, std::string const & workername, Worker & w)
{
	//  We insert this worker, but to keep some consistency in ordering, we tell
	//  him where to insert.

	std::vector<Object_Ptr>::iterator i = m_incorporated_workers.begin();

	while
		(i != m_incorporated_workers.end()
		 &&
		 workername <= dynamic_cast<Worker &>(*i->get(egbase)).name())
		++i;
	if (i == m_incorporated_workers.end()) {
		m_incorporated_workers.insert(i, &w);
		return;
	}

	while
		(i != m_incorporated_workers.end()
		 &&
		 w.serial() <= dynamic_cast<Worker &>(*i->get(egbase)).serial())
		++i;

	m_incorporated_workers.insert(i, &w);
}

/*
===============
Create an instance of a ware and make sure it gets carried out of the warehouse.
===============
*/
WareInstance & Warehouse::launch_item(Game & game, Ware_Index const ware) {
	// Create the item
	WareInstance & item =
		*new WareInstance(ware, tribe().get_ware_descr(ware));
	item.init(game);

	m_supply->remove_wares(ware, 1);

	do_launch_item(game, item);

	return item;
}


/*
===============
Get a carrier to actually move this item out of the warehouse.
===============
*/
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


/*
===============
Swallow the item, adding it to out inventory.
===============
*/
void Warehouse::incorporate_item(Game & game, WareInstance & item)
{
	m_supply->add_wares(item.descr_index(), 1);
	return item.destroy(game);
}


/*
===============
Called when a transfer for one of the idle Requests completes.
===============
*/
void Warehouse::idle_request_cb
	(Game            &       game,
	 Request         &,
	 Ware_Index        const ware,
	 Worker          * const w,
	 PlayerImmovable &       target)
{
	if (w)
		w->schedule_incorporate(game);
	else
		dynamic_cast<Warehouse &>(target).m_supply->add_wares(ware, 1);
}


Building & Warehouse_Descr::create_object() const {
	return *new Warehouse(*this);
}


bool Warehouse::can_create_worker(Game &, Ware_Index const worker) const {
	if (not (worker < m_supply->get_workers().get_nrwareids()))
		throw wexception
			("worker type %d does not exists (max is %d)",
			 worker.value(), m_supply->get_workers().get_nrwareids().value());

	if (Worker_Descr const * const w_desc = tribe().get_worker_descr(worker)) {
		// First watch if we can build it
		if (!w_desc->buildable())
			return false;

		// Now see if we have the resources
		Worker_Descr::Buildcost const & buildcost = w_desc->buildcost();
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
					 w_desc->descname().c_str(), input_name.c_str(),
					 tribe().name().c_str());
		}
		return true;
	}
	else
		throw wexception
			("Can not create worker of desired type : %d", worker.value());
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
}


bool Warehouse::canAttack()
{
	return get_conquers() > 0;
}

void Warehouse::aggressor(Soldier & enemy)
{
	if (!get_conquers())
		return;

	Game & game = dynamic_cast<Game &>(owner().egbase());
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
		 	 FindBobEnemySoldier(owner())))
		return;

	Ware_Index const soldier_index = tribe().worker_index("soldier");
	Requirements noreq;

	if (!count_workers(game, soldier_index, noreq))
		return;

	Soldier & defender =
		dynamic_cast<Soldier &>(launch_worker(game, soldier_index, noreq));
	defender.startTaskDefense(game, false);
	new Battle(game, defender, enemy);
}

bool Warehouse::attack(Soldier & enemy)
{
	Game & game = dynamic_cast<Game &>(owner().egbase());
	Ware_Index const soldier_index = tribe().worker_index("soldier");
	Requirements noreq;

	if (count_workers(game, soldier_index, noreq)) {
		Soldier & defender =
			dynamic_cast<Soldier &>(launch_worker(game, soldier_index, noreq));
		defender.startTaskDefense(game, true);
		new Battle(game, defender, enemy);
		return true;
	}

	set_defeating_player(enemy.owner().player_number());
	schedule_destroy(game);
	return false;
}

};
