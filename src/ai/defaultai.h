/*
 * Copyright (C) 2008-2010, 2012 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_AI_DEFAULTAI_H
#define WL_AI_DEFAULTAI_H

#include <map>
#include <memory>
#include <unordered_set>

#include "ai/ai_help_structs.h"
#include "ai/computer_player.h"
#include "base/i18n.h"
#include "logic/immovable.h"
#include "logic/ship.h"
#include "logic/soldier.h"
#include "logic/trainingsite.h"

namespace Widelands {
struct Road;
struct Flag;
}

/**
 * Default Widelands Computer Player (defaultAI)
 *
 * The behaviour of defaultAI is controlled via \ref DefaultAI::think() and all
 * functions called by \ref DefaultAI::think().
 * At the moment defaultAI should be able to build up a basic infrastructure
 * including food, mining and smithing infrastructure and a basic street net.
 * It should be able to expand it's territory and to recruit some soldiers from
 * the weapons made out of it's mined resources.
 * It does only construct buildable and allowed (scenario mode) buildings.
 * It behaves after preciousness_ of a ware, which can be defined in wares conf
 * file. The higher the preciousness_, the more will defaultAI care for that ware
 * and will try to build up an infrastructure to create that ware.
 *
 * \NOTE Network safeness:
 * - The current implementation does not care about network safe randomness, as
 *   only the host is running the computer player code and sends it's player
 *   commands to all other players. If this network behaviour is changed,
 *   remember to change some time() in network save random functions.
 */
// TODO(unknown): Improvements:
// - Improve different initialization types (Strong, Normal, Weak)
// - Improve update code - currently the whole buildable area owned by defaultAI
//   is rechecked after construction of a building or a road. Instead it would
//   be better to write down the changed coordinates and only check those and
//   surrounding ones. Same applies for other parts of the code:
//   e.g. check_militarysite checks the whole visible area for enemy area, but
//   it would already be enough, if it checks the outer circle ring.
// - improvements and speedups in the whole defaultAI code.
// - handling of trainingsites (if supply line is broken - send some soldiers
//   out, to have some more forces. Reincrease the number of soldiers that
//   should be trained if inputs_ get filled again.).
struct DefaultAI : ComputerPlayer {
	enum class Type {
		kVeryWeak,
		kWeak,
		kNormal,
	};

	DefaultAI(Widelands::Game&, const Widelands::PlayerNumber, DefaultAI::Type);
	~DefaultAI();
	void think() override;

	enum class WalkSearch : uint8_t {kAnyPlayer, kOtherPlayers, kEnemy};
	enum class WoodPolicy : uint8_t {kDismantleRangers, kStopRangers, kAllowRangers};
	enum class NewShip : uint8_t {kBuilt, kFoundOnLoad};
	enum class PerfEvaluation : uint8_t {kForConstruction, kForDismantle};

	enum class Tribes : uint8_t {
		kNone,
		kBarbarians,
		kAtlanteans,
		kEmpire
	};


	/// Implementation for Strong
	struct NormalImpl : public ComputerPlayer::Implementation {
		NormalImpl() {
			name = "normal";
			/** TRANSLATORS: This is the name of an AI used in the game setup screens */
			descname = _("Normal AI");
			icon_filename = "pics/ai_normal.png";
		}
		ComputerPlayer* instantiate(Widelands::Game& game,
		                            Widelands::PlayerNumber const p) const override {
			return new DefaultAI(game, p, DefaultAI::Type::kNormal);
		}
	};

	struct WeakImpl : public ComputerPlayer::Implementation {
		WeakImpl() {
			name = "weak";
			/** TRANSLATORS: This is the name of an AI used in the game setup screens */
			descname = _("Weak AI");
			icon_filename = "pics/ai_weak.png";
		}
		ComputerPlayer* instantiate(Widelands::Game& game,
		                            Widelands::PlayerNumber const p) const override {
			return new DefaultAI(game, p, DefaultAI::Type::kWeak);
		}
	};

	struct VeryWeakImpl : public ComputerPlayer::Implementation {
		VeryWeakImpl() {
			name = "very_weak";
			/** TRANSLATORS: This is the name of an AI used in the game setup screens */
			descname = _("Very Weak AI");
			icon_filename = "pics/ai_very_weak.png";
		}
		ComputerPlayer* instantiate(Widelands::Game& game,
		                            Widelands::PlayerNumber const p) const override {
			return new DefaultAI(game, p, DefaultAI::Type::kVeryWeak);
		}
	};

	static NormalImpl normal_impl;
	static WeakImpl weak_impl;
	static VeryWeakImpl very_weak_impl;

private:
	void late_initialization();

