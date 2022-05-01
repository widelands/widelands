/*
 * Copyright (C) 2011-2022 by the Widelands Development Team
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

#include "logic/map_objects/findbob.h"

#include "base/macros.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/player.h"

namespace Widelands {

bool FindBobAttribute::accept(Bob* const bob) const {
	return bob->has_attribute(attrib);
}

bool FindBobEnemySoldier::accept(Bob* const imm) const {
	if (upcast(Soldier, soldier, imm)) {
		if (soldier->is_on_battlefield() &&
		    ((player == nullptr) || soldier->owner().is_hostile(*player)) &&
		    (soldier->get_current_health() != 0u)) {
			return true;
		}
	}
	return false;
}

bool FindBobShip::accept(Bob* bob) const {
	return bob->descr().type() == MapObjectType::SHIP;
}

bool FindBobCritter::accept(Bob* bob) const {
	return bob->descr().type() == MapObjectType::CRITTER;
}

}  // namespace Widelands
