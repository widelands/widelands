/*
 * Copyright (C) 2009-2017 by the Widelands Development Team
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

#ifndef WL_AI_AI_HELP_STRUCTS_H
#define WL_AI_AI_HELP_STRUCTS_H

#include <list>
#include <queue>
#include <unordered_set>

#include "ai/ai_hints.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "logic/ai_dna_handler.h"
#include "logic/findnode.h"
#include "logic/game.h"
#include "logic/map.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/map_object.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/world/terrain_description.h"
#include "logic/map_objects/world/world.h"
#include "logic/player.h"

namespace Widelands {

class ProductionSite;
class MilitarySite;

enum class ExtendedBool : uint8_t { kUnset, kTrue, kFalse };

enum class MutatingIntensity : uint8_t { kNo, kNormal, kAgressive };

enum class BuildingNecessity : uint8_t {
	kForced,
	kNeeded,
	kNotNeeded,
	kUnset,
	kNotBuildable,
	kAllowed,
	kNeededPending,
	kForbidden
};

// A building type can have no, one or multiple of these attributes
enum class BuildingAttribute : uint8_t {
	kRanger,
	kBuildable,
	kLumberjack,
	kPort,
	kNeedsRocks,
	kWell,
	kNeedsCoast,
	kHunter,
	kFisher,
	kShipyard,
	kBarracks,
	kSpaceConsumer,
	kRecruitment,
	kBuildingMatProducer,
	kUpgradeSubstitutes,
	kUpgradeExtends,
	kLogRefiner,
	kIronMine,
	kNeedsSeafaring,
	kSupportingProducer,
};

enum class AiType : uint8_t { kVeryWeak, kWeak, kNormal };

enum class ExpansionMode : uint8_t { kResources = 0, kSpace = 1, kEconomy = 2, kBoth = 3 };

enum class AiModeBuildings : uint8_t { kAnotherAllowed, kOnLimit, kLimitExceeded };

enum class SchedulerTaskId : uint8_t {
	kBbuildableFieldsCheck,
	kMineableFieldsCheck,
	kRoadCheck,
	kUnbuildableFCheck,
	kCheckEconomies,
	kProductionsitesStats,
	kConstructBuilding,
	kCheckProductionsites,
	kCheckShips,
	KMarineDecisions,
	kCheckMines,
	kWareReview,
	kPrintStats,
	kCheckMilitarysites,
	kCheckTrainingsites,
	kCountMilitaryVacant,
	kCheckEnemySites,
	kManagementUpdate,
	kUpdateStats,
	kUnset
};

enum class DnaParent : uint8_t { kPrimary, kSecondary };

// This is a simplification of a curve, to avoid repeated calculation
const std::vector<std::vector<int8_t>> neuron_curves = {
   {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100},
   {0, 0, 1, 2, 4, 6, 9, 12, 16, 20, 25, 30, 36, 42, 49, 56, 64, 72, 81, 90, 100},
   {0, 17, 25, 32, 38, 44, 49, 53, 58, 62, 66, 70, 74, 78, 81, 84, 88, 91, 94, 97, 100},
};

// TODO(tiborb): this should be replaced by command line switch
constexpr int kFNeuronBitSize = 32;
constexpr int kMutationRatePosition = 42;

constexpr uint32_t kNever = std::numeric_limits<uint32_t>::max();

struct CheckStepRoadAI {
	CheckStepRoadAI(Player* const pl, uint8_t const mc, bool const oe);

	bool allowed(const Map&, FCoords start, FCoords end, int32_t dir, CheckStep::StepId) const;
	bool reachable_dest(const Map&, const FCoords& dest) const;

	Player* player;
	uint8_t movecaps;
	bool open_end;
};

// We are looking for fields we can walk on
// and owned by hostile player.
struct FindNodeEnemy {
	FindNodeEnemy(Player* p, Game& g);

	bool accept(const Map&, const FCoords& fc) const;

	Player* player;
	Game& game;
};

// We are looking for buildings owned by hostile player
// (sometimes there is a enemy's teritorry without buildings, and
// this confuses the AI)

struct FindNodeEnemiesBuilding {
	FindNodeEnemiesBuilding(Player* p, Game& g);

	bool accept(const Map&, const FCoords& fc) const;

	Player* player;
	Game& game;
};

// We need to count walkable fields owned by enemy
struct FindEnemyNodeWalkable {
	FindEnemyNodeWalkable(Player* p, Game& g);

	bool accept(const Map&, const FCoords& fc) const;

	Player* player;
	Game& game;
};

// Sometimes we need to know how many nodes our allies owns
struct FindNodeAllyOwned {
	FindNodeAllyOwned(Player* p, Game& g, PlayerNumber n);

	bool accept(const Map&, const FCoords& fc) const;

	Player* player;
	Game& game;
	PlayerNumber player_number;
};

// When looking for unowned terrain to acquire, we must
// pay speciall attention to fields where mines can be built.
// Fields should be completely unowned
struct FindNodeUnownedMineable {
	FindNodeUnownedMineable(Player* p, Game& g, int32_t t = INVALID_INDEX);

	bool accept(const Map&, const FCoords& fc) const;

	Player* player;
	Game& game;
	int32_t ore_type;
};

// When looking for unowned terrain to acquire, we must
// consider if any buildings can be built on unowned land.
struct FindNodeUnownedBuildable {
	FindNodeUnownedBuildable(Player* p, Game& g);

	bool accept(const Map&, const FCoords& fc) const;

	Player* player;
	Game& game;
};

// Unowned but walkable fields nearby
struct FindNodeUnownedWalkable {
	FindNodeUnownedWalkable(Player* p, Game& g);

	bool accept(const Map&, const FCoords& fc) const;

	Player* player;
	Game& game;
};

// Looking only for mines-capable fields nearby
// of specific type
struct FindNodeMineable {
	FindNodeMineable(Game& g, DescriptionIndex r);

	bool accept(const Map&, const FCoords& fc) const;

	Game& game;
	int32_t res;
};

// Fishers and fishbreeders must be built near water
struct FindNodeWater {
	explicit FindNodeWater(const World& world);

	bool accept(const Map& /* map */, const FCoords& coord) const;

