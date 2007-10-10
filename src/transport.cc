/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

/**
 * @file
 * The entire transport subsystem comes into this file.
 *
 * What does _not_ belong in here: road renderer, client-side road building.
 *
 * What _does_ belong in here:
 * Flags, Roads, the logic behind ware pulls and pushes.
 *
 * \todo split this up into two files per class (.h and .cc)
*/

#include "transport.h"

#include "building.h"
#include "carrier.h"
#include "editor_game_base.h"
#include "fileread.h"
#include "filewrite.h"
#include "game.h"
#include "instances.h"
#include "log.h"
#include "player.h"
#include "soldier.h"
#include "tribe.h"
#include <vector>
#include "warehouse.h"
#include "wexception.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"
#include "worker.h"

#include <cstdarg>
#include <stdio.h>


Map_Object_Descr g_flag_descr;


/**
 * Whenever a WareInstance is idle, it issues an IdleWareSupply.
*/
class IdleWareSupply : public Supply {
public:
	IdleWareSupply(WareInstance* ware);
	virtual ~IdleWareSupply();

	void set_economy(Economy* e);

public: // implementation of Supply
	virtual PlayerImmovable* get_position(Game* g);
	virtual int32_t get_amount(const int32_t ware) const;
	virtual bool is_active() const throw ();

	virtual WareInstance* launch_item(Game* g, int32_t ware);
	virtual Worker* launch_worker(Game* g, int32_t ware) __attribute__ ((noreturn));

	virtual Soldier* launch_soldier(Game* g, int32_t ware, Requeriments* req) __attribute__ ((noreturn));
	virtual int32_t get_passing_requeriments (Game* g, int32_t ware, Requeriments* r) __attribute__ ((noreturn));
	virtual void mark_as_used (Game* g, int32_t ware, Requeriments* r) __attribute__ ((noreturn));
private:
	WareInstance * m_ware;
	Economy      * m_economy;
};

/**
 * Initialize the Supply and update the economy.
*/
IdleWareSupply::IdleWareSupply(WareInstance* ware) : m_ware(ware), m_economy(0)
{set_economy(m_ware->get_economy());}

/**
 * Cleanup.
*/
IdleWareSupply::~IdleWareSupply()
{
	set_economy(0);
}

/**
 * Add/remove self from economies as necessary.
*/
void IdleWareSupply::set_economy(Economy* e)
{
	if (e == m_economy)
		return;

	const Item_Ware_Descr::Index ware = m_ware->descr_index();

	if (m_economy)
		m_economy->remove_ware_supply(ware, this);

	m_economy = e;

	if (m_economy)
		m_economy->add_ware_supply(ware, this);
}

/**
 * Figure out the player immovable that this ware belongs to.
*/
PlayerImmovable* IdleWareSupply::get_position(Game* g)
{
	Map_Object* loc = m_ware->get_location(g);

	if (loc->get_type() >= Map_Object::BUILDING)
		return (PlayerImmovable*)loc;

	if (loc->has_attribute(Map_Object::WORKER))
		return ((Worker*)loc)->get_location(g);

	return 0;
}

int32_t IdleWareSupply::get_amount(const int32_t ware) const
{
	return (ware == m_ware->descr_index()) ? 1 : 0;
}

bool IdleWareSupply::is_active()  const throw ()
{
	return not m_ware->is_moving();
}

/**
 * The item is already "launched", so we only need to return it.
*/
WareInstance* IdleWareSupply::launch_item(Game *, int32_t ware) {
	if (ware != m_ware->descr_index())
		throw wexception
			("IdleWareSupply: ware(%u) (type = %i) requested for %i",
			 m_ware->get_serial(),
			 m_ware->descr_index(),
			 ware);

	return m_ware;
}

Worker* IdleWareSupply::launch_worker(Game *, int32_t)
{
	throw wexception("IdleWareSupply::launch_worker makes no sense");
}

Soldier* IdleWareSupply::launch_soldier(Game *, int32_t, Requeriments *)
{
	throw wexception("IdleWareSupply::launch_soldier makes no sense");
}

int32_t IdleWareSupply::get_passing_requeriments(Game *, int32_t, Requeriments *)
{
	throw wexception("IdleWareSupply::get_passing_requeriments makes no sense");
}

void IdleWareSupply::mark_as_used (Game *, int32_t, Requeriments *) {
	// By now, wares have not need to have this method
	throw wexception("IdleWareSupply::mark_as_used makes no sense");
}


/*
==============================================================================

WareInstance IMPLEMENTATION

==============================================================================
*/


WareInstance::WareInstance
(const Item_Ware_Descr::Index i, const Item_Ware_Descr * const ware_descr)
:
Map_Object   (ware_descr),
m_economy    (0),
m_descr_index(i),
m_ware_descr (ware_descr),
m_supply     (0),
m_transfer   (0)
{}

WareInstance::~WareInstance()
{
	if (m_supply) {
		molog("Ware %u still has supply %p\n", m_descr_index, m_supply);
		delete m_supply;
	}
}

int32_t WareInstance::get_type() const throw ()
{
	return WARE;
}

void WareInstance::init(Editor_Game_Base* g)
{
	Map_Object::init(g);
}

void WareInstance::cleanup(Editor_Game_Base* g)
{
	//molog("WareInstance::cleanup\n");

	Map_Object* location = m_location.get(g);

	// Unlink from our current location, if necessary
	if (location && location->get_type() == FLAG) {
		Flag* flag = (Flag*)location;

		flag->remove_item(g, this);
	}

		delete m_supply;
		m_supply = 0;

	if (Game * const game = dynamic_cast<Game *>(g)) {
		cancel_moving();
		set_location(game, 0);
	}

	//molog("  done\n");

	Map_Object::cleanup(g);
}

/**
 * Ware accounting
*/
void WareInstance::set_economy(Economy* e)
{
	if (m_economy == e)
		return;

	if (m_economy) m_economy->remove_wares(m_descr_index, 1);

	m_economy = e;
	if (m_supply)
		m_supply->set_economy(e);

	if (m_economy) m_economy->add_wares(m_descr_index, 1);
}

/**
 * Change the current location.
 * Once you've assigned a ware to its new location, you usually have to call
 * \ref update() as well.
*/
void WareInstance::set_location(Editor_Game_Base* g, Map_Object* location)
{
	Map_Object* oldlocation = m_location.get(g);

	if (oldlocation == location)
		return;

	m_location = location;

	if (location)
	{
		Economy* eco = 0;

		if (location->get_type() >= BUILDING)
			eco = ((PlayerImmovable*)location)->get_economy();
		else if (location->has_attribute(WORKER))
			eco = ((Worker*)location)->get_economy();

		if (oldlocation && get_economy()) {
			if (get_economy() != eco)
				throw wexception("WareInstance::set_location() implies change of economy");
		} else {
			set_economy(eco);
		}
	}
	else
	{
		set_economy(0);
	}
}

/**
 * Callback for the return-to-warehouse timer.
*/
void WareInstance::act(Game *, uint32_t)
{}

/**
 * Performs the state updates necessary for the current location:
 * - if it's a building, acknowledge the Request or incorporate into warehouse
 * - if it's a flag and we have no request, start the return to warehouse timer
 * and issue a Supply
 *
 * \note \ref update() may result in the deletion of this object.
*/
void WareInstance::update(Game* g)
{
	Map_Object* loc = m_location.get(g);

	if (!m_ware_descr) // Upsy, we're not even intialized. Happens on load
		return;

	// Reset our state if we're not on location or outside an economy
	if (!loc || !get_economy()) {
		cancel_moving();
		return;
	}

	// Update whether we have a Supply or not
	if (!m_transfer || m_transfer->is_idle()) {
		if (!m_supply)
			m_supply = new IdleWareSupply(this);
	} else {
		delete m_supply;
		m_supply = 0;
	}

	// Deal with transfers
	if (m_transfer)
	{
		bool success;
		PlayerImmovable* location;
		PlayerImmovable* nextstep;

		if (loc->get_type() >= BUILDING)
			location = (PlayerImmovable*)loc;
		else
			return; // wait

		nextstep = m_transfer->get_next_step(location, &success);
		m_transfer_nextstep = nextstep;

		if (!nextstep) {
			Transfer* t = m_transfer;

			m_transfer = 0;

			if (success) {
				t->has_finished();
				return;
			} else {
				t->has_failed();

				cancel_moving();
				update(g);
				return;
			}
		}

		switch (location->get_type()) {
		case BUILDING:
			if (nextstep != location->get_base_flag())
				throw wexception("MO(%u): ware: move from building to non-baseflag", get_serial());

			// There are some situations where we might end up in a warehouse as
			// part of a requested route, and we need to move out of it again, e.g.:
			//  - we were requested just when we were being carried into the warehouse
			//  - we were carried into a harbour/warehouse to be shipped across the sea,
			//    but a better, land-based route has been found
			if (location->has_attribute(WAREHOUSE))
			{
				Warehouse* wh = (Warehouse*)location;

				wh->do_launch_item(g, this);
				return;
			}

			throw wexception
				("MO(%u): ware: can't move from building %u to %u (not a "
				 "warehouse)",
				 get_serial(), location->get_serial(), nextstep->get_serial());

		case FLAG:
			if (nextstep->get_type() == BUILDING && nextstep->get_base_flag() != location)
				((Flag*)location)->call_carrier(g, this, nextstep->get_base_flag());
			else
				((Flag*)location)->call_carrier(g, this, nextstep);
			break;
		}

		return;
	}
}

/**
 * Set ware state so that it follows the given transfer.
*/
void WareInstance::set_transfer(Game* g, Transfer* t)
{
	// Reset current transfer
	if (m_transfer) {
		m_transfer->has_failed();
		m_transfer = 0;
	}

	// Set transfer state
	m_transfer = t;

	update(g);
}

/**
 * The transfer has been cancelled, just stop moving.
*/
void WareInstance::cancel_transfer(Game* g)
{
	m_transfer = 0;

	update(g);
}

/**
 * We are moving when there's a transfer, it's that simple.
*/
bool WareInstance::is_moving() const throw ()
{
	return m_transfer;
}

/**
 * Call this function if movement + potential request need to be cancelled for
 * whatever reason.
*/
void WareInstance::cancel_moving() {
	if (m_transfer) {
		m_transfer->has_failed();
		m_transfer = 0;
	}
}

/**
 * Return the next flag we should be moving to, or the final target if the route
 * has been completed successfully.
*/
PlayerImmovable* WareInstance::get_next_move_step(Game* g)
{
	if (m_transfer)
		return (PlayerImmovable*)m_transfer_nextstep.get(g);
	else
		return 0;
}



/*
==============================================================================

Flag IMPLEMENTATION

==============================================================================
*/

/**
 * Create the flag. Initially, it doesn't have any attachments.
*/
Flag::Flag() :
PlayerImmovable(g_flag_descr),
m_anim(0),
m_building(0),
m_item_capacity(8),
m_item_filled(0),
m_items(new PendingItem[m_item_capacity]),
m_always_call_for_flag(0)
{
	for (uint32_t i = 0; i < 6; ++i) m_roads[i] = 0;
}

/**
 * Shouldn't be necessary to do anything, since die() always calls
 * cleanup() first.
*/
Flag::~Flag()
{
	if (m_item_filled)
		log("Flag: ouch! items left\n");
	delete[] m_items;

	if (m_building)
		log("Flag: ouch! building left\n");

	if (m_flag_jobs.size())
		log("Flag: ouch! flagjobs left\n");

	for (int32_t i = 0; i < 6; i++)
		if (m_roads[i])
			log("Flag: ouch! road left\n");
}

/**
 * Create a flag at the given location
*/
Flag *Flag::create(Editor_Game_Base *g, Player *owner, Coords coords)
{
	Road *road = 0;
	BaseImmovable *imm = g->get_map()->get_immovable(coords);

	Flag *flag = new Flag();
	flag->set_owner(owner);
	flag->m_position = coords;

	if (imm && imm->get_type() == Map_Object::ROAD)
	{
		// we split a road
		road = (Road*)imm;
		road->get_economy()->add_flag(flag);
	}
	else
	{
		// a new, standalone flag is created
		(new Economy(owner))->add_flag(flag);
	}

	if (road)
		road->presplit(g, coords);
	flag->init(g);
	if (road)
		road->postsplit(g, flag);
	return flag;
}

int32_t Flag::get_type() const throw ()
{
	return FLAG;
}

int32_t Flag::get_size() const throw ()
{
	return SMALL;
}

bool Flag::get_passable() const throw ()
{
	return true;
}

Flag *Flag::get_base_flag()
{
	return this;
}

