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

#include "base/wexception.h"
#include "economy/economy.h"
#include "economy/request.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

/**
 * Pre-initialize a WaresQueue
*/
WaresQueue::WaresQueue
	(PlayerImmovable &       _owner,
	 DescriptionIndex        const _ware,
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
	if (m_ware != INVALID_INDEX)
		update();
}


/**
 * Clear the queue appropriately.
*/
void WaresQueue::cleanup() {
	assert(m_ware != INVALID_INDEX);

	if (m_filled && m_owner.get_economy())
		m_owner.get_economy()->remove_wares(m_ware, m_filled);

	m_filled = 0;
	m_max_size = 0;
	m_max_fill = 0;

	update();

	m_ware = INVALID_INDEX;
}

/**
 * Fix filled <= size and requests.
 * You must call this after every call to set_*()
*/
void WaresQueue::update() {
	assert(m_ware != INVALID_INDEX);

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
void WaresQueue::set_callback(CallbackFn * const fn, void * const data)
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
	 DescriptionIndex        const ware,
#ifndef NDEBUG
	 Worker          * const w,
#else
	 Worker          *,
#endif
	 PlayerImmovable & target)
{
	WaresQueue & wq =
		dynamic_cast<Building&>(target).waresqueue(ware);

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
	if (m_ware != INVALID_INDEX) {
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
	if (m_ware != INVALID_INDEX) {
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

constexpr uint16_t kCurrentPacketVersion = 2;

void WaresQueue::write(FileWrite & fw, Game & game, MapObjectSaver & mos)
{
	fw.unsigned_16(kCurrentPacketVersion);

	//  Owner and callback is not saved, but this should be obvious on load.
	fw.c_string
		(owner().tribe().get_ware_descr(m_ware)->name().c_str());
	fw.signed_32(m_max_size);
	fw.signed_32(m_max_fill);
	fw.signed_32(m_filled);
	fw.signed_32(m_consume_interval);
	if (m_request) {
		fw.unsigned_8(1);
		m_request->write(fw, game, mos);
	} else
		fw.unsigned_8(0);
}


void WaresQueue::read(FileRead & fr, Game & game, MapObjectLoader & mol)
{
	uint16_t const packet_version = fr.unsigned_16();
	try {
		if (packet_version == kCurrentPacketVersion) {
			delete m_request;
			m_ware             = owner().tribe().ware_index(fr.c_string  ());
			m_max_size         =                            fr.unsigned_32();
			m_max_fill = fr.signed_32();
			m_filled           =                            fr.unsigned_32();
			m_consume_interval =                            fr.unsigned_32();
			if                                             (fr.unsigned_8 ()) {
				m_request =                          //  TODO(unknown): Change Request::read
					new Request                       //  to a constructor.
						(m_owner,
						 0,
						 WaresQueue::request_callback,
						 wwWORKER);
				m_request->read(fr, game, mol);
			} else
				m_request = nullptr;

			//  Now Economy stuff. We have to add our filled items to the economy.
			if (m_owner.get_economy())
				add_to_economy(*m_owner.get_economy());
		} else {
			throw UnhandledVersionError("WaresQueue", packet_version, kCurrentPacketVersion);
		}
	} catch (const GameDataError & e) {
		throw GameDataError("waresqueue: %s", e.what());
	}
}

}
