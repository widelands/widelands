/*
 * Copyright (C) 2004-2020 by the Widelands Development Team
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
	for (Supplies::iterator item_iter = supplies_.begin(); item_iter != supplies_.end();
	     ++item_iter) {

		if (*item_iter == &supp) {
			*item_iter = *(supplies_.end() - 1);
			supplies_.pop_back();
			return;
		}
	}
	throw wexception("SupplyList::remove: not in list");
}

/**
 * Return whether there is at least one available
 * supply that can match the given request.
 */
bool SupplyList::have_supplies(Game& game, const Request& req) {
	for (const Supply* supply : supplies_) {
		if (supply->nr_supplies(game, req)) {
			return true;
		}
	}
	return false;
}
}  // namespace Widelands