/**
 * Call this only from Economy code!
*/
void Flag::set_economy(Economy *e)
{
	Economy *old = get_economy();

	if (old == e)
		return;

	PlayerImmovable::set_economy(e);

	for (int32_t i = 0; i < m_item_filled; i++)
		m_items[i].item->set_economy(e);

	if (m_building)
		m_building->set_economy(e);

	for (std::list<FlagJob>::const_iterator it = m_flag_jobs.begin(); it != m_flag_jobs.end(); ++it)
		it->request->set_economy(e);

	for (int32_t i = 0; i < 6; i++) {
		if (m_roads[i])
			m_roads[i]->set_economy(e);
	}
}

/**
 * Call this only from the Building init!
*/
void Flag::attach_building(Editor_Game_Base *g, Building *building)
{
	assert(!m_building || m_building==building);

	m_building = building;

	const Map  & map = g->map();
	g->set_road
		(map.get_fcoords(map.tl_n(m_position)), Road_SouthEast, Road_Normal);

	m_building->set_economy(get_economy());
}

/**
 * Call this only from the Building cleanup!
*/
void Flag::detach_building(Editor_Game_Base *g)
{
	assert(m_building);

	m_building->set_economy(0);

	const Map & map = g->map();
	g->set_road
		(map.get_fcoords(map.tl_n(m_position)), Road_SouthEast, Road_None);

	m_building = 0;
}

/**
 * Call this only from the Road init!
*/
void Flag::attach_road(int32_t dir, Road *road)
{
	assert(!m_roads[dir-1] || m_roads[dir-1]==road);

	m_roads[dir-1] = road;
	m_roads[dir-1]->set_economy(get_economy());
}

/**
 * Call this only from the Road init!
*/
void Flag::detach_road(int32_t dir)
{
	assert(m_roads[dir-1]);

	m_roads[dir-1]->set_economy(0);
	m_roads[dir-1] = 0;
}

/**
 * Return neighbouring flags.
*/
void Flag::get_neighbours(Neighbour_list *neighbours)
{
	for (int32_t i = 0; i < 6; i++) {
		Road *road = m_roads[i];
		if (!road)
			continue;

		Neighbour n;
		n.road = road;
		n.flag = road->get_flag(Road::FlagEnd);
		if (n.flag != this)
			n.cost = road->get_cost(Road::FlagStart);
		else {
			n.flag = road->get_flag(Road::FlagStart);
			n.cost = road->get_cost(Road::FlagEnd);
		}

		assert(n.flag != this);
		neighbours->push_back(n);
	}

	// I guess this would be the place to add other ports if a port building
	// is attached to this flag
	// Or maybe other hosts of a carrier pigeon service, or a wormhole connection
	// point or whatever ;)
}

/**
 * Return the road that leads to the given flag.
*/
Road *Flag::get_road(Flag *flag)
{
	for (int32_t i = 0; i < 6; i++) {
		Road *road = m_roads[i];
		if (!road)
			continue;

		if (road->get_flag(Road::FlagStart) == flag || road->get_flag(Road::FlagEnd) == flag)
			return road;
	}

	return 0;
}

/**
 * Returns true if the flag can hold more items.
*/
bool Flag::has_capacity()
{
	return (m_item_filled < m_item_capacity);
}

/**
 * Signal the given bob by interrupting its task as soon as capacity becomes
 * free.
*/
void Flag::wait_for_capacity(Game *, Worker* bob)
{
	m_capacity_wait.push_back(bob);
}

void Flag::add_item(Game* g, WareInstance* item)
{
	PendingItem* pi;

	assert(m_item_filled < m_item_capacity);

	pi = &m_items[m_item_filled++];
	pi->item = item;
	pi->pending = false;
	pi->nextstep = 0;

	item->set_location(g, this);
	item->update(g); // will call call_carrier() if necessary
}

/**
 * \return true if an item is currently waiting for a carrier to the given Flag.
 *
 * \note Due to fetch_from_flag() semantics, this function makes no sense
 * for a  building destination.
*/
bool Flag::has_pending_item(Game *, Flag * dest) {
	int32_t i;

	for (i = 0; i < m_item_filled; i++) {
		if (!m_items[i].pending)
			continue;

		if (m_items[i].nextstep != dest)
			continue;

		return true;
	}

	return false;
}

/**
 * Called by carrier code to indicate that the carrier is moving to pick up an
 * item.
 * \return true if an item is actually waiting for the carrier.
*/
bool Flag::ack_pending_item(Game *, Flag * destflag) {
	int32_t i;

	for (i = 0; i < m_item_filled; i++) {
		if (!m_items[i].pending)
			continue;

		if (m_items[i].nextstep != destflag)
			continue;

		m_items[i].pending = false;
		return true;
	}

	return false;
}

/**
 * Wake one sleeper from the capacity queue.
*/
void Flag::wake_up_capacity_queue(Game* g)
{
	while (m_capacity_wait.size()) {
		Worker* w = (Worker*)m_capacity_wait[0].get(g);

		m_capacity_wait.erase(m_capacity_wait.begin());

		if (!w)
			continue;

		molog("Flag: wake up one from wait queue.\n");

		if (w->wakeup_flag_capacity(g, this))
			break;
	}
}

/**
 * Called by carrier code to retrieve one of the items on the flag that is meant
 * for that carrier.
 *
 * This function may return 0 even if \ref ack_pending_item() has already been
 * called successfully.
*/
WareInstance* Flag::fetch_pending_item(Game* g, PlayerImmovable* dest)
{
	WareInstance* item;
	int32_t best_index = -1;

	for (int32_t i = 0; i < m_item_filled; i++) {
		if (m_items[i].nextstep != dest)
			continue;

		// We prefer to retrieve items that have already been acked
		if (best_index < 0 || !m_items[i].pending)
			best_index = i;
	}

	if (best_index < 0)
		return 0;

	// move the other items up the list and return this one
	item = m_items[best_index].item;
	m_item_filled--;
	memmove(&m_items[best_index], &m_items[best_index+1], sizeof(m_items[0]) * (m_item_filled - best_index));

	item->set_location(g, 0);

	// wake up capacity wait queue
	wake_up_capacity_queue(g);

	return item;
}

/**
 * Force a removal of the given item from this flag.
 * Called by \ref WareInstance::cleanup()
*/
void Flag::remove_item(Editor_Game_Base* g, WareInstance* item)
{
	for (int32_t i = 0; i < m_item_filled; i++) {
		if (m_items[i].item != item)
			continue;

		m_item_filled--;
		memmove(&m_items[i], &m_items[i+1], sizeof(m_items[0]) * (m_item_filled - i));

		if (Game * const game = dynamic_cast<Game *>(g))
			wake_up_capacity_queue(game);

		return;
	}

	throw wexception("MO(%u): Flag::remove_item: item %u not on flag",
	                 get_serial(), item->get_serial());
}

/**
 * If nextstep is not null, a carrier will be called to move this item to
 * the given flag or building.
 *
 * If nextstep is null, the internal data will be reset to indicate that the
 * item isn't going anywhere right now.
 *
 * nextstep is compared with the cached data, and a new carrier is only called
 * if that data hasn't changed.
 *
 * This behaviour is overriden by m_always_call_for_step, which is set by
 * update_items() to ensure that new carriers are called when roads are
 * split, for example.
*/
void Flag::call_carrier(Game* g, WareInstance* item, PlayerImmovable* nextstep)
{
	PendingItem* pi = 0;
	int32_t i;

	// Find the PendingItem entry
	for (i = 0; i < m_item_filled; i++) {
		if (m_items[i].item != item)
			continue;

		pi = &m_items[i];
		break;
	}

	assert(pi);

	// Deal with the non-moving case quickly
	if (!nextstep) {
		pi->nextstep = 0;
		pi->pending = false;
		return;
	}

	// Find out whether we need to do anything
	if (pi->nextstep == nextstep && pi->nextstep != m_always_call_for_flag)
		return; // no update needed

	pi->nextstep = nextstep;
	pi->pending = false;

	// Deal with the building case
	if (nextstep == get_building())
	{
		molog("Flag::call_carrier(%u): Tell building to fetch this item\n", item->get_serial());

		if (!get_building()->fetch_from_flag(g)) {
			pi->item->cancel_moving();
			pi->item->update(g);
		}

		return;
	}

	// Deal with the normal (flag) case
	assert(nextstep->get_type() == FLAG);

	for (int32_t dir = 1; dir <= 6; dir++) {
		Road* road = get_road(dir);
		Flag* other;
		Road::FlagId flagid;

		if (!road)
			continue;

		if (road->get_flag(Road::FlagStart) == this) {
			flagid = Road::FlagStart;
			other = road->get_flag(Road::FlagEnd);
		} else {
			flagid = Road::FlagEnd;
			other = road->get_flag(Road::FlagStart);
		}

		if (other != nextstep)
			continue;

		// Yes, this is the road we want; inform it
		if (road->notify_ware(g, flagid))
			return;

		// If the road doesn't react to the ware immediately, we try other roads:
		// They might lead to the same flag!
	}

	// Nothing found, just let it be picked up by somebody
	pi->pending = true;
	return;
}

/**
 * Called whenever a road gets broken or split.
 * Make sure all items on this flag are rerouted if necessary.
 *
 * \note When two roads connect the same two flags, and one of these roads
 * is removed, this might cause the carrier(s) on the other road to
 * move unnecessarily. Fixing this could potentially be very expensive and
 * fragile.
 * A similar thing can happen when a road is split.
*/
void Flag::update_items(Game* g, Flag* other)
{
	m_always_call_for_flag = other;

	for (int32_t i = 0; i < m_item_filled; i++)
		m_items[i].item->update(g);

	m_always_call_for_flag = 0;
}

void Flag::init(Editor_Game_Base *g)
{
	PlayerImmovable::init(g);

	set_position(g, m_position);

	m_anim = owner().tribe().get_flag_anim();
	m_animstart = g->get_gametime();
}

/**
 * Detach building and free roads.
*/
void Flag::cleanup(Editor_Game_Base *g)
{
	//molog("Flag::cleanup\n");

	while (m_flag_jobs.size()) {
		delete m_flag_jobs.begin()->request;
		m_flag_jobs.erase(m_flag_jobs.begin());
	}

	while (m_item_filled) {
		WareInstance* item = m_items[--m_item_filled].item;

		item->set_location((Game*)g, 0);
		item->destroy((Game*)g);
	}

	//molog("  items destroyed\n");

	if (m_building) {
		m_building->remove(g); // immediate death
		assert(!m_building);
	}

	for (int32_t i = 0; i < 6; i++) {
		if (m_roads[i]) {
			m_roads[i]->remove(g); // immediate death
			assert(!m_roads[i]);
		}
	}

	get_economy()->remove_flag(this);

	unset_position(g, m_position);

	//molog("  done\n");

	PlayerImmovable::cleanup(g);
}

/**
 * Destroy the building as well.
 *
 * \note This is needed in addition to the call to m_building->remove() in
 * \ref Flag::cleanup(). This function is needed to ensure a fire is created
 * when a player removes a flag.
*/
void Flag::destroy(Editor_Game_Base* g)
{
	if (m_building) {
		m_building->destroy(g);
		assert(!m_building);
	}

	PlayerImmovable::destroy(g);
}

/**
 * Add a new flag job to request the worker with the given ID, and to execute
 * the given program once it's completed.
*/
void Flag::add_flag_job(Game *, int32_t workerware, std::string programname) {
	FlagJob j;

	j.request = new Request(this, workerware,
	                        &Flag::flag_job_request_callback, this, Request::WORKER);
	j.program = programname;

	m_flag_jobs.push_back(j);
}

/**
 * This function is called when one of the flag job workers arrives on
 * the flag. Give him his job.
*/
void Flag::flag_job_request_callback
(Game *, Request * rq, int32_t, Worker * w, void * data)
{
	Flag* flag = (Flag*)data;

	assert(w);

	for (std::list<FlagJob>::iterator it = flag->m_flag_jobs.begin(); it != flag->m_flag_jobs.end(); ++it) {
		if (it->request != rq)
			continue;

		delete rq;

		w->start_task_program(it->program);

		flag->m_flag_jobs.erase(it);
		return;
	}

	flag->molog("BUG: flag_job_request_callback: worker not found in list\n");
}

/*
==============================================================================

Road IMPLEMENTATION

==============================================================================
*/

// dummy instance because Map_Object needs a description
Map_Object_Descr g_road_descr;

/**
 * Most of the actual work is done in init.
*/
Road::Road() :
PlayerImmovable  (g_road_descr),
m_type           (0),
m_desire_carriers(0),
m_carrier_request(0)
{
	m_flags[0] = m_flags[1] = 0;
	m_flagidx[0] = m_flagidx[1] = -1;
}

/**
 * Most of the actual work is done in cleanup.
 */
Road::~Road()
{
	if (m_carrier_request) {
		log("Road::~Road: carrier request left\n");
		delete m_carrier_request;
	}
}

