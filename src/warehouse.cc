/*
 * Copyright (C) 2002-2004 by Widelands Development Team
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

#include "editor_game_base.h"
#include "error.h"
#include "game.h"
#include "player.h"
#include "profile.h"
#include "transport.h"
#include "warehouse.h"
#include "wexception.h"
#include "worker.h"


static const int CARRIER_SPAWN_INTERVAL = 2500;


/*
WarehouseSupply is the implementation of Supply that is used by Warehouses.
It also manages the list of wares in the warehouse.
*/
class WarehouseSupply : public Supply {
public:
	WarehouseSupply(Warehouse* wh);
	virtual ~WarehouseSupply();

	void set_economy(Economy* e);

	const WareList &get_wares() const { return m_wares; }
	int stock(int id) const { return m_wares.stock(id); }
	void add_wares(int id, int count);
	void remove_wares(int id, int count);

public: // Supply implementation
	virtual PlayerImmovable* get_position(Game* g);
	virtual int get_amount(Game* g, int ware);
	virtual bool is_active(Game* g);

	virtual WareInstance* launch_item(Game* g, int ware);
	virtual Worker* launch_worker(Game* g, int ware);

private:
	Economy*		m_economy;
	WareList		m_wares;
	Warehouse*	m_warehouse;
};


/*
===============
WarehouseSupply::WarehouseSupply

Initialize the supply
===============
*/
WarehouseSupply::WarehouseSupply(Warehouse* wh)
{
	m_warehouse = wh;
	m_economy = 0;
}


