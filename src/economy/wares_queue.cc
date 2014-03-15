/*
 * Copyright (C) 2004, 2006-2011 by the Widelands Development Team
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

#include "economy/wares_queue.h"

#include "economy/economy.h"
#include "economy/request.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/player.h"
#include "logic/tribe.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "map_io/widelands_map_map_object_saver.h"
#include "wexception.h"

namespace Widelands {

/**
 * Pre-initialize a WaresQueue
*/
WaresQueue::WaresQueue
	(PlayerImmovable &       _owner,
	 Ware_Index        const _ware,
	 uint8_t           const _max_size)
	:
	m_owner           (_owner),
	m_ware            (_ware),
	m_max_size        (_max_size),
	m_max_fill        (_max_size),
	m_filled          (0),
	m_consume_interval(0),
	m_request         (nullptr),
	m_callback_fn     (nullptr),
	m_callback_data   (nullptr)
{
	if (m_ware)
		update();
}


/**
 * Clear the queue appropriately.
*/
void WaresQueue::cleanup() {
	assert(m_ware);

	if (m_filled && m_owner.get_economy())
		m_owner.get_economy()->remove_wares(m_ware, m_filled);

	m_filled = 0;
	m_max_size = 0;
	m_max_fill = 0;

	update();

	m_ware = Ware_Index::Null();
}

/**
 * Fix filled <= size and requests.
 * You must call this after every call to set_*()
*/
void WaresQueue::update() {
	assert(m_ware);

	if (m_filled > m_max_size) {
		if (m_owner.get_economy())
			m_owner.get_economy()->remove_wares(m_ware, m_filled - m_max_size);
		m_filled = m_max_size;
	}

	if (m_filled < m_max_fill)
	{
		if (!m_request)
			m_request =
				new Request
					(m_owner,
					 m_ware,
					 WaresQueue::request_callback,
					 wwWARE);

		m_request->set_count(m_max_fill - m_filled);
		m_request->set_required_interval(m_consume_interval);
	}
	else
	{
		delete m_request;
		m_request = nullptr;
	}
}

/**
 * Set the callback function that is called when an item has arrived.
*/
void WaresQueue::set_callback(callback_t * const fn, void * const data)
{
	m_callback_fn = fn;
	m_callback_data = data;
}

/**
 * Called when an item arrives at the owning building.
*/
void WaresQueue::request_callback
	(Game            &       game,
	 Request         &,
	 Ware_Index        const ware,
#ifndef NDEBUG
	 Worker          * const w,
#else
	 Worker          *,
#endif
	 PlayerImmovable & target)
{
	WaresQueue & wq =
		ref_cast<Building, PlayerImmovable>(target).waresqueue(ware);

	assert(!w); // WaresQueue can't hold workers
	assert(wq.m_filled < wq.m_max_size);
	assert(wq.m_ware == ware);

	// Update
	wq.set_filled(wq.m_filled + 1);

	if (wq.m_callback_fn)
		(*wq.m_callback_fn)(game, &wq, ware, wq.m_callback_data);
}

/**
 * Remove the wares in this queue from the given economy (used in accounting).
*/
void WaresQueue::remove_from_economy(Economy & e)
{
	if (m_ware) {
		e.remove_wares(m_ware, m_filled);
		if (m_request)
			m_request->set_economy(nullptr);
	}
}

/**
 * Add the wares in this queue to the given economy (used in accounting)
*/
void WaresQueue::add_to_economy(Economy & e)
{
	if (m_ware) {
		e.add_wares(m_ware, m_filled);
		if (m_request)
			m_request->set_economy(&e);
	}
}

/**
 * Change size of the queue.
 */
void WaresQueue::set_max_size(const uint32_t size)
{
	uint32_t old_size = m_max_size;
	m_max_size = size;

	// make sure that max fill is reduced as well if the max size is decreased
	// because this is very likely what the user wanted to only consume so
	// and so many wares in the first place. If it is increased, keep the
	// max fill fill as it was
	set_max_fill(std::min(m_max_fill, m_max_fill - (old_size - m_max_size)));

	update();
}

/**
 * Change the number of wares that should be available in this queue
 *
 * This is basically the same as setting the maximum size,
 * but if there are more wares than that in the queue, they will not get
 * lost (the building should drop them).
 */
void WaresQueue::set_max_fill(uint32_t size)
{
	if (size > m_max_size)
		size = m_max_size;

	m_max_fill = size;

	update();
}

/**
 * Change fill status of the queue.
 */
void WaresQueue::set_filled(const uint32_t filled) {
	if (m_owner.get_economy()) {
		if (filled > m_filled)
			m_owner.get_economy()->add_wares(m_ware, filled - m_filled);
		else if (filled < m_filled)
			m_owner.get_economy()->remove_wares(m_ware, m_filled - filled);
	}

	m_filled = filled;

	update();
}

/**
 * Set the time between consumption of items when the owning building
 * is consuming at full speed.
 *
 * This interval is merely a hint for the Supply/Request balancing code.
*/
void WaresQueue::set_consume_interval(const uint32_t time)
{
	m_consume_interval = time;

	update();
}

/**
 * Read and write
 */
#define WARES_QUEUE_DATA_PACKET_VERSION 2
void WaresQueue::Write(FileWrite & fw, Game & game, Map_Map_Object_Saver & mos)
{
	fw.Unsigned16(WARES_QUEUE_DATA_PACKET_VERSION);

	//  Owner and callback is not saved, but this should be obvious on load.
	fw.CString
		(owner().tribe().get_ware_descr(m_ware)->name().c_str());
	fw.Signed32(m_max_size);
	fw.Signed32(m_max_fill);
	fw.Signed32(m_filled);
	fw.Signed32(m_consume_interval);
	if (m_request) {
		fw.Unsigned8(1);
		m_request->Write(fw, game, mos);
	} else
		fw.Unsigned8(0);
}


void WaresQueue::Read(FileRead & fr, Game & game, Map_Map_Object_Loader & mol)
{
	uint16_t const packet_version = fr.Unsigned16();
	try {
		if (packet_version == WARES_QUEUE_DATA_PACKET_VERSION or packet_version == 1) {
			delete m_request;
			m_ware             = owner().tribe().ware_index(fr.CString  ());
			m_max_size         =                            fr.Unsigned32();
			if (packet_version == 1)
				m_max_fill = m_max_size;
			else
				m_max_fill = fr.Signed32();
			m_filled           =                            fr.Unsigned32();
			m_consume_interval =                            fr.Unsigned32();
			if                                             (fr.Unsigned8 ()) {
				m_request =                          //  FIXME Change Request::Read
					new Request                       //  FIXME to a constructor.
						(m_owner,                      //  FIXME
						 Ware_Index::First(),          //  FIXME
						 WaresQueue::request_callback, //  FIXME
						 wwWORKER);             //  FIXME
				m_request->Read(fr, game, mol);      //  FIXME
			} else
				m_request = nullptr;

			//  Now Economy stuff. We have to add our filled items to the economy.
			if (m_owner.get_economy())
				add_to_economy(*m_owner.get_economy());
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const game_data_error & e) {
		throw game_data_error("waresqueue: %s", e.what());
	}
}

}