/**
 * Create a road between the given flags, using the given path.
*/
Road *Road::create(Editor_Game_Base *g, int32_t type, Flag *start, Flag *end, const Path &path)
{
	assert(start->get_position() == path.get_start());
	assert(end->get_position() == path.get_end());

	Road *r = new Road();
	r->set_owner(start->get_owner());
	r->m_type = type;
	r->m_flags[FlagStart] = start;
	r->m_flags[FlagEnd] = end;
	// m_flagidx is set when attach_road() is called, i.e. in init()
	r->set_path(g, path);
	r->init(g);

	return r;
}

int32_t Road::get_type() const throw ()
{
	return ROAD;
}

int32_t Road::get_size() const throw ()
{
	return SMALL;
}

bool Road::get_passable() const throw ()
{
	return true;
}

Flag *Road::get_base_flag()
{
	return m_flags[FlagStart];
}

/**
 * Return the cost of getting from fromflag to the other flag.
*/
int32_t Road::get_cost(FlagId fromflag)
{
	return m_cost[fromflag];
}

/**
 * Set the new path, calculate costs.
 * You have to set start and end flags before calling this function.
*/
void Road::set_path(Editor_Game_Base *g, const Path &path)
{
	assert(path.get_nsteps() >= 2);
	assert(path.get_start() == m_flags[FlagStart]->get_position());
	assert(path.get_end() == m_flags[FlagEnd]->get_position());

	m_path = path;
	g->get_map()->calc_cost(path, &m_cost[FlagStart], &m_cost[FlagEnd]);

	// Figure out where carriers should idle
	m_idle_index = path.get_nsteps() / 2;
}

/**
 * Add road markings to the map
*/
void Road::mark_map(Editor_Game_Base *g)
{
	Map *map = g->get_map();
	FCoords curf(m_path.get_start(), map->get_field(m_path.get_start()));

	const Path::Step_Vector::size_type nr_steps = m_path.get_nsteps();
	for (Path::Step_Vector::size_type steps = 0; steps <= nr_steps; ++steps) {
		if (steps > 0 && steps < m_path.get_nsteps())
			set_position(g, curf);

		// mark the road that leads up to this field
		if (steps > 0) {
			const Direction dir  = get_reverse_dir(m_path[steps - 1]);
			const Direction rdir = 2 * (dir - Map_Object::WALK_E);

			if (rdir <= 4) g->set_road(curf, rdir, m_type);
		}

		// mark the road that leads away from this field
		if (steps < m_path.get_nsteps()) {
			const Direction dir  = m_path[steps];
			const Direction rdir = 2 * (dir - Map_Object::WALK_E);

			if (rdir <= 4) g->set_road(curf, rdir, m_type);

			map->get_neighbour(curf, dir, &curf);
		}
	}
}

/**
 * Remove road markings from the map
*/
void Road::unmark_map(Editor_Game_Base *g)
{
	Map *map = g->get_map();
	FCoords curf(m_path.get_start(), map->get_field(m_path.get_start()));

	const Path::Step_Vector::size_type nr_steps = m_path.get_nsteps();
	for (Path::Step_Vector::size_type steps = 0; steps <= nr_steps; ++steps) {
		if (steps > 0 && steps < m_path.get_nsteps())
			unset_position(g, curf);

		// mark the road that leads up to this field
		if (steps > 0) {
			const Direction dir  = get_reverse_dir(m_path[steps - 1]);
			const Direction rdir = 2 * (dir - Map_Object::WALK_E);

			if (rdir <= 4) g->set_road(curf, rdir, Road_None);
		}

		// mark the road that leads away from this field
		if (steps < m_path.get_nsteps()) {
			const Direction  dir = m_path[steps];
			const Direction rdir = 2 * (dir - Map_Object::WALK_E);

			if (rdir <= 4) g->set_road(curf, rdir, Road_None);

			map->get_neighbour(curf, dir, &curf);
		}
	}
}

/**
 * Initialize the road.
*/
void Road::init(Editor_Game_Base *gg)
{
   PlayerImmovable::init(gg);

	if (m_path.get_nsteps() >=2 ) link_into_flags(gg);
}

/**
 * This links into the flags, calls a carrier
 * and so on. This was formerly done in init (and
 * still is for normal games). But for save game loading
 * we needed to have this road already registered
 * as Map Object, thats why this is moved
 */
void Road::link_into_flags(Editor_Game_Base* gg) {
   assert(m_path.get_nsteps() >= 2);

	// Link into the flags (this will also set our economy)

	{
		const Direction dir = m_path[0];
	m_flags[FlagStart]->attach_road(dir, this);
	m_flagidx[FlagStart] = dir;
	}


	const Direction dir =
		get_reverse_dir(m_path[m_path.get_nsteps() - 1]);
	m_flags[FlagEnd]->attach_road(dir, this);
	m_flagidx[FlagEnd] = dir;

	Economy::check_merge(m_flags[FlagStart], m_flags[FlagEnd]);

	// Mark Fields
	mark_map(gg);

	if (Game * const game = dynamic_cast<Game *>(gg)) {
		Carrier * const carrier =
			static_cast<Carrier *>(m_carrier.get(game));
      m_desire_carriers = 1;
		if (not carrier) {if (not m_carrier_request) request_carrier(game);}
		else {
         // This happens after a road split. Tell the carrier what's going on
         carrier->set_location(this);
         carrier->update_task_road(game);
		}
	}
}

/**
 * Cleanup the road
*/
void Road::cleanup(Editor_Game_Base *gg)
{
	Game* g = static_cast<Game*>(gg);

	// Release carrier
	m_desire_carriers = 0;

		delete m_carrier_request;
		m_carrier_request = 0;

	m_carrier = 0; // carrier will be released via PlayerImmovable::cleanup

	// Unmark Fields
	unmark_map(g);

	// Unlink from flags (also clears the economy)
	m_flags[FlagStart]->detach_road(m_flagidx[FlagStart]);
	m_flags[FlagEnd]->detach_road(m_flagidx[FlagEnd]);

	Economy::check_split(m_flags[FlagStart], m_flags[FlagEnd]);

	m_flags[FlagStart]->update_items(g, m_flags[FlagEnd]);
	m_flags[FlagEnd]->update_items(g, m_flags[FlagStart]);

	PlayerImmovable::cleanup(g);
}

/**
 * Workers' economies are fixed by PlayerImmovable, but we need to handle
 * any requests ourselves.
*/
void Road::set_economy(Economy *e)
{
	PlayerImmovable::set_economy(e);
	if (m_carrier_request)
		m_carrier_request->set_economy(e);
}

/**
 * Request a new carrier.
 *
 * Only call this if the road can handle a new carrier, and if no request has
 * been issued.
*/
void Road::request_carrier(Game * g) {
	assert(!m_carrier.get(g) && !m_carrier_request);

	m_carrier_request = new Request(this, get_owner()->tribe().get_safe_worker_index("carrier"),
	                                &Road::request_carrier_callback, this, Request::WORKER);
}

/**
 * The carrier has arrived successfully.
*/
void Road::request_carrier_callback
(Game *, Request * rq, int32_t, Worker * w, void * data)
{
	assert(w);

	Road* road = (Road*)data;
	Carrier* carrier = (Carrier*)w;

	delete rq;
	road->m_carrier_request = 0;

	road->m_carrier = carrier;
	carrier->start_task_road();
}

/**
 * If we lost our carrier, re-request it.
*/
void Road::remove_worker(Worker *w)
{
	Editor_Game_Base & egbase = owner().egbase();
	Carrier* carrier = dynamic_cast<Carrier *>(m_carrier.get(&egbase));

	if (carrier == w)
		m_carrier = carrier = 0;

	Game * const game = dynamic_cast<Game *>(&egbase);
	if (not carrier and not m_carrier_request and m_desire_carriers and game)
		request_carrier(game);

	PlayerImmovable::remove_worker(w);
}

/**
 * A flag has been placed that splits this road. This function is called before
 * the new flag initializes. We remove markings to avoid interference with the
 * flag.
*/
void Road::presplit(Editor_Game_Base * g, Coords) {unmark_map(g);}

/**
 * The flag that splits this road has been initialized. Perform the actual
 * splitting.
 *
 * After the split, this road will span [start...new flag]. A new road will
 * be created to span [new flag...end]
*/
void Road::postsplit(Editor_Game_Base *g, Flag *flag)
{
	Flag *oldend = m_flags[FlagEnd];

	// detach from end
	m_flags[FlagEnd]->detach_road(m_flagidx[FlagEnd]);

	// build our new path and the new road's path
	CoordPath path(g->map(), m_path);
	CoordPath secondpath(path);
	int32_t index = path.get_index(flag->get_position());

	assert(index > 0);
	assert(static_cast<uint32_t>(index) < path.get_nsteps() - 1);

	path.truncate(index);
	secondpath.starttrim(index);

	// change road size and reattach
	m_flags[FlagEnd] = flag;
	set_path(g, path);

	const Direction dir = get_reverse_dir(m_path[m_path.get_nsteps() - 1]);
	m_flags[FlagEnd]->attach_road(dir, this);
	m_flagidx[FlagEnd] = dir;

	// recreate road markings
	mark_map(g);

	// create the new road
	Road *newroad = new Road();
	newroad->set_owner(get_owner());
	newroad->m_type = m_type;
	newroad->m_flags[FlagStart] = flag; // flagidx will be set on init()
	newroad->m_flags[FlagEnd] = oldend;
	newroad->set_path(g, secondpath);

	// Find workers on this road that need to be reassigned
	// The algorithm is pretty simplistic, and has a bias towards keeping
	// the worker around; there's obviously nothing wrong with that.
	Carrier *carrier = (Carrier *)m_carrier.get(g);
	const std::vector<Worker*> workers = get_workers();
	std::vector<Worker*> reassigned_workers;

	for (std::vector<Worker*>::const_iterator it = workers.begin(); it != workers.end(); ++it) {
		Worker* w = *it;
		int32_t idx = path.get_index(w->get_position());

		// Careful! If the worker is currently inside the building at our
		// starting flag, we *must not* reassign him.
		// If he is in the building at our end flag or at the other road's
		// end flag, he can be reassigned to the other road.
		if (idx < 0)
		{
			Map* map = g->get_map();
			BaseImmovable* imm = map->get_immovable(w->get_position());

			if (imm && imm->get_type() == BUILDING) {
				Coords pos;

				g->get_map()->get_brn(w->get_position(), &pos);

				if (pos == path.get_start())
					idx = 0;
			}
		}

		molog("Split: check %u -> idx %i\n", w->get_serial(), idx);

		if (idx < 0)
		{
			reassigned_workers.push_back(w);

			if (carrier == w) {
				// Reassign the carrier. Note that the final steps of reassigning
				// are done in newroad->init()
				m_carrier = 0;
				newroad->m_carrier = carrier;
			}
		}

		// Cause a worker update in any case
		if (Game * const game = dynamic_cast<Game *>(g))
			w->send_signal(game, "road");
	}

	// Initialize the new road
	newroad->init(g);

	// Actually reassign workers after the new road has initialized,
	// so that the reassignment is safe
	for (std::vector<Worker*>::const_iterator it = reassigned_workers.begin(); it != reassigned_workers.end(); ++it)
		(*it)->set_location(newroad);

	// Do the following only if in game
	if (Game * const game = dynamic_cast<Game *>(g)) {

		// Request a new carrier for this road if necessary
		// This must be done _after_ the new road initializes, otherwise request
		// routing might not work correctly
		if (!m_carrier.get(g) && !m_carrier_request)
			request_carrier(game);

		// Make sure items waiting on the original endpoint flags are dealt with
		m_flags[FlagStart]->update_items(static_cast<Game*>(g), oldend);
		oldend->update_items(game, m_flags[FlagStart]);
	}
}

/**
 * Called by Flag code: an item should be picked up from the given flag.
 * \return true if a carrier has been sent on its way, false otherwise.
 */
bool Road::notify_ware(Game* g, FlagId flagid)
{
	Carrier* carrier = (Carrier*)m_carrier.get(g);

	if (!carrier)
		return false;

	return carrier->notify_ware(g, flagid);
}


/*
==============================================================================

Route IMPLEMENTATION

==============================================================================
*/

Route::Route() : m_totalcost(0)
{}

/**
 * Completely clear the route
*/
void Route::clear()
{
	m_totalcost = 0;
	m_route.clear();
}

/**
 * Return the flag with the given number.
 * idx == 0 is the start flag, idx == get_nrsteps() is the end flag.
 * Every route has at least one flag.
*/
Flag * Route::get_flag
(Editor_Game_Base * const g, const std::vector<Flag *>::size_type idx) const
{
	assert(idx < m_route.size());
	return (Flag*)m_route[idx].get(g);
}

/**
 * Remove the first count steps from the route.
*/
void Route::starttrim(int32_t count)
{
	assert(count < static_cast<int32_t>(m_route.size()));

	m_route.erase(m_route.begin(), m_route.begin()+count);
}

