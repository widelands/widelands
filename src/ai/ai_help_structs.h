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
	   : player_(pl), movecaps_(mc), open_end_(oe) {
	}

	void set_openend(bool const oe) {
		open_end_ = oe;
	}

	bool allowed(Map&, FCoords start, FCoords end, int32_t dir, CheckStep::StepId) const;
	bool reachable_dest(Map&, FCoords dest) const;

	Player* player_;
	uint8_t movecaps_;
	bool open_end_;
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

	FindNodeEnemy(Player* p, Game& g) : player(p), game(g) {
	}
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

	FindNodeEnemiesBuilding(Player* p, Game& g) : player(p), game(g) {
	}
};

// When looking for unowned terrain to acquire, we are actually
// only interested in fields we can walk on.
// Fields should either be completely unowned or owned by an opposing player_
struct FindNodeUnowned {
	bool accept(const Map&, const FCoords& fc) const {
		return (fc.field->nodecaps() & MOVECAPS_WALK) &&
		       ((fc.field->get_owned_by() == 0) ||
		        player_->is_hostile(*game.get_player(fc.field->get_owned_by()))) &&
		       (!only_enemies_ || (fc.field->get_owned_by() != 0));
	}

	Player* player_;
	Game& game;
	bool only_enemies_;

	FindNodeUnowned(Player* p, Game& g, bool oe = false) : player_(p), game(g), only_enemies_(oe) {
	}
};

// Sometimes we need to know how many nodes our allies owns
struct FindNodeAllyOwned {
	bool accept(const Map&, const FCoords& fc) const {
		return (fc.field->nodecaps() & MOVECAPS_WALK) && (fc.field->get_owned_by() != 0) &&
		       (fc.field->get_owned_by() != pn) &&
		       !player_->is_hostile(*game.get_player(fc.field->get_owned_by()));
	}

	Player* player_;
	Game& game;
	PlayerNumber pn;

	FindNodeAllyOwned(Player* p, Game& g, PlayerNumber n) : player_(p), game(g), pn(n) {
	}
};

// When looking for unowned terrain to acquire, we must
// pay speciall attention to fields where mines can be built.
// Fields should be completely unowned
struct FindNodeUnownedMineable {
	bool accept(const Map&, const FCoords& fc) const {

		return (fc.field->nodecaps() & BUILDCAPS_MINE) && (fc.field->get_owned_by() == 0);
	}

	Player* player_;
	Game& game;

	FindNodeUnownedMineable(Player* p, Game& g) : player_(p), game(g) {
	}
};

// Unowned but walkable fields nearby
struct FindNodeUnownedWalkable {
	bool accept(const Map&, const FCoords& fc) const {

		return (fc.field->nodecaps() & MOVECAPS_WALK) && (fc.field->get_owned_by() == 0);
	}

	Player* player_;
	Game& game;

	FindNodeUnownedWalkable(Player* p, Game& g) : player_(p), game(g) {
	}
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
	int32_t cost_;
	int32_t distance_;

	NearFlag(const Flag& f, int32_t const c, int32_t const d) : flag(&f), cost_(c), distance_(d) {
	}

	bool operator<(const NearFlag& f) const {
		return cost_ > f.cost_;
	}

	bool operator == (Flag const* const f) const {
		return flag == f;
	}
};

struct CompareDistance {
	bool operator()(const NearFlag& a, const NearFlag& b) const {
		return a.distance_ < b.distance_;
	}
};

// ordering nearflags by biggest reduction
struct CompareShortening {
	bool operator()(const NearFlag& a, const NearFlag& b) const {
		return (a.cost_ - a.distance_) > (b.cost_ - b.distance_);
	}
};

struct WalkableSpot {
	Coords coords;
	bool has_flag_;

	int32_t cost_;
	void* eco;

	int16_t from_;
	int16_t neighbours[6];
};
}

struct BuildableField {
	Widelands::FCoords coords;

	uint32_t field_info_expiration_;

	bool preferred_;
	bool enemy_nearby_;

