/*
 * Copyright (C) 2004, 2006-2013 by the Widelands Development Team
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

#include "economy/transfer.h"

#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/portdock.h"
#include "economy/request.h"
#include "economy/road.h"
#include "economy/ware_instance.h"
#include "logic/game.h"
#include "logic/immovable.h"
#include "logic/player.h"
#include "logic/warehouse.h"
#include "logic/worker.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "map_io/widelands_map_map_object_saver.h"
#include "upcast.h"

namespace Widelands {

Transfer::Transfer(Game & game, Request & req, WareInstance & it) :
	m_game(game),
	m_request(&req),
	m_destination(&req.target()),
	m_ware(&it),
	m_worker(nullptr)
{
	m_ware->set_transfer(game, *this);
}

Transfer::Transfer(Game & game, Request & req, Worker & w) :
	m_game(game),
	m_request(&req),
	m_destination(&req.target()),
	m_ware(nullptr),
	m_worker(&w)
{
	m_worker->start_task_transfer(game, this);
}

/**
 * Create a transfer without linking it into the
 * given ware instance and without a request.
 */
Transfer::Transfer(Game & game, WareInstance & w) :
	m_game(game),
	m_request(nullptr),
	m_ware(&w),
	m_worker(nullptr)
{
}

/**
 * Create a transfer without linking it into the given
 * worker and without a request.
 */
Transfer::Transfer(Game & game, Worker & w) :
	m_game(game),
	m_request(nullptr),
	m_ware(nullptr),
	m_worker(&w)
{
}

/**
 * Cleanup.
 */
Transfer::~Transfer()
{
	if (m_worker) {
		assert(!m_ware);

		m_worker->cancel_task_transfer(m_game);
	} else if (m_ware) {
		m_ware->cancel_transfer(m_game);
	}

}

/**
 * Override the \ref Request of this transfer.
 *
 * \note Only use for loading savegames
 */
void Transfer::set_request(Request * req)
{
	assert(!m_request);
	assert(req);

	if (&req->target() != m_destination.get(m_game)) {
		if (m_destination.is_set())
			log
				("WARNING: Transfer::set_request req->target (%u) "
				 "vs. destination (%u) mismatch\n",
				 req->target().serial(), m_destination.serial());
		m_destination = &req->target();
	}
	m_request = req;
}

/**
 * Set the destination for a transfer that has no associated \ref Request.
 */
void Transfer::set_destination(PlayerImmovable & imm)
{
	assert(!m_request);

	m_destination = &imm;
}

/**
 * Get this transfer's destination.
 */
PlayerImmovable * Transfer::get_destination(Game & g)
{
	return m_destination.get(g);
}

/**
 * Determine where we should be going from our current location.
 */
