/*
 * Copyright (C) 2008-2022 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_ATTACK_TARGET_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_ATTACK_TARGET_H

#include "base/macros.h"

namespace Widelands {

class Soldier;

// This is the maximum radius that a military building can protect in the sense
// that an enemy soldier that enters the player's territory will call \ref
// AttackTarget::enemy_soldier_approaches if it is that close.
constexpr int kMaxProtectionRadius = 25;

class AttackTarget {
public:
	AttackTarget() = default;
	virtual ~AttackTarget() {
	}

	// Determines whether this building can be attacked right now.
	virtual bool can_be_attacked() const = 0;

	virtual void set_allow_conquer(PlayerNumber, bool) const = 0;
	virtual bool get_allow_conquer(PlayerNumber) const = 0;

	// Called by an enemy soldier that enters a node with distance
	// less than or equal to \ref kMaxProtectionRadius from the building.
	//
	// This allows the building to send protective forces to intercept
	// the soldier.
	virtual void enemy_soldier_approaches(const Soldier& enemy) const = 0;

	// Called by a soldier who is standing on the building's flag
	// to attack the building.
	//
	// The building must send a soldier for defense, and return \c true.
	// Otherwise, i.e. if the building cannot defend itself anymore,
	// it must destroy itself or turn over to the attacking player,
	// and return \c false.
	enum class AttackResult {
		// Returned when a soldier was launched in defense of the building.
		DefenderLaunched,

		// Returned if the building cannot defend itself any longer.
		Defenseless
	};
	virtual AttackResult attack(Soldier* attacker) const = 0;

private:
	DISALLOW_COPY_AND_ASSIGN(AttackTarget);
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_ATTACK_TARGET_H
