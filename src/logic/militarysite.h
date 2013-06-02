/*
 * Copyright (C) 2002-2004, 2007-2009 by the Widelands Development Team
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

#ifndef MILITARYSITE_H
#define MILITARYSITE_H

#include "attackable.h"
#include "productionsite.h"
#include "requirements.h"
#include "soldiercontrol.h"

namespace Widelands {

class Soldier;

struct MilitarySite_Descr : public ProductionSite_Descr {
	MilitarySite_Descr
		(char const * name, char const * descname,
		 const std::string & directory, Profile &,  Section & global_s,
		 const Tribe_Descr & tribe);

	virtual Building & create_object() const;

	virtual uint32_t get_conquers() const {return m_conquer_radius;}
	uint32_t get_max_number_of_soldiers () const throw () {
		return m_num_soldiers;
	}
	uint32_t get_heal_per_second        () const throw () {
		return m_heal_per_second;
	}

private:
	uint32_t m_conquer_radius;
	uint32_t m_num_soldiers;
	uint32_t m_heal_per_second;
};

class MilitarySite :
	public ProductionSite, public SoldierControl, public Attackable
{
	friend struct Map_Buildingdata_Data_Packet;
	MO_DESCR(MilitarySite_Descr);

public:
	MilitarySite(const MilitarySite_Descr &);
	virtual ~MilitarySite();

	char const * type_name() const throw () {return "militarysite";}
	virtual std::string get_statistics_string();

	virtual void init(Editor_Game_Base &);
	virtual void cleanup(Editor_Game_Base &);
	virtual void act(Game &, uint32_t data);
	virtual void remove_worker(Worker &);

	virtual void set_economy(Economy *);
	virtual bool get_building_work(Game &, Worker &, bool success);

	// Begin implementation of SoldierControl
	virtual std::vector<Soldier *> presentSoldiers() const;
	virtual std::vector<Soldier *> stationedSoldiers() const;
	virtual uint32_t minSoldierCapacity() const throw ();
	virtual uint32_t maxSoldierCapacity() const throw ();
	virtual uint32_t soldierCapacity() const;
	virtual void setSoldierCapacity(uint32_t capacity);
	virtual void dropSoldier(Soldier &);
	virtual int incorporateSoldier(Editor_Game_Base & game, Soldier & s);
	// End implementation of SoldierControl

	// Begin implementation of Attackable
	virtual Player & owner() const {return Building::owner();}
	virtual bool canAttack();
	virtual void aggressor(Soldier &);
	virtual bool attack   (Soldier &);
	// End implementation of Attackable

	/**
	 * Launch the given soldier on an attack towards the given
	 * target building.
	 */
	void sendAttacker(Soldier &, Building &, uint8_t);

	/// This methods are helper for use at configure this site.
	void set_requirements  (const Requirements &);
	void clear_requirements();
	const Requirements & get_requirements () const {
		return m_soldier_requirements;
	}

	void reinit_after_conqueration(Game &);

	void update_soldier_request();

	void preferSkilledSoldiers();
	void preferAnySoldiers();
	void preferCheapSoldiers();

	bool preferringSkilledSoldiers() const;
	bool preferringAnySoldiers() const;
	bool preferringCheapSoldiers() const;

	static const uint8_t soldier_trainlevel_any = 0;
	static const uint8_t soldier_trainlevel_rookie = 1;
	static const uint8_t soldier_trainlevel_hero = 2;

protected:
	void conquer_area(Editor_Game_Base &);

	virtual void create_options_window
		(Interactive_GameBase &, UI::Window * & registry);

private:
	bool isPresent(Soldier &) const;
	static void request_soldier_callback
		(Game &, Request &, Ware_Index, Worker *, PlayerImmovable &);

	Map_Object * popSoldierJob
		(Soldier *, bool * stayhome = 0, uint8_t * retreat = 0);
	bool haveSoldierJob(Soldier &);
	bool military_presence_kept(Game &);
	void informPlayer(Game &, bool discovered = false);
	bool update_upgrade_requirements();
	void update_normal_soldier_request();
	void update_upgrade_soldier_request();
	int incorporateUpgradedSoldier(Editor_Game_Base & game, Soldier & s);
	void update_soldier_request_impl(bool incd);


private:
	Requirements m_soldier_requirements;
	//RequireOr m_soldier_upgrade_requirements;
	Requirements m_soldier_upgrade_requirements;
	uint32_t     m_soldier_upgrade_required_min;
	uint32_t     m_soldier_upgrade_required_max;
	Request    * m_soldier_normal_request;
	Request    * m_soldier_upgrade_request;
	bool m_didconquer;
	uint32_t m_capacity;

	/**
	 * Next gametime where we should heal something.
	 */
	int32_t m_nexthealtime;

	struct SoldierJob {
		Soldier    * soldier;
		Object_Ptr  enemy;
		bool        stayhome;
		uint8_t     retreat;
	};
	std::vector<SoldierJob> m_soldierjobs;
	uint8_t soldier_preference;
	int32_t next_swap_soldiers_time;
	bool soldier_upgrade_try; // optimization -- if everybody is zero-level, do not downgrade
	bool doing_upgrade_request;
};

}

#endif
