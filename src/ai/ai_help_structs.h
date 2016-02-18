/*
 * Copyright (C) 2009-2010 by the Widelands Development Team
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

enum class ExtendedBool : uint8_t {kUnset, kTrue, kFalse};
enum class BuildingNecessity : uint8_t
	{kForced, kNeeded, kNotNeeded, kUnset, kNotBuildable, kAllowed, kNeededPending, kForbidden};
enum class AiModeBuildings : uint8_t
	{kAnotherAllowed, kOnLimit, kLimitExceeded};
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
		kUnset
	};

struct CheckStepRoadAI {
	CheckStepRoadAI(Player* const pl, uint8_t const mc, bool const oe)
		: player(pl), movecaps(mc), open_end(oe) {
	}

	bool allowed(Map&, FCoords start, FCoords end, int32_t dir, CheckStep::StepId) const;
	bool reachable_dest(Map&, FCoords dest) const;

	Player* player;
	uint8_t movecaps;
	bool open_end;
};

// We are looking for fields we can walk on
// and owned by hostile player.
struct FindNodeEnemy {
	bool accept(const Map&, const FCoords& fc) const {
		return (fc.field->nodecaps() & MOVECAPS_WALK) && fc.field->get_owned_by() != 0 &&
		       player->is_hostile(*game.get_player(fc.field->get_owned_by()));
	}

	Player* player;
	Game& game;

	FindNodeEnemy(Player* p, Game& g) : player(p), game(g) {}
};

// We are looking for buildings owned by hostile player
// (sometimes there is a enemy's teritorry without buildings, and
// this confuses the AI)

struct FindNodeEnemiesBuilding {
	bool accept(const Map&, const FCoords& fc) const {
		return (fc.field->get_immovable()) && fc.field->get_owned_by() != 0 &&
		       player->is_hostile(*game.get_player(fc.field->get_owned_by()));
	}

	Player* player;
	Game& game;

	FindNodeEnemiesBuilding(Player* p, Game& g) : player(p), game(g) {}
};

// When looking for unowned terrain to acquire, we are actually
// only interested in fields we can walk on.
// Fields should either be completely unowned or owned by an opposing player_
struct FindNodeUnowned {
	bool accept(const Map&, const FCoords& fc) const {
		return (fc.field->nodecaps() & MOVECAPS_WALK) &&
		       ((fc.field->get_owned_by() == 0) ||
				  player->is_hostile(*game.get_player(fc.field->get_owned_by()))) &&
				 (!only_enemies || (fc.field->get_owned_by() != 0));
	}

	Player* player;
	Game& game;
	bool only_enemies;

	FindNodeUnowned(Player* p, Game& g, bool oe = false) : player(p), game(g), only_enemies(oe) {}
};

// Sometimes we need to know how many nodes our allies owns
struct FindNodeAllyOwned {
	bool accept(const Map&, const FCoords& fc) const {
		return (fc.field->nodecaps() & MOVECAPS_WALK) && (fc.field->get_owned_by() != 0) &&
				 (fc.field->get_owned_by() != player_number) &&
				 !player->is_hostile(*game.get_player(fc.field->get_owned_by()));
	}

	Player* player;
	Game& game;
	PlayerNumber player_number;

	FindNodeAllyOwned(Player* p, Game& g, PlayerNumber n) : player(p), game(g), player_number(n) {}
};

// When looking for unowned terrain to acquire, we must
// pay speciall attention to fields where mines can be built.
// Fields should be completely unowned
struct FindNodeUnownedMineable {
	bool accept(const Map&, const FCoords& fc) const {
		return (fc.field->nodecaps() & BUILDCAPS_MINE) && (fc.field->get_owned_by() == 0);
	}

	Player* player;
	Game& game;

	FindNodeUnownedMineable(Player* p, Game& g) : player(p), game(g) {}
};

// Unowned but walkable fields nearby
struct FindNodeUnownedWalkable {
	bool accept(const Map&, const FCoords& fc) const {
		return (fc.field->nodecaps() & MOVECAPS_WALK) && (fc.field->get_owned_by() == 0);
	}

	Player* player;
	Game& game;

	FindNodeUnownedWalkable(Player* p, Game& g) : player(p), game(g) {}
};

// Looking only for mines-capable fields nearby
// of specific type
struct FindNodeMineable {
	bool accept(const Map&, const FCoords& fc) const {

		return (fc.field->nodecaps() & BUILDCAPS_MINE) && (fc.field->get_resources() == res);
	}

	Game& game;
	int32_t res;

	FindNodeMineable(Game& g, int32_t r) : game(g), res(r) {
	}
};

// Fishers and fishbreeders must be built near water
struct FindNodeWater {
	FindNodeWater(const World& world) : world_(world) {
	}

	bool accept(const Map& /* map */, const FCoords& coord) const {
		return (world_.terrain_descr(coord.field->terrain_d()).get_is() &
		        TerrainDescription::Is::kWater) ||
		       (world_.terrain_descr(coord.field->terrain_r()).get_is() &
		        TerrainDescription::Is::kWater);
	}