	void update_all_buildable_fields(uint32_t);
	void update_all_mineable_fields(uint32_t);
	void update_all_not_buildable_fields();

	void update_buildable_field(BuildableField&, uint16_t = 6, bool = false);
	void update_mineable_field(MineableField&);

	void update_productionsite_stats();

	// for productionsites
	Widelands::BuildingNecessity check_building_necessity
		(BuildingObserver& bo, PerfEvaluation purpose, uint32_t);
	// for militarysites (overloading the function)
	Widelands::BuildingNecessity check_building_necessity
		(uint8_t, uint32_t);

	void sort_task_pool();
	void sort_by_priority();
	void set_taskpool_task_time(uint32_t, Widelands::SchedulerTaskId);
	uint32_t get_taskpool_task_time(Widelands::SchedulerTaskId);

	bool construct_building(uint32_t);

	uint32_t coords_hash(Widelands::Coords coords) {
		uint32_t hash = coords.x << 16 | coords.y;
		return hash;
	}

	Widelands::Coords coords_unhash(uint32_t hash) {
		Widelands::Coords coords;
		coords.x = hash >> 16;  // is cast needed here???
		coords.y = hash;
		return coords;
	}

	// all road management is invoked by function improve_roads()
	// if needed it calls create_shortcut_road() with a flag from which
	// new road should be considered (or is needed)
	bool improve_roads(uint32_t);
	bool create_shortcut_road(const Widelands::Flag&,
	                          uint16_t maxcheckradius,
	                          int16_t minReduction,
	                          const int32_t gametime);
	// trying to identify roads that might be removed
	bool dispensable_road_test(Widelands::Road&);
	bool check_economies();
	bool check_productionsites(uint32_t);
	bool check_trainingsites(uint32_t);
	bool check_mines_(uint32_t);
	bool check_militarysites(uint32_t);
	bool marine_main_decisions();
	bool check_ships(uint32_t);
	bool check_enemy_sites(uint32_t);
	void print_stats();
	// return single number of strength of vector of soldiers
	int32_t calculate_strength(const std::vector<Widelands::Soldier*>);
	uint32_t get_stocklevel_by_hint(size_t);
	uint32_t get_stocklevel(BuildingObserver&);
	uint32_t get_warehoused_stock(Widelands::DescriptionIndex wt);
	uint32_t get_stocklevel(Widelands::DescriptionIndex);  // count all direct outputs_
	void review_wares_targets(uint32_t);
	void count_military_vacant_positions();

	// sometimes scanning an area in radius gives inappropriate results, so this is to verify that
	// other player is accessible
	// via walking
	bool other_player_accessible(uint32_t max_distance,
	                             uint32_t* tested_fields,
	                             uint16_t* mineable_fields_count,
	                             const Widelands::Coords starting_spot,
	                             const WalkSearch type);

	int32_t recalc_with_border_range(const BuildableField&, int32_t);

	void
	consider_productionsite_influence(BuildableField&, Widelands::Coords, const BuildingObserver&);
	// considering trees, rocks, mines, water, fishes for candidate for colonization (new port)
	uint8_t spot_scoring(Widelands::Coords candidate_spot);

	EconomyObserver* get_economy_observer(Widelands::Economy&);
	BuildingObserver& get_building_observer(char const*);

	void gain_immovable(Widelands::PlayerImmovable&);
	void lose_immovable(const Widelands::PlayerImmovable&);
	void gain_building(Widelands::Building&);
	void lose_building(const Widelands::Building&);
	void gain_ship(Widelands::Ship&, NewShip);
	void expedition_management(ShipObserver&);
	void out_of_resources_site(const Widelands::ProductionSite&);
	void soldier_trained(const Widelands::TrainingSite&);

	bool check_supply(const BuildingObserver&);

	void print_land_stats();

	//checks whether first value is in range, or lesser then...
	template<typename T> void check_range(const T, const  T, const  T, const char *);
	template<typename T> void check_range(const T, const  T, const char *);

private:
	// Variables of default AI
	DefaultAI::Type type_;

	// collect statistics on how many times which job was run
	uint32_t sched_stat_[20] = {0};

	Widelands::Player* player_;
	Widelands::TribeDescr const* tribe_;

	std::vector<BuildingObserver> buildings_;
	uint32_t num_prod_constructionsites;
	uint32_t num_ports;

	int16_t last_attacked_player_;
	uint32_t last_attack_time_;
	// check ms in this interval - will auto-adjust
	uint32_t enemysites_check_delay_;

	// helping scores for building new military sites
	int32_t target_military_score_;
	int32_t least_military_score_;

	WoodPolicy wood_policy_;

