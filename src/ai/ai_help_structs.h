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
#include <unordered_set>

#include "ai/ai_hints.h"
#include "economy/flag.h"
#include "economy/road.h"
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

enum class ExpansionMode : uint8_t { kResources = 0, kSpace = 1, kEconomy = 2, kBoth = 3};
//enum class ScoreBlock : uint8_t {kResource = 0, kLand = 1, kEnemy = 2, kDismantle = 3};

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

const std::vector<std::vector<int8_t>> neuron_curves = {
   {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100},
   {0, 0, 1, 2, 4, 6, 9, 12, 16, 20, 25, 30, 36, 42, 49, 56, 64, 72, 81, 90, 100},
   {0, 17, 25, 32, 38, 44, 49, 53, 58, 62, 66, 70, 74, 78, 81, 84, 88, 91, 94, 97, 100},
};

constexpr int magic_numbers_size = 150;
constexpr int neuron_pool_size = 80;
constexpr int f_neuron_pool_size = 60;
constexpr int f_neuron_bit_size = 32;
constexpr int MutationRatePosition = 42;

constexpr uint32_t kNever = std::numeric_limits<uint32_t>::max();

struct CheckStepRoadAI {
	CheckStepRoadAI(Player* const pl, uint8_t const mc, bool const oe);

	bool allowed(Map&, FCoords start, FCoords end, int32_t dir, CheckStep::StepId) const;
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
	FindNodeUnownedMineable(Player* p, Game& g, const int32_t t = INVALID_INDEX);

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
	FindNodeWater(const World& world);

	bool accept(const Map& /* map */, const FCoords& coord) const;

private:
	const World& world_;
};

// This is to be used for shipyards to make sure the water is wide enough
// Open water is a field where all 6 adjacent triangles are water
struct FindNodeOpenWater {
	// 'world' is unused, but we need to fit the template.
	FindNodeOpenWater(const World& /* world */) {
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
			return (a.cost - a.distance) > (b.cost - b.distance);
		}
	};

	NearFlag(const Flag& f, int32_t const c, int32_t const d);

	bool operator<(const NearFlag& f) const {
		return cost > f.cost;
	}

	bool operator==(Flag const* const f) const {
		return flag == f;
	}

	Flag const* flag;
	int32_t cost;
	int32_t distance;
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
	BuildableField(const Widelands::FCoords& fc);

	int32_t own_military_sites_nearby_();

	Widelands::FCoords coords;

	uint32_t field_info_expiration;

	bool preferred;
	bool enemy_nearby;
	bool enemy_accessible_;

	uint16_t unowned_land_nearby;
	uint16_t enemy_owned_land_nearby;
	uint16_t unowned_buildable_spots_nearby;
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
	int16_t enemy_military_sites;	//Including unfinished
	int16_t ally_military_presence;
	// stationed (manned) military buildings nearby
	int16_t military_stationed;
	// stationed (manned) military buildings nearby
	// unconnected buildings nearby
	bool unconnected_nearby;
	int16_t military_unstationed;
	int16_t own_non_military_nearby;
	Widelands::ExtendedBool is_portspace;
	bool port_nearby;  // to increase priority if a port is nearby,
	// especially for new colonies
	Widelands::ExtendedBool portspace_nearby;  // prefer military buildings closer to the portspace
	int32_t max_buildcap_nearby;
	// it is not necessary to check resources (stones, fish...) too frequently as they do not change
	// fast
	// this stores time of last check
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
	MineableField(const Widelands::FCoords& fc);

	Widelands::FCoords coords;
	uint32_t field_info_expiration;
	bool preferred;
	int32_t mines_nearby;
	// this is to provide that a mine is not built on the edge of mine area
	int32_t same_mine_fields_nearby;
};

struct EconomyObserver {
	EconomyObserver(Widelands::Economy& e);

	Widelands::Economy& economy;
	std::list<Widelands::Flag const*> flags;
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

	char const* name;
	Widelands::DescriptionIndex id;
	Widelands::BuildingDescr const* desc;

	Type type;

	bool plants_trees;
	bool recruitment;  // is "producing" workers?
	Widelands::BuildingNecessity new_building;
	uint32_t new_building_overdue;
	int32_t primary_priority;
	bool is_buildable;
	bool need_trees;   // lumberjack = true
	bool need_rocks;   // quarry = true
	bool mines_water;  // wells
	bool need_water;   // fisher, fish_breeder = true
	bool is_hunter;    // need to identify hunters
	bool is_fisher;    // need to identify fishers
	bool is_port;
	bool is_shipyard;
	bool is_barracks;
	bool space_consumer;       // farm, vineyard... = true
	bool expansion_type;       // military building used that can be used to control area
	bool fighting_type;        // military building built near enemies
	bool mountain_conqueror;   // military building built near mountains
	uint32_t prohibited_till;  // do not build before (ms)
	uint32_t forced_after;     // do not wait until ware is needed
	uint8_t max_ts_proportion; 

