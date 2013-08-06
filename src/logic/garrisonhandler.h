/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#ifndef GARRISON_HANDLER_H
#define GARRISON_HANDLER_H

#include "logic/instances.h"
#include "logic/garrison.h"
#include "logic/requirements.h"
#include "logic/worker.h"

namespace Widelands {

struct Building;
struct Request;
class Soldier;

#define GARRISON_SWAP_TIMEOUT 20000
/**
 * Implementation of the Garrison interface to used by GarrisonOwner.
 */
class GarrisonHandler : public Garrison {
friend struct Map_Buildingdata_Data_Packet;
public:
	/**
	 * Create a new garrison handler
	 * \param building The building that holds this garrison. Must implements GarrisonOwner
	 * \param min_soldiers The minimum soldiers to be present in the garrison
	 * \param max_soldiers The maximum soldiers that can be present in this garrison
	 * \param conquer_radius The radius of the area conqered by this garrison
	 * \param heal_per_second The speed at which soldiers will be healed
	 * \param soldier_pref The soldier preference for rookies or heroes
	 * \param passive A passive garrison is only helpful to keep some soldiers around. It can't
	 * attack nor be attacked. I think of training site or ships
	 */
	GarrisonHandler
		(Building& building, uint32_t min_soldiers, uint32_t max_soldiers,
		 uint32_t conquer_radius, uint32_t heal_per_second, SoldierPref soldier_pref,
		 bool passive = false);
	virtual ~GarrisonHandler();

	/**
	 * Must be called once
	 */
	void init(Editor_Game_Base &);
	/**
	 * Must be called once after conqueral
	 */
	void reinit_after_conqueral(Game & game);
	/**
	 * Cleanup handled objects
	 */
	void cleanup(Editor_Game_Base &);
	/**
	 * Eequests must be cleaned up after the building has
	 * been cleaned up, hence this seperate function.
	 */
	void cleanup_requests(Editor_Game_Base &);
	/**
	 * Must be called regularly. It is not time dependant though.
	 */
	void act(Game &);
	/**
	 * Must be called when the owner building changes economy
	 */
	void set_economy(Economy * const e);
	/**
	 * Pop the given soldier out of the building, while keeping it
	 * in the garrion.
	 */
	void popSoldier(Soldier* soldier);
	/**
	 * Get some work for a soldier that asks for it
	 */
	bool get_garrison_work(Game & game, Soldier* soldier);
	/**
	 * Set the requirements for that soldier. This will be used when filling
	 * request for soldiers.
	 */
	void set_soldier_requirements(Requirements req);

