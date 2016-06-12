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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_MILITARYSITE_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_MILITARYSITE_H

#include <memory>

#include "base/macros.h"
#include "economy/request.h"
#include "logic/map_objects/attackable.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/requirements.h"
#include "logic/map_objects/tribes/soldiercontrol.h"
#include "scripting/lua_table.h"

namespace Widelands {

class Soldier;
class World;

class MilitarySiteDescr : public BuildingDescr {
public:
	MilitarySiteDescr(const std::string& init_descname, const LuaTable& t, const EditorGameBase& egbase);
	~MilitarySiteDescr() override {}

	Building & create_object() const override;

	uint32_t get_conquers() const override {return conquer_radius_;}
	Quantity get_max_number_of_soldiers () const {
		return num_soldiers_;
	}
	uint32_t get_heal_per_second        () const {
		return heal_per_second_;
	}

	bool     prefers_heroes_at_start_;
	std::string occupied_str_;
	std::string aggressor_str_;
	std::string attack_str_;
	std::string defeated_enemy_str_;
	std::string defeated_you_str_;


private:
	uint32_t conquer_radius_;
	Quantity num_soldiers_;
	uint32_t heal_per_second_;
	DISALLOW_COPY_AND_ASSIGN(MilitarySiteDescr);
};

class MilitarySite :
	public Building, public SoldierControl, public Attackable
{
	friend class MapBuildingdataPacket;
	MO_DESCR(MilitarySiteDescr)

public:
	// I assume elsewhere, that enum SoldierPreference fits to uint8_t.
	enum SoldierPreference  : uint8_t {
		kNoPreference,
		kPrefersRookies,
		kPrefersHeroes,
	};

	MilitarySite(const MilitarySiteDescr &);
	virtual ~MilitarySite();

	void init(EditorGameBase &) override;
	void cleanup(EditorGameBase &) override;
	void act(Game &, uint32_t data) override;
	void remove_worker(Worker &) override;

	void set_economy(Economy *) override;
	bool get_building_work(Game &, Worker &, bool success) override;

	// Begin implementation of SoldierControl
	std::vector<Soldier *> present_soldiers() const override;
	std::vector<Soldier *> stationed_soldiers() const override;
	Quantity min_soldier_capacity() const override;
	Quantity max_soldier_capacity() const override;
	Quantity soldier_capacity() const override;
	void set_soldier_capacity(Quantity capacity) override;
	void drop_soldier(Soldier &) override;
	int incorporate_soldier(EditorGameBase & game, Soldier & s) override;

	// Begin implementation of Attackable
	Player & owner() const override {return Building::owner();}
	bool can_attack() override;
	void aggressor(Soldier &) override;
	bool attack   (Soldier &) override;
	// End implementation of Attackable

	/// Launch the given soldier on an attack towards the given
	/// target building.
	void send_attacker(Soldier &, Building &);

	/// This methods are helper for use at configure this site.
	void set_requirements  (const Requirements &);
	void clear_requirements();
	const Requirements & get_requirements () const {
		return soldier_requirements_;
	}

	void reinit_after_conqueration(Game &);

	void update_soldier_request(bool i = false);

	void set_soldier_preference(SoldierPreference);
	SoldierPreference get_soldier_preference() const {
			return soldier_preference_;
	}

protected:
	void conquer_area(EditorGameBase &);

	void create_options_window(InteractiveGameBase&, UI::Window*& registry) override;

private:
	void update_statistics_string(std::string*) override;

	bool is_present(Soldier &) const;
	static void request_soldier_callback
		(Game &, Request &, DescriptionIndex, Worker *, PlayerImmovable &);

	MapObject * pop_soldier_job
		(Soldier *, bool * stayhome = nullptr);
	bool has_soldier_job(Soldier &);
	bool military_presence_kept(Game &);
	void notify_player(Game &, bool discovered = false);
	bool update_upgrade_requirements();
	void update_normal_soldier_request();
	void update_upgrade_soldier_request();
	bool incorporate_upgraded_soldier(EditorGameBase & game, Soldier & s);
	Soldier * find_least_suited_soldier();
	bool drop_least_suited_soldier(bool new_has_arrived, Soldier * s);


private:
	Requirements soldier_requirements_; // This is used to grab a bunch of soldiers: Anything goes
	RequireAttribute soldier_upgrade_requirements_; // This is used when exchanging soldiers.
	std::unique_ptr<Request> normal_soldier_request_;  // filling the site
	std::unique_ptr<Request> upgrade_soldier_request_; // seeking for better soldiers
	bool didconquer_;
	Quantity capacity_;

	/**
	 * Next gametime where we should heal something.
	 */
	int32_t nexthealtime_;

	struct SoldierJob {
		Soldier    * soldier;
		ObjectPointer  enemy;
		bool        stayhome;
	};
	std::vector<SoldierJob> soldierjobs_;
	SoldierPreference soldier_preference_;
	int32_t next_swap_soldiers_time_;
	bool soldier_upgrade_try_; // optimization -- if everybody is zero-level, do not downgrade
	bool doing_upgrade_request_;
};

}

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_MILITARYSITE_H
