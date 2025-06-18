/*
 * Copyright (C) 2002-2025 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_MILITARYSITE_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_MILITARYSITE_H

#include <memory>

#include "base/macros.h"
#include "economy/soldier_request.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/soldiercontrol.h"
#include "scripting/lua_table.h"

namespace Widelands {

class MilitarySiteDescr : public BuildingDescr {
public:
	MilitarySiteDescr(const std::string& init_descname,
	                  const LuaTable& t,
	                  Descriptions& descriptions);
	~MilitarySiteDescr() override = default;

	[[nodiscard]] Building& create_object() const override;

	[[nodiscard]] uint32_t get_conquers() const override {
		return conquer_radius_;
	}
	void set_conquers(uint32_t c) {
		conquer_radius_ = c;
	}
	[[nodiscard]] Quantity get_max_number_of_soldiers() const {
		return num_soldiers_;
	}
	void set_max_number_of_soldiers(Quantity q) {
		num_soldiers_ = q;
	}
	[[nodiscard]] uint32_t get_heal_per_second() const {
		return heal_per_second_;
	}
	void set_heal_per_second(uint32_t h) {
		heal_per_second_ = h;
	}

	bool prefers_heroes_at_start_;
	std::string occupied_str_;
	std::string aggressor_str_;
	std::string attack_str_;
	std::string defeated_enemy_str_;
	std::string defeated_you_str_;

private:
	uint32_t conquer_radius_{0U};
	Quantity num_soldiers_{0U};
	uint32_t heal_per_second_{0U};
	DISALLOW_COPY_AND_ASSIGN(MilitarySiteDescr);
};

class MilitarySite : public Building {
	friend class MapBuildingdataPacket;
	MO_DESCR(MilitarySiteDescr)

public:
	explicit MilitarySite(const MilitarySiteDescr&);
	~MilitarySite() override;

	bool init(EditorGameBase&) override;
	void cleanup(EditorGameBase&) override;
	void act(Game&, uint32_t data) override;
	void remove_worker(Worker&) override;

	void set_economy(Economy*, WareWorker) override;
	bool get_building_work(Game&, Worker&, bool success) override;

	/// Launch the given soldier on an attack towards the given
	/// target building.
	void send_attacker(Soldier&, Building&);

	void reinit_after_conqueration(Game&);

	void update_soldier_request(bool i = false);

	void set_soldier_preference(SoldierPreference);
	[[nodiscard]] SoldierPreference get_soldier_preference() const {
		return soldier_request_manager_.get_preference();
	}

	std::unique_ptr<const BuildingSettings> create_building_settings() const override;

protected:
	void conquer_area(EditorGameBase&);

private:
	void update_statistics_string(std::string*) override;

	static void
	request_soldier_callback(Game&, Request&, DescriptionIndex, Worker*, PlayerImmovable&);

	MapObject* pop_soldier_job(Soldier*, bool* stayhome = nullptr);
	bool has_soldier_job(Soldier&);
	bool military_presence_kept(Game&);
	void notify_player(Game&, bool discovered = false);
	bool incorporate_upgraded_soldier(EditorGameBase& egbase, Soldier& s);
	Soldier* find_least_suited_soldier();
	bool drop_least_suited_soldier(bool new_soldier_has_arrived, Soldier* newguy);

	// We can be attacked if we have stationed soldiers.
	class AttackTarget : public Widelands::AttackTarget {
	public:
		explicit AttackTarget(MilitarySite* military_site) : military_site_(military_site) {
		}

		bool can_be_attacked() const override;
		void enemy_soldier_approaches(const Soldier&) const override;
		Widelands::AttackTarget::AttackResult attack(Soldier*) const override;

		void set_allow_conquer(PlayerNumber p, bool c) const override {
			allow_conquer_[p] = c;
		}
		bool get_allow_conquer(PlayerNumber p) const override {
			auto it = allow_conquer_.find(p);
			return it == allow_conquer_.end() || it->second;
		}

	private:
		friend class MapBuildingdataPacket;
		MilitarySite* const military_site_;
		mutable std::map<PlayerNumber, bool> allow_conquer_;
	};

	class SoldierControl : public Widelands::SoldierControl {
	public:
		explicit SoldierControl(MilitarySite* military_site) : military_site_(military_site) {
		}

		[[nodiscard]] std::vector<Soldier*> present_soldiers() const override;
		[[nodiscard]] std::vector<Soldier*> stationed_soldiers() const override;
		[[nodiscard]] std::vector<Soldier*> associated_soldiers() const override;
		[[nodiscard]] Quantity min_soldier_capacity() const override;
		[[nodiscard]] Quantity max_soldier_capacity() const override;
		[[nodiscard]] Quantity soldier_capacity() const override;
		void set_soldier_capacity(Quantity capacity) override;
		void drop_soldier(Soldier&) override;
		int incorporate_soldier(EditorGameBase& egbase, Soldier& s) override;

	private:
		MilitarySite* const military_site_;
	};

	AttackTarget attack_target_;
	SoldierControl soldier_control_;
	bool didconquer_{false};
	Quantity capacity_;

	/**
	 * Next gametime where we should heal something.
	 */
	Time nexthealtime_{0U};

	struct SoldierJob {
		Soldier* soldier;
		ObjectPointer enemy;
		bool stayhome;
	};
	std::vector<SoldierJob> soldierjobs_;
	Time next_swap_soldiers_time_{0U};
	SoldierRequestManager soldier_request_manager_;

	static constexpr size_t kNoOfStatisticsStringCases = 4U;
	std::vector<std::map<std::tuple<int, int, int>, std::string>> statistics_string_cache_{
	   kNoOfStatisticsStringCases};
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_MILITARYSITE_H