	std::list<Widelands::FCoords> unusable_fields;
	std::list<BuildableField*> buildable_fields;
	std::list<BlockedField> blocked_fields;
	std::unordered_set<uint32_t> port_reserved_coords;
	std::list<MineableField*> mineable_fields;
	std::list<Widelands::Flag const*> new_flags;
	std::list<Widelands::Coords> flags_to_be_removed;
	std::list<Widelands::Road const*> roads;
	std::list<EconomyObserver*> economies;
	std::list<ProductionSiteObserver> productionsites;
	std::list<ProductionSiteObserver> mines_;
	std::list<MilitarySiteObserver> militarysites;
	std::list<WarehouseSiteObserver> warehousesites;
	std::list<TrainingSiteObserver> trainingsites;
	std::list<ShipObserver> allships;
	// This is a vector that is filled up on initiatlization
	// and no items are added/removed afterwards
	std::vector<SchedulerTask> taskPool;
	std::map<uint32_t, EnemySiteObserver> enemy_sites;
	// it will map mined material to observer
	std::map<int32_t, MineTypesObserver> mines_per_type;
	// returns count of mines of the same type (output)
	uint32_t mines_in_constr() const;
	uint32_t mines_built() const;
	std::map<int32_t, MilitarySiteSizeObserver> msites_per_size;
	// returns count of militarysites
	uint32_t msites_in_constr() const;
	uint32_t msites_built() const;

	int32_t limit_cnt_target(int32_t, int32_t);

	std::vector<WareObserver> wares;

	uint32_t next_ai_think_;
	uint32_t next_mine_construction_due_;
	uint32_t inhibit_road_building_;
	uint32_t time_of_last_construction_;
	uint32_t enemy_last_seen_;

	uint16_t numof_warehouses_;

	bool new_buildings_stop_;

	// when territory is expanded for every candidate field benefits are calculated
	// but need for water, space, mines can vary
	// so if 255 = resource is needed, 0 = not needed
	int32_t resource_necessity_territory_;
	int32_t resource_necessity_mines_;
	int32_t resource_necessity_water_;
	bool resource_necessity_water_needed_;  // unless atlanteans

	// average count of trees around cutters
	uint32_t trees_around_cutters_;

	uint16_t military_last_dismantle_;
	uint32_t military_last_build_;  // sometimes expansions just stops, this is time of last military
	                                // building build

	bool seafaring_economy;          // false by default, until first port space is found
	uint32_t colony_scan_area_;  // distance from a possible port that is scanned for owned territory
	// it decreases with failed scans
	uint32_t expedition_start_time_;
	bool no_more_expeditions_;
	uint32_t expedition_ship_;
	int16_t ships_utilization_; //0-10000 to avoid floats, used for decision for building new ships

	int32_t spots_;  // sum of buildable fields
	int32_t vacant_mil_positions_;  // sum of vacant positions in militarysites and training sites
	// statistics for training sites per type
	int16_t ts_basic_count_;
	int16_t ts_basic_const_count_;
	int16_t ts_advanced_count_;
	int16_t ts_advanced_const_count_;
	int16_t ts_without_trainers_;

	// This stores highest priority for new buildings except for militarysites
	int32_t highest_nonmil_prio_;

	// This stores highest priority for new buildings except for militarysites
	int32_t highest_nonmil_prio_;

	// this is helping counter to track how many scheduler tasks are too delayed
	// the purpose is to print out a warning that the game is pacing too fast
	int32_t scheduler_delay_counter_;

	int16_t ai_personality_military_loneliness_;
	uint32_t ai_personality_attack_margin_;
	int32_t ai_personality_wood_difference_;
	uint32_t ai_productionsites_ratio_;
	uint32_t ai_personality_early_militarysites;

	// this is a bunch of patterns that have to identify weapons and armors for input queues of trainingsites
	std::vector<std::string> const armors_and_weapons =
		{"ax", "lance", "armor", "helm", "lance", "trident", "tabard", "shield", "mask"};

	enum {kReprioritize, kStopShipyard, kStapShipyard};

	std::vector<int16_t> marineTaskQueue_;

	std::unique_ptr<Notifications::Subscriber<Widelands::NoteFieldPossession>>
	   field_possession_subscriber_;
	std::unique_ptr<Notifications::Subscriber<Widelands::NoteImmovable>> immovable_subscriber_;
	std::unique_ptr<Notifications::Subscriber<Widelands::NoteProductionSiteOutOfResources>>
	   outofresource_subscriber_;
	std::unique_ptr<Notifications::Subscriber<Widelands::NoteTrainingSiteSoldierTrained>>
	   soldiertrained_subscriber_;
	std::unique_ptr<Notifications::Subscriber<Widelands::NoteShipMessage>> shipnotes_subscriber_;
};

#endif  // end of include guard: WL_AI_DEFAULTAI_H