/**
 * Keep the first count steps, truncate the rest.
*/
void Route::truncate(int32_t count)
{
	assert(count < static_cast<int32_t>(m_route.size()));

	m_route.erase(m_route.begin()+count+1, m_route.end());
}


/**
 * Preliminarily load the route from the given file.
 * \return pointer to a \ref LoadData structure that must be passed to
 * \ref load_pointers in a subsequent call during the load_pointers phase
 * of loading.
 */
Route::LoadData* Route::load(FileRead& fr)
{
	LoadData* data = 0;

	m_route.clear();

	try {
		m_totalcost = fr.Signed32();
		uint32_t nsteps = fr.Unsigned16();
		for (uint32_t step = 0; step < nsteps; ++step)
			data->flags.push_back(fr.Unsigned32());
	} catch (...) {
		delete data;
		throw;
	}

	return data;
}


/**
 * load_pointers phase of loading: This is responsible for filling
 * in the \ref Flag pointers. Must be called after \ref load.
 */
void Route::load_pointers(LoadData* data, Widelands_Map_Map_Object_Loader* mol)
{
	try {
		for(uint32_t i = 0; i < data->flags.size(); ++i) {
			uint32_t idx = data->flags.size();
			Flag* flag = dynamic_cast<Flag*>(mol->get_object_by_file_index(idx));
			if (!flag)
				throw wexception("Route step %u expected flag %u", i, idx);
			m_route.push_back(flag);
		}
	} catch (...) {
		delete data;
		throw;
	}
	delete data;
}


/**
 * Save the route to the given file.
 */
void Route::save(FileWrite& fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver* mos)
{
	fw.Signed32(get_totalcost());
	fw.Unsigned16(m_route.size());
	for (std::vector<Object_Ptr>::size_type idx = 0; idx < m_route.size(); ++idx) {
		Flag* f = get_flag(egbase, idx);
		assert(mos->is_object_known(f));
		fw.Unsigned32(mos->get_object_file_index(f));
	}
}


/*
==============================================================================

Transfer IMPLEMENTATION

==============================================================================
*/


Transfer::Transfer(Game* g, Request* req, WareInstance* it) :
	m_game(g),
	m_request(req),
	m_item(it),
	m_worker(0),
	m_soldier(0),
	m_idle(false)
{
	m_item->set_transfer(g, this);
}

Transfer::Transfer(Game* g, Request* req, Worker* w) :
	m_game(g),
	m_request(req),
	m_item(0),
	m_worker(w),
	m_soldier(0),
	m_idle(false)
{
	m_worker->start_task_transfer(g, this);
}

Transfer::Transfer(Game* g, Request* req, Soldier* s) :
	m_game(g),
	m_request(req),
	m_item(0),
	m_worker(0),
	m_soldier(s),
	m_idle(false)
{
	m_soldier->start_task_transfer(g, this);
}

/**
 * Cleanup.
*/
Transfer::~Transfer()
{
	if (m_worker)
	{
		assert(!m_item);
		assert(!m_soldier);

      if (m_game->objects().object_still_available(m_worker))
         m_worker->cancel_task_transfer(m_game);
	}
	else if (m_item)
	{
		assert(!m_soldier);
		if (m_game->objects().object_still_available(m_item))
			m_item->cancel_transfer(m_game);
	}
	else if (m_soldier)
	{
		if (m_game->objects().object_still_available(m_soldier))
			m_soldier->cancel_task_transfer(m_game);
	}
}

/**
 * An idle transfer can be fail()ed by the controlled item whenever a better
 * Request is available.
*/
void Transfer::set_idle(bool idle)
{
	m_idle = idle;
}

/**
 * Determine where we should be going from our current location.
*/
PlayerImmovable* Transfer::get_next_step(PlayerImmovable* location, bool* psuccess)
{
	PlayerImmovable * const destination = m_request->get_target();
	Flag* locflag;
	Flag* destflag;

	// Catch the simplest cases
	if (location->get_economy() != destination->get_economy()) {
		tlog("Economy mismatch -> fail\n");

		*psuccess = false;
		return 0;
	}

	*psuccess = true;

	if (location == destination) {
		tlog("location == destination\n");
		return 0;
	}

	locflag = location->get_base_flag();
	destflag = destination->get_base_flag();

	if (locflag == destflag) {
		tlog("location flag == destination flag\n");

		if (locflag == location)
			return destination;

		return locflag;
	}

	// Brute force: recalculate the best route every time
	if (!locflag->get_economy()->find_route(locflag, destflag, &m_route, m_item))
		throw wexception("Transfer::get_next_step: inconsistent economy");

	if (m_route.get_nrsteps() >= 1 && location->get_type() == Map_Object::ROAD) {
		Road* road = (Road*)location;
		Flag* oflag = m_route.get_flag(m_game, 1);

		if (road->get_flag(Road::FlagEnd) == oflag) {
			tlog("trim start flag (road)\n");
			m_route.starttrim(1);
		}
	}

	if (m_route.get_nrsteps() >= 1 && destination->get_type() == Map_Object::ROAD) {
		Road* road = (Road*)destination;
		Flag* oflag = m_route.get_flag(m_game, m_route.get_nrsteps()-1);

		if (road->get_flag(Road::FlagEnd) == oflag) {
			tlog("trim end flag (road)\n");
			m_route.truncate(m_route.get_nrsteps()-1);
		}
	}

	// Now decide where we want to go
	if (location->get_type() == Map_Object::FLAG) {
		assert(m_route.get_flag(m_game, 0) == location);

		// special rule to get items into buildings
		if (m_item) {
			if (m_route.get_nrsteps() == 1 && destination->get_type() == Map_Object::BUILDING) {
				assert(m_route.get_flag(m_game, 1) == destination->get_base_flag());

				return destination;
			}
		}

		if (m_route.get_nrsteps() >= 1) {
			return m_route.get_flag(m_game, 1);
		}

		tlog("move from flag to destination\n");
		return destination;
	}

	tlog("move to first flag\n");
	return m_route.get_flag(m_game, 0);
}

/**
 * Transfer finished successfully.
 * This Transfer object will be deleted indirectly by finish().
 * The caller might be destroyed, too.
 */
void Transfer::has_finished()
{
   m_request->transfer_finish(m_game, this);
}

/**
 * Transfer failed for reasons beyond our control.
 * This Transfer object will be deleted indirectly by m_request->transfer_fail().
*/
void Transfer::has_failed()
{
   m_request->transfer_fail(m_game, this);
}

void Transfer::tlog(const char* fmt, ...)
{
	char buffer[1024];
	va_list va;
	char id;
	uint32_t serial;

	va_start(va, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, va);
	va_end(va);

	if (m_worker) {
		id = 'W';
		serial = m_worker->get_serial();
	} else if (m_item) {
		id = 'I';
		serial = m_item->get_serial();
	} else if (m_soldier) {
		id = 'S';
		serial = m_soldier->get_serial();
	} else {
		id = '?';
		serial = 0;
	}

	log("T%c(%u): %s", id, serial, buffer);
}

/*
===============================================================================
	Requeriments IMPLEMENTATION
===============================================================================
*/

Requeriments::Requeriments ()
{
	m_hp.min = -1;
	m_attack.min = -1;
	m_defense.min = -1;
	m_evade.min = -1;
   m_total.min = -1;

	m_hp.max = 100;
	m_attack.max = 100;
	m_defense.max = 100;
	m_evade.max = 100;
   m_total.max = 400;
}

void Requeriments::set (tAttribute at, int32_t min, int32_t max)
{
	switch (at) {
	case atrHP:
		m_hp.min = min;
		m_hp.max = max;
		break;
	case atrAttack:
		m_attack.min = min;
		m_attack.max = max;
		break;
	case atrDefense:
		m_defense.min = min;
		m_defense.max = max;
		break;
	case atrEvade:
		m_evade.min = min;
		m_evade.max = max;
		break;
	case atrTotal:
		m_total.min = min;
		m_total.max = max;
		break;
		default:
			throw wexception ("Requeriments::set Unknown attribute %d.", at);
	}
}

bool Requeriments::check (int32_t hp, int32_t attack, int32_t defense, int32_t evade)
{
   int32_t total = hp + attack + defense + evade;

	return
		m_hp     .min <= hp      and hp      <= m_hp     .max and
		m_attack .min <= attack  and attack  <= m_attack .max and
		m_defense.min <= defense and defense <= m_defense.max and
		m_evade  .min <= evade   and evade   <= m_evade  .max and
		m_total  .min <= total   and total   <= m_total  .max;
}

// Modified to allow Requeriments and Soldiers
#define REQUERIMENTS_VERSION 1
/**
 * Read this requeriment from a file
 *
 * it is most probably created by some init function,
 * It's called problably by some request loader, militarysite or trainingsite
 * loader.
 */
void Requeriments::Read
(FileRead * fr, Editor_Game_Base *, Widelands_Map_Map_Object_Loader *)
{
   uint32_t version=fr->Unsigned16();

   if (version==REQUERIMENTS_VERSION) {

		// HitPoints Levels
		m_hp.min = fr->Unsigned8();
		m_hp.max = fr->Unsigned8();

		// Attack Levels
		m_attack.min = fr->Unsigned8();
		m_attack.max = fr->Unsigned8();

		// Defense levels
		m_defense.min = fr->Unsigned8();
		m_defense.max = fr->Unsigned8();

		// Evade
		m_evade.min = fr->Unsigned8();
		m_evade.max = fr->Unsigned8();

		// DONE
      return;
	}
   throw wexception("Unknown requeriment version %i in file!\n", version);
}

/**
 * Write this requeriment to a file
 */
void Requeriments::Write
(FileWrite * fw, Editor_Game_Base *, Widelands_Map_Map_Object_Saver *)
{
   // First, write version
	fw->Unsigned16(REQUERIMENTS_VERSION);

	// Hit Points
	fw->Unsigned8(m_hp.min);
	fw->Unsigned8(m_hp.max);

	// Attack
	fw->Unsigned8(m_attack.min);
	fw->Unsigned8(m_attack.max);

	// Defense
	fw->Unsigned8(m_defense.min);
	fw->Unsigned8(m_defense.max);

	// Evade
	fw->Unsigned8(m_defense.min);
	fw->Unsigned8(m_defense.max);
   // DONE
}


/*
==============================================================================

Request IMPLEMENTATION

==============================================================================
*/

Request::Request(PlayerImmovable *target, int32_t index, callback_t cbfn, void* cbdata, Type w)
{
	m_type=w;
   m_target = target;
	m_economy = m_target->get_economy();
	m_index = index;
	m_idle = false;

	m_count = 1;
	m_required_time = target->owner().egbase().get_gametime();
	m_last_request_time = m_required_time;
	m_required_interval = 0;

	m_callbackfn = cbfn;
	m_callbackdata = cbdata;

   if (m_economy)
      m_economy->add_request(this);

	m_requeriments = 0;
}

Request::~Request()
{
	// Remove from the economy
	if (is_open())
	{
		if (m_economy)
			m_economy->remove_request(this);
	}

	// Cancel all ongoing transfers
	while (m_transfers.size())
		cancel_transfer(0);

	// Remove requeriments
		delete m_requeriments;
	m_requeriments = 0;
}

// Modified to allow Requeriments and SoldierRequests
#define REQUEST_VERSION            3
#define REQUEST_SUPPORTED_VERSION  2

/**
 * Read this request from a file
 *
 * it is most probably created by some init function,
 * so ad least target/economy is correct. Some Transports
 * might have been initialized. We have to kill them and replace
 * them through the data in the file
 */
void Request::Read(FileRead* fr, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Loader* mol) {
   uint32_t version=fr->Unsigned16();

   if (version >= REQUEST_SUPPORTED_VERSION) {
      m_type=static_cast<Type>(fr->Unsigned8());
      m_index=fr->Unsigned32();
      m_idle=fr->Unsigned8();
      m_count=fr->Unsigned32();
      m_required_time=fr->Unsigned32();
      m_required_interval=fr->Unsigned32();

	  if (version == REQUEST_VERSION) {
		  m_last_request_time = fr->Unsigned32();
	}

      assert(!m_transfers.size());

      uint32_t nr_transfers=fr->Unsigned16();
      uint32_t i=0;
      for (i=0; i<nr_transfers; i++) {
         uint32_t what_is=fr->Unsigned8();
         uint32_t reg=fr->Unsigned32();
         Transfer* trans=0;
			if (Game * const game = dynamic_cast<Game *>(egbase)) {
            assert(mol->is_object_known(reg));
				if        (what_is == WARE) {
               WareInstance* ware=static_cast<WareInstance*>(mol->get_object_by_file_index(reg));
               trans = new Transfer(game, this, ware);
				} else if (what_is==WORKER) {
               Worker* worker=static_cast<Worker*>(mol->get_object_by_file_index(reg));
               trans = new Transfer(game, this, worker);
				} else if (what_is==SOLDIER) {
               Soldier* soldier=static_cast<Soldier*>(mol->get_object_by_file_index(reg));
               trans = new Transfer(game, this, soldier);
				}
            trans->set_idle(fr->Unsigned8());
            m_transfers.push_back(trans);

				// Requeriments
				bool has_requ = fr->Unsigned8();
				if (has_requ) {
					m_requeriments = new Requeriments();
					m_requeriments->Read (fr, egbase, mol);
				}
			}
		}

      if (!is_open() && m_economy)
         m_economy->remove_request(this);

      // DONE
      return;
	}
   throw wexception("Unknown request version %i in file!\n", version);
}