	uint8_t unowned_land_nearby_;
	// to identify that field is too close to border and no production building should be built there
	bool near_border_;
	uint8_t unowned_mines_spots_nearby_;
	uint8_t trees_nearby_;
	uint8_t rocks_nearby_;
	int16_t water_nearby_;
	int16_t distant_water_;
	int8_t fish_nearby_;
	int8_t critters_nearby_;
	int8_t ground_water_;  // used by wells
	uint8_t space_consumers_nearby_;
	uint8_t rangers_nearby_;
	// to manage the military better following variables exists:
	// capacity of nearby buildings:
	int16_t area_military_capacity_;
	// distance to near buldings:
	int16_t military_loneliness_;
	// count of military buildings in construction
	// when making decision on new mulitary buildings it considers also
	// unowned fields and mines, but this information is not quite correct as there
	// are construction sites that will change this once they are built
	int16_t military_in_constr_nearby_;
	// actual count of soldiers in nearby buldings
	int16_t area_military_presence_;
	// stationed (manned) military buildings nearby
	int16_t military_stationed_;
	// stationed (manned) military buildings nearby
	int16_t military_unstationed_;
	bool is_portspace_;
	bool port_nearby_;  // to increase priority if a port is nearby,
	// especially for new colonies
	Widelands::ExtendedBool portspace_nearby_;  // prefer military buildings closer to the portspace
	int32_t max_buildcap_nearby_;
	// it is not necessary to check resources (stones, fish...) too frequently as they do not change fast
	// this stores time of last check
	uint32_t last_resources_check_time_;

	std::vector<uint8_t> consumers_nearby_;
	std::vector<uint8_t> producers_nearby_;
	// and for rangers, fishbreeders:
	std::vector<uint8_t> supporters_nearby_;

	BuildableField(const Widelands::FCoords& fc)
	   : coords(fc),
	     field_info_expiration_(20000),
	     preferred_(false),
	     enemy_nearby_(0),
	     unowned_land_nearby_(0),
	     near_border_(false),
	     unowned_mines_spots_nearby_(0),
	     trees_nearby_(0),
	     // explanation of starting values
	     // this is done to save some work for AI (CPU utilization)
	     // base rules are:
		  // count of rocks can only decrease, so  amount of rocks
	     // is recalculated only when previous count is positive
	     // count of water fields are stable, so if the current count is
	     // non-negative, water is not recaldulated
	     rocks_nearby_(1),
	     water_nearby_(-1),
	     distant_water_(0),
	     fish_nearby_(-1),
	     critters_nearby_(-1),
	     ground_water_(1),
	     space_consumers_nearby_(0),
	     rangers_nearby_(0),
	     area_military_capacity_(0),
	     military_loneliness_(1000),
	     military_in_constr_nearby_(0),
	     area_military_presence_(0),
	     military_stationed_(0),
	     military_unstationed_(0),
	     is_portspace_(false),
	     port_nearby_(false),
	     portspace_nearby_(Widelands::ExtendedBool::kUnset),
	     max_buildcap_nearby_(0),
	     last_resources_check_time_(0) {
	}

	int32_t own_military_sites_nearby_() {
		return military_stationed_ + military_unstationed_;
	}
};

struct MineableField {
	Widelands::FCoords coords;

	uint32_t field_info_expiration_;

	bool preferred_;

	int32_t mines_nearby_;
	// this is to provide that a mine is not built on the edge of mine area
	int32_t same_mine_fields_nearby_;

	MineableField(const Widelands::FCoords& fc)
	   : coords(fc),
	     field_info_expiration_(20000),
	     preferred_(false),
	     mines_nearby_(0),
	     same_mine_fields_nearby_(0) {
	}
};

struct EconomyObserver {
	Widelands::Economy& economy;
	std::list<Widelands::Flag const*> flags;
	int32_t dismantle_grace_time_;

	EconomyObserver(Widelands::Economy& e) : economy(e) {
		dismantle_grace_time_ = std::numeric_limits<int32_t>::max();
	}
};

struct BuildingObserver {
	char const* name;
	Widelands::DescriptionIndex id;
	Widelands::BuildingDescr const* desc;

	enum {
		BORING,
		CONSTRUCTIONSITE,
		PRODUCTIONSITE,
		MILITARYSITE,
		WAREHOUSE,
		TRAININGSITE,
		MINE
	} type;

	bool plants_trees_;
	bool recruitment_;  // is "producing" workers?
	Widelands::BuildingNecessity new_building_;
	uint32_t new_building_overdue_;
	int32_t primary_priority_;
	bool is_buildable_;
	bool need_trees_;   // lumberjack = true
	bool need_rocks_;  // quarry = true
	bool mines_water_;  // wells
	bool need_water_;   // fisher, fish_breeder = true
	bool is_hunter_;    // need to identify hunters
	bool is_fisher_;    // need to identify fishers
	bool is_port_;
	bool is_shipyard_;
	bool space_consumer_;       // farm, vineyard... = true
	bool expansion_type_;       // military building used that can be used to control area
	bool fighting_type_;        // military building built near enemies
	bool mountain_conqueror_;   // military building built near mountains
	uint32_t prohibited_till_;  // do not build before (ms)
	uint32_t forced_after_;     // do not wait until ware is needed
	TrainingSiteType trainingsite_type_;

