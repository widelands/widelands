/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#include "widelands.h"
#include "pic.h"
#include "tribe.h"
#include "ware.h"
#include "worker.h"


/*
==============================================================================

Ware_Descr IMPLEMENTATION

==============================================================================
*/

/*
===============
Ware_Descr::Ware_Descr
Ware_Descr::~Ware_Descr
===============
*/
Ware_Descr::Ware_Descr(const char *name)
{
	snprintf(m_name, sizeof(m_name), "%s", name);
}

Ware_Descr::~Ware_Descr()
{
}


/*
==============================================================================

Item_Ware_Descr IMPLEMENTATION
	
==============================================================================
*/

/*
===============
Item_Ware_Descr::Item_Ware_Descr
Item_Ware_Descr::~Item_Ware_Descr
===============
*/
Item_Ware_Descr::Item_Ware_Descr(const char *name)
	: Ware_Descr(name)
{
}

Item_Ware_Descr::~Item_Ware_Descr()
{
}

/*
===============
Item_Ware_Descr::is_worker
===============
*/	
bool Item_Ware_Descr::is_worker()
{
	return false;
}


/*
==============================================================================

Worker_Ware_Descr IMPLEMENTATION
	
==============================================================================
*/

/*
===============
Worker_Ware_Descr::Worker_Ware_Descr
Worker_Ware_Descr::~Worker_Ware_Descr
===============
*/
Worker_Ware_Descr::Worker_Ware_Descr(const char *name)
	: Ware_Descr(name)
{
}

Worker_Ware_Descr::~Worker_Ware_Descr()
{
}

/*
===============
Worker_Ware_Descr::is_worker
===============
*/
bool Worker_Ware_Descr::is_worker()
{
	return true;
}

/*
===============
Worker_Ware_Descr::get_worker

Return the worker corresponding to the given tribe.
===============
*/
Worker_Descr *Worker_Ware_Descr::get_worker(Tribe_Descr *tribe)
{
	Worker_map::iterator it = m_workers.find(tribe);
	if (it == m_workers.end())
		throw wexception("No worker %s in tribe %s", get_name(), tribe->get_name());
	return it->second;
}

/*
===============
Worker_Ware_Descr::add_worker

Add a worker for the given tribe
===============
*/
void Worker_Ware_Descr::add_worker(Tribe_Descr *tribe, Worker_Descr *worker)
{
	assert(!strcmp(worker->get_name(), get_name()));
	assert(worker->get_tribe() == tribe);
	
	m_workers[tribe] = worker;
}

