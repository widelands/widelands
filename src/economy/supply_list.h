/*
 * Copyright (C) 2004-2022 by the Widelands Development Team
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

#ifndef WL_ECONOMY_SUPPLY_LIST_H
#define WL_ECONOMY_SUPPLY_LIST_H

#include <cstddef>
#include <vector>

namespace Widelands {

class Game;
class Request;
struct Supply;

/**
 * SupplyList is used in the Economy to keep track of supplies.
 */
struct SupplyList {
	void add_supply(Supply&);
	void remove_supply(Supply&);

	size_t get_nrsupplies() const {
		return supplies_.size();
	}
	const Supply& operator[](size_t const idx) const {
		return *supplies_[idx];
	}
	Supply& operator[](size_t const idx) {
		return *supplies_[idx];
	}

	bool have_supplies(const Game& game, const Request&);

private:
	// TODO(klaus-halfmann): try to use a Map or Set
	// (-> keep in mind that the iteration order needs to be platform-independent though)
	using Supplies = std::vector<Supply*>;
	Supplies supplies_;
};
}  // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_SUPPLY_LIST_H
