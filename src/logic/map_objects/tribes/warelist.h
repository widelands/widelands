/*
 * Copyright (C) 2002-2003, 2006-2009 by the Widelands Development Team
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
#include <vector>

#include <boost/bind.hpp>
#include <boost/signals2.hpp>

#include "logic/widelands.h"

namespace Widelands {

/**
 * WareList is a simple wrapper around an array of ware types.
 * It is useful for warehouses and for economy-wide inventory.
 */
struct WareList {
	WareList() {}
	~WareList();

	void clear() {m_wares.clear();} /// Clear the storage

	using WareCount = uint32_t;
	using WareCountVector = std::vector<WareCount>;

	/// \return Highest possible ware id
	DescriptionIndex get_nrwareids() const {return DescriptionIndex(m_wares.size());}

	void add   (DescriptionIndex, WareCount = 1);
	void add(const WareList &);
	void remove(DescriptionIndex, WareCount = 1);
	void remove(const WareList & wl);
	WareCount stock(DescriptionIndex) const;

	void set_nrwares(DescriptionIndex const i) {
		assert(m_wares.empty());
		m_wares.resize(i, 0);
	}

	bool operator== (const WareList &)    const;
	bool operator!= (const WareList & wl) const {return !(*this == wl);}

	mutable boost::signals2::signal<void ()> changed;

private:
	WareCountVector m_wares;
};

}

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_WARELIST_H