private:
	const World& world_;
};

// This is to be used for shipyards to make sure the water is wide enough
// Open water is a field where all 6 adjacent triangles are water
struct FindNodeOpenWater {
	// 'world' is unused, but we need to fit the template.
	explicit FindNodeOpenWater(const World& /* world */) {
	}

	bool accept(const Map& /* map */, const FCoords& coord) const;
};

struct FindNodeWithFlagOrRoad {
	bool accept(const Map&, FCoords) const;
};

struct NearFlag {
	// ordering nearflags by biggest reduction
	struct CompareShortening {
		bool operator()(const NearFlag& a, const NearFlag& b) const {
			return a.current_road_distance > b.current_road_distance;
		}
	};
	NearFlag();
	NearFlag(const Flag* f, int32_t const c);

	bool operator<(const NearFlag& f) const {
		return current_road_distance > f.current_road_distance;
	}

	bool operator==(Flag const* const f) const {
		return flag == f;
	}

	Flag const* flag;
	bool to_be_checked;
	uint32_t current_road_distance;
};

// FIFO like structure for pairs <gametime,id>, where id is optional
// used to count events within a time frame - duration_ (older ones are
// stripped with strip_old function)
struct EventTimeQueue {
	EventTimeQueue();

	void push(uint32_t, uint32_t = std::numeric_limits<uint32_t>::max());
	uint32_t count(uint32_t, uint32_t = std::numeric_limits<uint32_t>::max());
	void strip_old(uint32_t);

private:
	const uint32_t duration_ = 20 * 60 * 1000;
	// FIFO container where newest goes to the front
	std::deque<std::pair<uint32_t, uint32_t>> queue;
};

struct WalkableSpot {
	Coords coords;
	bool has_flag;

	int32_t cost;
	void* eco;

	int16_t from;
	int16_t neighbours[6];
};

struct BuildableField {
	explicit BuildableField(const Widelands::FCoords& fc);

	Widelands::FCoords coords;

	uint32_t field_info_expiration;

	bool preferred;
	bool enemy_nearby;
	bool enemy_accessible_;

