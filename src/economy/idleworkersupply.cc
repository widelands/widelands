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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "economy/idleworkersupply.h"

#include "economy/economy.h"
#include "economy/request.h"
#include "logic/game.h"
#include "logic/player.h"
#include "logic/requirements.h"
#include "logic/soldier.h"
#include "logic/tribe.h"
#include "logic/warehouse.h"
#include "logic/worker.h"
#include "wexception.h"

namespace Widelands {

/**
 * Automatically register with the worker's economy.
 */
IdleWorkerSupply::IdleWorkerSupply(Worker & w) : m_worker (w), m_economy(nullptr)
{
	set_economy(w.get_economy());
}


/**
 * Automatically unregister from economy.
 */
IdleWorkerSupply::~IdleWorkerSupply()
{
	set_economy(nullptr);
}


/**
 * Add/remove this supply from the Economy as appropriate.
 */
void IdleWorkerSupply::set_economy(Economy * const e)
{
	if (m_economy != e) {
		if (m_economy)
			m_economy->remove_supply(*this);
		if ((m_economy = e))
			m_economy->   add_supply(*this);
	}
}

/**
 * Worker is walking around the road network, so active by definition.
 */
bool IdleWorkerSupply::is_active() const
{
	return true;
}

bool IdleWorkerSupply::has_storage() const
{
	return m_worker.get_transfer();
}

void IdleWorkerSupply::get_ware_type(WareWorker & type, Ware_Index & ware) const
{
	type = wwWORKER;
	ware = m_worker.worker_index();
}

/**
 * Return the worker's position.
 */
PlayerImmovable * IdleWorkerSupply::get_position(Game & game)
{
	return m_worker.get_location(game);
}


uint32_t IdleWorkerSupply::nr_supplies(const Game &, const Request & req) const
{
	assert
		(req.get_type() != wwWORKER or
		 req.get_index() < m_worker.descr().tribe().get_nrworkers());
	if
		(req.get_type() == wwWORKER &&
		 m_worker.descr().can_act_as(req.get_index()) &&
		 req.get_requirements().check(m_worker))
		return 1;

	return 0;
}

WareInstance & IdleWorkerSupply::launch_ware(Game &, const Request &)
{
	throw wexception("IdleWorkerSupply::launch_ware() makes no sense.");
}


/**
 * No need to explicitly launch the worker.
 */
Worker & IdleWorkerSupply::launch_worker(Game &, const Request & req)
{
	if (req.get_type() != wwWORKER)
		throw wexception("IdleWorkerSupply: not a worker request");
	if
		(!m_worker.descr().can_act_as(req.get_index()) ||
		 !req.get_requirements().check(m_worker))
		throw wexception("IdleWorkerSupply: worker type mismatch");

	return m_worker;
}

void IdleWorkerSupply::send_to_storage(Game & game, Warehouse * wh)
{
	assert(!has_storage());

	Transfer * t = new Transfer(game, m_worker);
	t->set_destination(*wh);
	m_worker.start_task_transfer(game, t);
}

}