private:
	const World& world_;
};

struct FindNodeWithFlagOrRoad {
	bool accept(const Map&, FCoords) const;
};

struct NearFlag {
	Flag const* flag;
	int32_t cost;
	int32_t distance;

	NearFlag(const Flag& f, int32_t const c, int32_t const d) : flag(&f), cost(c), distance(d) {
	}

	bool operator<(const NearFlag& f) const {
		return cost > f.cost;
	}

	bool operator == (Flag const* const f) const {
		return flag == f;
	}
};

struct CompareDistance {
	bool operator()(const NearFlag& a, const NearFlag& b) const {
		return a.distance < b.distance;
	}
};

// ordering nearflags by biggest reduction
struct CompareShortening {
	bool operator()(const NearFlag& a, const NearFlag& b) const {
		return (a.cost - a.distance) > (b.cost - b.distance);
	}
};

struct WalkableSpot {
	Coords coords;
	bool has_flag;

	int32_t cost;
	void* eco;

	int16_t from;
	int16_t neighbours[6];
};
}

struct BuildableField {
	Widelands::FCoords coords;

	uint32_t field_info_expiration;

	bool preferred;
	bool enemy_nearby;

	uint8_t unowned_land_nearby;
	// to identify that field is too close to border and no production building should be built there
	bool near_border;
	uint8_t unowned_mines_spots_nearby;
	uint8_t trees_nearby;
	uint8_t rocks_nearby;
	int16_t water_nearby;
	int16_t distant_water;
	int8_t fish_nearby;
	int8_t critters_nearby;
	int8_t ground_water;  // used by wells
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
	int16_t area_military_presence;
	// stationed (manned) military buildings nearby
	int16_t military_stationed;
	// stationed (manned) military buildings nearby
	int16_t military_unstationed;
	bool is_portspace;
	bool port_nearby;  // to increase priority if a port is nearby,
	// especially for new colonies
	Widelands::ExtendedBool portspace_nearby;  // prefer military buildings closer to the portspace
	int32_t max_buildcap_nearby;
	// it is not necessary to check resources (stones, fish...) too frequently as they do not change fast
	// this stores time of last check
	uint32_t last_resources_check_time;

	std::vector<uint8_t> consumers_nearby;
	std::vector<uint8_t> producers_nearby;
	// and for rangers, fishbreeders:
	std::vector<uint8_t> supporters_nearby;

