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
===============
Ware_Descr::load_graphics [virtual]

Load descriptive icons etc... here
===============
*/
void Ware_Descr::load_graphics()
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
	m_menu_pic = 0;
}

Item_Ware_Descr::~Item_Ware_Descr()
{
}


/*
===============
Item_Ware_Descr::load_graphics
===============
*/
void Item_Ware_Descr::load_graphics()
{
	// TODO: actually load the menu pic
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


/*
==============================================================================

WareList IMPLEMENTATION

==============================================================================
*/

/*
===============
WareList::WareList

Zero-initialize the storage.
===============
*/
WareList::WareList()
{
}

/*
===============
WareList::~WareList

Delete the list. Print a warning message if the storage is not empty.
This is because most of the time, a WareList should be zeroed by cleanup
operations before the destructor is called. If you are sure of what you're 
doing, call clear().
===============
*/
WareList::~WareList()
{
	for(uint id = 0; id < m_wares.size(); id++) {
		if (m_wares[id])
			log("WareList: %i items of %i left.\n", m_wares[id], id);
	}
}

/*
===============
WareList::operator=

Assignment operator
===============
*/
WareList &WareList::operator=(const WareList &wl)
{
	m_wares = wl.m_wares;
	return *this;
}

/*
===============
WareList::clear

Clear the storage.
===============
*/
void WareList::clear()
{
	m_wares.clear();
}
	
/*
===============
WareList::add

Add the given number of items (default = 1) to the storage.
===============
*/
void WareList::add(int id, int count)
{
	if (!count)
		return;

	assert(id >= 0);
	
	if (id >= (int)m_wares.size())
		m_wares.resize(id+1, 0);
	m_wares[id] += count;
	assert(m_wares[id] >= count);
}

void WareList::add(const WareList &wl)
{
	if (wl.m_wares.size() > m_wares.size())
		m_wares.reserve(wl.m_wares.size());
		
	for(uint id = 0; id < wl.m_wares.size(); id++)
		if (wl.m_wares[id])
			add(id, wl.m_wares[id]);
}

/*
===============
WareList::remove

Remove the given number of items (default = 1) from the storage.
===============
*/
void WareList::remove(int id, int count)
{
	if (!count)
		return;

	assert(id >= 0 && id < (int)m_wares.size());
	assert(m_wares[id] >= count);
	m_wares[id] -= count;
}

void WareList::remove(const WareList &wl)
{
	for(uint id = 0; id < wl.m_wares.size(); id++)
		if (wl.m_wares[id])
			remove(id, wl.m_wares[id]);
}

/*
===============
WareList::stock

Return the number of wares of a given type stored in this storage.
===============
*/
int WareList::stock(int id) const
{
	assert(id >= 0);
	
	if (id >= (int)m_wares.size())
		return 0;
	return m_wares[id];
}

/*
===============
operator==
operator!=

Two WareLists are only equal when they contain the exact same stock of
of all wares types.
===============
*/
bool operator==(const WareList &wl1, const WareList &wl2)
{
	uint i = 0;
	
	while(i < wl1.m_wares.size()) {
		int count = wl1.m_wares[i];
		if (i < wl2.m_wares.size()) {
			if (count != wl2.m_wares[i])
				return false;
		} else {
			if (count) // wl2 has 0 stock per definition
				return false;
		}
		i++;
	}
	
	while(i < wl2.m_wares.size()) {
		if (wl2.m_wares[i]) // wl1 has 0 stock per definition
			return false;
		i++;
	}
	
	return true;
}

bool operator!=(const WareList &wl1, const WareList &wl2)
{
	return !(wl1 == wl2);
}
