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
#include "logic/checkstep.h"
#include "logic/findnode.h"
#include "logic/game.h"
#include "logic/instances.h"
#include "logic/map.h"
#include "logic/player.h"
#include "logic/ship.h"
#include "logic/world/terrain_description.h"
#include "logic/world/world.h"

namespace Widelands {

class ProductionSite;
class MilitarySite;

enum class ExtendedBool : uint8_t {kUnset, kTrue, kFalse};

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
		        TerrainDescription::Type::kWater) ||
		       (world_.terrain_descr(coord.field->terrain_r()).get_is() &
		        TerrainDescription::Type::kWater);
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

struct BlockedField {
	Widelands::FCoords coords;
	uint32_t blocked_until_;

	BlockedField(Widelands::FCoords c, int32_t until) : coords(c), blocked_until_(until) {
	}
};

struct BuildableField {
	Widelands::FCoords coords;

	uint32_t field_info_expiration_;

	bool preferred_;
	bool enemy_nearby_;

	uint8_t unowned_land_nearby_;
	// to identify that field is too close to border and no production building should be built there
	bool near_border_;
	uint8_t unowned_mines_pots_nearby_;
	uint8_t trees_nearby_;
	uint8_t stones_nearby_;
	int16_t water_nearby_;
	int16_t distant_water_;
	int8_t fish_nearby_;
	int8_t critters_nearby_;
	int8_t ground_water_;  // used by wells
	uint8_t space_consumers_nearby_;
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

	std::vector<uint8_t> consumers_nearby_;
	std::vector<uint8_t> producers_nearby_;

	BuildableField(const Widelands::FCoords& fc)
	   : coords(fc),
	     field_info_expiration_(20000),
	     preferred_(false),
	     enemy_nearby_(0),
	     unowned_land_nearby_(0),
	     near_border_(false),
	     unowned_mines_pots_nearby_(0),
	     trees_nearby_(0),
	     // explanation of starting values
	     // this is done to save some work for AI (CPU utilization)
	     // base rules are:
	     // count of stones can only decrease, so  amount of stones
	     // is recalculated only when previous count is positive
	     // count of water fields are stable, so if the current count is
	     // non-negative, water is not recaldulated
	     stones_nearby_(1),
	     water_nearby_(-1),
	     distant_water_(0),
	     fish_nearby_(-1),
	     critters_nearby_(-1),
	     ground_water_(1),
	     space_consumers_nearby_(0),
	     area_military_capacity_(0),
	     military_loneliness_(1000),
	     military_in_constr_nearby_(0),
	     area_military_presence_(0),
	     military_stationed_(0),
	     military_unstationed_(0),
	     is_portspace_(false),
	     port_nearby_(false),
	     portspace_nearby_(Widelands::ExtendedBool::kUnset) {
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
	Widelands::BuildingIndex id;
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

	bool prod_build_material_;
	bool plants_trees_;
	bool recruitment_;  // is "producing" workers?
	bool is_buildable_;
	bool need_trees_;   // lumberjack = true
	bool need_stones_;  // quarry = true
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

	bool unoccupied_;
	uint16_t unconnected_;  // to any warehouse (count of such buildings)

	int32_t mines_;           // type of resource it mines_
	uint16_t mines_percent_;  // % of res it can mine
	uint32_t current_stats_;

	std::vector<int16_t> inputs_;
	std::vector<int16_t> outputs_;
	std::vector<Widelands::WareIndex> critical_built_mat_;

	bool upgrade_substitutes_;

	// It seems that fish and meat are subsitutes (for trainingsites), so
	// when testing if a trainingsite is supplied enough
	// we count the wares together
	std::unordered_set<Widelands::WareIndex> substitute_inputs_;
	int32_t substitutes_count_;

	int16_t production_hint_;

	// information needed for decision on new building construction
	// these should be calculated only once during one run of construct_building()
	// function
	Widelands::ExtendedBool output_needed_;
	int16_t max_preciousness;
	int16_t max_needed_preciousness_;

	int32_t cnt_built_;
	int32_t cnt_under_construction_;
	int32_t cnt_target_;  // number of buildings as target

	// used to track amount of wares produced by building
	uint32_t stocklevel_;
	int32_t stocklevel_time;  // time when stocklevel_ was last time recalculated
	int32_t last_dismantle_time_;
	int32_t construction_decision_time_;
	bool build_material_shortage_;

	int32_t total_count() const {
		return cnt_built_ + cnt_under_construction_;
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
	uint8_t no_resources_count;
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
	int32_t last_command_time = 0;
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

	MineTypesObserver() : in_construction(0), finished(0) {
	}
};

#endif  // end of include guard: WL_AI_AI_HELP_STRUCTS_H
