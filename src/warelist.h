/*
 * Copyright (C) 2002-2003, 2006-2008 by the Widelands Development Team
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

#ifndef WARELIST_H
#define WARELIST_H

#include "widelands.h"

#include <SDL.h>

#include <cassert>
#include <vector>

namespace Widelands {

/**
 * WareList is a simple wrapper around an array of ware types.
 * It is useful for warehouses and for economy-wide inventory.
 */
struct WareList {
	WareList() {};
	~WareList();

	void clear() {m_wares.clear();} /// Clear the storage

	typedef uint32_t count_type;
	typedef std::vector<count_type> vector_type;
	typedef vector_type::size_type size_type;

	/// \return Highest possible ware id
	Ware_Index get_nrwareids() const {return Ware_Index(m_wares.size());}

	void add   (Ware_Index, count_type = 1);
	void add(const WareList &wl);
	void remove(Ware_Index, count_type = 1);
	void remove(WareList const & wl);
	count_type stock(Ware_Index) const;

	void set_nrwares(Ware_Index const i) {
		assert(m_wares.size() == 0);
		m_wares.resize(i.value(), 0);
	}

	bool operator== (WareList const &)    const;
	bool operator!= (WareList const & wl) const {return not (*this == wl);}

private:
	vector_type m_wares;
};

};

#endif