	bool enemy_wh_nearby;
	uint16_t unowned_land_nearby;
	uint16_t enemy_owned_land_nearby;
	uint16_t unowned_buildable_spots_nearby;
	uint16_t unowned_portspace_vicinity_nearby;
	uint16_t nearest_buildable_spot_nearby;
	// to identify that field is too close to border and no production building should be built there
	bool near_border;
	uint16_t unowned_mines_spots_nearby;
	uint16_t unowned_iron_mines_nearby;
	uint8_t trees_nearby;
	uint8_t rocks_nearby;
	int16_t water_nearby;
	int16_t open_water_nearby;
	int16_t distant_water;
	int8_t fish_nearby;
	int8_t critters_nearby;
	ResourceAmount ground_water;  // used by wells
	uint8_t space_consumers_nearby;
	uint8_t rangers_nearby;
	// to manage the military better following variables exists:
	// capacity of nearby buildings:
	int16_t area_military_capacity;
	// distance to near buldings:
	int16_t military_loneliness;
	// count of military buildings in construction
	// when making decision on new mulitary buildings it considers also
	// unowned fields and mines, but this information is not quite correct as there
	// are construction sites that will change this once they are built
	int16_t military_in_constr_nearby;
	// actual count of soldiers in nearby buldings
	int16_t own_military_presence;
	int16_t enemy_military_presence;
	int16_t enemy_military_sites;  // Including unfinished
	int16_t ally_military_presence;
	// stationed (manned) military buildings nearby
	int16_t military_stationed;
	// unconnected buildings nearby
	bool unconnected_nearby;
	int16_t military_unstationed;
	int16_t own_non_military_nearby;
	bool defense_msite_allowed;
	Widelands::ExtendedBool is_portspace;
	bool port_nearby;  // to increase priority if a port is nearby,
	// especially for new colonies
	Widelands::ExtendedBool portspace_nearby;  // special fields intended for ports
	int32_t max_buildcap_nearby;
	// It is not necessary to check resources (stones, fish...) too frequently as they do not change
	// fast. This stores the time of the last check.
	uint32_t last_resources_check_time;
	int32_t military_score_;
	bool inland;
	uint16_t local_soldier_capacity;
	bool is_militarysite;

	std::vector<uint8_t> consumers_nearby;
	std::vector<uint8_t> producers_nearby;
	// and for rangers, fishbreeders:
	std::vector<uint8_t> supporters_nearby;
};

struct MineableField {
	explicit MineableField(const Widelands::FCoords& fc);

	Widelands::FCoords coords;
	uint32_t field_info_expiration;
	bool preferred;
	int32_t mines_nearby;
	// this is to provide that a mine is not built on the edge of mine area
	int32_t same_mine_fields_nearby;
};

struct EconomyObserver {
	explicit EconomyObserver(Widelands::Economy& e);

	Widelands::Economy& economy;
	std::deque<Widelands::Flag const*> flags;
	int32_t dismantle_grace_time;
};

struct BuildingObserver {

	enum class Type {
		kBoring,
		kConstructionsite,
		kProductionsite,
		kMilitarysite,
		kWarehouse,
		kTrainingsite,
		kMine
	};

	int32_t total_count() const;
	Widelands::AiModeBuildings aimode_limit_status();
	bool buildable(Widelands::Player& p);

	// Convenience functions for is_what
	bool is(BuildingAttribute) const;
	void set_is(BuildingAttribute);
	void unset_is(BuildingAttribute);

	char const* name;
	Widelands::DescriptionIndex id;
	Widelands::BuildingDescr const* desc;

	Type type;

	Widelands::BuildingNecessity new_building;
	uint32_t new_building_overdue;
	int32_t primary_priority;

	bool expansion_type;       // military building used that can be used to control area
	bool fighting_type;        // military building built near enemies
	bool mountain_conqueror;   // military building built near mountains
	uint32_t prohibited_till;  // do not build before (ms)
	uint32_t forced_after;     // do not wait until ware is needed
	uint8_t max_trainingsites_proportion;

	uint16_t unconnected_count;  // to any warehouse (count of such buildings)

	DescriptionIndex mines;  // type of resource it mines_
	uint16_t mines_percent;  // % of res it can mine
	uint32_t current_stats;

	uint32_t basic_amount;  // basic amount for basic economy as defined in init.lua

	std::vector<Widelands::DescriptionIndex> inputs;
	std::vector<Widelands::DescriptionIndex> outputs;
	std::vector<Widelands::DescriptionIndex> positions;
	std::vector<Widelands::DescriptionIndex> critical_building_material;

	// It seems that fish and meat are subsitutes (for trainingsites), so
	// when testing if a trainingsite is supplied enough
	// we count the wares together
	std::unordered_set<Widelands::DescriptionIndex> substitute_inputs;
	int32_t substitutes_count;

	std::set<DescriptionIndex> production_hints;

