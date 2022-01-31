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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_EDITOR_TOOLS_MULTI_SELECT_H
#define WL_EDITOR_TOOLS_MULTI_SELECT_H

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <vector>

/**
 * This class allows for selection of more than just one
 * thing. Like more than one texture, more than one map object
 *
 * This is a helper class, no Editor Tool (might be usable in game too)
 */
struct MultiSelect {
	MultiSelect() : nr_enabled_(0) {
	}
	~MultiSelect() {
	}

	void enable(int32_t n, bool t) {
		if (static_cast<int32_t>(enabled_.size()) < n + 1)
			enabled_.resize(n + 1, false);

		if (enabled_[n] == t)
			return;
		enabled_[n] = t;
		if (t)
			++nr_enabled_;
		else
			--nr_enabled_;
		assert(0 <= nr_enabled_);
	}
	bool is_enabled(int32_t n) const {
		if (static_cast<int32_t>(enabled_.size()) < n + 1)
			return false;
		return enabled_[n];
	}
	int32_t get_nr_enabled() const {
		return nr_enabled_;
	}
	int32_t get_random_enabled() const {
		const int32_t rand_value =
		   static_cast<int32_t>(static_cast<double>(get_nr_enabled()) * rand() / (RAND_MAX + 1.0));
		int32_t i = 0;
		int32_t j = rand_value + 1;
		while (j) {
			if (is_enabled(i))
				--j;
			++i;
		}
		return i - 1;
	}

private:
	int32_t nr_enabled_;
	std::vector<bool> enabled_;
};

#endif  // end of include guard: WL_EDITOR_TOOLS_MULTI_SELECT_H