	// Garrison implementation
	virtual Player & owner() const;
	virtual bool canAttack() const;
	virtual void aggressor(Soldier &);
	virtual bool attack(Soldier &);
	virtual const std::vector<Soldier *> presentSoldiers() const;
	virtual const std::vector<Soldier *> stationedSoldiers() const;
	virtual uint32_t minSoldierCapacity() const;
	virtual uint32_t maxSoldierCapacity() const;
	virtual uint32_t soldierCapacity() const;
	virtual void setSoldierCapacity(uint32_t capacity);
	virtual void dropSoldier(Soldier &);
	virtual int incorporateSoldier(Editor_Game_Base &, Soldier &);
	virtual int outcorporateSoldier(Editor_Game_Base &, Soldier &);
	virtual uint32_t conquerRadius() const;
	virtual void set_soldier_preference(SoldierPref p);
	virtual SoldierPref get_soldier_preference() const {
		return m_soldier_preference;
	}
	virtual void sendAttacker(Soldier & soldier, Building & target, uint8_t retreat);


private:
	enum class InfoType : uint8_t {
		AGGRESSSED,
		UNDER_ATTACK,
		GARRISON_LOST,
		GARRISON_CAPTURED,
		GARRISON_OCCUPIED
	};
	// Helper methods
	/**
	 * Sends a message to the owner
	 */
	void inform_owner(Game&, InfoType);
	/**
	 * Check if a soldier is actually present in building
	 */
	bool isPresent(Soldier &) const;
	/**
	 * Conquers the area once a soldier occupied the site
	 */
	void conquer_area(Editor_Game_Base &);
	/**
	 * Check if we keep military influence once our garrison
	 * has been conquered
	 */
	bool military_presence_kept(Game &);
	/**
	 * Update the soldier requests. If upgraded_incorporated, a soldier
	 * requested by an upgrade request has just been incorporated. This
	 * method handle requesting for normal case (filling slots) or for
	 * upgrading to a better suited soldier
	 */
	void update_soldier_request(bool upgraded_incorporated = false);
	/**
	 * Request new soldiers or evict some to fill the capacity
	 */
	void update_normal_soldier_request();
	/* There are two kinds of soldier requests: "normal", which is used whenever the military site needs
	* more soldiers, and "upgrade" which is used when there is a preference for either heroes or
	* rookies.
	*
	* In case of normal requests, the military site is filled. In case of upgrade requests, only one guy
	* is exchanged at a time.
	*
	* There would be more efficient ways to get well trained soldiers. Now, new buildings appearing in battle
	* field are more vulnerable at the beginning. This is intentional. The purpose of this upgrade thing is
	* to reduce the benefits of site micromanagement. The intention is not to make gameplay easier
	* in other ways.
	*/
	void update_upgrade_soldier_request();
	/**
	 * The callback function, called whenever one of our soldier requested arrived
	 */
	static void request_soldier_callback
		(Game &, Request &, Ware_Index, Worker *, PlayerImmovable &);
	/**
	 * The transfer callback function, called whenever one of our requested
	 * soldier started to move towards the garrison
	 */
	static void request_soldier_transfer_callback
		(Game &, Request &, Ware_Index, Worker *, PlayerImmovable &);
	/*
	* When upgrading soldiers, we do not ask for just any soldiers, but soldiers
	* that are better than what we already have. This routine sets the requirements
	* used by the request.
	*
	* The routine returns true if upgrade request thresholds have changed. This information could be
	* used to decide whether the soldier-Request should be upgraded. Additionally, the m_try_soldier_upgrade
	* field will be set so to know if an attempt to get better soldier sould be performed.
	*/
	bool update_upgrade_requirements();
	/**
	 * Called when an upgraded soldier comes to join the garrison
	 */
	bool incorporateUpgradedSoldier(Editor_Game_Base & game, Soldier & s);
	/**
	 * Find the least suited soldier for this garrison. We don't play
	 * in the same league
	 */
	Soldier * find_least_suited_soldier();
	/**
	 * Try to drop the lease suited soldier, return true on success.
	 * If a new soldier s has arrived, extra care will be taken to not drop
	 * anyone if the new one is even worst
	 */
	bool drop_least_suited_soldier(Soldier * s = nullptr);
	/**
	 * Return true if we hold a running job for that soldied
	 */
	bool haveSoldierJob(Soldier &) const;
	/**
	* \return the enemy, if any, that the given soldier was scheduled
	* to attack, and remove the job.
	*/
	Map_Object * popSoldierJob(Soldier *, bool * stayhome = 0, uint8_t * retreat = 0);

	// Basic fields
	Building& m_building;
	uint32_t m_min_capacity;
	uint32_t m_max_capacity;
	uint32_t m_capacity;
	bool m_passive;

	// Healing
	uint32_t m_heal_per_second;
	uint32_t m_last_heal_time;

	// Requests
	Requirements m_soldier_requirements; // This is used to grab a bunch of soldiers: Anything goes
	RequireAttribute m_soldier_upgrade_requirements; // This is used when exchanging soldiers.
	std::unique_ptr<Request> m_normal_soldier_request;  // filling the site
	std::unique_ptr<Request> m_upgrade_soldier_request; // seeking for better soldiers

	// Conqueral
	uint32_t m_conquer_radius;
	bool m_didconquer;

	// Job
	struct SoldierJob {
		Soldier    * soldier;
		Object_Ptr  enemy;
		bool        stayhome;
		uint8_t     retreat;
	};
	std::vector<SoldierJob> m_soldierjobs;

	// Soldier preferences
	SoldierPref m_soldier_preference;
	uint32_t m_last_swap_soldiers_time;
	bool m_try_soldier_upgrade; // optimization -- if everybody is zero-level, do not downgrade
	bool m_doing_upgrade_request;
};

}

#endif
