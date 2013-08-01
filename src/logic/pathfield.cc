/*
 * Copyright (C) 2008-2009 by the Widelands Development Team
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

#include "logic/pathfield.h"

#include "container_iterate.h"
#include "wexception.h"

namespace Widelands {

Pathfields::Pathfields(uint32_t const nrfields)
	: fields(new Pathfield[nrfields]), cycle(0)
{}


PathfieldManager::PathfieldManager() : m_nrfields(0) {}


void PathfieldManager::setSize(uint32_t const nrfields)
{
	if (m_nrfields != nrfields)
		m_list.clear();

	m_nrfields = nrfields;
}

boost::shared_ptr<Pathfields> PathfieldManager::allocate()
{
	container_iterate_const(List, m_list, i)
		if (i.current->use_count() == 1) {
			++(*i.current)->cycle;
			if (!(*i.current)->cycle)
				clear(*i.current);
			return *i.current;
		}

	if (m_list.size() >= 8)
		throw wexception("PathfieldManager::allocate: unbounded nesting?");

	boost::shared_ptr<Pathfields> pf(new Pathfields(m_nrfields));
	clear(pf);
	m_list.push_back(pf);
	return pf;
}

void PathfieldManager::clear(const boost::shared_ptr<Pathfields> & pf)
{
	for (uint32_t i = 0; i < m_nrfields; ++i)
		pf->fields[i].cycle = 0;
	pf->cycle = 1;
}

}
