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

#include "economy/supply_list.h"

#include "base/wexception.h"
#include "economy/request.h"
#include "economy/supply.h"

namespace Widelands {

/**
 * Add a supply to the list.
 */
void SupplyList::add_supply(Supply& supp) {
	supplies_.push_back(&supp);
}

/**
 * Remove a supply from the list.
 */
void SupplyList::remove_supply(Supply& supp) {
	if (supplies_.empty()) {
		throw wexception("SupplyList::remove: list is empty");
	}
	for (Supplies::iterator item_iter = supplies_.begin(); item_iter != supplies_.end();
	     ++item_iter) {

		if (*item_iter == &supp) {
			// Copy last element to current positon, avoids shifts
			*item_iter = *(supplies_.end() - 1);
			supplies_.pop_back();
			return;
		}
		// no extra code for last element, copy will be a noop then
	}
	throw wexception("SupplyList::remove: not in list");
}

/**
 * Return whether there is at least one available
 * supply that can match the given request.
 */
bool SupplyList::have_supplies(const Game& game, const Request& req) {
	for (const Supply* supply : supplies_) {
		if (supply->nr_supplies(game, req) != 0u) {
			return true;
		}
	}
	return false;
}
}  // namespace Widelands
