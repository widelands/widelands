/*
 * Copyright (C) 2008-2009 by the Widelands Development Team
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

#ifndef ATTACKABLE_H
#define ATTACKABLE_H

namespace Widelands {

class Player;
class Soldier;

enum {
	/**
	 * This is the maximum radius that a military building can protect
	 * in the sense that an enemy soldier that enters the player's territory
	 * will call \ref Attackable::aggressor if it is that close.
	 */
	MaxProtectionRadius = 25
};

/**
 * Buildings can implement this interface to indicate that
 * they can be attacked.
 */
struct Attackable {
	/**
	 * Return the player that owns this attackable.
	 */
	virtual Player & owner() const = 0;

	/**
	 * Determines whether this building can be attacked right now.
	 *
	 * This should only return false for military sites that have not
	 * been occupied yet.
	 */
	virtual bool canAttack() = 0;

	/**
	 * Called by an enemy soldier that enters a node with distance
	 * less than or equal to \ref MaxProtectionRadius from the building.
	 *
	 * This allows the building to send protective forces to intercept
	 * the soldier.
	 */
	virtual void aggressor(Soldier &) = 0;

	/**
	 * Called by a soldier who is standing on the building's flag
	 * to attack the building.
	 *
	 * The building must send a soldier for defense, and return \c true.
	 * Otherwise, i.e. if the building cannot defend itself anymore,
	 * it must destroy itself or turn over to the attacking player,
	 * and return \c false.
	 *
	 * \return \c true if a soldier was launched in defense of the building,
	 * or \c false if the building cannot defend itself any longer.
	 */
	virtual bool attack(Soldier &) = 0;

protected:
	virtual ~Attackable() {}
};

}

#endif
