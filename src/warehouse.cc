/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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
#include "warehousesupply.h"

#include "carrier.h"
#include "editor_game_base.h"
#include "game.h"
#include "player.h"
#include "profile.h"
#include "request.h"
#include "soldier.h"
#include "transport.h"
#include "wexception.h"
#include "worker.h"
#include "tribe.h"

#include "log.h"

#include "upcast.h"

namespace Widelands {

static const int32_t CARRIER_SPAWN_INTERVAL = 2500;


WarehouseSupply::~WarehouseSupply()
{
	if (m_economy)
	{
		log
			("WarehouseSupply::~WarehouseSupply: Warehouse %u still belongs to "
			 "an economy",
			 m_warehouse->get_serial());
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
void WarehouseSupply::set_nrwares(int32_t i) {
	assert(m_wares.get_nrwareids() == 0);

	m_wares.set_nrwares(i);
}
void WarehouseSupply::set_nrworkers(int32_t i) {
	assert(m_workers.get_nrwareids() == 0);

	m_workers.set_nrwares(i);
}



/*
===============
WarehouseSupply::set_economy

Add and remove our wares and the Supply to the economies as necessary.
===============
*/
void WarehouseSupply::set_economy(Economy* e)
{
	if (e == m_economy)
		return;

	if (m_economy) {
		for (Ware_Index::value_t i = 0; i < m_wares  .get_nrwareids(); ++i) {
			if (m_wares.stock(i)) {
				m_economy->remove_wares(i, m_wares.stock(i));
				m_economy->remove_ware_supply(i, this);
			}
		}
		for (Ware_Index::value_t i = 0; i < m_workers.get_nrwareids(); ++i) {
			if (m_workers.stock(i)) {
				m_economy->remove_workers(i, m_workers.stock(i));
				m_economy->remove_worker_supply(i, this);
			}
		}
	}

	m_economy = e;

	if (m_economy) {
		for (Ware_Index::value_t i = 0; i < m_wares.get_nrwareids(); ++i) {
			if (m_wares.stock(i)) {
				m_economy->add_wares(i, m_wares.stock(i));
				m_economy->add_ware_supply(i, this);
			}
		}
		for (Ware_Index::value_t i = 0; i < m_workers.get_nrwareids(); ++i) {
			if (m_workers.stock(i)) {
				m_economy->add_workers(i, m_workers.stock(i));
				m_economy->add_worker_supply(i, this);
			}
		}
	}
}


/*
===============
WarehouseSupply::add_wares

Add wares and update the economy.
===============
*/
void WarehouseSupply::add_wares     (Ware_Index const id, uint32_t const count)
{
	if (!count)
		return;

	if (!m_wares.stock(id))
		m_economy->add_ware_supply(id, this);

	m_economy->add_wares(id, count);
	m_wares.add(id, count);
}


/*
===============
WarehouseSupply::remove_wares

Remove wares and update the economy.
===============
*/
void WarehouseSupply::remove_wares  (Ware_Index const id, uint32_t const count)
{
	if (!count)
		return;

	m_wares.remove(id, count);
	m_economy->remove_wares(id, count);

	if (!m_wares.stock(id))
		m_economy->remove_ware_supply(id, this);
}

/*
===============
WarehouseSupply::add_workers

Add workers and update the economy.

===============
*/
void WarehouseSupply::add_workers   (Ware_Index const id, uint32_t const count)
{
	if (!count)
		return;

	if (!m_workers.stock(id))
		m_economy->add_worker_supply(id, this);

	m_economy->add_workers(id, count);
	m_workers.add(id, count);
}


/*
===============
WarehouseSupply::remove_workers

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

	if (!m_workers.stock(id))
		m_economy->remove_worker_supply(id, this);
}

/*
===============
WarehouseSupply::get_position

Return the position of the Supply, i.e. the owning Warehouse.
===============
*/
PlayerImmovable* WarehouseSupply::get_position(Game *) {return m_warehouse;}


/*
===============
WarehouseSupply::get_amount

Return our stock of the given ware.
===============
*/
int32_t WarehouseSupply::get_amount(const int32_t ware) const
{return m_wares.stock(ware);}


/*
===============
WarehouseSupply::is_active

Warehouse supplies are never active.
===============
*/
bool WarehouseSupply::is_active() const throw () {return false;}


/*
===============
Launch a ware as item.
===============
*/
WareInstance & WarehouseSupply::launch_item(Game * game, int32_t ware) {
	assert(m_wares.stock(ware));

	return m_warehouse->launch_item(game, ware);
}


/*
===============
WarehouseSupply::launch_worker

Launch a ware as worker.
===============
*/
Worker* WarehouseSupply::launch_worker(Game* g, int32_t ware)
{
	assert(m_workers.stock(ware));

	return m_warehouse->launch_worker(g, ware);
}


/*
===============
WarehouseSupply::launch_soldier

Launch a ware as soldier.
===============
*/
Soldier* WarehouseSupply::launch_soldier(Game* g, int32_t ware, Requeriments* req)
{
	assert(m_workers.stock(ware));

	return m_warehouse->launch_soldier(g, ware, req);
}

/*
===============
WarehouseSupply::get_passing_requeriments

Launch a ware as soldier.
===============
*/
int32_t WarehouseSupply::get_passing_requeriments(Game* g, int32_t ware, Requeriments* req)
{
	assert(m_workers.stock(ware));

	return m_warehouse->get_soldiers_passing (g, ware, req);
}

/*
===============
WarehouseSupply::mark_as_used
===============
*/
void WarehouseSupply::mark_as_used (Game* g, int32_t ware, Requeriments* r)
{
	m_warehouse->mark_as_used (g, ware, r);
}



/*
==============================================================================

Warehouse building

==============================================================================
*/

/*
===============
Warehouse_Descr::Warehouse_Descr

Initialize with sane defaults
===============
*/
Warehouse_Descr::Warehouse_Descr
(const Tribe_Descr & tribe_descr, const std::string & warehouse_name)
:
Building_Descr(tribe_descr, warehouse_name),
m_subtype     (Subtype_Normal),
m_conquers    (0)
{}

/*
===============
Warehouse_Descr::parse

Parse the additional warehouse settings from the given profile and directory
===============
*/
void Warehouse_Descr::parse
	(char       const * const directory,
	 Profile          * const prof,
	 EncodeData const * const encdata)
{
	Building_Descr::parse(directory, prof, encdata);

	Section* global = prof->get_safe_section("global");
	const char* string;

	string = global->get_safe_string("subtype");
	if (!strcasecmp(string, "HQ")) {
		m_subtype = Subtype_HQ;
	} else if (!strcasecmp(string, "port")) {
		m_subtype = Subtype_Port;
	} else if (!strcasecmp(string, "none")) {
		//
	} else
		throw wexception
			("Unsupported warehouse subtype '%s'. Possible values: none, HQ, "
			 "port",
			 string);

	if (m_subtype == Subtype_HQ) {
		m_conquers = global->get_int("conquers");
		if (0 < m_conquers)
			m_workarea_info[m_conquers].insert(descname() + _(" conquer"));
	}
}


/*
==============================

IMPLEMENTATION

==============================
*/

/*
===============
Warehouse::Warehouse

Initialize a warehouse (zero contents, etc...)
===============
*/
Warehouse::Warehouse(const Warehouse_Descr & warehouse_descr) :
Building            (warehouse_descr),
m_supply            (new WarehouseSupply(this)),
m_next_carrier_spawn(0)
{}


/*
===============
Warehouse::Warehouse

Cleanup
===============
*/
Warehouse::~Warehouse()
{
	delete m_supply;
}


/*
===============
Warehouse::init

Conquer the land around the HQ on init.
===============
*/
void Warehouse::init(Editor_Game_Base* gg)
{
	Building::init(gg);

	m_supply->set_nrwares(get_owner()->tribe().get_nrwares());//  FIXME
	m_supply->set_nrworkers(get_owner()->tribe().get_nrworkers());//  FIXME

	if (upcast(Game, game, gg)) {
		for (int32_t i = 0; i < get_owner()->tribe().get_nrwares(); ++i) {//  FIXME
			Request & req =
				*new Request
				(this, i, &Warehouse::idle_request_cb, this, Request::WARE);

			req.set_idle(true);

			m_requests.push_back(&req);
		}
		for (int32_t i = 0; i < get_owner()->tribe().get_nrworkers(); ++i) {//  FIXME
			Request & req =
				*new Request
				(this, i, &Warehouse::idle_request_cb, this, Request::WORKER);

			req.set_idle(true);

			m_requests.push_back(&req);
		}
		m_next_carrier_spawn = schedule_act(game, CARRIER_SPAWN_INTERVAL);
		m_next_military_act  = schedule_act(game, 1000);
	}

	// Even though technically, a warehouse might be completely empty,
	// we let warehouse see always for simplicity's sake (since there's
	// almost always going to be a carrier inside, that shouldn't hurt).
	Player & player = owner();
	Map & map = gg->map();
	player.see_area
		(Area<FCoords>(map.get_fcoords(get_position()), vision_range()));
}


/*
===============
Warehouse::cleanup

Destroy the warehouse.
===============
*/
void Warehouse::cleanup(Editor_Game_Base* gg)
{
	if (upcast(Game, game, gg)) {

		while (m_requests.size()) {
			delete m_requests.back();
			m_requests.pop_back();
		}

		//  all cached workers are unbound and freed
		while (m_incorporated_workers.size()) {
			//  If the game ends and this worker has been created before this
			//  warehouse, it might already be deleted. So do not try and free him
			if (upcast(Worker, worker, m_incorporated_workers.begin()->get(gg)))
				worker->reset_tasks(game);
			m_incorporated_workers.erase(m_incorporated_workers.begin());
		}
	}
	if (const uint32_t conquer_raduis = get_conquers())
		gg->unconquer_area
			(Player_Area<Area<FCoords> >
			 (owner().get_player_number(),
			  Area<FCoords>
			  (gg->map().get_fcoords(get_position()), conquer_raduis)),
			 m_defeating_player);

	// Unsee the area that we started seeing in init()
	Player & player = owner();
	Map & map = gg->map();
	player.unsee_area
			(Area<FCoords>(map.get_fcoords(get_position()), vision_range()));

	Building::cleanup(gg);
}


/// Act regularly to create carriers. According to intelligence, this is some
/// highly advanced technology. Not only do the settlers have no problems with
/// birth control, they don't even need anybody to procreate. They must have
/// built-in DNA samples in those warehouses. And what the hell are they doing,
/// killing useless tribesmen! The Borg? Or just like Soylent Green?
/// Or maybe I should just stop writing comments that late at night ;-)
void Warehouse::act(Game* g, uint32_t data)
{
	const Tribe_Descr & tribe = owner().tribe();
	if (g->get_gametime() - m_next_carrier_spawn >= 0) {
		const int32_t id = tribe.get_safe_worker_index("carrier");
		int32_t stock = m_supply->stock_workers(id);
		int32_t tdelta = CARRIER_SPAWN_INTERVAL;

		if (stock < 100) {
			tdelta -= 4*(100 - stock);
			insert_workers(id, 1);
		} else if (stock > 100) {
			tdelta -= 4*(stock - 100);
			if (tdelta < 10)
				tdelta = 10;
			remove_workers(id, 1);
		}

		m_next_carrier_spawn = schedule_act(g, tdelta);
	}

	//  Military stuff: Kill the soldiers that are dead.
	if (g->get_gametime() - m_next_military_act >= 0) {
		const int32_t ware = tribe.get_safe_worker_index("soldier");
		const Worker_Descr & workerdescr = *tribe.get_worker_descr(ware);
		const std::string & workername = workerdescr.name();
		//  Look if we got one in stock of those.
		for
			(std::vector<Object_Ptr>::iterator it = m_incorporated_workers.begin();
			 it != m_incorporated_workers.end();
			 ++it)
		{
			Worker const & worker = dynamic_cast<Worker const &>(*it->get(g));
			if (worker.name() == workername) {
				upcast(Soldier const, soldier, &worker);

				//  Soldier dead ...
				if (not soldier or soldier->get_current_hitpoints() == 0) {
					m_incorporated_workers.erase(it);
					m_supply->remove_workers(ware, 1);
					continue;
				}
				//  If warehouse can heal, this is the place to put it.
			}
		}
		m_next_military_act = schedule_act (g, 1000);
	}
	Building::act(g, data);
}


/*
===============
Warehouse::set_economy

Transfer our registration to the new economy.
===============
*/
void Warehouse::set_economy(Economy* e)
{
	Economy* old = get_economy();

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


/*
===============
Warehouse::get_wares
===============
*/
const WareList& Warehouse::get_wares() const
{
	return m_supply->get_wares();
}

/*
===============
Warehouse::get_workers
===============
*/
const WareList& Warehouse::get_workers() const
{
	return m_supply->get_workers();
}



/*
===============
Warehouse::create_wares

Magically create wares in this warehouse. Updates the economy accordingly.
===============
*/
void Warehouse::insert_wares(int32_t id, int32_t count)
{
	assert(get_economy());

	m_supply->add_wares(id, count);
}


/*
===============
Warehouse::destroy_wares

Magically destroy wares.
===============
*/
void Warehouse::remove_wares(int32_t id, int32_t count)
{
	assert(get_economy());

	m_supply->remove_wares(id, count);
}

/*
===============
Warehouse::create_workers

Magically create workers in this warehouse. Updates the economy accordingly.
===============
*/
void Warehouse::insert_workers(int32_t id, int32_t count)
{
	assert(get_economy());

	m_supply->add_workers(id, count);
}


/*
===============
Warehouse::destroy_workers

Magically destroy workers.
===============
*/
void Warehouse::remove_workers(int32_t id, int32_t count)
{
	assert(get_economy());

	m_supply->remove_workers(id, count);
}


/*
===============
Warehouse::fetch_from_flag

Launch a carrier to fetch an item from our flag.
===============
*/
bool Warehouse::fetch_from_flag(Game* g)
{
	int32_t carrierid;

	carrierid = get_owner()->tribe().get_safe_worker_index("carrier");

	if (!m_supply->stock_workers(carrierid)) // XXX yep, let's cheat
		insert_workers(carrierid, 1);

	launch_worker(g, carrierid)->start_task_fetchfromflag();

	return true;
}


/*
===============
Warehouse::launch_worker

Start a worker of a given type. The worker will be assigned a job by the caller.
===============
*/
Worker * Warehouse::launch_worker(Game * game, int32_t const ware) {
	assert(m_supply->stock_workers(ware));

	Worker* worker;

	const Worker_Descr & workerdescr = *owner().tribe().get_worker_descr(ware);

	//  look if we got one of those in stock
	const std::string & workername = workerdescr.name();
	std::vector<Object_Ptr>::const_iterator const incorporated_workers_end =
		m_incorporated_workers.end();
	for
		(std::vector<Object_Ptr>::iterator it = m_incorporated_workers.begin();;
		 ++it)
		if (it == m_incorporated_workers.end()) {
			// None found, create a new one (if available)
			worker = &workerdescr.create(*game, owner(), *this, m_position);
			break;
		} else {
			worker = dynamic_cast<Worker *>(it->get(game));
			if (worker->name() == workername) {
				//  one found, make him available
				worker->reset_tasks(game);  //  forget everything you did
				worker->set_location(this); //  back in a economy
				m_incorporated_workers.erase(it);
				break;
			}
		}

	m_supply->remove_workers(ware, 1);


	return worker;
}


/*
===============
Warehouse::launch_soldier

Start a soldier or certain level. The soldier will be assigned a job by the caller.
===============
*/
Soldier* Warehouse::launch_soldier
	(Game * g, Ware_Index const ware, Requeriments* r)
{
	assert(m_supply->stock_workers(ware));

	Soldier* soldier;

	const Worker_Descr & workerdescr = *owner().tribe().get_worker_descr(ware);
	//  look if we got one of those in stock
	const std::string & workername = workerdescr.name();
	std::vector<Object_Ptr>::iterator i = m_incorporated_workers.begin();
	for (; i != m_incorporated_workers.end(); ++i)
	{
		if (static_cast<Worker*>(i->get(g))->name()==workername)
		{
			soldier = static_cast<Soldier*>(i->get(g));
			if
				(not r
				 or
				 r->check
				 (soldier->get_level(atrHP),
				  soldier->get_level(atrAttack),
				  soldier->get_level(atrDefense),
				  soldier->get_level(atrEvade)))
				break;
			else
				continue;
		}
	}

	soldier = 0;
	if (i == m_incorporated_workers.end()) {
		//  None found, create a new one (if available).
		soldier = &dynamic_cast<Soldier &>
			(workerdescr.create(*g, owner(), *this, m_position));
	} else {
		//  one found, make him available
		soldier = dynamic_cast<Soldier *>(i->get(g));
		soldier->reset_tasks(g); // Forget everything you did
		soldier->mark(false);
		soldier->set_location(this);
		m_incorporated_workers.erase(i);
	}

	m_supply->remove_workers(ware, 1);

	return soldier;
}



/*
===============
Warehouse::mark_as_used

Mark a soldier as used by a request.
===============
*/
void Warehouse::mark_as_used
	(Game * g, Ware_Index const  ware, Requeriments * const r)
{
	assert(m_supply->stock_workers(ware));

	Worker_Descr* workerdescr;

	workerdescr = get_owner()->tribe().get_worker_descr(ware);
	//  look if we got one of those in stock
	std::string workername=workerdescr->name();
	std::vector<Object_Ptr>::iterator i;
	for
		(i = m_incorporated_workers.begin();
		 i != m_incorporated_workers.end();
		 ++i)
	{
		if (static_cast<Worker*>(i->get(g))->name()==workername)
		{
			Soldier & soldier = dynamic_cast<Soldier &>(*i->get(g));
			if (!soldier.is_marked())
			{
				if
					(not r
					 or
					 r->check
					 (soldier.get_level(atrHP),
					  soldier.get_level(atrAttack),
					  soldier.get_level(atrDefense),
					  soldier.get_level(atrEvade)))
					break;
				else
					continue;
			}
		}
	}

	if (i != m_incorporated_workers.end())
		// one found
		dynamic_cast<Soldier &>(*i->get(g)).mark(true);
}


/*
===============
This is the opposite of launch_worker: destroy the worker and add the
appropriate ware to our warelist
===============
*/
void Warehouse::incorporate_worker(Game* g, Worker* w)
{
	assert(w->get_owner() == get_owner());

	Ware_Index index = get_owner()->tribe().worker_index(w->name().c_str());
	WareInstance* item = w->fetch_carried_item(g); // rescue an item

	//  We remove carriers, but we keep other workers around.
	if (dynamic_cast<Carrier const *>(w)) {
		w->remove(g);
		w = 0;
	} else {
		//  This is to prevent having soldiers that only are used one time and
		//  become allways 'marked'
		if (upcast(Soldier, soldier, w))
			soldier->mark(false);

		sort_worker_in(g, w->name(), w);
		w->set_location(0); // No more in a economy
		w->start_task_idle(g, 0, -1); // bind the worker into this house, hide him on the map
	}

	m_supply->add_workers(index, 1);

	if (item)
		incorporate_item(g, item);
}

/*
 * Sort the worker into the right position in m_incorporated_workers
 */
void Warehouse::sort_worker_in(Editor_Game_Base* g, std::string workername, Worker* w) {
	//  We insert this worker, but to keep some consistency in ordering, we tell
	//  him where to insert.

	std::vector<Object_Ptr>::iterator i = m_incorporated_workers.begin();

	while
		(i!= m_incorporated_workers.end()
		 &&
		 workername <= dynamic_cast<Worker &>(*i->get(g)).name())
		++i;
	if (i == m_incorporated_workers.end()) {
		m_incorporated_workers.insert(i, w);
		return;
	}

	while
		(i != m_incorporated_workers.end()
		 &&
		 w->get_serial() <= dynamic_cast<Worker &>(*i->get(g)).get_serial())
		++i;

	m_incorporated_workers.insert(i, w);
}

/*
===============
Create an instance of a ware and make sure it gets carried out of the warehouse.
===============
*/
WareInstance & Warehouse::launch_item(Game * game, Ware_Index const ware) {
	// Create the item
	WareInstance & item =
		*new WareInstance(ware, owner().tribe().get_ware_descr(ware));
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
void Warehouse::do_launch_item(Game * game, WareInstance & item)
{
	// Create a carrier
	const Tribe_Descr & tribe = owner().tribe();
	Ware_Index const carrierid = tribe.worker_index("carrier");
	const Worker_Descr & workerdescr = *tribe.get_worker_descr(carrierid);

	Worker & worker = workerdescr.create(*game, owner(), *this, m_position);

	// Yup, this is cheating.
	if (m_supply->stock_workers(carrierid))
		m_supply->remove_workers(carrierid, 1);

	// Setup the carrier
	worker.start_task_dropoff(game, &item);
}


/*
===============
Warehouse::incorporate_item

Swallow the item, adding it to out inventory.
===============
*/
void Warehouse::incorporate_item(Game* g, WareInstance* item)
{
	item->destroy(g);

	m_supply->add_wares(item->descr_index(), 1);
}


/*
===============
Warehouse::idle_request_cb [static]

Called when a transfer for one of the idle Requests completes.
===============
*/
void Warehouse::idle_request_cb
(Game     *       game,
 Request  *,
 Ware_Index const ware,
 Worker   * const w,
 void     * const data)
{
	if (w)
		w->schedule_incorporate(game);
	else
		static_cast<Warehouse *>(data)->m_supply->add_wares(ware, 1);
}


/*
===============
Warehouse_Descr::create_object
===============
*/
Building * Warehouse_Descr::create_object() const
{return new Warehouse(*this);}

/*
===============
Warehouse::get_soldiers_passing
===============
*/
int32_t Warehouse::get_soldiers_passing
	(Game * game, Ware_Index const w, Requeriments * const r)
{
	int32_t number = 0;

	log ("Warehouse::get_soldiers_passing :");

	assert(m_supply->stock_workers(w));

	for //  look if we got one of those in stock of
		(std::vector<Object_Ptr>::iterator i = m_incorporated_workers.begin();
		 i != m_incorporated_workers.end();
		 ++i)
		if (upcast(Soldier, soldier, i->get(game))) {

			// Its a marked soldier, we cann't supply it !
			if (!soldier->is_marked()) {
				if (r) {
					if
						(r->check
						 (soldier->get_level(atrHP),
						  soldier->get_level(atrAttack),
						  soldier->get_level(atrDefense),
						  soldier->get_level(atrEvade)))
					{
						log ("+");
						++number;
					}
				}
				else
				{
					log ("+");
					++number;
				}
			}
			else
				log ("M");
		}
	log ("\n");
	return number;
}


/*
===============
Warehouse::can_create_worker
===============
*/
bool Warehouse::can_create_worker(Game *, Ware_Index const worker) {
	if (m_supply->get_workers().get_nrwareids() <= worker.value())
		throw wexception
			("Worker type %d doesn't exists! (max is %d)",
			 worker.value(), m_supply->get_workers().get_nrwareids());

	const Tribe_Descr & tribe = owner().tribe();
	if (Worker_Descr const * const w_desc = tribe.get_worker_descr(worker)) {
		bool enought_wares;

		// First watch if we can build it
		if (!w_desc->get_buildable())
			return false;
		enought_wares = true;

		// Now see if we have the resources
		const char & worker_name = *w_desc->name().c_str();
		const Worker_Descr::BuildCost & buildcost = w_desc->get_buildcost();
		const Worker_Descr::BuildCost::const_iterator buildcost_end =
			buildcost.end();
		for
			(Worker_Descr::BuildCost::const_iterator it = buildcost.begin();
			 it != buildcost.end();
			 ++it)
		{
			const char * input_name = it->name.c_str();
			if (Ware_Index id_w = tribe.ware_index(input_name)) {
				if (m_supply->stock_wares(id_w) < it->amount) {
					molog
						(" %s: Need more %s for creation\n",
						 &worker_name, input_name);
					enought_wares = false;
				}
			} else {
				input_name = it->name.c_str();
				id_w = tribe.worker_index(input_name);
				if (m_supply->stock_workers(id_w) < it->amount) {
					molog
						(" %s: Need more %s for creation\n",
						 &worker_name, input_name);
					enought_wares = false;
				}
			}
		}
		return enought_wares;
	}
	else
		throw wexception
			("Can not create worker of desired type : %d", worker.value());
}

/*
=============
Warehouse::create_worker
+=============
*/
void Warehouse::create_worker(Game * game, Ware_Index const worker) {
	if (!can_create_worker (game, worker))
		throw wexception
			("Warehouse::create_worker WE CANN'T CREATE A %u WORKER",
			 worker.value());

	const Tribe_Descr & tribe = owner().tribe();
	if (const Worker_Descr * const w_desc = tribe.get_worker_descr(worker)) {
		const Worker_Descr::BuildCost & buildcost = w_desc->get_buildcost();
		const Worker_Descr::BuildCost::const_iterator buildcost_end =
			buildcost.end();
		for
			(Worker_Descr::BuildCost::const_iterator it = buildcost.begin();
			 it != buildcost.end();
			 ++it)
		{
			const char & material_name = *it->name.c_str();
			int32_t id_w = tribe.get_ware_index(&material_name);
			if (id_w != -1) remove_wares(id_w, it->amount);
			else {
				id_w = tribe.get_safe_worker_index(&material_name);
				remove_workers(id_w, it->amount);
			}
		}

		incorporate_worker
			(game, &w_desc->create(*game, owner(), *this, m_position));

		molog (" We have created a(n) %s\n", w_desc->name().c_str());

	} else
		throw wexception
			("Can not create worker of desired type : %d", worker.value());


}

/// Down here, only military methods !! ;)

bool Warehouse::has_soldiers()
{
	const Editor_Game_Base & egbase = owner().egbase();
	const std::vector<Object_Ptr>::const_iterator end =
		m_incorporated_workers.end();
	for
		(std::vector<Object_Ptr>::const_iterator it =
		 m_incorporated_workers.begin();
		 it != end;
		 ++it)
		if (dynamic_cast<const Soldier *>(it->get(&egbase))) return true;
	return false;
}

// A warhouse couldn't be conquered, this building is destroyed ...
void Warehouse::conquered_by (Player* pl)
{
	molog ("Warehouse::conquered_by- ");
	assert (pl);
	molog ("destroying\n");
	cleanup(&pl->egbase());
}

};