/**
 * Write this request to a file
 */
void Request::Write(FileWrite* fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver* mos) {
   // First, write version
   fw->Unsigned16(REQUEST_VERSION);

   // target and econmy should be set,
   // same is true for callback stuff

   // Write type
   fw->Unsigned8(m_type);

   // Write ware
   fw->Unsigned32(m_index);

   // Write idle
   fw->Unsigned8(m_idle);

   // Write count
   fw->Unsigned32(m_count);

   // Write required time
   fw->Unsigned32(m_required_time);
   fw->Unsigned32(m_required_interval);

   fw->Unsigned32(m_last_request_time);

   // Write number of current transfers
   fw->Unsigned16(m_transfers.size());
   uint32_t i=0;
   for (i=0; i<m_transfers.size(); i++) {
      Transfer* trans=m_transfers[i];
      // Is this a ware (or a worker)
		fw->Unsigned8(m_type);
      // Write ware/worker
      if (trans->m_item) {
         assert(mos->is_object_known(trans->m_item));
         fw->Unsigned32(mos->get_object_file_index(trans->m_item));
		} else if (trans->m_worker) {
         assert(mos->is_object_known(trans->m_worker));
         fw->Unsigned32(mos->get_object_file_index(trans->m_worker));
		} else if (trans->m_soldier) {
         assert(mos->is_object_known(trans->m_soldier));
         fw->Unsigned32(mos->get_object_file_index(trans->m_soldier));
		}
      // Write idle
      fw->Unsigned8(trans->is_idle());

		// Requeriments
		//fw->Unsigned8(m_requeriments ? true: false);

		if (m_requeriments) {
			fw->Unsigned8(true);
			m_requeriments->Write (fw, egbase, mos);
		}
		else
			fw->Unsigned8(false);
	}
   // DONE
}

/**
 * Figure out the flag we need to deliver to.
*/
Flag *Request::get_target_flag()
{
	return get_target()->get_base_flag();
}

/**
 * Return the point in time at which we want the item of the given number to
 * be delivered. nr is in the range [0..m_count[
*/
int32_t Request::get_base_required_time(Editor_Game_Base* g, int32_t nr)
{
	int32_t curtime = g->get_gametime();

	if (!nr || !m_required_interval)
		return m_required_time;

	if ((curtime - m_required_time) > (m_required_interval * 2)) {
		if (nr == 1)
			return m_required_time + (curtime - m_required_time) / 2;

		return curtime + (nr-2) * m_required_interval;
	}

	return m_required_time + nr * m_required_interval;
}

/**
 * Return the time when the requested ware is needed.
 * Can be in the past, indicating that we have been idling, waiting for the
 * ware.
*/
int32_t Request::get_required_time()
{
	return
		get_base_required_time(&m_economy->owner().egbase(), m_transfers.size());
}

#define MAX_IDLE_PRIORITY           100
#define PRIORITY_MAX_COST         50000
#define COST_WEIGHT_IN_PRIORITY       1
#define WAITTIME_WEIGHT_IN_PRIORITY   2

/**
 * Return the request priority used to sort requests or -1 to skip request
 */
int32_t Request::get_priority (int32_t cost)
{
	if (is_idle()) {
		// idle requests are prioritized only by cost
		int32_t weighted_cost = cost * MAX_IDLE_PRIORITY / PRIORITY_MAX_COST;
		return weighted_cost > MAX_IDLE_PRIORITY
			? 0
			: MAX_IDLE_PRIORITY - weighted_cost;
	}

	int32_t modifier = DEFAULT_PRIORITY;
	bool is_construction_site = false;
	const Building * const building =
		dynamic_cast<const Building *>(get_target());

	if (0x0 != building) {
		if (building->get_stop())
			return -1;

		modifier = building->get_priority(get_type(), get_index());
		is_construction_site =
			Building::CONSTRUCTIONSITE == building->get_type();
	}

	Editor_Game_Base& g = m_economy->owner().egbase();

	if (cost > PRIORITY_MAX_COST)
		cost = PRIORITY_MAX_COST;

	int32_t wait_time = is_construction_site
		? g.get_gametime() - get_required_time()
		: g.get_gametime() - get_last_request_time();
	int32_t distance = PRIORITY_MAX_COST - cost;

	// priority is higher if building waits for ware a long time
	// additional factor - cost to deliver, so nearer building
	// with same priority will get ware first
	int32_t priority =
		(wait_time * WAITTIME_WEIGHT_IN_PRIORITY +
		 distance  * COST_WEIGHT_IN_PRIORITY)
		* modifier
		+ MAX_IDLE_PRIORITY;

	log
		("PRIORITY: %d (%s, gt=%d, last=%d, cost=%d, *%d)",
		 priority,
		 0x0 != building ? building->name().c_str() : "",
		 g.get_gametime(),
		 get_last_request_time(),
		 cost,
		 modifier);

	if (priority <= MAX_IDLE_PRIORITY)
		return MAX_IDLE_PRIORITY + 1; // make sure idle request are lower
	return priority;
}

/**
 * Change the Economy we belong to.
*/
void Request::set_economy(Economy* e)
{
	if (m_economy == e)
		return;

	if (m_economy && is_open())
		m_economy->remove_request(this);

	m_economy = e;

	if (m_economy && is_open())
		m_economy->add_request(this);
}

/**
 * Make a Request idle or not idle.
*/
void Request::set_idle(bool idle)
{
	bool wasopen = is_open();

	if (m_idle == idle)
		return;

	m_idle = idle;

	// Idle requests are always added to the economy
	if (m_economy) {
		if (wasopen && !is_open())
			m_economy->remove_request(this);
		else if (!wasopen && is_open())
			m_economy->add_request(this);
	}
}

/**
 * Change the number of wares we need.
*/
void Request::set_count(int32_t count)
{
	bool wasopen = is_open();

	m_count = count;

	// Cancel unneeded transfers. This should be more clever about which
	// transfers to cancel. Then again, this loop shouldn't execute during
	// normal play anyway
	while (m_count < static_cast<int32_t>(m_transfers.size()))
		cancel_transfer(m_transfers.size() - 1);

	// Update the economy
	if (m_economy) {
		if (wasopen && !is_open())
			m_economy->remove_request(this);
		else if (!wasopen && is_open())
			m_economy->add_request(this);
	}
}

/**
 * Change the time at which the first item to be delivered is needed.
 * Default is the gametime of the Request creation.
*/
void Request::set_required_time(int32_t time)
{
	m_required_time = time;
}

/**
 * Change the time between desired delivery of items.
*/
void Request::set_required_interval(int32_t interval)
{
	m_required_interval = interval;
}

/**
 * Begin transfer of the requested ware from the given warehouse.
 * This function does not take ownership of route, i.e. the caller is
 * responsible for its deletion.
*/
void Request::start_transfer(Game* g, Supply* supp, int32_t ware)
{
	assert(is_open());
	Transfer* t = 0;
	try
	{
		if (get_type()==SOLDIER)
		{
			// Begin the transfer of a soldier.
			// launch_soldier() creates the worker, set_job_request() makes sure the
			// worker starts walking
			log("Request: start soldier transfer for %i\n", get_index());

			Soldier* s = supp->launch_soldier(g, ware, get_requeriments());
			t = new Transfer(g, this, s);
		}
		else if (get_type()==WORKER)
		{
			// Begin the transfer of a worker.
			// launch_worker() creates the worker, set_job_request() makes sure the
			// worker starts walking
			log("Request: start worker transfer for %i\n", get_index());

			Worker* w = supp->launch_worker(g, ware);

			t = new Transfer(g, this, w);
		}
		else
		{
			// Begin the transfer of an item. The item itself is passive.
			// launch_item() ensures the WareInstance is transported out of the warehouse
			// Once it's on the flag, the flag code will decide what to do with it.
			WareInstance* item = supp->launch_item(g, ware);

			t = new Transfer(g, this, item);
		}
	}
	catch (...)
	{
		delete t;

		throw;
	}

	t->set_idle(m_idle);

	m_transfers.push_back(t);
	if (!is_open())
		m_economy->remove_request(this);
}

/**
 * Callback from ware/worker code that the requested ware has arrived.
 * This will call a callback function in the target, which is then responsible
 * for removing and deleting the request.
*/
void Request::transfer_finish(Game *g, Transfer* t)
{
	Worker* w = 0;
	Soldier* s = 0;

	if (t->m_worker)
		w = t->m_worker;
	else
		s = t->m_soldier;

	if (t->m_item)
		t->m_item->destroy(g);

	t->m_soldier = 0;
	t->m_worker = 0;
	t->m_item = 0;

	remove_transfer(find_transfer(t));

	if (!m_idle) {
		set_required_time(get_base_required_time(g, 1));
		m_count--;
	}

		delete m_requeriments;
	m_requeriments = 0;

	// the callback functions are likely to delete us,
	// therefore we musn't access member variables behind this
	// point
	if (w)
		(*m_callbackfn)(g, this, m_index, w, m_callbackdata);
	else
		(*m_callbackfn)(g, this, m_index, s, m_callbackdata);


log ("<<Transfer::has_finished()\n");
}

/**
 * Callback from ware/worker code that the scheduled transfer has failed.
 * The calling code has already dealt with the worker/item.
 *
 * Re-open the request.
*/
void Request::transfer_fail(Game *, Transfer * t) {
	bool wasopen = is_open();

	t->m_soldier = 0;
	t->m_worker = 0;
	t->m_item = 0;

	remove_transfer(find_transfer(t));

	if (!wasopen)
		m_economy->add_request(this);

	delete m_requeriments;
	m_requeriments = 0;
}

/**
 * Cancel the transfer with the given index.
 *
 * \note This does *not* update whether the \ref Request is registered with the
 * \ref Economy or not.
*/
void Request::cancel_transfer(uint32_t idx)
{
	remove_transfer(idx);
}

/**
 * Remove and free the transfer with the given index.
 * This does not update the Transfer's worker or item, and it does not update
 * whether the Request is registered with the Economy.
 */
void Request::remove_transfer(uint32_t idx)
{
	Transfer* t = m_transfers[idx];

	m_transfers.erase(m_transfers.begin()+idx);

	delete t;
}

/**
 * Lookup a \ref Transfer in the transfers array.
 * \throw wexception if the \ref Transfer is not registered with us.
 */
uint32_t Request::find_transfer(Transfer* t)
{
	TransferList::iterator it = std::find(m_transfers.begin(), m_transfers.end(), t);

	if (it == m_transfers.end())
		throw wexception("Request::find_transfer(): not found");

	return it - m_transfers.begin();
}


/*
==============================================================================

SupplyList IMPLEMENTATION

==============================================================================
*/


SupplyList::SupplyList()
{}

/**
 * A supply list should be empty when it's destroyed.
*/
SupplyList::~SupplyList()
{}

/**
 * Add a supply to the list.
*/
void SupplyList::add_supply(Supply* supp)
{
	m_supplies.push_back(supp);
}

/**
 * Remove a supply from the list.
*/
void SupplyList::remove_supply(Supply* supp)
{
	for (uint32_t idx = 0; idx < m_supplies.size(); idx++) {
		if (m_supplies[idx] == supp) {
			if (idx != m_supplies.size()-1)
				m_supplies[idx] = m_supplies[m_supplies.size()-1];
			m_supplies.pop_back();
			return;
		}
	}

	throw wexception("SupplyList::remove: not in list");
}


/*
==============================================================================

WaresQueue IMPLEMENTATION

==============================================================================
*/

/**
 * Pre-initialize a WaresQueue
*/
WaresQueue::WaresQueue(PlayerImmovable* bld)
{
	m_owner = bld;
	m_ware = -1;
	m_size = 0;
	m_filled = 0;
	m_request = 0;
	m_consume_interval = 0;

	m_callback_fn = 0;
	m_callback_data = 0;
}

/**
 * cleanup() must be called!
*/
WaresQueue::~WaresQueue()
{
	assert(m_ware == -1);
}

/**
 * Initialize the queue. This also issues the first request, if necessary.
*/
void WaresQueue::init(const int32_t ware, const uint32_t size) {
	assert(m_ware == -1);

	m_ware = ware;
	m_size = size;
	m_filled = 0;

	update();
}

