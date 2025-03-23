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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_TRAININGSITE_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_TRAININGSITE_H

#include <memory>

#include "base/macros.h"
#include "logic/map_objects/tribes/productionsite.h"
#include "logic/map_objects/tribes/soldiercontrol.h"
#include "logic/map_objects/tribes/training_attribute.h"

struct TrainingSiteWindow;

namespace Widelands {

// Unique key to address each training level of each war art
using TypeAndLevel = uint32_t;

class TrainingSiteDescr : public ProductionSiteDescr {
public:
	TrainingSiteDescr(const std::string& init_descname,
	                  const LuaTable& table,
	                  Descriptions& descriptions);
	~TrainingSiteDescr() override = default;

	[[nodiscard]] Building& create_object() const override;

	[[nodiscard]] Quantity get_max_number_of_soldiers() const {
		return num_soldiers_;
	}
	void set_max_number_of_soldiers(Quantity q) {
		num_soldiers_ = q;
	}
	[[nodiscard]] bool get_train_health() const {
		return train_health_;
	}
	[[nodiscard]] bool get_train_attack() const {
		return train_attack_;
	}
	[[nodiscard]] bool get_train_defense() const {
		return train_defense_;
	}
	[[nodiscard]] bool get_train_evade() const {
		return train_evade_;
	}

	[[nodiscard]] unsigned get_min_level(TrainingAttribute) const;
	[[nodiscard]] unsigned get_max_level(TrainingAttribute) const;
	[[nodiscard]] int32_t get_max_stall() const {
		return max_stall_;
	}
	void set_max_stall(int32_t trainer_patience) {
		max_stall_ = trainer_patience;
	}

	[[nodiscard]] const std::string& no_soldier_to_train_message() const {
		return no_soldier_to_train_message_;
	}

	[[nodiscard]] const std::string& no_soldier_for_training_level_message() const {
		return no_soldier_for_training_level_message_;
	}

	[[nodiscard]] const ProductionProgram::Groups& get_training_cost(const TypeAndLevel& upgrade_step) const {
		return training_costs_.at(upgrade_step);
	}

private:
	void update_level(TrainingAttribute attrib, unsigned from_level, unsigned to_level);

	//  TODO(unknown): These variables should be per soldier type. They should be in a
	//  struct and there should be a vector, indexed by Soldier_Index,
	//  with that struct structs as element type.
	/** Maximum number of soldiers for a training site */
	Quantity num_soldiers_;
	/** Number of rounds w/o successful training, after which a soldier is kicked out. */
	uint32_t max_stall_;
	/** Whether this site can train health */
	bool train_health_{false};
	/** Whether this site can train attack */
	bool train_attack_{false};
	/** Whether this site can train defense */
	bool train_defense_{false};
	/** Whether this site can train evasion */
	bool train_evade_{false};

	/** Minimum health a soldier needs to train at this site */
	unsigned min_health_{std::numeric_limits<uint32_t>::max()};
	/** Minimum attack a soldier needs to train at this site */
	unsigned min_attack_{std::numeric_limits<uint32_t>::max()};
	/** Minimum defense a soldier needs to train at this site */
	unsigned min_defense_{std::numeric_limits<uint32_t>::max()};
	/** Minimum evade a soldier needs to train at this site */
	unsigned min_evade_{std::numeric_limits<uint32_t>::max()};

	/** Maximum health a soldier can acquire at this site */
	unsigned max_health_{0U};
	/** Maximum attack a soldier can acquire at this site */
	unsigned max_attack_{0U};
	/** Maximum defense a soldier can acquire at this site */
	unsigned max_defense_{0U};
	/** Maximum evasion a soldier can acquire at this site */
	unsigned max_evade_{0U};

	std::map<TypeAndLevel, ProductionProgram::Groups> training_costs_;

	std::string no_soldier_to_train_message_;
	std::string no_soldier_for_training_level_message_;

	DISALLOW_COPY_AND_ASSIGN(TrainingSiteDescr);
};

/**
 * A building to change soldiers' abilities.
 * Soldiers can gain health, or experience in attack, defense and evasion.
 *
 * \note  A training site does not change influence areas. If you lose the
 *        surrounding strongholds, the training site will burn even if it
 *        contains soldiers!
 */
class TrainingSite : public ProductionSite {
	friend class MapBuildingdataPacket;
	MO_DESCR(TrainingSiteDescr)
	friend struct ::TrainingSiteWindow;

	struct Upgrade {
		TypeAndLevel key;
		std::string program_name;

		enum class Status : uint8_t {
			kDisabled = 0,     // the input queue settings prevent this upgrade
			kNotPossible = 1,  // one or more wares are missing
			kWait = 2,         // one or more wares are missing, but they all have active transfers
			kCanStart = 3      // all necessary wares are available
		};
		Status status{Status::kNotPossible};
		std::vector<Soldier*> candidates;

