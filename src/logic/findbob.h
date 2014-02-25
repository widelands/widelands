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

#ifndef LOGIC_FINDBOB_H
#define LOGIC_FINDBOB_H

#include "logic/map.h"

namespace Widelands {


struct FindBobAttribute : public FindBob {
	FindBobAttribute(uint32_t const attrib) : m_attrib(attrib) {}

	virtual bool accept(Bob *) const override;

	uint32_t m_attrib;
	virtual ~FindBobAttribute() {}  // make gcc shut up
};

/**
 * Find soldiers which are hostile to the given player (or all soldiers
 * if player is 0).
 */
struct FindBobEnemySoldier : public FindBob {
	FindBobEnemySoldier(Player * _player) : player(_player) {}

	virtual bool accept(Bob *) const override;

	Player * player;
};

struct FindBobShip : FindBob {
	virtual bool accept(Bob * bob) const override;
};


} // namespace Widelands

#endif // LOGIC_FINDBOB_H
