/*
 * Copyright (C) 2011 by the Widelands Development Team
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

#include "logic/findbob.h"

#include "logic/player.h"
#include "logic/soldier.h"
#include "upcast.h"

namespace Widelands {

bool FindBobAttribute::accept(Bob * const bob) const
{
	return bob->has_attribute(m_attrib);
}

bool FindBobEnemySoldier::accept(Bob * const imm) const
{
	if (upcast(Soldier, soldier, imm))
		if
			(soldier->isOnBattlefield() &&
			 (!player || soldier->owner().is_hostile(*player)) &&
			 soldier->get_current_hitpoints())
			return true;

	return false;
}

bool FindBobShip::accept(Bob * bob) const
{
	return bob->get_bob_type() == Bob::SHIP;
}

} // namespace Widelands
