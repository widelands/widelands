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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
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
		 std::string const & directory, Profile &,  Section & global_s,
		 Tribe_Descr const & tribe, EncodeData const *);

	virtual Building & create_object() const;

	virtual uint32_t get_conquers() const {return m_conquer_radius;}
	uint32_t get_max_number_of_soldiers () const throw () {
		return m_num_soldiers;
	}
	uint32_t get_max_number_of_medics   () const throw () {
		return m_num_medics;
	}
	uint32_t get_heal_per_second        () const throw () {
		return m_heal_per_second;
	}
	uint32_t get_heal_increase_per_medic() const throw () {
		return m_heal_incr_per_medic;
	}

private:
	uint32_t m_conquer_radius;
	uint32_t m_num_soldiers;
	uint32_t m_num_medics;
	uint32_t m_heal_per_second;
	uint32_t m_heal_incr_per_medic;
};

class MilitarySite :
	public ProductionSite, public SoldierControl, public Attackable
{
	friend struct Map_Buildingdata_Data_Packet;
	MO_DESCR(MilitarySite_Descr);

public:
	MilitarySite(MilitarySite_Descr const &);
	virtual ~MilitarySite();

	void prefill
		(Game &, uint32_t const *, uint32_t const *, Soldier_Counts const *);

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
	// End implementation of SoldierControl

	// Begin implementation of Attackable
	virtual bool canAttack();
	virtual void aggressor(Soldier &);
	virtual bool attack   (Soldier &);
	// End implementation of Attackable

	/**
	 * Launch the given soldier on an attack towards the given
	 * target building.
	 */
	void sendAttacker(Soldier &, Building &);

	/// This methods are helper for use at configure this site.
	void set_requirements  (Requirements const &);
	void clear_requirements();
	Requirements const & get_requirements () const {
		return m_soldier_requirements;
	}

	void update_soldier_request();

protected:
	void conquer_area(Game &);

	virtual void create_options_window
		(Interactive_GameBase &, UI::Window * & registry);

private:
	bool isPresent(Soldier &) const;
	static void request_soldier_callback
		(Game &, Request &, Ware_Index, Worker *, PlayerImmovable &);

	Map_Object * popSoldierJob(Soldier *, bool * stayhome = 0);
	bool haveSoldierJob(Soldier &);
	void informPlayer(Game &, bool discovered = false);

private:
	Requirements m_soldier_requirements;
	Request    * m_soldier_request;
	bool m_didconquer;
	uint32_t m_capacity;

	/**
	 * Next gametime where we should heal something.
	 */
	int32_t m_nexthealtime;

	struct SoldierJob {
		Soldier * soldier;
		Object_Ptr enemy;
		bool stayhome;
	};
	std::vector<SoldierJob> m_soldierjobs;
};

}

#endif