	uint16_t unconnected_count;  // to any warehouse (count of such buildings)

	DescriptionIndex mines;  // type of resource it mines_
	uint16_t mines_percent;  // % of res it can mine
	uint32_t current_stats;

	std::vector<Widelands::DescriptionIndex> inputs;
	std::vector<Widelands::DescriptionIndex> outputs;
	std::vector<Widelands::DescriptionIndex> critical_building_material;

	bool produces_building_material;

	// an enhancement to this building:
	// produces all wares as current building, and perhaps more
	bool upgrade_substitutes;
	// produces some additional wares
	bool upgrade_extends;

	// It seems that fish and meat are subsitutes (for trainingsites), so
	// when testing if a trainingsite is supplied enough
	// we count the wares together
	std::unordered_set<Widelands::DescriptionIndex> substitute_inputs;
	int32_t substitutes_count;

	int16_t production_hint;

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
};

struct ProductionSiteObserver {
	Widelands::ProductionSite* site;
	uint32_t built_time;
	uint32_t unoccupied_till;
	uint8_t stats_zero;
	uint32_t no_resources_since;
	bool upgrade_pending;
	BuildingObserver* bo;
};

struct MilitarySiteObserver {
	Widelands::MilitarySite* site;
	BuildingObserver* bo;
	uint8_t checks;
	// when considering attack most military sites are inside territory and should be skipped during
	// evaluation
	bool enemies_nearby;
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
	EnemySiteObserver();

	bool is_warehouse;
	int32_t attack_soldiers_strength;
	int32_t attack_soldiers_competency;
	int32_t defenders_strength;
	uint8_t stationed_soldiers;
	uint32_t last_time_attackable;
	uint32_t last_tested;
	int16_t score;
	Widelands::ExtendedBool mines_nearby;
	int16_t no_attack_counter;
};

// as all mines have 3 levels, AI does not know total count of mines per mined material
// so this observer will be used for this
struct MineTypesObserver {
	MineTypesObserver();

	uint16_t total_count() const;

	uint16_t in_construction;
	uint16_t finished;
};

struct Neuron {
	Neuron(int8_t, uint8_t, uint16_t);
	void recalculate();
	void set_weight(int8_t w);
	int8_t get_weight() {
		return weight;
	};
	int8_t get_result(uint8_t);
	int8_t get_result_safe(int32_t, bool = false);
	void set_type(uint8_t);
	uint8_t get_type() {
		return type;
	};
	uint16_t get_id() {
		return id;
	};
	int32_t get_lowest_pos() {
		return lowest_pos;
	};
	int32_t get_highest_pos() {
		return highest_pos;
	};

private:
	int8_t results[21];
	int8_t weight;
	uint8_t type;
	uint16_t id;
	int32_t lowest_pos;
	int32_t highest_pos;
};

struct FNeuron {
	FNeuron(uint32_t);

	void flip_bit(uint8_t);
	// void set(uint8_t);
	bool get_result(bool, bool, bool, bool bool4 = true, bool bool5 = true);
	bool get_position(uint8_t);
	uint32_t get_int();

private:
	std::bitset<f_neuron_bit_size> core;
};

struct ExpansionType { //NOCOM
	ExpansionType();
	
	void set_expantion_type(ExpansionMode);
	//int16_t get_proportions(ScoreBlock, bool = false);
	ExpansionMode get_expansion_type() {return type;};
	
	private:
	ExpansionMode type;
	
	//// columns ScoreBlock: kResource = 0, kLand = 1, kEnemy = 2, kDismantle = 3
	//uint8_t proportion_table[5][4] = {
		//{ 70, 10, 40,  0}, //kResources
		//{ 10, 70, 80,  0}, //kSpace
		//{ 35, 35, 50,  0}, //kEconomy
		//{ 40, 40, 65,  0},  //kBoth	
		//{ 20, 20, 65, 60},  //kDismantle					
	//};
		
};

// This is to keep all data related to AI magic numbers
struct ManagementData {
	ManagementData();

	std::vector<Neuron> neuron_pool;
	std::vector<FNeuron> f_neuron_pool;
	Widelands::Player::AiPersistentState* pd;

