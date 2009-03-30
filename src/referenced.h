/*
 * Copyright (C) 2008 by the Widelands Development Team
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

#ifndef REFERENCED_H
#define REFERENCED_H

#include "referencer.h"

#include "log.h"

#include <cassert>
#include <stdint.h>
#include <map>

template<typename Subtype> struct Referenced {
	typedef std::map<Referencer<Subtype> const *, uint32_t> Referencers;
	Referencers const & referencers() const throw () {return m_referencers;}
	void   reference(Referencer<Subtype> const & referencer) {
		typename Referencers::iterator it = m_referencers.find(&referencer);
		if (it == m_referencers.end())
			m_referencers[&referencer] = 1;
		else
			++it->second;
	}
	void unreference(Referencer<Subtype> const & referencer) {
		typename Referencers::iterator it = m_referencers.find(&referencer);
		if (it == m_referencers.end()) {
			log
				("WARNING: %p was unreferenced by %p, which is not in the set of "
				 "referencers.\n",
				 this, &referencer);
		} else if (--it->second == 0)
			m_referencers.erase(it);
	}
private:
	Referencers m_referencers;
};

#endif