	// information needed for decision on new building construction
	int16_t max_preciousness;
	int16_t max_needed_preciousness;

	int32_t cnt_built;
	int32_t cnt_under_construction;
	int32_t cnt_target;           // number of buildings as target
	int32_t cnt_limit_by_aimode;  // limit imposed by weak or normal AI mode

	int32_t cnt_upgrade_pending;  // number of buildings that are to be upgraded

	// used to track amount of wares produced by building
	uint32_t stocklevel_count;
	uint32_t stocklevel_time;  // time when stocklevel_ was last time recalculated
	uint32_t last_dismantle_time;
	uint32_t construction_decision_time;
	uint32_t last_building_built;

	uint32_t unoccupied_count;

	bool build_material_shortage;

private:
	std::set<BuildingAttribute> is_what;
};

struct ProductionSiteObserver {
	Widelands::ProductionSite* site = nullptr;
	uint32_t built_time = 0U;
	uint32_t unoccupied_till = 0U;
	uint32_t no_resources_since = kNever;
	bool upgrade_pending = false;
	uint32_t dismantle_pending_since = kNever;
	BuildingObserver* bo = nullptr;
};

struct MilitarySiteObserver {
	Widelands::MilitarySite* site;
	BuildingObserver* bo;
	uint16_t understaffed;
	uint32_t last_change;  // to prevent switching the occupancy policy too fast
	uint32_t built_time;
};

struct TrainingSiteObserver {
	Widelands::TrainingSite* site;
	BuildingObserver* bo;
};

struct WarehouseSiteObserver {
	Widelands::Warehouse* site;
	BuildingObserver* bo;
};

struct ShipObserver {
	Widelands::Ship* ship;
	bool waiting_for_command_ = false;
	uint32_t last_command_time = 0;
	bool escape_mode = false;

	// direction by which the ship circumvents an island
	// this is the last circle-island command's direction
	Widelands::IslandExploreDirection island_circ_direction =
	   Widelands::IslandExploreDirection::kClockwise;
};

struct WareObserver {
	uint8_t producers;
	uint8_t consumers;
	uint8_t preciousness;
};

// Computer player does not get notification messages about enemy militarysites
// and warehouses, so following is collected based on observation
// It is conventient to have some information preserved, like nearby minefields,
// when it was attacked, whether it is warehouse and so on
// Also AI test more such targets when considering attack and calculated score is
// is stored in the observer
struct EnemySiteObserver {
	bool is_warehouse = false;
	int32_t attack_soldiers_strength = 0;
	int32_t attack_soldiers_competency = 0;
	int32_t defenders_strength = 0;
	uint8_t stationed_soldiers = 0U;
	uint32_t last_time_seen = 0U;
	uint32_t last_tested = 0U;
	int16_t score = 0;
	Widelands::ExtendedBool mines_nearby = Widelands::ExtendedBool::kUnset;
	uint32_t last_time_attacked = 0U;
	uint32_t attack_counter = 0U;
};

// as all mines have 3 levels, AI does not know total count of mines per mined material
// so this observer will be used for this
struct MineTypesObserver {
	MineTypesObserver();

	uint16_t total_count() const;

	uint16_t in_construction;
	uint16_t finished;
	bool is_critical;
	uint16_t unoccupied;
};

constexpr int kNeuronWeightLimit = 100;
constexpr size_t kNeuronMaxPosition = 20;
constexpr size_t kSecondParentProbability = 50;

// A bunch of parameters used for trainig AI (for calculation of fitness function result)
constexpr int16_t kCurrentLandDivider = 2;
constexpr int16_t kLandDeltaMultiplier = 1;
constexpr int16_t kBonus = 1000;
constexpr int16_t kAttackersMultiplier = 1;
constexpr int16_t kAttackBonus = 100;
constexpr int16_t kTrainedSoldiersScore = 250;
constexpr int16_t kConqueredWhBonus = 300;
constexpr int16_t kStrengthMultiplier = 30;
constexpr int16_t kPSitesRatioMultiplier = 1;

struct Neuron {
	static int clip_weight_to_range(int w) {
		assert(w < 125);
		assert(w > -125);
		return std::max(-kNeuronWeightLimit, std::min(kNeuronWeightLimit, w));
	}