	void mutate(uint32_t, PlayerNumber = 0);
	void review(uint32_t, PlayerNumber, int32_t, uint32_t, uint32_t, uint16_t, uint32_t, int16_t, uint8_t);
	void dump_data();
	void initialize(uint8_t, bool reinitializing = false);
	uint16_t new_neuron_id() {
		next_neuron_id += 1;
		return next_neuron_id - 1;
	};
	void reset_neuron_id() {
		next_neuron_id = 0;
	}
	uint16_t new_bi_neuron_id() {
		next_bi_neuron_id += 1;
		return next_bi_neuron_id - 1;
	};
	void reset_bi_neuron_id() {
		next_bi_neuron_id = 0;
	}
	int16_t get_military_number_at(uint8_t);
	void set_military_number_at(uint8_t, int16_t);
	int8_t shift_weight_value(int8_t, bool = true);
	bool test_consistency();

private:
	int32_t score;
	uint8_t primary_parent;
	uint32_t last_mutate_time;
	uint16_t review_count;
	uint16_t next_neuron_id;
	uint16_t next_bi_neuron_id;
	uint16_t performance_change;
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

	bool operator<(SchedulerTask other) const;

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

	FlagsForRoads(int32_t mr) : min_reduction(mr) {
	}

	struct Candidate {
		Candidate();
		Candidate(uint32_t coords, int32_t distance, bool economy);

		uint32_t coords_hash;
		int32_t new_road_length;
		int32_t current_roads_distance;
		int32_t air_distance;
		int32_t reduction_score;
		bool different_economy;
		bool new_road_possible;
		bool accessed_via_roads;

		bool operator<(const Candidate& other) const;
		bool operator==(const Candidate& other) const;
		void calculate_score();
	};

	int32_t min_reduction;
	// This is the core of this object - candidate flags ordered by score
	std::set<Candidate> queue;

	void add_flag(Widelands::Coords coords, int32_t air_dist, bool diff_economy) {
		queue.insert(Candidate(coords.hash(), air_dist, diff_economy));
	}

	uint32_t count() {
		return queue.size();
	}

	// This is for debugging and development purposes
	void print();
	// during processing we need to pick first one uprocessed flag (with best score so far)
	bool get_best_uncalculated(uint32_t* winner);
	// When we test candidate flag if road can be built to it, there are two possible outcomes:
	void road_possible(Widelands::Coords coords, uint32_t distance);
	void road_impossible(Widelands::Coords coords);
	// Updating walking distance over existing roads
	void set_road_distance(Widelands::Coords coords, int32_t distance);
	// Finally we query the flag that we will build a road to
	bool get_winner(uint32_t* winner_hash);
};

// This is a struct that stores strength of players, info on teams and provides some outputs from
// these data
struct PlayersStrengths {
private:
	struct PlayerStat {
		PlayerStat();
		PlayerStat(Widelands::TeamNumber tc,
		           bool en,
		           uint32_t pp,
		           uint32_t op,
		           uint32_t o60p,
		           uint32_t cs,
		           uint32_t land,
		           uint32_t oland,
		           uint32_t o60l);

		Widelands::TeamNumber team_number;
		bool is_enemy;
		uint32_t players_power;
		uint32_t old_players_power;
		uint32_t old60_players_power;
		uint32_t players_casualities;
		uint32_t last_time_seen;
		uint32_t players_land;
		uint32_t old_players_land;
		uint32_t old60_players_land;
	};

public:
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
	uint32_t get_player_casualities(Widelands::PlayerNumber pn);
	bool players_in_same_team(Widelands::PlayerNumber pl1, Widelands::PlayerNumber pl2);
	bool strong_enough(Widelands::PlayerNumber pl);
	void set_last_time_seen(uint32_t, Widelands::PlayerNumber);
	bool player_seen_lately(Widelands::PlayerNumber, uint32_t);
	bool get_is_enemy(Widelands::PlayerNumber);
	uint8_t enemies_seen_lately_count(uint32_t);
	bool any_enemy_seen_lately(uint32_t);
	uint32_t enemy_last_seen();
	PlayerNumber this_player_number;

private:
	// This is the core part of this struct
	std::map<Widelands::PlayerNumber, PlayerStat> all_stats;

	// Number of team, sum of players' strength
	std::map<Widelands::TeamNumber, uint32_t> team_powers;
};
}  // namespace Widelands

#endif  // end of include guard: WL_AI_AI_HELP_STRUCTS_H
