/*
 * Copyright (C) 2008-2022 by the Widelands Development Team
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

#include <chrono>
#include <ctime>
#include <memory>

#include "ai/ai_help_structs.h"
#include "ai/computer_player.h"
#include "base/i18n.h"
#include "economy/economy.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/trainingsite.h"

namespace Widelands {
struct Road;
}  // namespace Widelands

namespace AI {

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
 * \note Network safeness:
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

	DefaultAI(Widelands::Game&, const Widelands::PlayerNumber, AiType);
	~DefaultAI() override;
	void think() override;

	enum class WalkSearch : uint8_t { kAnyPlayer, kOtherPlayers, kEnemy };
	enum class WoodPolicy : uint8_t { kDismantleRangers, kStopRangers, kAllowRangers };
	enum class NewShip : uint8_t { kBuilt, kFoundOnLoad };
	enum class FleetStatus : uint8_t { kNeedShip = 0, kEnoughShips = 1, kDoNothing = 2 };
	enum class PerfEvaluation : uint8_t { kForConstruction, kForDismantle };
	enum class BasicEconomyBuildingStatus : uint8_t { kEncouraged, kDiscouraged, kNeutral, kNone };

	enum class SoldiersStatus : uint8_t { kFull = 0, kEnough = 1, kShortage = 3, kBadShortage = 6 };

	enum class WareWorker : uint8_t { kWare, kWorker };

	/// Implementation for Strong
	struct NormalImpl : public ComputerPlayer::Implementation {
		NormalImpl()
		   : Implementation(
		        "normal",
		        /** TRANSLATORS: This is the name of an AI used in the game setup screens */
		        _("Normal AI"),
		        "images/ai/ai_normal.png",
		        Implementation::Type::kDefault) {
		}
		ComputerPlayer* instantiate(Widelands::Game& game,
		                            Widelands::PlayerNumber const p) const override {
			return new DefaultAI(game, p, AiType::kNormal);
		}
	};

	struct WeakImpl : public ComputerPlayer::Implementation {
		WeakImpl()
		   : Implementation(
		        "weak",
		        /** TRANSLATORS: This is the name of an AI used in the game setup screens */
		        _("Weak AI"),
		        "images/ai/ai_weak.png",
		        Implementation::Type::kDefault) {
		}
		ComputerPlayer* instantiate(Widelands::Game& game,
		                            Widelands::PlayerNumber const p) const override {
			return new DefaultAI(game, p, AiType::kWeak);
		}
	};

	struct VeryWeakImpl : public ComputerPlayer::Implementation {
		VeryWeakImpl()
		   : Implementation(
		        "very_weak",
		        /** TRANSLATORS: This is the name of an AI used in the game setup screens */
		        _("Very Weak AI"),
		        "images/ai/ai_very_weak.png",
		        Implementation::Type::kDefault) {
		}
		ComputerPlayer* instantiate(Widelands::Game& game,
		                            Widelands::PlayerNumber const p) const override {
			return new DefaultAI(game, p, AiType::kVeryWeak);
		}
	};

	static NormalImpl normal_impl;
	static WeakImpl weak_impl;
	static VeryWeakImpl very_weak_impl;

