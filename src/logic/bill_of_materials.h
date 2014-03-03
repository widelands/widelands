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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WARE_TYPES_H
#define WARE_TYPES_H

#include <vector>

#include "logic/widelands.h"

namespace Widelands {
typedef std::pair<Ware_Index, uint32_t> WareAmount;
typedef std::vector<WareAmount> BillOfMaterials;

// range structure for iterating ware range with index
struct ware_range
{
	ware_range(const BillOfMaterials & range) :
		i(0), current(range.begin()), end(range.end()) {}
	ware_range & operator++ () {
		++i; ++current; return *this;
	}
	bool empty() const {return current == end;}
	operator bool() const {return not empty();}

	uint8_t i;
	BillOfMaterials::const_iterator current;
private:
	BillOfMaterials::const_iterator const end;
};

}

#endif