	Neuron(int8_t, uint8_t, uint16_t);
	void recalculate();
	void set_weight(int8_t w);
	int8_t get_weight() {
		return weight;
	}
	int8_t get_result(size_t);
	int8_t get_result_safe(int32_t, bool = false);
	void set_type(uint8_t);
	uint8_t get_type() {
		return type;
	}
	uint16_t get_id() {
		return id;
	}

private:
	int8_t results[21];  // kPositions + 1
	int8_t weight;
	uint8_t type;
	uint16_t id;
};

struct FNeuron {
	FNeuron(uint32_t, uint16_t);

	void flip_bit(uint8_t);
	bool get_result(bool, bool, bool, bool bool4 = true, bool bool5 = true);
	bool get_position(uint8_t);
	uint32_t get_int();
	uint16_t get_id() {
		return id;
	}

private:
	std::bitset<kFNeuronBitSize> core;
	uint16_t id;
};

struct ExpansionType {
	ExpansionType();

	void set_expantion_type(ExpansionMode);
	ExpansionMode get_expansion_type() {
		return type;
	}

private:
	ExpansionMode type;
};

// This is to keep all data related to AI magic numbers
struct ManagementData {
	void mutate(PlayerNumber = 0);
	void new_dna_for_persistent(uint8_t, Widelands::AiType);
	void copy_persistent_to_local();
	void review(uint32_t gametime,
	            PlayerNumber pn,
	            uint32_t land,
	            uint32_t max_e_land,
	            uint32_t old_land,
	            uint16_t attackers,
	            int16_t trained_soldiers,
	            int16_t latest_attackers,
	            uint16_t conq_ws,
	            uint16_t strength,
	            uint32_t existing_ps);
	void dump_data(PlayerNumber);
	uint16_t new_neuron_id() {
		++next_neuron_id;
		return next_neuron_id - 1;
	}
	void reset_neuron_id() {
		next_neuron_id = 0;
	}
	uint16_t new_bi_neuron_id() {
		++next_bi_neuron_id;
		return next_bi_neuron_id - 1;
	}
	void reset_bi_neuron_id() {
		next_bi_neuron_id = 0;
	}
	void set_ai_training_mode() {
		ai_training_mode_ = true;
	}

	int16_t get_military_number_at(uint8_t);
	void set_military_number_at(uint8_t, int16_t);
	MutatingIntensity do_mutate(uint8_t, int16_t);
	int8_t shift_weight_value(int8_t, bool = true);
	void test_consistency(bool = false);

	std::vector<Neuron> neuron_pool;
	std::vector<FNeuron> f_neuron_pool;
	Widelands::Player::AiPersistentState* persistent_data;

private:
	void dump_output(Widelands::Player::AiPersistentState, PlayerNumber);

	int32_t score = 1;
	uint8_t primary_parent = 255;
	uint16_t next_neuron_id = 0U;
	uint16_t next_bi_neuron_id = 0U;
	uint16_t performance_change = 0U;
	Widelands::AiType ai_type = Widelands::AiType::kNormal;
	bool ai_training_mode_ = false;
	uint16_t pref_number_probability = 200;
	AiDnaHandler ai_dna_handler;
};

// this is used to count militarysites by their size
struct MilitarySiteSizeObserver {
	MilitarySiteSizeObserver();

	uint16_t in_construction;
	uint16_t finished;
};

// this represents a scheduler task
struct SchedulerTask {
	SchedulerTask(const uint32_t time,
	              const Widelands::SchedulerTaskId t,
	              const uint8_t p,
	              const char* d);

	bool operator<(const SchedulerTask& other) const;

	uint32_t due_time;
	Widelands::SchedulerTaskId id;
	// used to sort jobs when AI has to perform more jobs at once
	uint8_t priority;
	// used only for debug purposes
	std::string descr;
};

// List of blocked fields with block time, with some accompanying functions
struct BlockedFields {
	void add(Coords coords, uint32_t till);
	uint32_t count();
	void remove_expired(uint32_t gametime);
	bool is_blocked(Coords coords);

private:
	// <hash of field coordinates, time till blocked>
	std::map<uint32_t, uint32_t> blocked_fields_;
};

// list of candidate flags to build roads, with some additional logic
struct FlagsForRoads {

	explicit FlagsForRoads(int32_t mr) : min_reduction(mr) {
	}

	struct Candidate {
		Candidate();
		Candidate(uint32_t coords, int32_t distance, bool different_economy);

		uint32_t coords_hash;
		int32_t new_road_length;
		int32_t current_road_length;
		int32_t air_distance;

