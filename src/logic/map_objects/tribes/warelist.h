/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_WARELIST_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_WARELIST_H

#include <cassert>

#include "logic/widelands.h"

namespace Widelands {

/**
 * WareList is a simple wrapper around an array of ware types.
 * It is useful for warehouses and for economy-wide inventory.
 */
struct WareList {
	WareList() {
	}
	~WareList();

	void clear() {
		wares_.clear();
	}  /// Clear the storage

	/// \return Highest possible ware id
	DescriptionIndex get_nrwareids() const {
		return DescriptionIndex(wares_.size());
	}

	void add(DescriptionIndex, Quantity = 1);
	void add(const WareList&);
	void remove(DescriptionIndex, Quantity = 1);
	void remove(const WareList& wl);
	Quantity stock(DescriptionIndex) const;

	void set_nrwares(DescriptionIndex const i) {
		assert(wares_.empty());
		wares_.resize(i, 0);
	}

	bool operator==(const WareList&) const;
	bool operator!=(const WareList& wl) const {
		return !(*this == wl);
	}

private:
	std::vector<Quantity> wares_;
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_WARELIST_H