	BuildableField(const Widelands::FCoords& fc)
	   : coords(fc),
		  field_info_expiration(20000),
		  preferred(false),
		  enemy_nearby(0),
		  unowned_land_nearby(0),
		  near_border(false),
		  unowned_mines_spots_nearby(0),
		  trees_nearby(0),
	     // explanation of starting values
	     // this is done to save some work for AI (CPU utilization)
	     // base rules are:
		  // count of rocks can only decrease, so  amount of rocks
	     // is recalculated only when previous count is positive
	     // count of water fields are stable, so if the current count is
	     // non-negative, water is not recaldulated
		  rocks_nearby(1),
		  water_nearby(-1),
		  distant_water(0),
		  fish_nearby(-1),
		  critters_nearby(-1),
		  ground_water(1),
		  space_consumers_nearby(0),
		  rangers_nearby(0),
		  area_military_capacity(0),
		  military_loneliness(1000),
		  military_in_constr_nearby(0),
		  area_military_presence(0),
		  military_stationed(0),
		  military_unstationed(0),
		  is_portspace(false),
		  port_nearby(false),
		  portspace_nearby(Widelands::ExtendedBool::kUnset),
		  max_buildcap_nearby(0),
		  last_resources_check_time(0) {
	}

	int32_t own_military_sites_nearby_() {
		return military_stationed + military_unstationed;
	}
};

struct MineableField {
	Widelands::FCoords coords;
	uint32_t field_info_expiration;
	bool preferred;
	int32_t mines_nearby;
	// this is to provide that a mine is not built on the edge of mine area
	int32_t same_mine_fields_nearby;

	MineableField(const Widelands::FCoords& fc)
	   : coords(fc),
		  field_info_expiration(20000),
		  preferred(false),
		  mines_nearby(0),
		  same_mine_fields_nearby(0) {
	}
};

struct EconomyObserver {
	Widelands::Economy& economy;
	std::list<Widelands::Flag const*> flags;
	int32_t dismantle_grace_time;

	EconomyObserver(Widelands::Economy& e) : economy(e) {
		dismantle_grace_time = std::numeric_limits<int32_t>::max();
	}
};

struct BuildingObserver {
	char const* name;
	Widelands::DescriptionIndex id;
	Widelands::BuildingDescr const* desc;

	enum class Type {
		kBoring,
		kConstructionsite,
		kProductionsite,
		kMilitarysite,
		kWarehouse,
		kTrainingsite,
		kMine
	} type;

	bool plants_trees;
	bool recruitment;  // is "producing" workers?
	Widelands::BuildingNecessity new_building;
	uint32_t new_building_overdue;
	int32_t primary_priority;
	bool is_buildable;
	bool need_trees;   // lumberjack = true
	bool need_rocks;  // quarry = true
	bool mines_water;  // wells
	bool need_water;   // fisher, fish_breeder = true
	bool is_hunter;    // need to identify hunters
	bool is_fisher;    // need to identify fishers
	bool is_port;
	bool is_shipyard;
	bool space_consumer;       // farm, vineyard... = true
	bool expansion_type;       // military building used that can be used to control area
	bool fighting_type;        // military building built near enemies
	bool mountain_conqueror;   // military building built near mountains
	uint32_t prohibited_till;  // do not build before (ms)
	uint32_t forced_after;     // do not wait until ware is needed
	TrainingSiteType trainingsite_type;

	uint16_t unconnected_count;  // to any warehouse (count of such buildings)

	int32_t mines;           // type of resource it mines_
	uint16_t mines_percent;  // % of res it can mine
	uint32_t current_stats;

	std::vector<int16_t> inputs;
	std::vector<int16_t> outputs;
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
	int32_t cnt_target;  // number of buildings as target
	int32_t cnt_limit_by_aimode; // limit imposed by weak or normal AI mode

	// used to track amount of wares produced by building
	uint32_t stocklevel;
	uint32_t stocklevel_time;  // time when stocklevel_ was last time recalculated
	uint32_t last_dismantle_time;
	uint32_t construction_decision_time;
	uint32_t last_building_built;

	uint32_t unoccupied_count;

	bool build_material_shortage;

