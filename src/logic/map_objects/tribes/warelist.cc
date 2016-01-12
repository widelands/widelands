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

#include "logic/map_objects/tribes/warelist.h"

#include "base/log.h"

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
void WareList::add(DescriptionIndex const i, const WareCount count) {
	if (!count)
		return;

	if (m_wares.size() <= i)
		m_wares.resize(i + 1, 0);
	m_wares[i] += count;
	assert(m_wares[i] >= count);

	changed();
}


void WareList::add(const WareList & wl)
{
	DescriptionIndex const nr_wares = wl.get_nrwareids();
	if (m_wares.size() < nr_wares)
		m_wares.reserve(nr_wares);
	for (DescriptionIndex i = 0; i < nr_wares; ++i)
		if (wl.m_wares[i])
			add(i, wl.m_wares[i]);
}


/**
 * Remove the given number of items (default = 1) from the storage.
 */
void WareList::remove(DescriptionIndex const i, const WareCount count) {
	if (!count)
		return;

	assert(i < m_wares.size());
	assert(m_wares[i] >= count);
	m_wares[i] -= count;

	changed();
}


void WareList::remove(const WareList & wl)
{
	DescriptionIndex const nr_wares = wl.get_nrwareids();
	for (DescriptionIndex i = 0; i < nr_wares; ++i)
		if (wl.m_wares[i])
			remove(i, wl.m_wares[i]);
}

/**
 * Return the number of wares of a given type stored in this storage.
 */
WareList::WareCount WareList::stock(DescriptionIndex const id) const {
	return id < m_wares.size() ? m_wares[id] : 0;
}


/**
 * Two WareLists are only equal when they contain the exact same stock of
 * all wares types.
*/
bool WareList::operator== (const WareList & wl) const
{
	uint32_t i = 0;

	while (i < wl.m_wares.size()) {
		const WareCount count = wl.m_wares[i];
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

}