		bool new_road_possible;

		bool operator<(const Candidate& other) const;
		bool operator==(const Candidate& other) const;
		int32_t reduction_score() const;
	};

	int32_t min_reduction;
	// This is the core of this object - candidate flags to be ordered by air_distance
	std::deque<Candidate> flags_queue;

	void add_flag(Widelands::Coords coords, int32_t air_dist, bool different_economy) {
		flags_queue.push_back(Candidate(coords.hash(), air_dist, different_economy));
	}

	uint32_t count() {
		return flags_queue.size();
	}
	bool has_candidate(uint32_t hash);

	// This is for debugging and development purposes
	void print();
	// during processing we need to pick first one uprocessed flag (with best score so far)
	bool get_best_uncalculated(uint32_t* winner);
	// When we test candidate flag if road can be built to it, there are two possible outcomes:
	void road_possible(Widelands::Coords coords, uint32_t new_road);
	// Updating walking distance over existing roads
	void set_cur_road_distance(Widelands::Coords coords, int32_t cur_distance);
	// Finally we query the flag that we will build a road to
	bool get_winner(uint32_t* winner_hash);
};

// This is a struct that stores strength of players, info on teams and provides some outputs from
// these data
struct PlayersStrengths {
private:
	struct PlayerStat {
		PlayerStat() = default;
		PlayerStat(Widelands::TeamNumber tc,
		           uint32_t pp,
		           uint32_t op,
		           uint32_t o60p,
		           uint32_t cs,
		           uint32_t land,
		           uint32_t oland,
		           uint32_t o60l);

		Widelands::TeamNumber team_number = 0U;
		uint32_t players_power = 0U;
		uint32_t old_players_power = 0U;
		uint32_t old60_players_power = 0U;
		uint32_t players_casualities = 0U;
		uint32_t last_time_seen = 0U;
		uint32_t players_land = 0U;
		uint32_t old_players_land = 0U;
		uint32_t old60_players_land = 0U;
	};

public:
	PlayersStrengths();
	// Inserting/updating data
	void add(Widelands::PlayerNumber pn,
	         Widelands::PlayerNumber opn,
	         Widelands::TeamNumber mytn,
	         Widelands::TeamNumber pltn,
	         uint32_t pp,
	         uint32_t op,
	         uint32_t o60p,
	         uint32_t cs,
	         uint32_t land,
	         uint32_t oland,
	         uint32_t o60l);
	void remove_stat(Widelands::PlayerNumber pn);
	void recalculate_team_power();

	// This is strength of player plus third of strength of other members of his team
	uint32_t get_modified_player_power(Widelands::PlayerNumber pn);
	uint32_t get_player_power(Widelands::PlayerNumber pn);
	uint32_t get_old_player_power(Widelands::PlayerNumber pn);
	uint32_t get_old60_player_power(Widelands::PlayerNumber pn);
	uint32_t get_player_land(Widelands::PlayerNumber pn);
	uint32_t get_old_player_land(Widelands::PlayerNumber pn);
	uint32_t get_old60_player_land(Widelands::PlayerNumber pn);
	uint32_t get_visible_enemies_power(uint32_t);
	uint32_t get_enemies_average_power();
	uint32_t get_enemies_average_land();
	uint32_t get_enemies_max_power();
	uint32_t get_enemies_max_land();
	uint32_t get_old_visible_enemies_power(uint32_t);
	bool players_in_same_team(Widelands::PlayerNumber pl1, Widelands::PlayerNumber pl2);
	bool strong_enough(Widelands::PlayerNumber pl);
	void set_last_time_seen(uint32_t, Widelands::PlayerNumber);
	bool player_seen_lately(Widelands::PlayerNumber, uint32_t);
	bool get_is_enemy(Widelands::PlayerNumber);
	uint8_t enemies_seen_lately_count(uint32_t);
	bool any_enemy_seen_lately(uint32_t);
	void set_update_time(uint32_t);
	uint32_t get_update_time();

private:
	// This is the core part of this struct
	std::map<Widelands::PlayerNumber, PlayerStat> all_stats;

	// Number of team, sum of players' strength
	std::map<Widelands::TeamNumber, uint32_t> team_powers;

	uint32_t update_time;
	PlayerNumber this_player_number;
	PlayerNumber this_player_team;
};
}  // namespace Widelands

#endif  // end of include guard: WL_AI_AI_HELP_STRUCTS_H