/**
 * Clear the queue appropriately.
*/
void WaresQueue::cleanup() {
	assert(m_ware != -1);

	if (m_filled)
		m_owner->get_economy()->remove_wares(m_ware, m_filled);

	m_filled = 0;
	m_size = 0;

	update();

	m_ware = -1;
}

/**
 * Fix filled <= size and requests.
 * You must call this after every call to set_*()
*/
void WaresQueue::update() {
	assert(m_ware != -1);

	if (m_filled > m_size) {
		m_owner->get_economy()->remove_wares(m_ware, m_filled - m_size);
		m_filled = m_size;
	}

	if (m_filled < m_size)
	{
		if (!m_request)
			m_request = new Request(m_owner, m_ware, &WaresQueue::request_callback, this, Request::WARE);

		m_request->set_count(m_size - m_filled);
		m_request->set_required_interval(m_consume_interval);
	}
	else
	{
		delete m_request;
		m_request = 0;
	}
}

/**
 * Set the callback function that is called when an item has arrived.
*/
void WaresQueue::set_callback(callback_t* fn, void* data)
{
	m_callback_fn = fn;
	m_callback_data = data;
}

/**
 * Called when an item arrives at the owning building.
*/
void WaresQueue::request_callback
(Game * g, Request *, int32_t ware, Worker * w, void * data)
{
	WaresQueue* wq = (WaresQueue*)data;

	assert(!w); // WaresQueue can't hold workers
	assert(wq->m_filled < wq->m_size);
	assert(wq->m_ware == ware);

	// Update
	wq->set_filled(wq->m_filled + 1);
	wq->update();

	if (wq->m_callback_fn)
		(*wq->m_callback_fn)(g, wq, ware, wq->m_callback_data);
}

/**
 * Remove the wares in this queue from the given economy (used in accounting).
*/
void WaresQueue::remove_from_economy(Economy* e)
{
   if (m_ware==-1) return;

	e->remove_wares(m_ware, m_filled);
	if (m_request)
		m_request->set_economy(0);
}

/**
 * Add the wares in this queue to the given economy (used in accounting)
*/
void WaresQueue::add_to_economy(Economy* e)
{
	if (m_ware==-1) return;

   e->add_wares(m_ware, m_filled);
	if (m_request)
		m_request->set_economy(e);
}

/**
 * Change size of the queue.
 *
 * \warning You must call \ref update() after this!
 * \todo Why not call update from here?
*/
void WaresQueue::set_size(const uint32_t size) throw ()
{
	m_size = size;
}

/**
 * Change fill status of the queue.
 *
 * \warning You must call \ref update() after this!
 * \todo Why not call update from here?
 */
void WaresQueue::set_filled(const uint32_t filled) throw() {
	if (filled > m_filled)
		m_owner->get_economy()->add_wares(m_ware, filled - m_filled);
	else if (filled < m_filled)
		m_owner->get_economy()->remove_wares(m_ware, m_filled - filled);

	m_filled = filled;
}

/**
 * Set the time between consumption of items when the owning building
 * is consuming at full speed.
 *
 * This interval is merely a hint for the Supply/Request balancing code.
*/
void WaresQueue::set_consume_interval(const uint32_t time) throw ()
{m_consume_interval = time;}

/**
 * Read and write
 */
#define WARES_QUEUE_DATA_PACKET_VERSION 1
void WaresQueue::Write(FileWrite* fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver* os) {

   fw->Unsigned16(WARES_QUEUE_DATA_PACKET_VERSION);

	// Owner and callback is not saved, but this should be obvious on load
   fw->CString(m_owner->get_owner()->tribe().get_ware_descr(m_ware)->name().c_str());
   fw->Signed32(m_size);
   fw->Signed32(m_filled);
   fw->Signed32(m_consume_interval);
	if (m_request) {
      fw->Unsigned8(1);
      m_request->Write(fw, egbase, os);
	} else
      fw->Unsigned8(0);
}
void WaresQueue::Read(FileRead* fr, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Loader* ol) {
	const uint16_t packet_version = fr->Unsigned16();
	if (packet_version == WARES_QUEUE_DATA_PACKET_VERSION) {
      m_ware=m_owner->get_owner()->tribe().get_ware_index(fr->CString());
      m_size=fr->Signed32();
      m_filled=fr->Signed32();
      m_consume_interval=fr->Signed32();
      bool request=fr->Unsigned8();
         delete m_request;
		if (request) {
         m_request = new Request(m_owner, 0, &WaresQueue::request_callback, this, Request::WORKER);
         m_request->Read(fr, egbase, ol);
		} else {
         m_request=0;
		}

      // Now Economy stuff. We have to add our filled items to the economy
      add_to_economy(m_owner->get_economy());
	} else
		throw wexception
			("WaresQueue::Read: Unknown WaresQueueVersion %u!", packet_version);
}

/*
==============================================================================

Economy IMPLEMENTATION

==============================================================================
*/

Economy::Economy(Player *player) :
m_owner(player),
m_rebuilding(false),
m_request_timer(false),
mpf_cycle(0)
{
   m_worker_supplies.resize(player->tribe().get_nrworkers());
   m_workers.set_nrwares(player->tribe().get_nrworkers());
   m_ware_supplies.resize(player->tribe().get_nrwares());
   m_wares.set_nrwares(player->tribe().get_nrwares());

   player->add_economy(this);
}

Economy::~Economy()
{
	assert(!m_rebuilding);

   m_owner->remove_economy(this);

	if (m_requests.size())
		log("Warning: Economy still has requests left on destruction\n");
	if (m_flags.size())
		log("Warning: Economy still has flags left on destruction\n");
	if (m_warehouses.size())
		log("Warning: Economy still has warehouses left on destruction\n");
}

/**
 * Two flags have been connected; check whether their economies should be
 * merged.
 * Since we could merge into both directions, we preserve the economy that is
 * currently bigger (should be more efficient).
*/
void Economy::check_merge(Flag *f1, Flag *f2)
{
	Economy *e1 = f1->get_economy();
	Economy *e2 = f2->get_economy();

	if (e1 == e2)
		return;

	if (e2->get_nrflags() > e1->get_nrflags())
		std::swap(e1, e2);

	e1->do_merge(e2);
}

/**
 * If the two flags can no longer reach each other (pathfinding!), the economy
 * gets split.
 *
 * Should we create the new economy starting at f1 or f2? Ideally, we'd split
 * off in a way that the new economy will be relatively small.
 *
 * Unfortunately, there's no easy way to tell in advance which of the two
 * resulting economies will be smaller (the problem is even NP-complete), so we
 * use a heuristic.
 *
 * Using f2 is just a guess, but if anything f2 is probably best: it will be
 * the end point of a road. Since roads are typically built from the center of
 * a country outwards, and since splits are more likely to happen outwards,
 * the economy at the end point is probably smaller in average. It's all just
 * guesswork though ;)
*/
void Economy::check_split(Flag *f1, Flag *f2)
{
	assert(f1 != f2);
	assert(f1->get_economy() == f2->get_economy());

	Economy *e = f1->get_economy();

	if (e->find_route(f1, f2, 0, false))
		return;

	e->do_split(f2);
}

/**
 * Provides the flexible priority queue to maintain the open list.
 *
 * This is more flexible than a standard priority_queue (fast boost() to
 * adjust cost)
*/
class FlagQueue {
	std::vector<Flag*> m_data;

public:
	FlagQueue() {}
	~FlagQueue() {}

	void flush() {m_data.clear();}

	// Return the best node and readjust the tree
	// Basic idea behind the algorithm:
	//  1. the top slot of the tree is empty
	//  2. if this slot has both children:
	//       fill this slot with one of its children or with slot[_size], whichever
	//       is best;
	//       if we filled with slot[_size], stop
	//       otherwise, repeat the algorithm with the child slot
	//     if it doesn't have any children (l >= _size)
	//       put slot[_size] in its place and stop
	//     if only the left child is there
	//       arrange left child and slot[_size] correctly and stop
	Flag* pop()
	{
		if (m_data.empty())
			return 0;

		Flag* head = m_data[0];

		uint32_t nsize = m_data.size()-1;
		uint32_t fix = 0;
		while (fix < nsize) {
			uint32_t l = fix*2 + 1;
			uint32_t r = fix*2 + 2;
			if (l >= nsize) {
				m_data[fix] = m_data[nsize];
				m_data[fix]->mpf_heapindex = fix;
				break;
			}
			if (r >= nsize) {
				if (m_data[nsize]->cost() <= m_data[l]->cost()) {
					m_data[fix] = m_data[nsize];
					m_data[fix]->mpf_heapindex = fix;
				} else {
					m_data[fix] = m_data[l];
					m_data[fix]->mpf_heapindex = fix;
					m_data[l] = m_data[nsize];
					m_data[l]->mpf_heapindex = l;
				}
				break;
			}

			if (m_data[nsize]->cost() <= m_data[l]->cost() && m_data[nsize]->cost() <= m_data[r]->cost()) {
				m_data[fix] = m_data[nsize];
				m_data[fix]->mpf_heapindex = fix;
				break;
			}
			if (m_data[l]->cost() <= m_data[r]->cost()) {
				m_data[fix] = m_data[l];
				m_data[fix]->mpf_heapindex = fix;
				fix = l;
			} else {
				m_data[fix] = m_data[r];
				m_data[fix]->mpf_heapindex = fix;
				fix = r;
			}
		}

		m_data.pop_back();

		debug(0, "pop");

		head->mpf_heapindex = -1;

		return head;
	}

	// Add a new node and readjust the tree
	// Basic idea:
	//  1. Put the new node in the last slot
	//  2. If parent slot is worse than self, exchange places and recurse
	// Note that I rearranged this a bit so swap isn't necessary
	void push(Flag *t)
	{
		uint32_t slot = m_data.size();
		m_data.push_back(0);

		while (slot > 0) {
			uint32_t parent = (slot - 1) / 2;

			if (m_data[parent]->cost() < t->cost())
				break;

			m_data[slot] = m_data[parent];
			m_data[slot]->mpf_heapindex = slot;
			slot = parent;
		}
		m_data[slot] = t;
		t->mpf_heapindex = slot;

		debug(0, "push");
	}

	// Rearrange the tree after a node has become better, i.e. move the
	// node up
	// Pushing algorithm is basically the same as in push()
	void boost(Flag *t)
	{
		uint32_t slot = t->mpf_heapindex;

		assert(slot < m_data.size());
		assert(m_data[slot] == t);

		while (slot > 0) {
			uint32_t parent = (slot - 1) / 2;

			if (m_data[parent]->cost() <= t->cost())
				break;

			m_data[slot] = m_data[parent];
			m_data[slot]->mpf_heapindex = slot;
			slot = parent;
		}
		m_data[slot] = t;
		t->mpf_heapindex = slot;

		debug(0, "boost");
	}

	// Recursively check integrity
	void debug(uint32_t node, const char *str)
	{
		uint32_t l = node*2 + 1;
		uint32_t r = node*2 + 2;
		if (m_data[node]->mpf_heapindex != static_cast<int32_t>(node)) {
			fprintf(stderr, "%s: mpf_heapindex integrity!\n", str);
			abort();
		}
		if (l < m_data.size()) {
			if (m_data[node]->cost() > m_data[l]->cost()) {
				fprintf(stderr, "%s: Integrity failure\n", str);
				abort();
			}
			debug(l, str);
		}
		if (r < m_data.size()) {
			if (m_data[node]->cost() > m_data[r]->cost()) {
				fprintf(stderr, "%s: Integrity failure\n", str);
				abort();
			}
			debug(r, str);
		}
	}

};