		Upgrade(TrainingAttribute attr, const uint16_t level);
		bool has_wares_and_candidate() const {
			return status == Upgrade::Status::kCanStart && !candidates.empty();
		}
	};

public:
	explicit TrainingSite(const TrainingSiteDescr&);

	bool init(EditorGameBase&) override;
	void cleanup(EditorGameBase&) override;
	void act(Game&, uint32_t data) override;

	void add_worker(Worker&) override;
	void remove_worker(Worker&) override;
	bool is_present(Worker& worker) const override;

	// TODO(tothxa): implement controlling by UI
	//               also make it SoldierPreference instead of bool
	bool get_build_heroes() const {
		return build_heroes_;
	}
	void set_build_heroes(bool b_heroes) {
		build_heroes_ = b_heroes;
	}

	void set_economy(Economy* e, WareWorker type) override;

	[[nodiscard]] unsigned current_training_level() const;
	[[nodiscard]] TrainingAttribute current_training_attribute() const;

	// Returns the previously selected soldier if still available, or tries to find a replacement
	Soldier* get_selected_soldier(Game& game, TrainingAttribute attr, unsigned level);

	std::unique_ptr<const BuildingSettings> create_building_settings() const override;

protected:
	void program_end(Game&, ProgramResult) override;

private:
	class SoldierControl : public Widelands::SoldierControl {
	public:
		explicit SoldierControl(TrainingSite* training_site) : training_site_(training_site) {
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
		TrainingSite* const training_site_;
	};

	void update_soldier_request(bool needs_update_statuses);
	static void
	request_soldier_callback(Game&, Request&, DescriptionIndex, Worker*, PlayerImmovable&);

	void find_and_start_next_program(Game&) override;

	// Only called when selected_soldier_ or the wares to train him or her are gone by the time
	// we need them.
	Soldier* pick_another_soldier(TrainingAttribute attr, unsigned level);

	// Takes preference in account, returns true if second is more preferred.
	bool compare_levels(unsigned first, unsigned second);

	// Used in initialization of TrainingSite
	void init_upgrades();
	void add_upgrades(TrainingAttribute attr);

	// Checks input queues and updates status and candidates of each Upgrade as well as lists of
	// soldiers who cannot start any upgrade
	void update_upgrade_statuses(bool select_next_step);

	void drop_unupgradable_soldiers(Game&);
	void drop_stalled_soldiers(Game&);

	// Only for loading from savegame!
	void set_current_training_step(uint8_t attr, uint16_t level);

	SoldierControl soldier_control_;

	/// Open requests for soldiers. The soldiers can be under way or unavailable
	Request* soldier_request_{nullptr};

	/** The soldiers currently at the training site*/
	std::vector<Soldier*> soldiers_;

	// Keep track of soldiers who cannot start any upgrade
	std::vector<Soldier*> untrainable_soldiers_;  // all statuses are Upgrade::Status::kDisabled
	std::vector<Soldier*> stalled_soldiers_;      // best status is Upgrade::Status::kNotPossible
	std::vector<Soldier*> waiting_soldiers_;      // best status is Upgrade::Status::kWait

	/** Number of soldiers that should be trained concurrently.
	 * Equal or less to maximum number of soldiers supported by a training site.
	 * There is no guarantee there really are capacity_ soldiers in the
	 * building - some of them might still be under way or even not yet
	 * available*/
	Quantity capacity_;

	/** True, \b request and upgrade already experienced soldiers first, when possible
	 * False, \b request and upgrade inexperienced soldiers first, when possible */
	bool build_heroes_{true};

	std::map<TypeAndLevel, Upgrade> upgrades_;
	std::map<TypeAndLevel, Upgrade>::iterator current_upgrade_;
	Upgrade::Status max_possible_status_{Upgrade::Status::kNotPossible};

	// The soldier we picked to be trained next
	OPtr<Soldier> selected_soldier_;

	// TODO(tothxa): Shouldn't ProductionSite already provide a searchable list of inputs?
	std::map<DescriptionIndex, InputQueue*> inputs_map_;

	// These are used for kicking out soldiers prematurely
	static const uint32_t training_state_multiplier_;
	uint32_t max_stall_val_;
	uint32_t failures_count_{0};

	bool force_rebuild_soldier_requests_{true};
};

/**
 * Note to be published when a soldier is leaving the training center
 */
// A note we're using to notify the AI
struct NoteTrainingSiteSoldierTrained {
	CAN_BE_SENT_AS_NOTE(NoteId::TrainingSiteSoldierTrained)

	// The trainingsite from where soldier is leaving.
	TrainingSite* ts;

	// The player that owns the ttraining site.
	Player* player;

	NoteTrainingSiteSoldierTrained(TrainingSite* const init_ts, Player* init_player)
	   : ts(init_ts), player(init_player) {
	}
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_TRAININGSITE_H