/*
===============
WarehouseSupply::~WarehouseSupply

Destroy the supply.
===============
*/
WarehouseSupply::~WarehouseSupply()
{
	if (m_economy) {
		log(
			"WarehouseSupply::~WarehouseSupply: "
			"Warehouse %u still belongs to an economy",
			m_warehouse->get_serial());
		set_economy(0);
	}

	// We're removed from the Economy. Therefore, the wares can simply
	// be cleared out. The global inventory will be okay.
	m_wares.clear();
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
		for(int i = 0; i < m_wares.get_nrwareids(); i++) {
			if (m_wares.stock(i)) {
				m_economy->remove_wares(i, m_wares.stock(i));
				m_economy->remove_supply(i, this);
			}
		}
	}

	m_economy = e;

	if (m_economy) {
		for(int i = 0; i < m_wares.get_nrwareids(); i++) {
			if (m_wares.stock(i)) {
				m_economy->add_wares(i, m_wares.stock(i));
				m_economy->add_supply(i, this);
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
void WarehouseSupply::add_wares(int id, int count)
{
	if (!count)
		return;

	if (!m_wares.stock(id))
		m_economy->add_supply(id, this);

	m_economy->add_wares(id, count);
	m_wares.add(id, count);
}


/*
===============
WarehouseSupply::remove_wares

Remove wares and update the economy.
===============
*/
void WarehouseSupply::remove_wares(int id, int count)
{
	if (!count)
		return;

	m_wares.remove(id, count);
	m_economy->remove_wares(id, count);

	if (!m_wares.stock(id))
		m_economy->remove_supply(id, this);
}


/*
===============
WarehouseSupply::get_position

Return the position of the Supply, i.e. the owning Warehouse.
===============
*/
PlayerImmovable* WarehouseSupply::get_position(Game* g)
{
	return m_warehouse;
}


/*
===============
WarehouseSupply::get_amount

Return our stock of the given ware.
===============
*/
int WarehouseSupply::get_amount(Game* g, int ware)
{
	return m_wares.stock(ware);
}


/*
===============
WarehouseSupply::is_active

Warehouse supplies are never active.
===============
*/
bool WarehouseSupply::is_active(Game* g)
{
	return false;
}


/*
===============
WarehouseSupply::launch_item

Launch a ware as item.
===============
*/
WareInstance* WarehouseSupply::launch_item(Game* g, int ware)
{
	assert(m_wares.stock(ware));

	return m_warehouse->launch_item(g, ware);
}


/*
===============
WarehouseSupply::launch_worker

Launch a ware as worker.
===============
*/
Worker* WarehouseSupply::launch_worker(Game* g, int ware)
{
	assert(m_wares.stock(ware));

	return m_warehouse->launch_worker(g, ware);
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
Warehouse_Descr::Warehouse_Descr(Tribe_Descr* tribe, const char* name)
	: Building_Descr(tribe, name)
{
	m_subtype = Subtype_Normal;
	m_conquers = 0;
}

/*
===============
Warehouse_Descr::parse

Parse the additional warehouse settings from the given profile and directory
===============
*/
void Warehouse_Descr::parse(const char* directory, Profile* prof,
	const EncodeData* encdata)
{
	add_attribute(Map_Object::WAREHOUSE);

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
		throw wexception(
			"Unsupported warehouse subtype '%s'. Possible values: none, HQ, port",
			string);

	if (m_subtype == Subtype_HQ)
		m_conquers = global->get_int("conquers");
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
Warehouse::Warehouse(Warehouse_Descr* descr)
	: Building(descr)
{
	m_supply = new WarehouseSupply(this);
	m_next_carrier_spawn = 0;
}


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

	if (get_descr()->get_subtype() == Warehouse_Descr::Subtype_HQ)
		gg->conquer_area(get_owner()->get_player_number(), m_position, get_descr());

	for(int i = 0; i < gg->get_nrwares(); i++) {
		Request* req = new Request(this, i, &Warehouse::idle_request_cb, this);

		req->set_idle(true);

		m_requests.push_back(req);
	}

	if (gg->is_game()) {
		Game* g=static_cast<Game*>(gg);

		m_next_carrier_spawn = schedule_act(g, CARRIER_SPAWN_INTERVAL);
	}
}


/*
===============
Warehouse::cleanup

Destroy the warehouse.
===============
*/
void Warehouse::cleanup(Editor_Game_Base* g)
{
	while(m_requests.size()) {
		Request* req = m_requests[m_requests.size()-1];

		m_requests.pop_back();

		delete req;
	}

	// TODO: un-conquer the area?
	Building::cleanup(g);
}


/*
===============
Warehouse::act

Act regularly to create carriers. According to intelligence, this is some
highly advanced technology. Not only do the settlers have no problems with
birth control, they don't even need anybody to procreate. They must have
built-in DNA samples in those warehouses. And what the hell are they doing,
killing useless tribesmen! The Borg? Or just like Soylent Green?
Or maybe I should just stop writing comments that late at night ;-)
===============
*/
void Warehouse::act(Game* g, uint data)
{
	if (g->get_gametime() - m_next_carrier_spawn >= 0)
	{
		int id = g->get_safe_ware_id("carrier");
		int stock = m_supply->stock(id);
		int tdelta = CARRIER_SPAWN_INTERVAL;

		if (stock < 100) {
			tdelta -= 4*(100 - stock);
			create_wares(id, 1);
		} else if (stock > 100) {
			tdelta -= 4*(stock - 100);
			if (tdelta < 10)
				tdelta = 10;
			destroy_wares(id, 1);
		}

		m_next_carrier_spawn = schedule_act(g, tdelta);
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

	for(uint i = 0; i < m_requests.size(); i++)
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
Warehouse::create_wares

Magically create wares in this warehouse. Updates the economy accordingly.
===============
*/
void Warehouse::create_wares(int id, int count)
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
void Warehouse::destroy_wares(int id, int count)
{
	assert(get_economy());

	m_supply->remove_wares(id, count);
}


/*
===============
Warehouse::fetch_from_flag

Launch a carrier to fetch an item from our flag.
===============
*/
bool Warehouse::fetch_from_flag(Game* g)
{
	int carrierid;
	Worker* worker;

	carrierid = g->get_ware_id("carrier");

	if (!m_supply->stock(carrierid)) // XXX yep, let's cheat
		create_wares(carrierid, 1);

	worker = launch_worker(g, carrierid);
	worker->start_task_fetchfromflag(g);

	return true;
}


/*
===============
Warehouse::launch_worker

Start a worker of a given type. The worker will be assigned a job by the caller.
===============
*/
Worker* Warehouse::launch_worker(Game* g, int ware)
{
	assert(m_supply->stock(ware));

	Ware_Descr* waredescr;
	Worker_Descr* workerdescr;
	Worker* worker;

	waredescr = g->get_ware_description(ware);
	assert(waredescr->is_worker());

	workerdescr =
		((Worker_Ware_Descr*)waredescr)->get_worker(get_owner()->get_tribe());

	worker = workerdescr->create(g, get_owner(), this, m_position);

	m_supply->remove_wares(ware, 1);

	return worker;
}


/*
===============
Warehouse::incorporate_worker

This is the opposite of launch_worker: destroy the worker and add the
appropriate ware to our warelist
===============
*/
void Warehouse::incorporate_worker(Game* g, Worker* w)
{
	assert(w->get_owner() == get_owner());

	int ware = w->get_ware_id();
	WareInstance* item = w->fetch_carried_item(g); // rescue an item

	w->remove(g);

	m_supply->add_wares(ware, 1);

	if (item)
		incorporate_item(g, item);
}


/*
===============
Warehouse::launch_item

Create an instance of a ware and make sure it gets carried out of the warehouse.
===============
*/
WareInstance* Warehouse::launch_item(Game* g, int ware)
{
	WareInstance* item;

	// Create the item
	item = new WareInstance(ware);
	item->init(g);

	m_supply->remove_wares(ware, 1);

	do_launch_item(g, item);

	return item;
}


/*
===============
Warehouse::do_launch_item

Get a carrier to actually move this item out of the warehouse.
===============
*/
void Warehouse::do_launch_item(Game* g, WareInstance* item)
{
	int carrierid;
	Ware_Descr* waredescr;
	Worker_Descr* workerdescr;
	Worker* worker;

	// Create a carrier
	carrierid = g->get_ware_id("carrier");
	waredescr = g->get_ware_description(carrierid);
	workerdescr =
		((Worker_Ware_Descr*)waredescr)->get_worker(get_owner()->get_tribe());

	worker = workerdescr->create(g, get_owner(), this, m_position);

	// Yup, this is cheating.
	if (m_supply->stock(carrierid))
		m_supply->remove_wares(carrierid, 1);

	// Setup the carrier
	worker->start_task_dropoff(g, item);
}


/*
===============
Warehouse::incorporate_item

Swallow the item, adding it to out inventory.
===============
*/
void Warehouse::incorporate_item(Game* g, WareInstance* item)
{
	int ware = item->get_ware();

	item->destroy(g);

	m_supply->add_wares(ware, 1);
}


/*
===============
Warehouse::idle_request_cb [static]

Called when a transfer for one of the idle Requests completes.
===============
*/
void Warehouse::idle_request_cb(Game* g, Request* rq, int ware, Worker* w,
	void* data)
{
	Warehouse* wh = (Warehouse*)data;

	if (w)
		w->schedule_incorporate(g);
	else
		wh->m_supply->add_wares(ware, 1);
}


/*
===============
Warehouse_Descr::create_object
===============
*/
Building* Warehouse_Descr::create_object()
{
	return new Warehouse(this);
}