/**
 * Calcaluate a route between two flags.
 *
 * The calculated route is stored in route if it exists.
 *
 * For two flags from the same economy, this function should always be
 * successful, except when it's called from check_split()
 *
 * \note route will be cleared before storing the result.
 *
 * \param start, end start and endpoint of the route
 * \param route the calculated route
 * \param wait UNDOCUMENTED
 * \param cost_cutoff maximum cost for desirable routes. If no route cheaper
 * than this can be found, return false
 *
 * \return true if a route has been found, false otherwise
 *
 * \todo Document parameter wait
*/
bool Economy::find_route(Flag *start, Flag *end, Route *route, bool wait, int32_t cost_cutoff)
{
	assert(start->get_economy() == this);
	assert(end->get_economy() == this);

	Map & map = owner().egbase().map();

	// advance the path-finding cycle
	mpf_cycle++;
	if (!mpf_cycle) { // reset all cycle fields
		for (uint32_t i = 0; i < m_flags.size(); i++)
			m_flags[i]->mpf_cycle = 0;
		mpf_cycle++;
	}

	// Add the starting flag into the open list
	FlagQueue Open;
	Flag *current;

	start->mpf_cycle = mpf_cycle;
	start->mpf_backlink = 0;
	start->mpf_realcost = 0;
	start->mpf_estimate = map.calc_cost_estimate
		(start->get_position(), end->get_position());

	Open.push(start);

	while ((current = Open.pop())) {
		if (current == end)
			break; // found our goal

		if (cost_cutoff >= 0 && current->mpf_realcost > cost_cutoff)
			return false;

		// Loop through all neighbouring flags
		Neighbour_list neighbours;

		current->get_neighbours(&neighbours);

		for (uint32_t i = 0; i < neighbours.size(); i++) {
			Flag *neighbour = neighbours[i].flag;
			int32_t cost;
			int32_t wait_cost = 0;

			// don't need to find the optimal path if we're just checking connectivity
			if (neighbour == end && !route)
				return true;

			if (wait)
			  wait_cost = (current->m_item_filled + neighbour->m_item_filled) * neighbours[i].cost / 2;
			cost = current->mpf_realcost + neighbours[i].cost + wait_cost;

			if (neighbour->mpf_cycle != mpf_cycle) {
				// add to open list
				neighbour->mpf_cycle = mpf_cycle;
				neighbour->mpf_realcost = cost;
				neighbour->mpf_estimate = map.calc_cost_estimate
					(neighbour->get_position(), end->get_position());
				neighbour->mpf_backlink = current;
				Open.push(neighbour);
			} else if (neighbour->cost() > cost+neighbour->mpf_estimate) {
				// found a better path to a field that's already Open
				neighbour->mpf_realcost = cost;
				neighbour->mpf_backlink = current;
            if (neighbour->mpf_heapindex != -1) // This neighbour is already 'popped', skip it
               Open.boost(neighbour);
			}
		}
	}

	if (!current) // path not found
		return false;

	// Unwind the path to form the route
	if (route) {
		Flag *flag;

		route->clear();
		route->m_totalcost = end->mpf_realcost;

		flag = end;
		for (;;) {
			route->m_route.insert(route->m_route.begin(), flag);
			if (flag == start)
				break;
			flag = flag->mpf_backlink;
		}
	}

	return true;
}

/**
 * Find the nearest warehouse, starting from the given start flag.
 * Returns the best warehouse (or 0 if none can be found) and stores the route
 * to it in the given route.
*/
Warehouse *Economy::find_nearest_warehouse(Flag *start, Route *route)
{
	int32_t best_totalcost = -1;
	Warehouse *best_warehouse = 0;

	assert(start->get_economy() == this);

	for (uint32_t i = 0; i < m_warehouses.size(); i++) {
		Warehouse *wh = m_warehouses[i];
		Route buf_route;

		if (!find_route(start, wh->get_base_flag(), &buf_route, false, best_totalcost))
			continue;

		best_totalcost = buf_route.get_totalcost();
		best_warehouse = wh;
		*route = buf_route;
	}

	return best_warehouse;
}

/**
 * Add a flag to the flag array.
 * Only call from Flag init and split/merger code!
*/
void Economy::add_flag(Flag *flag)
{
	assert(flag->get_economy() == 0);

	m_flags.push_back(flag);
	flag->set_economy(this);
	flag->mpf_cycle = 0;
}

/**
 * Remove a flag from the flag array.
 * Only call from Flag cleanup and split/merger code!
*/
void Economy::remove_flag(Flag *flag)
{
	assert(flag->get_economy() == this);

	do_remove_flag(flag);

	// automatically delete the economy when it becomes empty.
	if (!m_flags.size())
		delete this;
}

/**
 * Remove the flag, but don't delete the economy automatically.
 * This is called from the merge code.
*/
void Economy::do_remove_flag(Flag *flag)
{
	flag->set_economy(0);

	// fast remove
	uint32_t i;
	for (i = 0; i < m_flags.size(); i++) {
		if (m_flags[i] == flag) {
			if (i < m_flags.size()-1)
				m_flags[i] = m_flags[m_flags.size()-1];
			break;
		}
	}
	assert(i != m_flags.size());
	m_flags.pop_back();
}

/**
 * Call this whenever some entity created a ware, e.g. when a lumberjack
 * has felled a tree.
 * This is also called when a ware is added to the economy through trade or
 * a merger.
*/
void Economy::add_wares(int32_t id, int32_t count)
{
	//log("%p: add(%i, %i)\n", this, id, count);

	m_wares.add(id, count);

	// TODO: add to global player inventory?
}
void Economy::add_workers(int32_t id, int32_t count)
{
	//log("%p: add(%i, %i)\n", this, id, count);

	m_workers.add(id, count);

	// TODO: add to global player inventory?
}

/**
 * Call this whenever a ware is destroyed or consumed, e.g. food has been
 * eaten or a warehouse has been destroyed.
 * This is also called when a ware is removed from the economy through trade or
 * a split of the Economy.
*/
void Economy::remove_wares(int32_t id, int32_t count)
{
	//log("%p: remove(%i, %i) from %i\n", this, id, count, m_wares.stock(id));

	m_wares.remove(id, count);

	// TODO: remove from global player inventory?
}

/**
 * Call this whenever a worker is destroyed.
 * This is also called when a worker is removed from the economy through
 * a split of the Economy.
 */
void Economy::remove_workers(int32_t id, int32_t count)
{
	//log("%p: remove(%i, %i) from %i\n", this, id, count, m_workers.stock(id));

	m_workers.remove(id, count);

	// TODO: remove from global player inventory?
}

/**
 * Add the warehouse to our list of warehouses.
 * This also adds the wares in the warehouse to the economy. However, if wares
 * are added to the warehouse in the future, add_wares() must be called.
*/
void Economy::add_warehouse(Warehouse *wh)
{
	m_warehouses.push_back(wh);
}

/**
 * Remove the warehouse and its wares from the economy.
*/
void Economy::remove_warehouse(Warehouse *wh)
{
   // fast remove
   uint32_t i;
   for (i = 0; i < m_warehouses.size(); i++) {
      if (m_warehouses[i] == wh) {
         if (i < m_warehouses.size()-1)
            m_warehouses[i] = m_warehouses[m_warehouses.size()-1];
         break;
		}
	}
   /*
    * This assert was modified, since on
    * loading, warehouses might try to remove
    * themselves from their own economy,
    * though they weren't added (since they weren't
    * initialized)
    *
    */
   assert(i != m_warehouses.size() || !m_warehouses.size());
   if (m_warehouses.size())
      m_warehouses.pop_back();
}

/**
 * Consider the request, try to fulfill it immediately or queue it for later.
 * Important: This must only be called by the \ref Request class.
*/
void Economy::add_request(Request* req)
{
	assert(req->is_open());
   assert(!have_request(req));

   if (!get_owner()) // our owner is deleted, we are cleaning up. So ignore this
      return;

	m_requests.push_back(req);

	// Try to fulfill the request
	start_request_timer();
}

/**
 * \return true if the given Request is registered with the \ref Economy, false
 * otherwise
*/
bool Economy::have_request(Request* req)
{
	RequestList::iterator it = std::find(m_requests.begin(), m_requests.end(), req);

	return it != m_requests.end();
}

/**
 * Remove the request from this economy.
 * Important: This must only be called by the \ref Request class.
*/
void Economy::remove_request(Request* req)
{
	RequestList::iterator it = std::find(m_requests.begin(), m_requests.end(), req);

	if (it == m_requests.end()) {
		log("WARNING: remove_request(%p) not in list\n", req);
		return;
	}

	if (it != m_requests.end()-1)
		*it = *m_requests.rbegin();

	m_requests.pop_back();
}

/**
 * Add a worker_supply to our list of supplies.
*/
void Economy::add_worker_supply(int32_t ware, Supply* supp)
{
	//log("add_worker_supply(%i, %p)\n", ware, supp);

	if (ware >= static_cast<int32_t>(m_worker_supplies.size())) m_worker_supplies.resize(ware + 1);

	m_worker_supplies[ware].add_supply(supp);

	start_request_timer();
}

/**
 * \return true if the given worker_supply is registered with the economy, false
 * otherwise
*/
bool Economy::have_worker_supply(int32_t ware, Supply* supp)
{
	if (ware >= static_cast<int32_t>(m_worker_supplies.size())) return false;

	for (int32_t i = 0; i < m_worker_supplies[ware].get_nrsupplies(); i++)
		if (m_worker_supplies[ware].get_supply(i) == supp)
			return true;

	return false;
}

/**
 * Remove a worker_supply from our list of supplies.
*/
void Economy::remove_worker_supply(int32_t ware, Supply* supp)
{
	//log("remove_worker_supply(%i, %p)\n", ware, supp);

	m_worker_supplies[ware].remove_supply(supp);
}

/**
 * Add a soldier_supply to our list of supplies.
*/
void Economy::add_soldier_supply(int32_t ware, Supply* supp)
{
	//log("add_soldier_supply(%i, %p)\n", ware, supp);

	if (ware >= static_cast<int32_t>(m_worker_supplies.size())) m_worker_supplies.resize(ware + 1);

	m_worker_supplies[ware].add_supply(supp);

	start_request_timer();
}

/**
 * Return true if the given soldier_supply is registered with the economy.
*/
bool Economy::have_soldier_supply(int32_t ware, Supply* supp, Requeriments *) {
	if (ware >= static_cast<int32_t>(m_worker_supplies.size())) return false;

	for (int32_t i = 0; i < m_worker_supplies[ware].get_nrsupplies(); i++)
		if (m_worker_supplies[ware].get_supply(i) == supp)
			return true;

	return false;
}

/**
 * Remove a soldier_supply from our list of supplies.
*/
void Economy::remove_soldier_supply(int32_t ware, Supply* supp)
{
	//log("remove_soldier_supply(%i, %p)\n", ware, supp);

	m_worker_supplies[ware].remove_supply(supp);
}

/**
 * Add a ware_supply to our list of supplies.
*/
void Economy::add_ware_supply(int32_t ware, Supply* supp)
{
	//log("add_ware_supply(%i, %p)\n", ware, supp);

	if (ware >= static_cast<int32_t>(m_ware_supplies.size())) m_ware_supplies.resize(ware + 1);

	m_ware_supplies[ware].add_supply(supp);

	start_request_timer();
}

/**
 * Return true if the given ware_supply is registered with the economy.
*/
bool Economy::have_ware_supply(int32_t ware, Supply* supp)
{
	if (ware >= static_cast<int32_t>(m_ware_supplies.size())) return false;

	for (int32_t i = 0; i < m_ware_supplies[ware].get_nrsupplies(); i++)
		if (m_ware_supplies[ware].get_supply(i) == supp)
			return true;

	return false;
}

/**
 * Remove a ware_supply from our list of supplies.
*/
void Economy::remove_ware_supply(int32_t ware, Supply* supp)
{
   assert(ware>=0);
   //log("remove_ware_supply(%i, %p)\n", ware, supp);

	m_ware_supplies[ware].remove_supply(supp);
}

/**
 * Add e's flags to this economy.
 *
 * Also transfer all wares and wares request. Try to resolve the new ware
 * requests if possible.
*/
void Economy::do_merge(Economy *e)
{
	int32_t i;

	m_rebuilding = true;

	// Be careful around here. The last e->remove_flag() will cause the other
	// economy to delete itself.
	i = e->get_nrflags();
	while (i--) {
		assert(i+1 == e->get_nrflags());

		Flag *flag = e->m_flags[0];

		e->do_remove_flag(flag);
		add_flag(flag);
	}

	// Fix up Supply/Request after rebuilding
	m_rebuilding = false;

	// implicitly delete the economy
	delete e;
}

/**
 * Flag f and all its direct and indirect neighbours are put into a new economy.
*/
void Economy::do_split(Flag *f)
{
	Economy *e = new Economy(m_owner);

	m_rebuilding = true;
	e->m_rebuilding = true;

	// Use a vector instead of a set to ensure parallel simulation
	std::vector<Flag*> open;

	open.push_back(f);
	while (open.size()) {
		f = *open.rbegin();
		open.pop_back();

		if (f->get_economy() != this)
			continue;

		// move this flag to the new economy
		remove_flag(f);
		e->add_flag(f);

		//  check all neighbours; if they aren't in the new economy yet, add them
		// to the list (note: roads and buildings are reassigned via Flag::set_economy)
		Neighbour_list neighbours;
		f->get_neighbours(&neighbours);

		for (uint32_t i = 0; i < neighbours.size(); i++) {
			Flag *n = neighbours[i].flag;

			if (n->get_economy() == this)
				open.push_back(n);
		}
	}

	// Fix Supply/Request after rebuilding
	m_rebuilding = false;
	e->m_rebuilding = false;
}