	uint16_t unconnected_count_;  // to any warehouse (count of such buildings)

	int32_t mines_;           // type of resource it mines_
	uint16_t mines_percent_;  // % of res it can mine
	uint32_t current_stats_;

	std::vector<int16_t> inputs_;
	std::vector<int16_t> outputs_;
	std::vector<Widelands::DescriptionIndex> critical_built_mat_;

	bool built_mat_producer_;

	// an enhancement to this building:
	// produces all wares as current building, and perhaps more
	bool upgrade_substitutes_;
	// produces some additional wares
	bool upgrade_extends_;

	// It seems that fish and meat are subsitutes (for trainingsites), so
	// when testing if a trainingsite is supplied enough
	// we count the wares together
	std::unordered_set<Widelands::DescriptionIndex> substitute_inputs_;
	int32_t substitutes_count_;

	int16_t production_hint_;

	// information needed for decision on new building construction
	int16_t max_preciousness_;
	int16_t max_needed_preciousness_;

	int32_t cnt_built_;
	int32_t cnt_under_construction_;
	int32_t cnt_target_;  // number of buildings as target
	int32_t cnt_limit_by_aimode_; // limit imposed by weak or normal AI mode

	// used to track amount of wares produced by building
	uint32_t stocklevel_;
	uint32_t stocklevel_time;  // time when stocklevel_ was last time recalculated
	uint32_t last_dismantle_time_;
	uint32_t construction_decision_time_;
	uint32_t last_building_built_;

	uint32_t unoccupied_count_;

	bool build_material_shortage_;

	int32_t total_count() const {
		return cnt_built_ + cnt_under_construction_;
	}
	Widelands::AiModeBuildings aimode_limit_status() {
		if (total_count() > cnt_limit_by_aimode_) {
			return Widelands::AiModeBuildings::kLimitExceeded;
		} else if (total_count() == cnt_limit_by_aimode_) {
			return Widelands::AiModeBuildings::kOnLimit;
		} else {
			return Widelands::AiModeBuildings::kAnotherAllowed;
		}
	}
	bool buildable(Widelands::Player& player_) {
		return is_buildable_ && player_.is_building_type_allowed(id);
	}
};

struct ProductionSiteObserver {
	Widelands::ProductionSite* site;
	uint32_t built_time_;
	uint32_t unoccupied_till_;
	uint8_t stats_zero_;
	uint32_t no_resources_since_;
	BuildingObserver* bo;
};

struct MilitarySiteObserver {
	Widelands::MilitarySite* site;
	BuildingObserver* bo;
	uint8_t checks;
	// when considering attack most military sites are inside territory and should be skipped during
	// evaluation
	bool enemies_nearby_;
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
	Widelands::Coords expedition_start_point_;
	std::unordered_set<uint32_t> visited_spots_;

	// a ship circumvents all islands in the same direction, the value
	// is assigned only once
	Widelands::IslandExploreDirection island_circ_direction = Widelands::IslandExploreDirection::kClockwise;
	bool waiting_for_command_ = false;
	uint32_t last_command_time = 0;
};

struct WareObserver {
	uint8_t producers_;
	uint8_t consumers_;
	uint8_t preciousness_;
};

// Computer player does not get notification messages about enemy militarysites
// and warehouses, so following is collected based on observation
// It is conventient to have some information preserved, like nearby minefields,
// when it was attacked, whether it is warehouse and so on
// Also AI test more such targets when considering attack and calculated score is
// is stored in the observer
struct EnemySiteObserver {
	bool warehouse_;
	int32_t attack_soldiers_strength;
	int32_t defenders_strength;
	uint8_t stationed_soldiers;
	uint32_t last_time_attackable;
	uint32_t last_tested;
	int16_t score;
	Widelands::ExtendedBool mines_nearby;
	int16_t no_attack_counter;

	EnemySiteObserver()
	   : warehouse_(false),
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
		due_time(time), id(t), priority(p), descr(d){}

};

