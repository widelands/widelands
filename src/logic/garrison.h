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

#ifndef GARRISON_H
#define GARRISON_H

#include <vector>

#include "logic/widelands.h"

namespace Widelands {


class Building;
class Editor_Game_Base;
class Game;
struct Player;
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
 * A garrison represents a bunch of soldiers. Garrisons are owned by GarrisonOwner
 * buildings. This interface provides various methods to deal with soldier control
 * and attacks.
 */
class Garrison {
public:
	enum SoldierPref  : uint8_t {
		None,
		Rookies,
		Heroes,
	};
	/**
	 * Return the player that owns this attackable.
	 */
	virtual Player & owner() const = 0;

	/**
	 * Determines whether this building can be attacked right now.
	 *
	 * This should only return false for military sites that have not
	 * been occupied yet, or by passivve garrison such as training sites.
	 */
	virtual bool canAttack() const = 0;

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

	/**
	 * \return a list of soldiers that are currently present in the building.
	 */
	virtual const std::vector<Soldier *> presentSoldiers() const = 0;

	/**
	 * \return a list of soldiers that are currently stationed in the building.
	 * That is, all soldiers occupying a slot in the garrison.
	 */
	virtual const std::vector<Soldier *> stationedSoldiers() const = 0;

	/**
	 * \return the minimum number of soldiers that this building can be
	 * configured to hold.
	 */
	virtual uint32_t minSoldierCapacity() const = 0;

	/**
	 * \return the maximum number of soldiers that this building can be
	 * configured to hold.
	 */
	virtual uint32_t maxSoldierCapacity() const = 0;

	/**
	 * \return the number of soldiers this building is configured to hold
	 * right now.
	 */
	virtual uint32_t soldierCapacity() const = 0;

	/**
	 * Sets the capacity for soldiers of this building.
	 *
	 * New soldiers will be requested and old soldiers will be evicted
	 * as necessary.
	 */
	virtual void setSoldierCapacity(uint32_t capacity) = 0;
	void changeSoldierCapacity(int32_t const difference) {
		uint32_t const old_capacity = soldierCapacity();
		uint32_t const new_capacity =
			std::min
				(static_cast<uint32_t>
				 	(std::max
				 	 	(static_cast<int32_t>(old_capacity) + difference,
				 	 	 static_cast<int32_t>(minSoldierCapacity()))),
				 static_cast<uint32_t>(maxSoldierCapacity()));
		if (old_capacity != new_capacity)
			setSoldierCapacity(new_capacity);
	}
	/**
	 * Evict the given soldier from the building immediately,
	 * without changing the building's capacity.
	 *
	 * \note This has no effect if the soldier is currently involved in a battle
	 * or otherwise blocked from leaving the building.
	 */
	virtual void dropSoldier(Soldier &) = 0;

	/**
	 * Add a new soldier into this site. Returns -1 if there is no space
	 * for him, 0 on success
	 */
	virtual int incorporateSoldier(Editor_Game_Base &, Soldier &) = 0;

	/**
	 * Remove a soldier from the internal list. Most SoldierControls will be
	 * informed by the soldier when it is removed, but WareHouses for example
	 * will not.
	 */
	virtual int outcorporateSoldier(Editor_Game_Base &, Soldier &) = 0;

	/**
	 * Returns the conquer radius of this garrison
	 */
	virtual uint32_t conquerRadius() const = 0;
	/**
	 * Set the soldier preference. Heroes or rookies?
	 */
	virtual void set_soldier_preference(SoldierPref p) = 0;
	/**
	 * \return the current soldier preference
	 */
	virtual SoldierPref get_soldier_preference() const = 0;
	/**
	 * Try to send the given soldier to attack the fiven target building.
	 * This will fail if the soldier already has a job.
	 */
	virtual void sendAttacker(Soldier & soldier, Building & target, uint8_t retreat) = 0;
};


/**
 * A Garrison owner holds a garrison. This interface is to be implemented by
 * buildings that want to store some soldiers. The GarrionHandler Garrison
 * implementation may be used as the Garrison provider.
 */
class GarrisonOwner {
public:
	/**
	 * \return the garrison instance
	 */
	virtual Garrison* get_garrison() const = 0;
	/**
	 * @return the building owning the garrison
	 */
	virtual Building* get_building() = 0;
	/**
	 * Called when the garrison has been lost. \param defeating is set to the
	 * enemy owner, or this owner if we keep military presence. if \param captured,
	 * the enemy captured the building and a new one must be force built.
	 *
	 * Will not be called by passive garrisons
	 */
	virtual void garrison_lost(Game & game, Player_Number defeating, bool captured) = 0;
	/**
	 * Called when the garrison is occupied. You may light up the fire now
	 *
	 * Will not be called by passive garrison
	 */
	virtual void garrison_occupied() = 0;
	/**
	 * Called on the new site when a site has been conquered
	 *
	 * Will not be called by passive garrison
	 */
	virtual void reinit_after_conqueral(Game & game) = 0;
};

}

#endif
