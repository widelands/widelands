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

#ifndef WARE_TYPES_H
#define WARE_TYPES_H

#include "widelands.h"

#include <vector>

namespace Widelands {
typedef std::vector<std::pair<Ware_Index, uint32_t> > Ware_Types;

// range structure for iterating ware range with index
struct ware_range
{
	ware_range(Ware_Types const & range) : i(0), current(range.begin()), end(range.end()) {}
	ware_range &operator++() {++i;++current;return *this;}
	bool empty() const {return current==end;}
	operator bool() const { return empty() ? false: true;}

	uint8_t i;							// current integer index
	Ware_Types::const_iterator current;		// current iterator in Ware_Types range
private:
	Ware_Types::const_iterator const end;		// end iterator
};

}

#endif