	int32_t total_count() const {
		return cnt_built + cnt_under_construction;
	}
	Widelands::AiModeBuildings aimode_limit_status() {
		if (total_count() > cnt_limit_by_aimode) {
			return Widelands::AiModeBuildings::kLimitExceeded;
		} else if (total_count() == cnt_limit_by_aimode) {
			return Widelands::AiModeBuildings::kOnLimit;
		} else {
			return Widelands::AiModeBuildings::kAnotherAllowed;
		}
	}
	bool buildable(Widelands::Player& p) {
		return is_buildable && p.is_building_type_allowed(id);
	}
};

struct ProductionSiteObserver {
	Widelands::ProductionSite* site;
	uint32_t built_time;
	uint32_t unoccupied_till;
	uint8_t stats_zero;
	uint32_t no_resources_since;
	BuildingObserver* bo;
};

struct MilitarySiteObserver {
	Widelands::MilitarySite* site;
	BuildingObserver* bo;
	uint8_t checks;
	// when considering attack most military sites are inside territory and should be skipped during
	// evaluation
	bool enemies_nearby;
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
	Widelands::Coords expedition_start_point;
	std::unordered_set<uint32_t> visited_spots;

	// a ship circumvents all islands in the same direction, the value
	// is assigned only once
	Widelands::IslandExploreDirection island_circ_direction = Widelands::IslandExploreDirection::kClockwise;
	bool waiting_for_command_ = false;
	uint32_t last_command_time = 0;
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
	bool is_warehouse;
	int32_t attack_soldiers_strength;
	int32_t defenders_strength;
	uint8_t stationed_soldiers;
	uint32_t last_time_attackable;
	uint32_t last_tested;
	int16_t score;
	Widelands::ExtendedBool mines_nearby;
	int16_t no_attack_counter;

	EnemySiteObserver()
		: is_warehouse(false),
	     attack_soldiers_strength(0),
	     defenders_strength(0),
	     stationed_soldiers(0),
	     last_time_attackable(std::numeric_limits<uint32_t>::max()),
	     last_tested(0),
	     score(0),
	     mines_nearby(Widelands::ExtendedBool::kUnset),
	     no_attack_counter(0) {
	}
};

// as all mines have 3 levels, AI does not know total count of mines per mined material
// so this observer will be used for this
struct MineTypesObserver {
	uint16_t in_construction;
	uint16_t finished;

	uint16_t total_count() const {
		return in_construction + finished;
	}

	MineTypesObserver() : in_construction(0), finished(0) {
	}
};

// this is used to count militarysites by their size
struct MilitarySiteSizeObserver {
	uint16_t in_construction;
	uint16_t finished;

	MilitarySiteSizeObserver() : in_construction(0), finished(0) {
	}
};

// this represents a scheduler task
struct SchedulerTask {
	uint32_t due_time;
	Widelands::SchedulerTaskId id;
	// used to sort jobs when AI has to perform more jobs at once
	uint8_t priority;
	// used only for debug purposes
	std::string descr;

	bool operator<(SchedulerTask other) const {
		return priority > other.priority;
	}

	SchedulerTask
		(const uint32_t time, const Widelands::SchedulerTaskId t, const uint8_t p, const char* d):
		due_time(time), id(t), priority(p), descr(d) {}

};

// List of blocked fields with block time, with some accompanying functions
struct BlockedFields {
	// <hash of field coordinates, time till blocked>
	// of course hash of a blocked field has to be unique
	std::map<uint32_t, uint32_t> BlockedFields;

	// NOCOM(#codereview) better pass the Coords here and have the BlockedFields calculate the coords_hash,
	// so we can be sure that the hash is always correct.
	void add(uint32_t hash, uint32_t till) {
		if (BlockedFields.count(hash) == 0) {
			BlockedFields.insert(std::pair<uint32_t, uint32_t>(hash, till));
		} else if (BlockedFields[hash] < till) {
			BlockedFields[hash] = till;
		}
		// The third possibility is that a field has been already blocked for longer time than 'till'
	}

