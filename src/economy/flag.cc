/*
 * Copyright (C) 2004, 2006-2009 by the Widelands Development Team
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

#include "flag.h"

// Package includes
#include "road.h"
#include "economy.h"
#include "ware_instance.h"

#include "logic/building.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/instances.h"
#include "logic/player.h"
#include "request.h"
#include "tribe.h"
#include "upcast.h"
#include "wexception.h"
#include "logic/worker.h"

namespace Widelands {

Map_Object_Descr g_flag_descr("flag", "Flag");

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

	for (int32_t i = 0; i < 6; ++i)
		if (m_roads[i])
			log("Flag: ouch! road left\n");
}

/**
 * Create a flag at the given location
*/
Flag::Flag
	(Game & game, Player & owning_player, Coords const coords)
	:
	PlayerImmovable       (g_flag_descr),
	m_anim                (0),
	m_building            (0),
	m_item_capacity       (8),
	m_item_filled         (0),
	m_items               (new PendingItem[m_item_capacity]),
	m_always_call_for_flag(0)
{
	for (uint32_t i = 0; i < 6; ++i) m_roads[i] = 0;

	set_owner(&owning_player);

	set_flag_position(coords);


	upcast(Road, road, game.map().get_immovable(coords));
	//  we split a road, or a new, standalone flag is created
	(road ? road->get_economy() : new Economy(owning_player))->add_flag(*this);

	if (road)
		road->presplit(game, coords);
	init(game);
	if (road)
		road->postsplit(game, *this);
}

