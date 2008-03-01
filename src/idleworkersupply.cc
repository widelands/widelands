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

#include "game.h"
#include "idleworkersupply.h"
#include "player.h"
#include "requirements.h"
#include "soldier.h"
#include "tribe.h"
#include "worker.h"
#include "wexception.h"

namespace Widelands {

/**
 * Automatically register with the worker's economy.
 */
IdleWorkerSupply::IdleWorkerSupply(Worker* w): m_worker (w), m_economy(0)
{
	set_economy(w->get_economy());
}


/**
 * Automatically unregister from economy.
 */
IdleWorkerSupply::~IdleWorkerSupply()
{
	set_economy(0);
}


/**
 * Add/remove this supply from the Economy as appropriate.
 */
void IdleWorkerSupply::set_economy(Economy* e)
{
	if (m_economy == e)
		return;

	if (m_economy)
		m_economy->remove_worker_supply(m_worker->get_owner()->tribe().get_worker_index(m_worker->name().c_str()), this);

	m_economy = e;

	if (m_economy)
		m_economy->add_worker_supply(m_worker->get_owner()->tribe().get_worker_index(m_worker->name().c_str()), this);
}


/**
 * Return the worker's position.
 */
PlayerImmovable* IdleWorkerSupply::get_position(Game* g)
{
	return m_worker->get_location(g);
}


/**
 * It's just the one worker.
 */
int32_t IdleWorkerSupply::get_amount(const int32_t ware) const
{
	if (ware == m_worker->get_owner()->tribe().get_worker_index(m_worker->name().c_str()))
		return 1;

	return 0;
}


WareInstance & IdleWorkerSupply::launch_item(Game *, int32_t)
{
	throw wexception("IdleWorkerSupply::launch_item() makes no sense.");
}


/**
 * No need to explicitly launch the worker.
 */
Worker* IdleWorkerSupply::launch_worker(Game *, int32_t ware)
{
	assert(m_worker->descr().can_act_as(ware));

	return m_worker;
}


Soldier* IdleWorkerSupply::launch_soldier(Game *, int32_t, const Requirements & req)
{
	assert (m_worker->get_worker_type()==Worker_Descr::SOLDIER);

	Soldier* s = static_cast<Soldier*>(m_worker);

	if (req.check(s))
		return s;
	else
		throw wexception
			("IdleWorkerSupply::launch_soldier try to launch a soldiers that "
			 "doesn't accomplish the requirements.");
}


int32_t IdleWorkerSupply::get_passing_requirements(Game *, int32_t, const Requirements & req)
{
	assert (m_worker->get_worker_type()==Worker_Descr::SOLDIER);

	Soldier* s = static_cast<Soldier*>(m_worker);

	return req.check(s);
}


void IdleWorkerSupply::mark_as_used (Game *, int32_t ware, const Requirements & r)
{
	assert(ware == m_worker->get_owner()->tribe().get_worker_index(m_worker->name().c_str()));

	if (m_worker->get_worker_type() == Worker_Descr::SOLDIER) {

		Soldier* s = static_cast<Soldier*>(m_worker);

		if (r.check(s))
			dynamic_cast<Soldier &>(*m_worker).mark(true);

	} else {
		// Non-soldiers doesn't have any need to be marked (by now)
	}
}

};
