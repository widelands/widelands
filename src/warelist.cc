/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "warelist.h"

#include "log.h"

namespace Widelands {

/**
 * Delete the list. Print a warning message if the storage is not empty.
 *
 * This is because most of the time, a WareList should be zeroed by cleanup
 * operations before the destructor is called. If you are sure of what you're
 * doing, call clear().
 */
WareList::~WareList()
{
	for (uint32_t id = 0; id < m_wares.size(); ++id) {
		if (m_wares[id])
			log("WareList: %i items of %i left.\n", m_wares[id], id);
	}
}


/**
 * Add the given number of items (default = 1) to the storage.
 */
void WareList::add(const size_type id, const count_type count) {
	if (!count)
		return;

	if (id >= m_wares.size()) m_wares.resize(id + 1, 0);
	m_wares[id] += count;
	assert(m_wares[id] >= count);
}


void WareList::add(const WareList &wl)
{
	if (wl.m_wares.size() > m_wares.size())
		m_wares.reserve(wl.m_wares.size());

	for (uint32_t id = 0; id < wl.m_wares.size(); ++id)
		if (wl.m_wares[id])
			add(id, wl.m_wares[id]);
}


/**
 * Remove the given number of items (default = 1) from the storage.
 */
void WareList::remove(const size_type id, const count_type count) {
	if (!count)
		return;

	assert(id < m_wares.size());
   assert(m_wares[id] >= count);
	m_wares[id] -= count;
}


void WareList::remove(const WareList &wl)
{
	for (uint32_t id = 0; id < wl.m_wares.size(); ++id)
		if (wl.m_wares[id])
			remove(id, wl.m_wares[id]);
}

/**
 * Return the number of wares of a given type stored in this storage.
 */
int32_t WareList::stock(const size_type id) const {
	return id < m_wares.size() ? m_wares[id] : 0;
}


/**
 * Two WareLists are only equal when they contain the exact same stock of
 * all wares types.
*/
bool WareList::operator==(const WareList &wl) const
{
	uint32_t i = 0;

	while (i < wl.m_wares.size()) {
		const count_type count = wl.m_wares[i];
		if (i < m_wares.size()) {
			if (count != m_wares[i])
				return false;
		} else {
			if (count) // wl2 has 0 stock per definition
				return false;
		}
		++i;
	}

	while (i < m_wares.size()) {
		if (m_wares[i]) // wl1 has 0 stock per definition
			return false;
		++i;
	}

	return true;
}

};
