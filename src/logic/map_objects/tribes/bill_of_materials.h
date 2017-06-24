/*
 * Copyright (C) 2008-2017 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_BILL_OF_MATERIALS_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_BILL_OF_MATERIALS_H

#include <vector>

#include "logic/widelands.h"

namespace Widelands {
using WareAmount = std::pair<DescriptionIndex, Widelands::Quantity>;
using BillOfMaterials = std::vector<WareAmount>;

// range structure for iterating ware range with index
struct WareRange {
	explicit WareRange(const BillOfMaterials& range) : i(0), current(range.begin()), end(range.end()) {
	}
	WareRange& operator++() {
		++i;
		++current;
		return *this;
	}
	bool empty() const {
		return current == end;
	}
	operator bool() const {
		return !empty();
	}

	uint8_t i;
	BillOfMaterials::const_iterator current;

private:
	BillOfMaterials::const_iterator const end;
};
}

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_BILL_OF_MATERIALS_H
