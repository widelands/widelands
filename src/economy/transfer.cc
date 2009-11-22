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

#include "transfer.h"

// Package includes
#include "economy.h"
#include "flag.h"
#include "road.h"
#include "ware_instance.h"

#include "logic/game.h"
#include "logic/immovable.h"
#include "request.h"
#include "logic/worker.h"
#include "upcast.h"

namespace Widelands {

Transfer::Transfer(Game & game, Request & req, WareInstance & it) :
	m_game(game),
	m_request(req),
	m_item   (&it),
	m_worker(0),
	m_idle(false)
{
	m_item->set_transfer(game, *this);
}

Transfer::Transfer(Game & game, Request & req, Worker & w) :
	m_game(game),
	m_request(req),
	m_item(0),
	m_worker (&w),
	m_idle(false)
{
	m_worker->start_task_transfer(game, this);
}

/**
 * Cleanup.
*/
Transfer::~Transfer()
{
	if (m_worker) {
		assert(!m_item);

		m_worker->cancel_task_transfer(m_game);
	} else if (m_item) {
		m_item->cancel_transfer(m_game);
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
PlayerImmovable * Transfer::get_next_step
	(PlayerImmovable * const location, bool & success)
{
	PlayerImmovable & destination = m_request.target();

	// Catch the simplest cases
	if (not location or location->get_economy() != destination.get_economy()) {
		tlog("no location or economy mismatch -> fail\n");

		success = false;
		return 0;
	}

	success = true;

	if (location == &destination)
		return 0;

	Flag & locflag  = location  ->base_flag();
	Flag & destflag = destination.base_flag();

	if (&locflag == &destflag)
		return &locflag == location ? &destination : &locflag;

	// Brute force: recalculate the best route every time
	if (!locflag.get_economy()->find_route(locflag, destflag, &m_route, m_item))
		throw wexception("Transfer::get_next_step: inconsistent economy");

	if (m_route.get_nrsteps() >= 1)
		if (upcast(Road const, road, location))
			if (&road->get_flag(Road::FlagEnd) == &m_route.get_flag(m_game, 1))
				m_route.starttrim(1);

	if (m_route.get_nrsteps() >= 1)
		if (upcast(Road const, road, &destination))
			if
				(&road->get_flag(Road::FlagEnd)
				 ==
				 &m_route.get_flag(m_game, m_route.get_nrsteps() - 1))
				m_route.truncate(m_route.get_nrsteps() - 1);

	// Now decide where we want to go
	if (dynamic_cast<Flag const *>(location)) {
		assert(&m_route.get_flag(m_game, 0) == location);

		// special rule to get items into buildings
		if (m_item and m_route.get_nrsteps() == 1)
			if (dynamic_cast<Building const *>(&destination)) {
				assert(&m_route.get_flag(m_game, 1) == &destination.base_flag());

				return &destination;
			}

		if (m_route.get_nrsteps() >= 1) {
			return &m_route.get_flag(m_game, 1);
		}

		return &destination;
	}

	return &m_route.get_flag(m_game, 0);
}

/**
 * Transfer finished successfully.
 * This Transfer object will be deleted indirectly by finish().
 * The caller might be destroyed, too.
 */
void Transfer::has_finished()
{
	m_request.transfer_finish(m_game, *this);
}

/**
 * Transfer failed for reasons beyond our control.
 * This Transfer object will be deleted indirectly by
 * m_request->transfer_fail().
*/
void Transfer::has_failed()
{
	m_request.transfer_fail(m_game, *this);
}

void Transfer::tlog(char const * const fmt, ...)
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
		serial = m_worker->serial();
	} else if (m_item) {
		id = 'I';
		serial = m_item->serial();
	} else {
		id = '?';
		serial = 0;
	}

	log("T%c(%u): %s", id, serial, buffer);
}

}