// List of blocked fields with block time, with some accompanying functions
struct BlockedFields {
	// <hash of field coordinates, time till blocked>
	// of course hash of an blocked field is unique
	std::map<uint32_t, uint32_t> BlockedFields; //NOCOM or std::unordered_map rather?

	void add(uint32_t hash, uint32_t till){
		if (BlockedFields.count(hash) == 0) {
			BlockedFields.insert(std::pair<uint32_t, uint32_t>(hash, till));
		} else if (BlockedFields[hash] < till) {
			BlockedFields[hash] = till;
		}
		//third possibility is that a field has been already blocked for longer time than 'till'
	}

	uint32_t count(){
		return BlockedFields.size();
	}

	void remove_expired(uint32_t gametime) {
		std::vector<uint32_t> fields_to_remove;
		for (auto field: BlockedFields) {
			if (field.second<gametime) {
				fields_to_remove.push_back(field.first);
			}
		}
		while (!fields_to_remove.empty()) {
			BlockedFields.erase(fields_to_remove.back());
			fields_to_remove.pop_back();
		}
	}

	bool is_blocked(uint32_t hash){
		if (BlockedFields.count(hash) == 0) {
			return false;
		} else {
			return true;
		}
	}
};

struct PlayersStrengths {
	struct PlayerStat {
		//uint16_t pn_;
		uint8_t tn_;
		uint32_t players_power_;
		
		
		PlayerStat() {};
		PlayerStat(uint8_t tc, uint32_t pp)
		: tn_(tc), players_power_(pp) {
		}
	};
	std::map<uint16_t,PlayerStat> all_stats;
	
	// number of team, sum of players' strength
	std::map<uint8_t, uint32_t> team_powers;
	
	void add(uint16_t pn, uint8_t tn, uint32_t pp){
		if (all_stats.count(pn) == 0) {
			all_stats.insert(std::pair<uint16_t,PlayerStat>(pn, PlayerStat(tn, pp)));
		} else {
			all_stats[pn].players_power_ = pp;
		}

		assert(all_stats[pn].players_power_ == pp); //NOCOM
	}
	
	void recalculate_team_power() {
		team_powers.clear();
		for (auto& item: all_stats){
			if (item.second.tn_ > 0) { //is a member of a team
				if (team_powers.count(item.second.tn_) > 0){
					team_powers[item.second.tn_] += item.second.players_power_;
				} else {
					team_powers[item.second.tn_] = item.second.players_power_;
				}
			}
		}
	}
	
	// This is strength of player plus third of strength of other members of his team
	uint32_t get_modified_player_power(uint16_t pn){
		uint32_t result = 0;
		uint8_t team = 0;
		if (all_stats.count(pn) > 0) {
			result = all_stats[pn].players_power_;
			team = all_stats[pn].tn_;
		};
		if (team > 0 && team_powers.count(team) > 0) {
			result = result + (team_powers[team] - result) / 3;
		};
		return result;
	}
		

	
	bool players_in_same_team(uint16_t pl1,uint16_t pl2){
		if (all_stats.count(pl1) > 0 && all_stats.count(pl2) > 0 && pl1 != pl2) { 
			// team number 0 = no team
			return all_stats[pl1].tn_ > 0 && all_stats[pl1].tn_ == all_stats[pl2].tn_;
		} else {
			return false;
		}
	}
	
	bool strong_enough(uint16_t pl) {
		if (all_stats.count(pl) == 0) {
			return false;
		}
		uint32_t my_strength = all_stats[pl].players_power_;
		uint32_t strongest_oponent_strength=0;
		for (auto item : all_stats) {
			if(!players_in_same_team(item.first, pl) && pl != item.first) {
				if (get_modified_player_power(item.first) > strongest_oponent_strength) {
					strongest_oponent_strength = get_modified_player_power(item.first);
				}
			}
		}
		return my_strength > strongest_oponent_strength + 50;
	}

	
	void print_content(){ //NOCOM - remove
		printf ("printing players: \n");
		for (auto& item: all_stats){
			printf ("  %2d  %3d   %3d, team power: %3d %s\n",
				item.first, item.second.tn_, item.second.players_power_, get_modified_player_power(item.first),
				strong_enough(item.first)?", strong enough": " ");
		};
		//printf ("printing teams: \n");
		//for (auto& iter: team_powers){
			//printf (" %2d: %5d\n", iter.first, iter.second);
		//};
	}
		
};
		

#endif  // end of include guard: WL_AI_AI_HELP_STRUCTS_H