private:
	static constexpr int8_t kUncalculated = -1;
	static constexpr uint8_t kFalse = 0;
	static constexpr uint8_t kTrue = 1;

	static constexpr bool kAbsValue = true;
	static constexpr int32_t kSpotsTooLittle = 15;
	static constexpr Duration kManagementUpdateInterval{10 * 60 * 1000};
	static constexpr Duration kStatUpdateInterval{60 * 1000};
	static constexpr Duration kFlagWarehouseUpdInterval{15 * 1000};

	// common for defaultai.cc and defaultai_seafaring.cc
	static constexpr Duration kExpeditionMinDuration{60 * 60 * 1000};
	static constexpr Duration kExpeditionMaxDuration{210 * 60 * 1000};
	static constexpr Widelands::Serial kNoShip = Widelands::kInvalidSerial;
	static constexpr Duration kShipCheckInterval{5 * 1000};

	// used by defaultai_warfare.cc
	// duration of military campaign
	static constexpr Duration kCampaignDuration{15 * 60 * 1000};
	static constexpr Duration kTrainingSitesCheckInterval{15 * 1000};

	// Variables of default AI
	AiType type_;
	Widelands::Player* player_;
	Widelands::TribeDescr const* tribe_;

	// This points to persistent data stored in Player object
	Widelands::Player::AiPersistentState* persistent_data;

	void late_initialization();

	void update_all_buildable_fields(const Time&);
	void update_all_mineable_fields(const Time&);
	void update_all_not_buildable_fields(const Time&);
	void update_buildable_field(BuildableField&);
	void update_mineable_field(MineableField&);
	void update_productionsite_stats();
	unsigned find_immovables_nearby(
	   const std::set<std::pair<Widelands::MapObjectType,
	                            Widelands::MapObjectDescr::AttributeIndex>>& attribute_infos,
	   const Widelands::FCoords& position,
	   const WorkareaInfo& workarea_info) const;

	// for production sites
	BuildingNecessity
	check_building_necessity(BuildingObserver& bo, PerfEvaluation purpose, const Time&);
	BuildingNecessity check_warehouse_necessity(BuildingObserver&, const Time& gametime);
	void sort_task_pool();
	// void sort_by_priority(); NOCOM
	void set_taskpool_task_time(const Time&, SchedulerTaskId);
	const Time& get_taskpool_task_time(SchedulerTaskId);
	std::chrono::high_resolution_clock::time_point time_point;

	bool construct_building(const Time&);

	// all road management is invoked by function improve_roads()
	// if needed it calls create_shortcut_road() with a flag from which
	// new road should be considered (or is needed)
	bool improve_roads(const Time&);
	bool create_shortcut_road(const Widelands::Flag&, uint16_t maxcheckradius, const Time& gametime);
	// trying to identify roads that might be removed
	bool dispensable_road_test(const Widelands::Road&);
	bool dismantle_dead_ends();
	void collect_nearflags(std::map<uint32_t, NearFlag>&, const Widelands::Flag&, const uint16_t);
	// calculating distances from local warehouse to flags
	void check_flag_distances(const Time&);
	FlagWarehouseDistances flag_warehouse_distance;

	bool check_economies();
	bool check_productionsites(const Time&);
	bool check_mines_(const Time&);

	void print_stats(const Time&);

	uint32_t get_stocklevel(BuildingObserver&, const Time&, WareWorker = WareWorker::kWare) const;
	uint32_t
	   calculate_stocklevel(Widelands::DescriptionIndex,
	                        WareWorker = WareWorker::kWare) const;  // count all direct outputs_

	void review_wares_targets(const Time&);

	void update_player_stat(const Time&);

	// sometimes scanning an area in radius gives inappropriate results, so this is to verify that
	// other player is accessible
	// via walking
	bool other_player_accessible(uint32_t max_distance,
	                             uint32_t* tested_fields,
	                             uint16_t* mineable_fields_count,
	                             const Widelands::Coords& starting_spot,
	                             const WalkSearch& type);

	int32_t recalc_with_border_range(const BuildableField&, int32_t);

	void
	consider_productionsite_influence(BuildableField&, Widelands::Coords, const BuildingObserver&);

	EconomyObserver* get_economy_observer(Widelands::Economy&);
	uint8_t count_buildings_with_attribute(BuildingAttribute);
	uint32_t count_productionsites_without_buildings();
	BuildingObserver& get_building_observer(char const*);
	bool has_building_observer(char const*);
	BuildingObserver& get_building_observer(BuildingAttribute);
	BuildingObserver& get_building_observer(Widelands::DescriptionIndex);

	void gain_immovable(Widelands::PlayerImmovable&, bool found_on_load = false);
	void lose_immovable(const Widelands::PlayerImmovable&);
	void gain_building(Widelands::Building&, bool found_on_load);
	void lose_building(const Widelands::Building&);
	void out_of_resources_site(const Widelands::ProductionSite&);
	bool check_supply(const BuildingObserver&);
	bool set_inputs_to_zero(const ProductionSiteObserver&);
	void set_inputs_to_max(const ProductionSiteObserver&);
	void stop_site(const ProductionSiteObserver&);
	void initiate_dismantling(ProductionSiteObserver&, const Time&);

	// Checks whether first value is in range, or lesser then...
	template <typename T> void check_range(const T, const T, const T, const char*);
	template <typename T> void check_range(const T, const T, const char*);

	// Remove a member from std::deque
	template <typename T> bool remove_from_dqueue(std::deque<T const*>&, T const*);

	// Functions used for seafaring / defaultai_seafaring.cc
	Widelands::IslandExploreDirection randomExploreDirection();
	void gain_ship(Widelands::Ship&, NewShip);
	void check_ship_in_expedition(ShipObserver&, const Time&);
	void expedition_management(ShipObserver&);
	// considering trees, rocks, mines, water, fish for candidate for colonization (new port)
	uint8_t spot_scoring(Widelands::Coords candidate_spot);
	bool marine_main_decisions(const Time&);
	bool check_ships(const Time&);
	bool attempt_escape(ShipObserver& so);

	// finding and owner
	Widelands::PlayerNumber get_land_owner(const Widelands::Map&, uint32_t) const;

	// Functions used for war and training stuff / defaultai_warfare.cc
	bool check_militarysites(const Time& gametime);
	bool check_enemy_sites(const Time& gametime);
	void count_military_vacant_positions();
	bool check_trainingsites(const Time& gametime);
	// return single number of strength of vector of soldiers
	int32_t calculate_strength(const std::vector<Widelands::Soldier*>&);
	// for militarysites (overloading the function)
	BuildingNecessity check_building_necessity(BuildingObserver&, const Time&);
	void soldier_trained(const Widelands::TrainingSite&);
	bool critical_mine_unoccupied(const Time&);

	SoldiersStatus soldier_status_;
	int32_t vacant_mil_positions_average_;
	uint16_t attackers_count_;
	EventTimeQueue soldier_trained_log;
	EventTimeQueue soldier_attacks_log;

	// used by AI scheduler
	Time next_ai_think_;
	// this is helping counter to track how many scheduler tasks are too delayed
	// the purpose is to print out a warning that the game is pacing too fast
	int32_t scheduler_delay_counter_;

	std::map<Widelands::DescriptionIndex, WoodPolicy> wood_policy_;
	uint16_t trees_nearby_treshold_;

	std::vector<BuildingObserver> buildings_;
	std::vector<BuildingObserver> rangers_;
	std::deque<Widelands::FCoords> unusable_fields;
	std::deque<BuildableField*> buildable_fields;
	BlockedFields blocked_fields;
	std::unordered_set<uint32_t> ports_vicinity;
	std::unordered_set<uint32_t> ports_shipyard_region;
	PlayersStrengths player_statistics;
	ManagementData management_data;
	ExpansionType expansion_type;
	std::deque<MineableField*> mineable_fields;
	std::deque<Widelands::Flag const*> new_flags;
	std::deque<Widelands::Road const*> roads;
	std::deque<EconomyObserver*> economies;
	std::deque<ProductionSiteObserver> productionsites;
	std::deque<ProductionSiteObserver> mines_;
	std::deque<MilitarySiteObserver> militarysites;
	std::deque<WarehouseSiteObserver> warehousesites;
	std::deque<TrainingSiteObserver> trainingsites;
	std::deque<ShipObserver> allships;
	std::vector<WareObserver> wares;
	// This is a vector that is filled up on initiatlization
	// and no items are added/removed afterwards
	std::vector<std::shared_ptr<SchedulerTask>> taskPool;
	std::vector<std::shared_ptr<SchedulerTask>> current_task_queue;
	std::map<uint32_t, EnemySiteObserver> enemy_sites;
	std::set<uint32_t> enemy_warehouses;
	// it will map mined material to observer
	std::map<int32_t, MineTypesObserver> mines_per_type;
	std::vector<uint32_t> spots_avail;
	MineFieldsObserver mine_fields_stat;

	// used for statistics of buildings
	uint32_t numof_psites_in_constr;
	uint32_t num_ports;
	uint16_t numof_warehouses_;
	uint16_t numof_warehouses_in_const_;
	uint32_t mines_in_constr() const;
	uint32_t mines_built() const;
	std::map<int32_t, MilitarySiteSizeObserver> msites_per_size;
	// for militarysites
	uint32_t msites_in_constr() const;
	uint32_t msites_built() const;
	Time military_last_dismantle_;
	Time military_last_build_;  // sometimes expansions just stops, this is time of last
	                            // military building built
	Time time_of_last_construction_;
	Time next_mine_construction_due_;
	uint16_t fishers_count_;
	uint16_t bakeries_count_;

	Time first_iron_mine_built;

	// for training sites per type
	int16_t ts_finished_count_;
	int16_t ts_in_const_count_;
	int16_t ts_without_trainers_;

	// for roads
	Time last_road_dismantled_;  // uses to prevent too frequent road dismantling
	bool dead_ends_check_;       // Do we need to check and dismantle dead ends?

	Time last_attack_time_;
	// check ms in this interval - will auto-adjust
	Duration enemysites_check_delay_;

	int32_t spots_;  // sum of buildable fields

	int16_t productionsites_ratio_;

	bool resource_necessity_water_needed_;  // unless atlanteans or amazons

	// This stores highest priority for new buildings except for militarysites
	int32_t highest_nonmil_prio_;

	// if the basic economy is not established, there must be a non-empty list of remaining basic
	// buildings
	bool basic_economy_established;

	// id of iron as resource to identify iron mines in mines_per_type map
	int32_t iron_resource_id = Widelands::INVALID_INDEX;

	// Record with immovable attribute is created or collected by which building names and attributes
	std::map<Widelands::MapObjectDescr::AttributeIndex, std::set<ImmovableAttribute>>
	   buildings_immovable_attributes_;

	// this is a bunch of patterns that have to identify weapons and armors for input queues of
	// trainingsites
	// TODO(GunChleoc): Get rid of this hard-coding
	std::vector<std::string> const armors_and_weapons = {
	   "ax",      "armor",  "boots",  "garment", "helm",  "padded", "sword",
	   "trident", "tabard", "shield", "mask",    "spear", "warrior"};

	// seafaring related
	enum { kReprioritize, kStopShipyard, kStartShipyard };
	static Time last_seafaring_check_;
	// False by default, until Map::allows_seafaring() is true
	static bool map_allows_seafaring_;
	bool potential_wrong_shipyard_ = false;
	uint32_t expedition_ship_;
	Duration expedition_max_duration;
	std::vector<int16_t> marine_task_queue;
	std::unordered_set<uint32_t> expedition_visited_spots;

	std::vector<std::vector<int16_t>> AI_military_matrix;
	std::vector<int16_t> AI_military_numbers;

	uint16_t buil_material_mines_count = 0;

	bool ai_training_mode_ = false;

	// Notification subscribers
	std::unique_ptr<Notifications::Subscriber<Widelands::NoteFieldPossession>>
	   field_possession_subscriber_;
	std::unique_ptr<Notifications::Subscriber<Widelands::NoteImmovable>> immovable_subscriber_;
	std::unique_ptr<Notifications::Subscriber<Widelands::NoteProductionSiteOutOfResources>>
	   outofresource_subscriber_;
	std::unique_ptr<Notifications::Subscriber<Widelands::NoteTrainingSiteSoldierTrained>>
	   soldiertrained_subscriber_;
	std::unique_ptr<Notifications::Subscriber<Widelands::NoteShip>> shipnotes_subscriber_;
};

}  // namespace AI
#endif  // end of include guard: WL_AI_DEFAULTAI_H