void Flag::set_flag_position(Coords coords) {
	m_position = coords;
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


static std::string const flag_name = "flag";
std::string const & Flag::name() const throw () {return flag_name;}


Flag & Flag::base_flag()
{
	return *this;
}

/**
 * Call this only from Economy code!
*/
void Flag::set_economy(Economy *e)
{
	Economy * const old = get_economy();

	if (old == e)
		return;

	PlayerImmovable::set_economy(e);

	for (int32_t i = 0; i < m_item_filled; ++i)
		m_items[i].item->set_economy(e);

	if (m_building)
		m_building->set_economy(e);

	container_iterate_const(FlagJobs, m_flag_jobs, i)
		i.current->request->set_economy(e);

	for (int8_t i = 0; i < 6; ++i) {
		if (m_roads[i])
			m_roads[i]->set_economy(e);
	}
}

/**
 * Call this only from the Building init!
*/
void Flag::attach_building(Editor_Game_Base & egbase, Building & building)
{
	assert(!m_building || m_building == &building);

	m_building = &building;

	Map const & map = egbase.map();
	egbase.set_road
		(map.get_fcoords(map.tl_n(m_position)), Road_SouthEast, Road_Normal);

	building.set_economy(get_economy());
}

/**
 * Call this only from the Building cleanup!
*/
void Flag::detach_building(Editor_Game_Base & egbase)
{
	assert(m_building);

	m_building->set_economy(0);

	Map const & map = egbase.map();
	egbase.set_road
		(map.get_fcoords(map.tl_n(m_position)), Road_SouthEast, Road_None);

	m_building = 0;
}

/**
 * Call this only from the Road init!
*/
void Flag::attach_road(int32_t const dir, Road * const road)
{
	assert(!m_roads[dir - 1] || m_roads[dir - 1] == road);

	m_roads[dir - 1] = road;
	m_roads[dir - 1]->set_economy(get_economy());
}

/**
 * Call this only from the Road init!
*/
void Flag::detach_road(int32_t const dir)
{
	assert(m_roads[dir - 1]);

	m_roads[dir - 1]->set_economy(0);
	m_roads[dir - 1] = 0;
}

/**
 * Return neighbouring flags.
*/
void Flag::get_neighbours(RoutingNodeNeighbours *neighbours)
{
	for (int8_t i = 0; i < 6; ++i) {
		Road * const road = m_roads[i];
		if (!road)
			continue;

		Flag * f = &road->get_flag(Road::FlagEnd);
		int32_t nb_cost;
		if (f != this)
			nb_cost = road->get_cost(Road::FlagStart);
		else {
			f = &road->get_flag(Road::FlagStart);
			nb_cost = road->get_cost(Road::FlagEnd);
		}
		RoutingNodeNeighbour n(f, nb_cost);

		assert(n.get_neighbour() != this);
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
	for (int8_t i = 0; i < 6; ++i) {
		Road * const road = m_roads[i];
		if (!road)
			continue;

		if
			(&road->get_flag(Road::FlagStart) == flag ||
			 &road->get_flag(Road::FlagEnd)   == flag)
			return road;
	}

	return 0;
}


bool Flag::is_dead_end() const {
	if (get_building())
		return false;
	Flag const * first_other_flag = 0;
	for (uint8_t road_id = 6; road_id; --road_id)
		if (Road * const road = get_road(road_id)) {
			Flag & start = road->get_flag(Road::FlagStart);
			Flag & other = this == &start ? road->get_flag(Road::FlagEnd) : start;
			if (first_other_flag) {
				if (&other != first_other_flag)
					return false;
			} else
				first_other_flag = &other;
		}
	return true;
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
 *
 * The capacity queue is a simple FIFO queue.
 */
void Flag::wait_for_capacity(Game &, Worker & bob)
{
	m_capacity_wait.push_back(&bob);
}

/**
 * Remove the worker from the list of workers waiting for free capacity.
 */
void Flag::skip_wait_for_capacity(Game &, Worker & w)
{
	CapacityWaitQueue::iterator const it =
		std::find(m_capacity_wait.begin(), m_capacity_wait.end(), &w);
	if (it != m_capacity_wait.end())
		m_capacity_wait.erase(it);
}


void Flag::add_item(Game & game, WareInstance & item)
{

	assert(m_item_filled < m_item_capacity);

	PendingItem & pi = m_items[m_item_filled++];
	pi.item     = &item;
	pi.pending  = false;
	pi.nextstep = 0;

	item.set_location(game, this);
	item.update(game); //  will call call_carrier() if necessary
}

/**
 * \return true if an item is currently waiting for a carrier to the given Flag.
 *
 * \note Due to fetch_from_flag() semantics, this function makes no sense
 * for a  building destination.
*/
bool Flag::has_pending_item(Game &, Flag & dest) {
	for (int32_t i = 0; i < m_item_filled; ++i) {
		if (!m_items[i].pending)
			continue;

		if (m_items[i].nextstep != &dest)
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
bool Flag::ack_pending_item(Game &, Flag & destflag) {
	for (int32_t i = 0; i < m_item_filled; ++i) {
		if (!m_items[i].pending)
			continue;

		if (m_items[i].nextstep != &destflag)
			continue;

		m_items[i].pending = false;
		return true;
	}

	return false;
}

/**
 * Wake one sleeper from the capacity queue.
*/
void Flag::wake_up_capacity_queue(Game & game)
{
	while (m_capacity_wait.size()) {
		Worker * const w = m_capacity_wait[0].get(game);
		m_capacity_wait.erase(m_capacity_wait.begin());
		if (w and w->wakeup_flag_capacity(game, *this))
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
WareInstance * Flag::fetch_pending_item(Game & game, PlayerImmovable & dest)
{
	int32_t best_index = -1;

	for (int32_t i = 0; i < m_item_filled; ++i) {
		if (m_items[i].nextstep != &dest)
			continue;

		// We prefer to retrieve items that have already been acked
		if (best_index < 0 || !m_items[i].pending)
			best_index = i;
	}

	if (best_index < 0)
		return 0;

	// move the other items up the list and return this one
	WareInstance * const item = m_items[best_index].item;
	--m_item_filled;
	memmove
		(&m_items[best_index], &m_items[best_index + 1],
		 sizeof(m_items[0]) * (m_item_filled - best_index));

	item->set_location(game, 0);

	// wake up capacity wait queue
	wake_up_capacity_queue(game);

	return item;
}

/**
 * Force a removal of the given item from this flag.
 * Called by \ref WareInstance::cleanup()
*/
void Flag::remove_item(Game & game, WareInstance * const item)
{
	for (int32_t i = 0; i < m_item_filled; ++i) {
		if (m_items[i].item != item)
			continue;

		--m_item_filled;
		memmove
			(&m_items[i], &m_items[i + 1],
			 sizeof(m_items[0]) * (m_item_filled - i));

		wake_up_capacity_queue(game);

		return;
	}

	throw wexception
		("MO(%u): Flag::remove_item: item %u not on flag",
		 serial(), item->serial());
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
 * This behaviour is overridden by m_always_call_for_step, which is set by
 * update_items() to ensure that new carriers are called when roads are
 * split, for example.
*/
void Flag::call_carrier
	(Game & game, WareInstance & item, PlayerImmovable * const nextstep)
{
	PendingItem * pi = 0;
	int32_t i = 0;

	// Find the PendingItem entry
	for (; i < m_item_filled; ++i) {
		if (m_items[i].item != &item)
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
	if (nextstep == get_building()) {
		molog
			("Flag::call_carrier(%u): Tell building to fetch this item\n",
			 item.serial());

		if (!get_building()->fetch_from_flag(game)) {
			pi->item->cancel_moving();
			pi->item->update(game);
		}

		return;
	}

	// Deal with the normal (flag) case
	ref_cast<Flag const, PlayerImmovable const>(*nextstep);

	for (int32_t dir = 1; dir <= 6; ++dir) {
		Road * const road = get_road(dir);
		Flag *       other;
		Road::FlagId flagid;

		if (!road)
			continue;

		if (&road->get_flag(Road::FlagStart) == this) {
			flagid = Road::FlagStart;
			other = &road->get_flag(Road::FlagEnd);
		} else {
			flagid = Road::FlagEnd;
			other = &road->get_flag(Road::FlagStart);
		}

		if (other != nextstep)
			continue;

		// Yes, this is the road we want; inform it
		if (road->notify_ware(game, flagid))
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
void Flag::update_items(Game & game, Flag * const other)
{
	m_always_call_for_flag = other;

	for (int32_t i = 0; i < m_item_filled; ++i)
		m_items[i].item->update(game);

	m_always_call_for_flag = 0;
}

void Flag::init(Editor_Game_Base & egbase)
{
	PlayerImmovable::init(egbase);

	set_position(egbase, m_position);

	m_anim = owner().tribe().get_flag_anim();
	m_animstart = egbase.get_gametime();
}

/**
 * Detach building and free roads.
*/
void Flag::cleanup(Editor_Game_Base & egbase)
{
	//molog("Flag::cleanup\n");

	while (m_flag_jobs.size()) {
		delete m_flag_jobs.begin()->request;
		m_flag_jobs.erase(m_flag_jobs.begin());
	}

	while (m_item_filled) {
		WareInstance & item = *m_items[--m_item_filled].item;

		item.set_location(ref_cast<Game, Editor_Game_Base>(egbase), 0);
		item.destroy     (ref_cast<Game, Editor_Game_Base>(egbase));
	}

	//molog("  items destroyed\n");

	if (m_building) {
		m_building->remove(egbase); //  immediate death
		assert(!m_building);
	}

	for (int8_t i = 0; i < 6; ++i) {
		if (m_roads[i]) {
			m_roads[i]->remove(egbase); //  immediate death
			assert(!m_roads[i]);
		}
	}

	get_economy()->remove_flag(*this);

	unset_position(egbase, m_position);

	//molog("  done\n");

	PlayerImmovable::cleanup(egbase);
}

/**
 * Destroy the building as well.
 *
 * \note This is needed in addition to the call to m_building->remove() in
 * \ref Flag::cleanup(). This function is needed to ensure a fire is created
 * when a player removes a flag.
*/
void Flag::destroy(Editor_Game_Base & egbase)
{
	if (m_building) {
		m_building->destroy(egbase);
		assert(!m_building);
	}

	PlayerImmovable::destroy(egbase);
}

/**
 * Add a new flag job to request the worker with the given ID, and to execute
 * the given program once it's completed.
*/
void Flag::add_flag_job
	(Game &, Ware_Index const workerware, std::string const & programname)
{
	FlagJob j;

	j.request =
		new Request
			(*this, workerware, Flag::flag_job_request_callback, Request::WORKER);
	j.program = programname;

	m_flag_jobs.push_back(j);
}

/**
 * This function is called when one of the flag job workers arrives on
 * the flag. Give him his job.
*/
void Flag::flag_job_request_callback
	(Game            &       game,
	 Request         &       rq,
	 Ware_Index,
	 Worker          * const w,
	 PlayerImmovable &       target)
{
	Flag & flag = ref_cast<Flag, PlayerImmovable>(target);

	assert(w);

	container_iterate(FlagJobs, flag.m_flag_jobs, i)
		if (i.current->request == &rq) {
			delete &rq;

			w->start_task_program(game, i.current->program);

			flag.m_flag_jobs.erase(i.current);
			return;
		}

	flag.molog("BUG: flag_job_request_callback: worker not found in list\n");
}

}
