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

#include "logic/attackable.h"
#include "logic/productionsite.h"
#include "logic/requirements.h"
#include "logic/soldiercontrol.h"

namespace Widelands {

class Soldier;

struct MilitarySite_Descr : public ProductionSite_Descr {
	MilitarySite_Descr
		(char const * name, char const * descname,
		 const std::string & directory, Profile &,  Section & global_s,
		 const Tribe_Descr & tribe);

	virtual Building & create_object() const override;

	virtual uint32_t get_conquers() const override {return m_conquer_radius;}
	uint32_t get_max_number_of_soldiers () const {
		return m_num_soldiers;
	}
	uint32_t get_heal_per_second        () const {
		return m_heal_per_second;
	}

	bool     m_prefers_heroes_at_start;
	std::string m_occupied_str;
	std::string m_aggressor_str;
	std::string m_attack_str;
	std::string m_defeated_enemy_str;
	std::string m_defeated_you_str;
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
	// I assume elsewhere, that enum SoldierPreference fits to uint8_t.
	enum SoldierPreference  : uint8_t {
		kNoPreference,
		kPrefersRookies,
		kPrefersHeroes,
	};

	MilitarySite(const MilitarySite_Descr &);
	virtual ~MilitarySite();

	char const * type_name() const override {return "militarysite";}
	virtual std::string get_statistics_string() override;

	virtual void init(Editor_Game_Base &) override;
	virtual void cleanup(Editor_Game_Base &) override;
	virtual void act(Game &, uint32_t data) override;
	virtual void remove_worker(Worker &) override;

	virtual void set_economy(Economy *) override;
	virtual bool get_building_work(Game &, Worker &, bool success) override;

	// Begin implementation of SoldierControl
	virtual std::vector<Soldier *> presentSoldiers() const override;
	virtual std::vector<Soldier *> stationedSoldiers() const override;
	virtual uint32_t minSoldierCapacity() const override;
	virtual uint32_t maxSoldierCapacity() const override;
	virtual uint32_t soldierCapacity() const override;
	virtual void setSoldierCapacity(uint32_t capacity) override;
	virtual void dropSoldier(Soldier &) override;
	virtual int incorporateSoldier(Editor_Game_Base & game, Soldier & s) override;
	// End implementation of SoldierControl

	// Begin implementation of Attackable
	virtual Player & owner() const override {return Building::owner();}
	virtual bool canAttack() override;
	virtual void aggressor(Soldier &) override;
	virtual bool attack   (Soldier &) override;
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

	void update_soldier_request(bool i = false);

	void set_soldier_preference(SoldierPreference);
	SoldierPreference get_soldier_preference() const {
			return m_soldier_preference;
	}

protected:
	void conquer_area(Editor_Game_Base &);

	virtual void create_options_window
		(Interactive_GameBase &, UI::Window * & registry) override;

private:
	bool isPresent(Soldier &) const;
	static void request_soldier_callback
		(Game &, Request &, Ware_Index, Worker *, PlayerImmovable &);

	Map_Object * popSoldierJob
		(Soldier *, bool * stayhome = nullptr, uint8_t * retreat = nullptr);
	bool haveSoldierJob(Soldier &);
	bool military_presence_kept(Game &);
	void informPlayer(Game &, bool discovered = false);
	bool update_upgrade_requirements();
	void update_normal_soldier_request();
	void update_upgrade_soldier_request();
	bool incorporateUpgradedSoldier(Editor_Game_Base & game, Soldier & s);
	Soldier * find_least_suited_soldier();
	bool drop_least_suited_soldier(bool new_has_arrived, Soldier * s);


private:
	Requirements m_soldier_requirements; // This is used to grab a bunch of soldiers: Anything goes
	RequireAttribute m_soldier_upgrade_requirements; // This is used when exchanging soldiers.
	std::unique_ptr<Request> m_normal_soldier_request;  // filling the site
	std::unique_ptr<Request> m_upgrade_soldier_request; // seeking for better soldiers
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
	SoldierPreference m_soldier_preference;
	int32_t m_next_swap_soldiers_time;
	bool m_soldier_upgrade_try; // optimization -- if everybody is zero-level, do not downgrade
	bool m_doing_upgrade_request;
};

}

#endif
