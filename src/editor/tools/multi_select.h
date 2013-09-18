/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#ifndef MULTI_SELECT_H
#define MULTI_SELECT_H

#include <cassert>
#include <climits>
#include <cstdlib>
#include <vector>

#include <stdint.h>

/**
 * This class allows for selection of more than just one
 * thing. Like more than one texture, more than one map object
 *
 * This is a helper class, no Editor Tool (might be usable in game too)
*/
struct MultiSelect {
	MultiSelect() : m_nr_enabled(0) {}
	~MultiSelect() {}

	void enable(int32_t n, bool t) {
		if (static_cast<int32_t>(m_enabled.size()) < n + 1)
			m_enabled.resize(n + 1, false);

		if (m_enabled[n] == t)
			return;
		m_enabled[n] = t;
		if (t)
			++m_nr_enabled;
		else
			--m_nr_enabled;
		assert(0 <= m_nr_enabled);
	}
	bool is_enabled(int32_t n) const {
		if (static_cast<int32_t>(m_enabled.size()) < n + 1)
			return false;
		return m_enabled[n];
	}
	int32_t get_nr_enabled() const {return m_nr_enabled;}
	int32_t get_random_enabled() const {
		const int32_t rand_value =
		    static_cast<int32_t>
		    (static_cast<double>(get_nr_enabled())
		     *
		     rand() / (RAND_MAX + 1.0));
		int32_t i = 0;
		int32_t j = rand_value + 1;
		while (j) {if (is_enabled(i)) --j; ++i;}
		return i - 1;
	}

private:
	int32_t           m_nr_enabled;
	std::vector<bool> m_enabled;
};

#endif