PlayerImmovable * Transfer::get_next_step
	(PlayerImmovable * const location, bool & success)
{
	if (!location || !location->get_economy()) {
		tlog("no location or economy -> fail\n");
		success = false;
		return nullptr;
	}

	PlayerImmovable * destination =
		 m_destination.get(location->get_economy()->owner().egbase());
	if (!destination || destination->get_economy() != location->get_economy()) {
		tlog("destination disappeared or economy mismatch -> fail\n");
		success = false;
		return nullptr;
	}

	success = true;

	if (location == destination)
		return nullptr;

	Flag & locflag  = location->base_flag();
	Flag & destflag = destination->base_flag();

	if (&locflag == &destflag)
		return &locflag == location ? destination : &locflag;

	// Brute force: recalculate the best route every time
	if (!locflag.get_economy()->find_route(locflag, destflag, &m_route, m_ware ? wwWARE : wwWORKER)) {
		tlog("destination appears to have become split from current location -> fail\n");
		Economy::check_split(locflag, destflag);
		success = false;
		return nullptr;
	}

	if (m_route.get_nrsteps() >= 1)
		if (upcast(Road const, road, location))
			if (&road->get_flag(Road::FlagEnd) == &m_route.get_flag(m_game, 1))
				m_route.starttrim(1);

	if (m_route.get_nrsteps() >= 1)
		if (upcast(Road const, road, destination))
			if
				(&road->get_flag(Road::FlagEnd)
				 ==
				 &m_route.get_flag(m_game, m_route.get_nrsteps() - 1))
				m_route.truncate(m_route.get_nrsteps() - 1);

	// Reroute into PortDocks or the associated warehouse when appropriate
	if (m_route.get_nrsteps() >= 1) {
		Flag & curflag(m_route.get_flag(m_game, 0));
		Flag & nextflag(m_route.get_flag(m_game, 1));
		if (!curflag.get_road(nextflag)) {
			upcast(Warehouse, wh, curflag.get_building());
			assert(wh);

			PortDock * pd = wh->get_portdock();
			assert(pd);

			if (location == pd)
				return pd->get_dock(nextflag);
			if (location == wh)
				return pd;
			if (location == &curflag || m_ware)
				return wh;
			return &curflag;
		}

		if (m_ware && location == &curflag && m_route.get_nrsteps() >= 2) {
			Flag & nextnextflag(m_route.get_flag(m_game, 2));
			if (!nextflag.get_road(nextnextflag)) {
				upcast(Warehouse, wh, nextflag.get_building());
				assert(wh);

				return wh;
			}
		}
	}

	// Now decide where we want to go
	if (dynamic_cast<Flag const *>(location)) {
		assert(&m_route.get_flag(m_game, 0) == location);

		// special rule to get wares into buildings
		if (m_ware and m_route.get_nrsteps() == 1)
			if (dynamic_cast<Building const *>(destination)) {
				assert(&m_route.get_flag(m_game, 1) == &destflag);

				return destination;
			}

		if (m_route.get_nrsteps() >= 1) {
			return &m_route.get_flag(m_game, 1);
		}

		return destination;
	}

	return &m_route.get_flag(m_game, 0);
}

/**
 * Transfer finished successfully.
 * This Transfer object will be deleted.
 * The caller might be destroyed, too.
 */
void Transfer::has_finished()
{
	if (m_request) {
		m_request->transfer_finish(m_game, *this);
	} else {
		PlayerImmovable * destination = m_destination.get(m_game);
		assert(destination);
		if (m_worker) {
			destination->receive_worker(m_game, *m_worker);
			m_worker = nullptr;
		} else {
			destination->receive_ware(m_game, m_ware->descr_index());
			m_ware->destroy(m_game);
			m_ware = nullptr;
		}

		delete this;
	}
}

/**
 * Transfer failed for reasons beyond our control.
 * This Transfer object will be deleted.
*/
void Transfer::has_failed()
{
	if (m_request) {
		m_request->transfer_fail(m_game, *this);
	} else {
		delete this;
	}
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
	} else if (m_ware) {
		id = 'I';
		serial = m_ware->serial();
	} else {
		id = '?';
		serial = 0;
	}

	log("T%c(%u): %s", id, serial, buffer);
}

/*
==============================

Load/save support

==============================
*/

#define TRANSFER_SAVEGAME_VERSION 1

void Transfer::read(FileRead & fr, Transfer::ReadData & rd)
{
	uint8_t version = fr.Unsigned8();
	if (version != TRANSFER_SAVEGAME_VERSION)
		throw wexception("unhandled/unknown transfer version %u", version);

	rd.destination = fr.Unsigned32();
}

void Transfer::read_pointers
	(Map_Map_Object_Loader & mol, const Widelands::Transfer::ReadData & rd)
{
	if (rd.destination)
		m_destination = &mol.get<PlayerImmovable>(rd.destination);
}

void Transfer::write(Map_Map_Object_Saver & mos, FileWrite & fw)
{
	fw.Unsigned8(TRANSFER_SAVEGAME_VERSION);
	fw.Unsigned32(mos.get_object_file_index_or_zero(m_destination.get(m_game)));
	// not saving route right now, will be recaculated anyway
}

}
