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

#include "request.h"

#include "building.h"
#include "game.h"
#include "player.h"
#include "soldier.h"
#include "transport.h"
#include "tribe.h"
#include "upcast.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"
#include "worker.h"


namespace Widelands {

/*
==============================================================================

Request IMPLEMENTATION

==============================================================================
*/

Request::Request
	(PlayerImmovable * target,
	 Ware_Index const index,
	 callback_t cbfn,
	 void * const cbdata,
	 Type w)
	:
	m_type             (w),
	m_target           (target),
	m_economy          (m_target->get_economy()),
	m_index            (index),
	m_idle             (false),
	m_count            (1),
	m_callbackfn       (cbfn),
	m_callbackdata     (cbdata),
	m_required_time    (target->owner().egbase().get_gametime()),
	m_required_interval(0),
	m_last_request_time(m_required_time)
{
	if (m_economy)
		m_economy->add_request(this);
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
}

// Modified to allow Requirements and SoldierRequests
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
void Request::Read
	(FileRead * fr, Editor_Game_Base * egbase, Map_Map_Object_Loader * mol)
{
	uint16_t const version = fr->Unsigned16();
	if (version >= REQUEST_SUPPORTED_VERSION) {
		m_type              = static_cast<Type>(fr->Unsigned8());
		m_index             = static_cast<Ware_Index::value_t>(fr->Unsigned32());
		m_idle              = fr->Unsigned8();
		m_count             = fr->Unsigned32();
		m_required_time     = fr->Unsigned32();
		m_required_interval = fr->Unsigned32();

		if (version == REQUEST_VERSION)
			m_last_request_time = fr->Unsigned32();

		assert(!m_transfers.size());

		const uint16_t nr_transfers = fr->Unsigned16();
		for (uint16_t i = 0; i < nr_transfers; ++i) {
			uint8_t const what_is = fr->Unsigned8();
			if (what_is != WARE and what_is != WORKER and what_is != 2)
				throw wexception
					("Request::Read: while reading transfer %u: type is %u but "
					 "must be one of {%u (WARE), %u (WORKER), %u (SOLDIER)}",
					 i, what_is, WARE, WORKER, 2);
			uint32_t const reg = fr->Unsigned32();
			if (upcast(Game, game, egbase)) {
				if (not mol->is_object_known(reg))
					throw wexception
						("Request::Read: while reading %s transfer %u: %u is not "
						 "known",
						 what_is == WARE   ? "ware"   :
						 what_is == WORKER ? "worker" :
						 "soldier",
						 i, reg);
				Transfer * const trans =
					what_is == WARE ?
					new Transfer
					(game,
					 this,
					 &mol->get<WareInstance>(reg))
					:
					new Transfer
					(game,
					 this,
					 &mol->get<Worker>(reg));
				trans->set_idle(fr->Unsigned8());
				m_transfers.push_back(trans);

				if (fr->Unsigned8())
					m_requirements.Read (fr, egbase, mol);
			}
		}

		if (!is_open() && m_economy)
			m_economy->remove_request(this);
	} else
		throw wexception("Unknown request version %i in file!", version);
}

/**
 * Write this request to a file
 */
void Request::Write
	(FileWrite * fw, Editor_Game_Base * egbase, Map_Map_Object_Saver * mos)
{
	fw->Unsigned16(REQUEST_VERSION);

	//  Target and econmy should be set. Same is true for callback stuff.

	fw->Unsigned8(m_type);

	fw->Unsigned32(m_index.value());

	fw->Unsigned8(m_idle);

	fw->Unsigned32(m_count);

	fw->Unsigned32(m_required_time);
	fw->Unsigned32(m_required_interval);

	fw->Unsigned32(m_last_request_time);

	fw->Unsigned16(m_transfers.size()); //  Write number of current transfers.
	for (uint32_t i = 0; i < m_transfers.size(); ++i) {
		Transfer & trans = *m_transfers[i];
		//  is this a ware (or a worker)
		fw->Unsigned8(m_type);
		if        (trans.m_item) { //  write ware/worker
			assert(mos->is_object_known(trans.m_item));
			fw->Unsigned32(mos->get_object_file_index(trans.m_item));
		} else if (trans.m_worker) {
			assert(mos->is_object_known(trans.m_worker));
			fw->Unsigned32(mos->get_object_file_index(trans.m_worker));
		}
		fw->Unsigned8(trans.is_idle());

		fw->Unsigned8(true); // for version compatibility
		m_requirements.Write (fw, egbase, mos);
	}
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
		return
			weighted_cost > MAX_IDLE_PRIORITY ?
			0 : MAX_IDLE_PRIORITY - weighted_cost;
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
 * Begin transfer of the requested ware from the given supply.
 * This function does not take ownership of route, i.e. the caller is
 * responsible for its deletion.
*/
void Request::start_transfer(Game* g, Supply* supp)
{
	assert(is_open());

	::StreamWrite & ss = g->syncstream();
	ss.Unsigned32(0x01decafa); // appears as facade01 in sync stream
	ss.Unsigned32(get_target()->get_serial());
	ss.Unsigned32(supp->get_position(g)->get_serial());

	Transfer* t = 0;
	try
	{
		if (get_type() == WORKER)
		{
			// Begin the transfer of a soldier or worker.
			// launch_worker() creates or starts the worker
			log("Request: start soldier or worker transfer for %i\n", get_index().value());

			Worker* s = supp->launch_worker(g, this);
			ss.Unsigned32(s->get_serial());
			t = new Transfer(g, this, s);
		}
		else
		{
			// Begin the transfer of an item. The item itself is passive.
			// launch_item() ensures the WareInstance is transported out of the warehouse
			// Once it's on the flag, the flag code will decide what to do with it.
			WareInstance & item = supp->launch_item(g, this);
			ss.Unsigned32(item.get_serial());
			t = new Transfer(g, this, &item);
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
	Worker* w = t->m_worker;

	if (t->m_item)
		t->m_item->destroy(g);

	t->m_worker = 0;
	t->m_item = 0;

	remove_transfer(find_transfer(t));

	if (!m_idle) {
		set_required_time(get_base_required_time(g, 1));
		--m_count;
	}

	// the callback functions are likely to delete us,
	// therefore we musn't access member variables behind this
	// point
	(*m_callbackfn)(g, this, m_index, w, m_callbackdata);


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

	t->m_worker = 0;
	t->m_item = 0;

	remove_transfer(find_transfer(t));

	if (!wasopen)
		m_economy->add_request(this);
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


std::string Request::describe() const
{
	if (get_type() == WARE)
		return m_economy->owner().tribe().get_ware_descr(get_index())->name();
	else
		return m_economy->owner().tribe().get_worker_descr(get_index())->name();
}

}