	uint32_t count() {
		return BlockedFields.size();
	}

	void remove_expired(uint32_t gametime) {
		std::vector<uint32_t> fields_to_remove;
		for (auto field: BlockedFields) {
			if (field.second < gametime) {
				fields_to_remove.push_back(field.first);
			}
		}
		while (!fields_to_remove.empty()) {
			BlockedFields.erase(fields_to_remove.back());
			fields_to_remove.pop_back();
		}
	}

	// NOCOM(#codereview) better pass the Coords here and have the BlockedFields calculate the coords_hash.
	bool is_blocked(uint32_t hash) {
		if (BlockedFields.count(hash) == 0) {
			return false;
		} else {
			return true;
		}
	}
};

// NOCOM(#codereview): There are a lot of implementation details in this header file.
// Consider shifting the details to the .cc file.

// This is an struct that stores strength of players, info on teams and provides some outputs from these data
struct PlayersStrengths {
	struct PlayerStat {
		Widelands::TeamNumber team_number;
		uint32_t players_power;

		PlayerStat() {}
		PlayerStat(Widelands::TeamNumber tc, uint32_t pp) : team_number(tc), players_power(pp) {}
	};

	// This is the core part of this struct
	std::map<Widelands::PlayerNumber, PlayerStat> all_stats;

	// Number of team, sum of players' strength
	std::map<Widelands::TeamNumber, uint32_t> team_powers;

	// Inserting/updating data
	void add(Widelands::PlayerNumber pn, Widelands::TeamNumber tn, uint32_t pp) {
		if (all_stats.count(pn) == 0) {
			all_stats.insert(std::pair<Widelands::PlayerNumber, PlayerStat>(pn, PlayerStat(tn, pp)));
		} else {
			all_stats[pn].players_power = pp;
		}
	}

	void recalculate_team_power() {
		team_powers.clear();
		for (auto& item: all_stats) {
			if (item.second.team_number > 0) { // is a member of a team
				if (team_powers.count(item.second.team_number) > 0) {
					team_powers[item.second.team_number] += item.second.players_power;
				} else {
					team_powers[item.second.team_number] = item.second.players_power;
				}
			}
		}
	}

	// This is strength of player plus third of strength of other members of his team
	uint32_t get_modified_player_power(Widelands::PlayerNumber pn) {
		uint32_t result = 0;
		Widelands::TeamNumber team = 0;
		if (all_stats.count(pn) > 0) {
			result = all_stats[pn].players_power;
			team = all_stats[pn].team_number;
		};
		if (team > 0 && team_powers.count(team) > 0) {
			result = result + (team_powers[team] - result) / 3;
		};
		return result;
	}

	bool players_in_same_team(Widelands::PlayerNumber pl1, Widelands::PlayerNumber pl2) {
		if (all_stats.count(pl1) > 0 && all_stats.count(pl2) > 0 && pl1 != pl2) {
			// team number 0 = no team
			return all_stats[pl1].team_number > 0 && all_stats[pl1].team_number == all_stats[pl2].team_number;
		} else {
			return false;
		}
	}

	bool strong_enough(Widelands::PlayerNumber pl) {
		if (all_stats.count(pl) == 0) {
			return false;
		}
		uint32_t my_strength = all_stats[pl].players_power;
		uint32_t strongest_opponent_strength = 0;
		for (auto item : all_stats) {
			if (!players_in_same_team(item.first, pl) && pl != item.first) {
				if (get_modified_player_power(item.first) > strongest_opponent_strength) {
					strongest_opponent_strength = get_modified_player_power(item.first);
				}
			}
		}
		return my_strength > strongest_opponent_strength + 50;
	}
};

#endif  // end of include guard: WL_AI_AI_HELP_STRUCTS_H
