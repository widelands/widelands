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

#ifndef PATHFIELD_H
#define PATHFIELD_H

#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>

#include <vector>

#include <stdint.h>

namespace Widelands {

/**
 * Used in pathfinding. For better encapsulation, pathfinding structures
 * are separate from normal fields
 *
 * Costs are in milliseconds to walk.
 *
 * Note: member sizes chosen so that we get a 16byte (=nicely aligned)
 * structure
 */
struct Pathfield {
	int32_t heap_index; //  index of this field in heap, for backlinking
	int32_t real_cost;  //  true cost up to this field
	int32_t estim_cost; //  estimated cost till goal
	uint16_t cycle;
	uint8_t backlink;   //  how we got here (Map_Object::WALK_*)

	int32_t cost() const throw () {return real_cost + estim_cost;}
};

struct Pathfields {
	boost::scoped_array<Pathfield> fields;
	uint16_t cycle;

	Pathfields(uint16_t nrfields);
};

/**
 * Efficiently manages \ref Pathfields instances.
 *
 * This allows the use of more than one such structure at once,
 * which is required for pathfinding reentrancy.
 */
struct PathfieldManager {
	PathfieldManager();

	void setSize(uint32_t nrfields);
	boost::shared_ptr<Pathfields> allocate();

private:
	void clear(const boost::shared_ptr<Pathfields>& pf);

	typedef std::vector<boost::shared_ptr<Pathfields> > List;

	uint32_t m_nrfields;
	List m_list;
};

}

#endif // PATHFIELD_H