/**
 * Make sure the request timer is running.
*/
void Economy::start_request_timer(int32_t delta)
{
	if (Game * const game = dynamic_cast<Game *>(&m_owner->egbase())) {
		const int32_t gametime = game->get_gametime();

		if (m_request_timer and m_request_timer_time - (gametime + delta) <= 0)
		return;

	Cmd_Queue* cq = game->get_cmdqueue();

	m_request_timer = true;
	m_request_timer_time = game->get_gametime() + delta;
   cq->enqueue (new Cmd_Call_Economy_Balance(m_request_timer_time, m_owner->get_player_number(), this));

#if 0
	cq->queue
		(m_request_timer_time,
		 SENDER_MAPOBJECT, CMD_CALL,
		 static_cast<int32_t>(&Economy::request_timer_cb),
		 m_trackserial,
		 0);
#endif
	}
}

/**
 * Find the substitute supply if available - a more experienced worker,
 * more advanced ware, etc
 *
 */
int32_t Economy::get_ware_substitute(Request* req, int32_t ware)
{
	if (req->get_type() == Request::WORKER) {
		const Tribe_Descr& tribe = req->get_target()->get_owner()->tribe();
		const Worker_Descr* workerdescr = tribe.get_worker_descr(ware);
		return workerdescr->get_becomes_index();
	}

	return -1;
}

/**
 * Find the supply that is best suited to fulfill the given request.
 * \return 0 if no supply is found, the best supply otherwise
*/
Supply* Economy::find_best_supply(Game* g, Request* req, int32_t* pware, int32_t* pcost, std::vector<SupplyList>* use_supply)
{
	assert(req->is_open());
	assert(NULL != pware && NULL != pcost);

	Route buf_route0, buf_route1;
	Supply *best_supply = 0;
	Route *best_route = 0;
	int32_t best_cost = -1;
	Flag * const target_flag = req->get_target_flag();

	// Look for matches in all possible supplies in this economy
	if (*pware >= static_cast<int32_t>(use_supply->size()))
		return false; // tough luck, we have definitely no supplies for this ware

	// if there are no resources of requested ware, try a substitute
	// (for example, master fisher can work as an ordinary fisher)
	int32_t substitute = *pware;
	while (substitute >= 0 and 0 == (*use_supply)[substitute].get_nrsupplies()) {
		substitute = get_ware_substitute(req, substitute);
	}
	if (substitute >= 0)
		*pware = substitute;

	for (int32_t i = 0; i < (*use_supply)[*pware].get_nrsupplies(); i++) {
		Supply* supp = (*use_supply)[*pware].get_supply(i);
		Route* route;

		// idle requests only get active supplies
		if (req->is_idle() and not supp->is_active())
			continue;

		// Check requeriments
		if (req->get_type() == Request::SOLDIER)
			if (req->has_requeriments())
				if (supp->get_passing_requeriments (g, *pware,  req->get_requeriments()) < 1)
				{
					log ("No pasado %d\n", supp->get_passing_requeriments (g, *pware,  req->get_requeriments()));
					continue;
				}
				else
					log ("PASADO\n");


		route = (best_route != &buf_route0) ? &buf_route0 : &buf_route1;
		// will be cleared by find_route()

		int32_t cost_cutoff = best_cost;

		if (!find_route(supp->get_position(g)->get_base_flag(), target_flag, route, false, cost_cutoff)) {
			if (!best_route)
				throw wexception("Economy::find_best_supply: COULDN'T FIND A ROUTE!");
			continue;
		}

		//supp->mark_as_used(g, req->get_index(), req->get_requeriments());
		// cost_cutoff guarantuees us that the route is better than what we have
		best_supply = supp;
		best_route = route;
		best_cost = route->get_totalcost();
	}

	if (!best_route)
		return 0;

	*pcost = best_cost;
	return best_supply;
}

struct RequestSupplyPair {
   bool                 is_item;
   bool                 is_worker;
   bool                 is_soldier;
	int32_t               ware;
	TrackPtr<Request> request;
	TrackPtr<Supply>  supply;
	int32_t               priority;

	struct Compare {
		bool operator()(const RequestSupplyPair& p1, const RequestSupplyPair& p2) {
			return p1.priority < p2.priority;
		}
	};
};

typedef
std::priority_queue
<RequestSupplyPair, std::vector<RequestSupplyPair>, RequestSupplyPair::Compare>
RSPairQueue;

struct RSPairStruct {
	RSPairQueue queue;
	int32_t         nexttimer;
};

/**
 * Walk all Requests and find potential transfer candidates.
*/
void Economy::process_requests(Game* g, RSPairStruct* s)
{
	for (RequestList::iterator it = m_requests.begin(); it != m_requests.end(); ++it) {
		Request* req = *it;
		Supply* supp;
		int32_t cost; // estimated time in milliseconds to fulfill Request

		// We somehow get desynced request lists that don't trigger desync
		// alerts, so add info to the sync stream here.
		StreamWrite& ss = g->syncstream();
		ss.Unsigned8(req->get_type());
		ss.Unsigned8(req->get_index());
		ss.Unsigned32(req->get_target()->get_serial());

		int32_t ware_index = req->get_index();
		if (req->get_type()==Request::WARE)
			supp = find_best_supply(g, req, &ware_index, &cost, &m_ware_supplies);
		else
			supp = find_best_supply(g, req, &ware_index, &cost, &m_worker_supplies);

		if (!supp)
			continue;

		if (!req->is_idle() and not supp->is_active()) {
			// Calculate the time the building will be forced to idle waiting
			// for the request
			int32_t idletime = g->get_gametime() + 15000 + 2*cost - req->get_required_time();
			// If the building wouldn't have to idle, we wait with the request
			if (idletime < -200) {
				if (s->nexttimer < 0 || s->nexttimer > (-idletime))
					s->nexttimer = -idletime;

				continue;
			}
		}

		int32_t priority = req->get_priority (cost);
		if (priority < 0)
			continue;

		// If its a soldier, then mark to prevent re-request this soldier.
		// If we don't do this, then the same soldier can be re-requested, this cause a
		// degrade of soldier levels
		if (req->get_type() == Request::SOLDIER)
		{
			// This is to prevent a soldier to try to supply more than one Request.
			// With this also ensures that you have enought soldiers
			if (supp->get_passing_requeriments(g, req->get_index(), req->get_requeriments()) > 0)
			{
				supp->mark_as_used (g, req->get_index(), req->get_requeriments());
			}
			else
				continue;
		}

		// Otherwise, consider this request/supply pair for queueing
		RequestSupplyPair rsp;

		rsp.is_item = false;
		rsp.is_worker = false;
		rsp.is_soldier = false;

		switch (req->get_type()) {
		case Request::WARE:    rsp.is_item    = true; break;
		case Request::WORKER:  rsp.is_worker  = true; break;
		case Request::SOLDIER: rsp.is_soldier = true; break;
		}

		rsp.ware = ware_index;
		rsp.request = req;
		rsp.supply = supp;
		rsp.priority = priority;

		log
			("REQ: %u (%i) <- %u (ware %i), priority %i\n",
			 req->get_target()->get_serial(),
			 req->get_required_time(), supp->get_position(g)->get_serial(),
			 rsp.ware, rsp.priority);

		s->queue.push(rsp);
	}

	// TODO: This function should be called from time to time
	create_requested_workers (g);
}

/**
 * Walk all Requests and find requests of workers than aren't supplied. Then
 * try to create the worker at warehouses.
*/
void Economy::create_requested_workers(Game* g)
{
	/*
		Find the request of workers that can not be supplied
	*/
	if (get_nr_warehouses() > 0) {
		for (RequestList::iterator it = m_requests.begin(); it != m_requests.end(); ++it) {
			Request* req = *it;

			if (!req->is_idle() && ((req->get_type()==Request::WORKER) || (req->get_type()==Request::SOLDIER))) {
				int32_t index = req->get_index();
				int32_t num_wares = 0;
            Worker_Descr* w_desc=get_owner()->tribe().get_worker_descr(index);

				// Ignore it if is a worker that cann't be buildable
				if (!w_desc->get_buildable())
					continue;

				for (int32_t i = 0; i < m_worker_supplies[index].get_nrsupplies(); i++) {
					Supply* supp = m_worker_supplies[index].get_supply(i);

					if (not supp->is_active()) {
						if (req->has_requeriments())
						{
							if (supp->get_passing_requeriments (g, index, req->get_requeriments()))
								num_wares++;
						}
						else
						{
							num_wares++;
							continue;
						}
					} // if (supp->is_active)
				} // for (int32_t i = 0; i < m_worker_supplies)

				// If there aren't enough supplies...
				if (num_wares == 0) {

					uint32_t n_wh = 0;
					while (n_wh < get_nr_warehouses()) {
						if (m_warehouses[n_wh]->can_create_worker(g, index)) {
							m_warehouses[n_wh]->create_worker(g, index);
							//break;
						} // if (m_warehouses[n_wh]
						n_wh++;
					} // while (n_wh < get_nr_warehouses())
				} // if (num_wares == 0)
			} // if (req->is_open())
		} // for (RequestList::iterator
	} // if (get_nr_warehouses())
}

/**
 * Balance Requests and Supplies by collecting and weighing pairs, and
 * starting transfers for them.
*/
void Economy::balance_requestsupply()
{
	RSPairStruct rsps;

   m_request_timer = false;

	rsps.nexttimer = -1;

	if (Game *const game = dynamic_cast<Game *>(&m_owner->egbase())) {

	// Try to fulfill non-idle Requests
	process_requests(game, &rsps);

	// Now execute request/supply pairs
	while (rsps.queue.size()) {
		RequestSupplyPair rsp = rsps.queue.top();

		rsps.queue.pop();

		if (!rsp.request || !rsp.supply ||
			!have_request(rsp.request) ||
			(((rsp.is_soldier)  && !have_worker_supply(rsp.ware, rsp.supply)) ||
			((rsp.is_worker)  && !have_worker_supply(rsp.ware, rsp.supply)) ||
			((rsp.is_item) && !have_ware_supply(rsp.ware, rsp.supply)))) {
			log("NO: ware %i, priority %i\n", rsp.ware, rsp.priority);

			rsps.nexttimer = 200;
			continue;
		}

		log
			("HANDLE: %u -> %u, ware %i, priority %i\n",
			 rsp.request->get_target()->get_serial(),
			 rsp.supply->get_position(game)->get_serial(),
			 rsp.ware,
			 rsp.priority);

		rsp.request->start_transfer(game, rsp.supply, rsp.ware);
		rsp.request->set_last_request_time(owner().egbase().get_gametime());

		// for multiple wares
		if (rsp.request && have_request(rsp.request)) {
			log("  request is still around, reschedule timer\n");
			rsps.nexttimer = 200;
		}
	}

	// restart the timer, if necessary
	if (rsps.nexttimer > 0) {
		log("  nexttimer: %i\n", rsps.nexttimer);
		start_request_timer(rsps.nexttimer);
	}
	}
}

/**
 * Called by Cmd_Queue as requested by start_request_timer().
 * Call economy functions to balance supply and request.
*/
void Cmd_Call_Economy_Balance::execute(Game* g) {
   Player* plr=g->get_player(m_player);

   // If this economy has vanished, drop this
   // call silently
   if (!plr->has_economy(m_economy))
      return;

   if (!m_economy->should_run_balance_check(g->get_gametime()) && !m_force_balance)
      return;

   m_force_balance = false;
   m_economy->balance_requestsupply();
}

/**
 * Read and write
 */
#define CURRENT_CMD_CALL_ECONOMY_VERSION 1
void Cmd_Call_Economy_Balance::Read
(FileRead               & fr,
 Editor_Game_Base                & egbase,
 Widelands_Map_Map_Object_Loader & mol)
{
	const Uint16 packet_version = fr.Unsigned16();
	if (packet_version == CURRENT_CMD_CALL_ECONOMY_VERSION) {
		// Read Base Commands
		GameLogicCommand::Read(fr, egbase, mol);

		m_player  = fr.Unsigned8 ();
		m_economy = fr.Unsigned8 () ?
			egbase.get_player(m_player)->get_economy_by_number(fr.Unsigned16())
			:
			reinterpret_cast<Economy *>(0xffffffff); //  FIXME ?!?!?!

      m_force_balance = true; // on load, the first balance has to been forced
	} else
		throw wexception
			("Unknown version %u in Cmd_Call_Economy_Balance::Read()!",
			 packet_version);
}
void Cmd_Call_Economy_Balance::Write
(FileWrite             & fw,
 Editor_Game_Base               & egbase,
 Widelands_Map_Map_Object_Saver & mos)
{
	fw.Unsigned16(CURRENT_CMD_CALL_ECONOMY_VERSION);

	// Write Base Commands
	GameLogicCommand::Write(fw, egbase, mos);

	fw.Unsigned8 (m_player);

   // Economy
	const Player & player = egbase.player(m_player);
	const bool has_eco = player.has_economy(m_economy);
	fw.Unsigned8 (has_eco);
	if (has_eco) fw.Unsigned16(player.get_economy_number(m_economy));
}
