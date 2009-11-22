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

#include "wares_queue.h"

// Package includes
#include "economy.h"

#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "map_io/widelands_map_map_object_saver.h"
#include "logic/player.h"
#include "request.h"
#include "logic/tribe.h"
#include "wexception.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"

namespace Widelands {

/**
 * Pre-initialize a WaresQueue
*/
WaresQueue::WaresQueue
	(PlayerImmovable &       _owner,
	 Ware_Index        const _ware,
	 uint8_t           const _size,
	 uint8_t           const _filled)
	:
	m_owner           (_owner),
	m_ware            (_ware),
	m_size            (_size),
	m_filled          (_filled),
	m_consume_interval(0),
	m_request         (0),
	m_callback_fn     (0),
	m_callback_data   (0)
{}


/**
 * Clear the queue appropriately.
*/
void WaresQueue::cleanup() {
	assert(m_ware);

	if (uint8_t const count = m_filled)
		m_owner.get_economy()->remove_wares(m_ware, count);

	m_filled = 0;
	m_size = 0;

	update();

	m_ware = Ware_Index::Null();
}

/**
 * Fix filled <= size and requests.
 * You must call this after every call to set_*()
*/
void WaresQueue::update() {
	assert(m_ware);

	if (m_filled > m_size) {
		m_owner.get_economy()->remove_wares(m_ware, m_filled - m_size);
		m_filled = m_size;
	}

	if (m_filled < m_size)
	{
		if (!m_request)
			m_request =
				new Request
					(m_owner,
					 m_ware,
					 WaresQueue::request_callback,
					 Request::WARE);

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
	assert(wq.m_filled < wq.m_size);
	assert(wq.m_ware == ware);

	// Update
	wq.set_filled(wq.m_filled + 1);
	wq.update();

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
			m_request->set_economy(0);
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
void WaresQueue::set_filled(const uint32_t filled) throw () {
	if (filled > m_filled)
		m_owner.get_economy()->add_wares(m_ware, filled - m_filled);
	else if (filled < m_filled)
		m_owner.get_economy()->remove_wares(m_ware, m_filled - filled);

	m_filled = filled;
}

/**
 * Set the time between consumption of items when the owning building
 * is consuming at full speed.
 *
 * This interval is merely a hint for the Supply/Request balancing code.
*/
void WaresQueue::set_consume_interval(const uint32_t time) throw ()
{
	m_consume_interval = time;
}

/**
 * Read and write
 */
#define WARES_QUEUE_DATA_PACKET_VERSION 1
void WaresQueue::Write(FileWrite & fw, Game & game, Map_Map_Object_Saver * os)
{

	fw.Unsigned16(WARES_QUEUE_DATA_PACKET_VERSION);

	//  Owner and callback is not saved, but this should be obvious on load.
	fw.CString
		(owner().tribe().get_ware_descr(m_ware)->name().c_str());
	fw.Signed32(m_size);
	fw.Signed32(m_filled);
	fw.Signed32(m_consume_interval);
	if (m_request) {
		fw.Unsigned8(1);
		m_request->Write(fw, game, os);
	} else
		fw.Unsigned8(0);
}


void WaresQueue::Read(FileRead & fr, Game & game, Map_Map_Object_Loader * ol)
{
	uint16_t const packet_version = fr.Unsigned16();
	try {
		if (packet_version == WARES_QUEUE_DATA_PACKET_VERSION) {
			delete m_request;
			m_ware             = owner().tribe().ware_index(fr.CString  ());
			m_size             =                            fr.Unsigned32();
			m_filled           =                            fr.Unsigned32();
			m_consume_interval =                            fr.Unsigned32();
			if                                             (fr.Unsigned8 ()) {
				m_request =
					new Request
						(m_owner,
						 Ware_Index::First(),
						 WaresQueue::request_callback,
						 Request::WORKER);
				m_request->Read(fr, game, ol);
			} else
				m_request = 0;

			//  Now Economy stuff. We have to add our filled items to the economy.
			add_to_economy(*m_owner.get_economy());
		} else
			throw game_data_error
				(_("unknown/unhandled version %u"), packet_version);
	} catch (game_data_error const & e) {
		throw game_data_error(_("waresqueue: %s"), e.what());
	}
}

}
