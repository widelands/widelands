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

#include "request.h"

// Package includes
#include "economy.h"
#include "transfer.h"
#include "ware_instance.h"

#include "constructionsite.h"
#include "logic/game.h"
#include "logic/player.h"
#include "productionsite.h"
#include "soldier.h"
#include "tribe.h"
#include "upcast.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "map_io/widelands_map_map_object_saver.h"
#include "legacy.h"
#include "logic/warehouse.h"
#include "logic/worker.h"


namespace Widelands {

/*
==============================================================================

Request IMPLEMENTATION

==============================================================================
*/

Request::Request
	(PlayerImmovable & _target,
	 Ware_Index const index,
	 callback_t const cbfn,
	 Type       const w)
	:
	m_type             (w),
	m_target           (_target),
	m_target_building  (dynamic_cast<Building *>(&_target)),
	m_target_productionsite  (dynamic_cast<ProductionSite *>(&_target)),
	m_target_warehouse (dynamic_cast<Warehouse *>(&_target)),
	m_target_constructionsite (dynamic_cast<ConstructionSite *>(&_target)),
	m_economy          (_target.get_economy()),
	m_index            (index),
	m_idle             (false),
	m_count            (1),
	m_callbackfn       (cbfn),
	m_required_time    (_target.owner().egbase().get_gametime()),
	m_required_interval(0),
	m_last_request_time(m_required_time)
{
	assert(m_type == WARE or m_type == WORKER);
	if (w == WARE   and _target.owner().tribe().get_nrwares  () <= index)
		throw wexception
			("creating ware request with index %u, but tribe has only %u "
			 "ware types",
			 index.value(), _target.owner().tribe().get_nrwares  ().value());
	if (w == WORKER and _target.owner().tribe().get_nrworkers() <= index)
		throw wexception
			("creating worker request with index %u, but tribe has only %u "
			 "worker types",
			 index.value(), _target.owner().tribe().get_nrworkers().value());
	if (m_economy)
		m_economy->add_request(*this);
}

Request::~Request()
{
	// Remove from the economy
	if (is_open() and m_economy)
		m_economy->remove_request(*this);

	// Cancel all ongoing transfers
	while (m_transfers.size())
		cancel_transfer(0);
}

// Modified to allow Requirements and SoldierRequests
#define REQUEST_VERSION 4

/**
 * Read this request from a file
 *
 * it is most probably created by some init function,
 * so ad least target/economy is correct. Some Transports
 * might have been initialized. We have to kill them and replace
 * them through the data in the file
 */
void Request::Read
	(FileRead & fr, Game & game, Map_Map_Object_Loader * const mol)
{
	try {
		uint16_t const version = fr.Unsigned16();
		if (2 <= version and version <= REQUEST_VERSION) {
			Tribe_Descr const & tribe = m_target.owner().tribe();
			if (version <= 3) {
				//  Unfortunately, old versions wrote the index. The best thing
				//  that we can do with that is to look it up in a table.
				m_type = static_cast<Type>(fr.Unsigned8());
				if (m_type != WARE and m_type != WORKER)
					throw wexception
						("type is %u but must be %u (ware) or %u (worker)",
						 m_type, WARE, WORKER);
				uint32_t const legacy_index = fr.Unsigned32();
				m_index =
					m_type == WARE
					?
					Legacy::  ware_index
						(tribe,
						 m_target.descr().descname(),
						 "requests",
						 legacy_index)
					:
					Legacy::worker_index
						(tribe,
						 m_target.descr().descname(),
						 "requests",
						 legacy_index);
			} else {
				char const * const type_name = fr.CString();
				if (Ware_Index const wai = tribe.ware_index(type_name)) {
					m_type = WARE;
					m_index = wai;
				} else if (Ware_Index const woi = tribe.worker_index(type_name)) {
					m_type = WORKER;
					m_index = woi;
				} else
					throw wexception("request for unknown type \"%s\"", type_name);
			}
			m_idle              = fr.Unsigned8();
			m_count             = fr.Unsigned32();
			if (0 == m_count)
				throw wexception("count is 0");
			m_required_time     = fr.Unsigned32();
			m_required_interval = fr.Unsigned32();

			if (3 <= version)
				m_last_request_time = fr.Unsigned32();

			assert(m_transfers.empty());

			uint16_t const nr_transfers = fr.Unsigned16();
			for (uint16_t i = 0; i < nr_transfers; ++i)
				try {
					uint8_t const what_is = fr.Unsigned8();
					if (what_is != WARE and what_is != WORKER and what_is != 2)
						throw wexception
							("type is %u but must be one of {%u (WARE), %u (WORKER), "
							 "%u (SOLDIER)}",
							 what_is, WARE, WORKER, 2);
					uint32_t const reg = fr.Unsigned32();
					if (not mol->is_object_known(reg))
						throw wexception("%u is not known", reg);
					Transfer * const trans =
						what_is == WARE ?
						new Transfer(game, *this, mol->get<WareInstance>(reg)) :
						new Transfer(game, *this, mol->get<Worker>      (reg));
					trans->set_idle(fr.Unsigned8());
					m_transfers.push_back(trans);

					if (fr.Unsigned8())
						m_requirements.Read (fr, game, mol);
				} catch (_wexception const & e) {
					throw wexception("transfer %u: %s", i, e.what());
				}

			if (!is_open() && m_economy)
				m_economy->remove_request(*this);
		} else
			throw game_data_error(_("unknown/unhandled version %u"), version);
	} catch (_wexception const & e) {
		throw wexception("request: %s", e.what());
	}
}

/**
 * Write this request to a file
 */
void Request::Write
	(FileWrite & fw, Game & game, Map_Map_Object_Saver * mos) const
{
	fw.Unsigned16(REQUEST_VERSION);

	//  Target and econmy should be set. Same is true for callback stuff.

	assert(m_type == WARE or m_type == WORKER);
	Tribe_Descr const & tribe = m_target.owner().tribe();
	assert(m_type != WARE   or m_index < tribe.get_nrwares  ());
	assert(m_type != WORKER or m_index < tribe.get_nrworkers());
	fw.CString
		(m_type == WARE                          ?
		 tribe.get_ware_descr  (m_index)->name() :
		 tribe.get_worker_descr(m_index)->name());

	fw.Unsigned8(m_idle);

	fw.Unsigned32(m_count);

	fw.Unsigned32(m_required_time);
	fw.Unsigned32(m_required_interval);

	fw.Unsigned32(m_last_request_time);

	fw.Unsigned16(m_transfers.size()); //  Write number of current transfers.
	for (uint32_t i = 0; i < m_transfers.size(); ++i) {
		Transfer & trans = *m_transfers[i];
		//  is this a ware (or a worker)
		fw.Unsigned8(m_type);
		if        (trans.m_item) { //  write ware/worker
			assert(mos->is_object_known(*trans.m_item));
			fw.Unsigned32(mos->get_object_file_index(*trans.m_item));
		} else if (trans.m_worker) {
			assert(mos->is_object_known(*trans.m_worker));
			fw.Unsigned32(mos->get_object_file_index(*trans.m_worker));
		}
		fw.Unsigned8(trans.is_idle());

		fw.Unsigned8(true); //  for version compatibility
		m_requirements.Write (fw, game, mos);
	}
}

/**
 * Figure out the flag we need to deliver to.
*/
Flag & Request::target_flag() const
{
	return target().base_flag();
}

/**
 * Return the point in time at which we want the item of the given number to
 * be delivered. nr is in the range [0..m_count[
*/
int32_t Request::get_base_required_time
	(Editor_Game_Base & egbase, uint32_t const nr) const
{
	if (m_count <= nr)
		log
			("Request::get_base_required_time: WARNING nr = %u but count is %u, "
			 "which is not allowed according to the comment for this function\n",
			 nr, m_count);
	int32_t const curtime = egbase.get_gametime();

	if (!nr || !m_required_interval)
		return m_required_time;

	if ((curtime - m_required_time) > (m_required_interval * 2)) {
		if (nr == 1)
			return m_required_time + (curtime - m_required_time) / 2;

		assert(2 <= nr);
		return curtime + (nr - 2) * m_required_interval;
	}

	return m_required_time + nr * m_required_interval;
}

/**
 * Return the time when the requested ware is needed.
 * Can be in the past, indicating that we have been idling, waiting for the
 * ware.
*/
int32_t Request::get_required_time() const
{
	return
		get_base_required_time(m_economy->owner().egbase(), m_transfers.size());
}

//#define MAX_IDLE_PRIORITY           100
#define PRIORITY_MAX_COST         50000
#define COST_WEIGHT_IN_PRIORITY       1
#define WAITTIME_WEIGHT_IN_PRIORITY   2

/**
 * Return the request priority used to sort requests or -1 to skip request
 */
int32_t Request::get_priority (int32_t cost) const
{
	int MAX_IDLE_PRIORITY = 100;
	bool is_construction_site = false;
	int32_t modifier = DEFAULT_PRIORITY;

	if (m_target_building) {
		if (m_target_productionsite && m_target_productionsite->is_stopped())
			return -1;

		modifier = m_target_building->get_priority(get_type(), get_index());
		if (m_target_constructionsite)
			is_construction_site = true;
		else if (m_target_warehouse)
			// warehouses determine max idle priority
			return modifier;
	}


	if (is_idle()) //  idle requests are prioritized only by cost
		return
			std::max
				(0,
				 MAX_IDLE_PRIORITY - cost * MAX_IDLE_PRIORITY / PRIORITY_MAX_COST);

	if (cost > PRIORITY_MAX_COST)
		cost = PRIORITY_MAX_COST;

	// priority is higher if building waits for ware a long time
	// additional factor - cost to deliver, so nearer building
	// with same priority will get ware first
	//  make sure that idle request are lower
	return
		MAX_IDLE_PRIORITY
		+
		std::max
			(1,
			 ((m_economy->owner().egbase().get_gametime() -
			   (is_construction_site ?
			    get_required_time() : get_last_request_time()))
			  *
			  WAITTIME_WEIGHT_IN_PRIORITY
			  +
			  (PRIORITY_MAX_COST - cost) * COST_WEIGHT_IN_PRIORITY)
			 *
			 modifier);
}

/**
 * Change the Economy we belong to.
*/
void Request::set_economy(Economy * const e)
{
	if (m_economy != e) {
		if (m_economy && is_open())
			m_economy->remove_request(*this);
		m_economy = e;
		if (m_economy && is_open())
			m_economy->   add_request(*this);
	}
}

/**
 * Make a Request idle or not idle.
*/
void Request::set_idle(bool const idle)
{
	if (m_idle != idle) {
		bool const wasopen = is_open();
		m_idle = idle;
		if (m_economy) { //  Idle requests are always added to the economy.
			if       (wasopen && !is_open())
				m_economy->remove_request(*this);
			else if (!wasopen &&  is_open())
				m_economy->   add_request(*this);
		}
	}
}

/**
 * Change the number of wares we need.
*/
void Request::set_count(uint32_t const count)
{
	bool const wasopen = is_open();

	m_count = count;

	// Cancel unneeded transfers. This should be more clever about which
	// transfers to cancel. Then again, this loop shouldn't execute during
	// normal play anyway
	while (m_count < m_transfers.size())
		cancel_transfer(m_transfers.size() - 1);

	// Update the economy
	if (m_economy) {
		if (wasopen && !is_open())
			m_economy->remove_request(*this);
		else if (!wasopen && is_open())
			m_economy->add_request(*this);
	}
}

/**
 * Change the time at which the first item to be delivered is needed.
 * Default is the gametime of the Request creation.
*/
void Request::set_required_time(int32_t const time)
{
	m_required_time = time;
}

/**
 * Change the time between desired delivery of items.
*/
void Request::set_required_interval(int32_t const interval)
{
	m_required_interval = interval;
}

/**
 * Begin transfer of the requested ware from the given supply.
 * This function does not take ownership of route, i.e. the caller is
 * responsible for its deletion.
*/
void Request::start_transfer(Game & game, Supply & supp)
{
	assert(is_open());

	::StreamWrite & ss = game.syncstream();
	ss.Unsigned32(0x01decafa); // appears as facade01 in sync stream
	ss.Unsigned32(target().serial());
	ss.Unsigned32(supp.get_position(game)->serial());

	Transfer * t;
	if (get_type() == WORKER) {
		//  Begin the transfer of a soldier or worker.
		//  launch_worker() creates or starts the worker
		Worker & s = supp.launch_worker(game, *this);
		ss.Unsigned32(s.serial());
		t = new Transfer(game, *this, s);
	} else {
		//  Begin the transfer of an item. The item itself is passive.
		//  launch_item() ensures the WareInstance is transported out of the
		//  warehouse Once it's on the flag, the flag code will decide what to
		//  do with it.
		WareInstance & item = supp.launch_item(game, *this);
		ss.Unsigned32(item.serial());
		t = new Transfer(game, *this, item);
	}

	t->set_idle(m_idle);

	m_transfers.push_back(t);
	if (!is_open())
		m_economy->remove_request(*this);
}

/**
 * Callback from ware/worker code that the requested ware has arrived.
 * This will call a callback function in the target, which is then responsible
 * for removing and deleting the request.
*/
void Request::transfer_finish(Game & game, Transfer & t)
{
	Worker * const w = t.m_worker;

	if (t.m_item)
		t.m_item->destroy(game);

	t.m_worker = 0;
	t.m_item = 0;

	remove_transfer(find_transfer(t));

	if (!m_idle) {
		set_required_time(get_base_required_time(game, 1));
		--m_count;
	}

	// the callback functions are likely to delete us,
	// therefore we musn't access member variables behind this
	// point
	(*m_callbackfn)(game, *this, m_index, w, m_target);
}

/**
 * Callback from ware/worker code that the scheduled transfer has failed.
 * The calling code has already dealt with the worker/item.
 *
 * Re-open the request.
*/
void Request::transfer_fail(Game &, Transfer & t) {
	bool const wasopen = is_open();

	t.m_worker = 0;
	t.m_item = 0;

	remove_transfer(find_transfer(t));

	if (!wasopen)
		m_economy->add_request(*this);
}

/// Cancel the transfer with the given index.
///
/// \note This does *not* update whether the \ref Request is registered with
/// the \ref Economy or not.
void Request::cancel_transfer(uint32_t const idx)
{
	remove_transfer(idx);
}

/**
 * Remove and free the transfer with the given index.
 * This does not update the Transfer's worker or item, and it does not update
 * whether the Request is registered with the Economy.
 */
void Request::remove_transfer(uint32_t const idx)
{
	Transfer * const t = m_transfers[idx];

	m_transfers.erase(m_transfers.begin() + idx);

	delete t;
}

/**
 * Lookup a \ref Transfer in the transfers array.
 * \throw wexception if the \ref Transfer is not registered with us.
 */
uint32_t Request::find_transfer(Transfer & t)
{
	TransferList::const_iterator const it =
		std::find(m_transfers.begin(), m_transfers.end(), &t);

	if (it == m_transfers.end())
		throw wexception("Request::find_transfer(): not found");

	return it - m_transfers.begin();
}

}
