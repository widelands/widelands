/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#ifndef __S__MULTI_SELECT_H
#define __S__MULTI_SELECT_H

#include <cassert>
#include <vector>

/*
=============================
class MultiSelect

this class allows for selection of more than just one
thing. Like more than one texture, more than one map object

This is a helper class, no Editor Tool (might be usable in game too)
=============================
*/
struct MultiSelect {
      MultiSelect() {m_nr_enabled=0;}
      ~MultiSelect() {}

      void enable(int n, bool t) {
		if (static_cast<int>(m_enabled.size()) < n + 1)
            m_enabled.resize(n+1, false);

         if (m_enabled[n]==t) return;
         m_enabled[n]=t;
         if (t) ++m_nr_enabled;
         else --m_nr_enabled;
         assert(m_nr_enabled>=0);
		}
      inline bool is_enabled(int n) {
		if (static_cast<int>(m_enabled.size()) < n + 1) return false;
         return m_enabled[n];
		}
      inline int get_nr_enabled() {return m_nr_enabled;}
	int get_random_enabled() {
		const int rand_value = static_cast<int>
			(static_cast<double>(get_nr_enabled())
			 *
			 rand() / (RAND_MAX + 1.0));
         int i=0;
         int j=rand_value+1;
         while (j) {if (is_enabled(i)) j--; ++i;}
         return i-1;
		}

private:
      int m_nr_enabled;
      std::vector<bool> m_enabled;
};

#endif
