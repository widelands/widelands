/*
 * Copyright (C) 2008 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef SOLDIERCONTROL_H
#define SOLDIERCONTROL_H

#include <vector>

namespace Widelands {

class Soldier;

/**
 * This interface is implemented by buildings that explicitly house soldiers.
 *
 * It is used to implement queries for building UI and to implement commands
 * that change the soldiers stationed in a building.
 *
 * Soldiers are \em stationed in a building if that building is their
 * \ref Worker::location. Stationed soldiers are \em present in a building
 * if their current position is inside the building. So for a \ref TrainingSite,
 * the two concepts are equal. However, they're different for a \ref MilitarySite,
 * where soldiers can be outside in combat.
 */
struct SoldierControl {
	/**
	 * \return a list of soldiers that are currently present in the building.
	 */
	virtual std::vector<Soldier*> presentSoldiers() const = 0;

	/**
	 * \return a list of soldiers that are currently stationed in the building.
	 */
	virtual std::vector<Soldier*> stationedSoldiers() const = 0;

	/**
	 * \return the minimum number of soldiers that this building can be
	 * configured to hold.
	 */
	virtual uint32_t minSoldierCapacity() const = 0;

	/**
	 * \return the minimum number of soldiers that this building can be
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
				 maxSoldierCapacity());
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
	virtual void dropSoldier(Soldier*) = 0;

protected:
	virtual ~SoldierControl() {}
};

}

#endif // SOLDIERCONTROL_H
