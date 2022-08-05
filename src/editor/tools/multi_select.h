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

#include <cstdint>
#include <cstdlib>
#include <set>

/**
 * This class allows for selection of more than just one
 * thing. Like more than one texture, more than one map object
 *
 * This is a helper class, no Editor Tool (might be usable in game too)
 */
struct MultiSelect {
	MultiSelect() {
	}
	~MultiSelect() {
	}

	void enable(int32_t n, bool t) {
		if (t) {
			enabled_.insert(n);
		} else {
			enabled_.erase(n);
		}
	}

	bool is_enabled(int32_t n) const {
		return enabled_.find(n) != enabled_.end();
	}

	int32_t get_nr_enabled() const {
		return enabled_.size();
	}

	int32_t get_random_enabled() const {
		int32_t rand_value =
		   static_cast<int32_t>(static_cast<double>(get_nr_enabled()) * rand() / (RAND_MAX + 1.0));

		for (int32_t item : enabled_) {
			if (rand_value == 0) {
				return item;
			}
			--rand_value;
		}

		return -1;
	}

	void disable_all() {
		enabled_.clear();
	}

	int32_t count() const {
		return enabled_.size();
	}

	const std::set<int32_t>& get_enabled() const {
		return enabled_;
	}

private:
	std::set<int32_t> enabled_;
};

#endif  // end of include guard: WL_EDITOR_TOOLS_MULTI_SELECT_H
