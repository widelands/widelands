/*
 * Copyright (C) 2002-2003, 2006-2007 by the Widelands Development Team
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

#ifndef __WARELIST_H
#define __WARELIST_H

#include <vector>

/**
 * WareList is a simple wrapper around an array of ware types.
 * It is useful for warehouses and for economy-wide inventory.
 */
class WareList
{
	public:
		WareList() {};
		~WareList();

		/// Clear the storage
		void clear() {m_wares.clear();};

		/// \return Highest possible ware id
		const int get_nrwareids() const {return m_wares.size();} 

		void add(int id, int count = 1);
		void add(const WareList &wl);
		void remove(int id, int count = 1);
		void remove(const WareList &wl);
		int stock(int id) const;

		void set_nrwares(int i) {
			assert(m_wares.size()==0);
			m_wares.resize(i, 0);
		}

		bool operator==(const WareList &wl) const;
		bool operator!=(const WareList &wl) const {return not (*this==wl);}

	private:
		std::vector<int> m_wares;
};


#endif   // __WARELIST_H

