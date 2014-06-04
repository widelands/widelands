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

#ifndef AI_HELP_STRUCTS_H
#define AI_HELP_STRUCTS_H

#include <list>

#include "economy/flag.h"
#include "economy/road.h"
#include "logic/checkstep.h"
#include "logic/findnode.h"
#include "logic/game.h"
#include "logic/map.h"
#include "logic/player.h"

namespace Widelands {

class ProductionSite;
class MilitarySite;

struct CheckStepRoadAI {
	CheckStepRoadAI(Player* const pl, uint8_t const mc, bool const oe)
	   : player_(pl), movecaps_(mc), open_end_(oe) {
	}

	void set_openend(bool const oe) {
		open_end_ = oe;
	}

	bool allowed(Map&, FCoords start, FCoords end, int32_t dir, CheckStep::StepId) const;
	bool reachabledest(Map&, FCoords dest) const;

	Player* player_;
	uint8_t movecaps_;
	bool open_end_;
};

struct FindNodeEnemy {
	bool accept(const Map&, const FCoords& fc) const {
		// we are looking for fields we can walk on
		// and owned by hostile player.
		return (fc.field->nodecaps() & MOVECAPS_WALK) && fc.field->get_owned_by() != 0 &&
		       player->is_hostile(*game.get_player(fc.field->get_owned_by()));
	}

	Player* player;
	Game& game;

	FindNodeEnemy(Player* p, Game& g) : player(p), game(g) {
	}
};

struct FindNodeUnowned {
	bool accept(const Map&, const FCoords& fc) const {
		// when looking for unowned terrain to acquire, we are actually
		// only interested in fields we can walk on.
		// Fields should either be completely unowned or owned by an opposing player_
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

struct FindNodeUnownedMineable {
	bool accept(const Map&, const FCoords& fc) const {
		// when looking for unowned terrain to acquire, we are actually
		// only interested in fields where mines can be built.
		// Fields should be completely unowned
		return (fc.field->nodecaps() & BUILDCAPS_MINE) && (fc.field->get_owned_by() == 0);
	}

	Player* player_;
	Game& game;

	FindNodeUnownedMineable(Player* p, Game& g) : player_(p), game(g) {
	}
};

struct FindNodeWater {
	bool accept(const Map& map, const FCoords& coord) const {
		return (map.world().terrain_descr(coord.field->terrain_d()).get_is() & TERRAIN_WATER) ||
		       (map.world().terrain_descr(coord.field->terrain_r()).get_is() & TERRAIN_WATER);
	}
};

struct FindNodeWithFlagOrRoad {
	Economy* economy;
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

	bool operator==(Flag const* const f) const {
		return flag == f;
	}
};

struct CompareDistance {
	bool operator()(const NearFlag& a, const NearFlag& b) const {
		return a.distance_ < b.distance_;
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
	int32_t blocked_until_;

	BlockedField(Widelands::FCoords c, int32_t until) : coords(c), blocked_until_(until) {
	}
};

struct BuildableField {
	Widelands::FCoords coords;

	int32_t next_update_due_;

	bool reachable;
	bool preferred_;
	bool enemy_nearby_;

	uint8_t unowned_land_nearby_;
	uint8_t unowned_mines_pots_nearby_;
	uint8_t trees_nearby_;
	uint8_t stones_nearby_;
	int8_t water_nearby_;
	uint8_t space_consumers_nearby_;
	// to manage the military better following variables exists:
	// capacity of nearby buildings:
	int16_t military_capacity_;
	// distance to near buldings:
	int16_t military_loneliness_;
	// count of military buildings in construction
	// when making decision on new mulitary buildings it considers also
	// unowned fields and mines, but this information is not quite correct as there
	// are construction sites that will change this once they are built
	int16_t military_in_constr_nearby_;
	// actual count of soldiers in nearby buldings
	int16_t military_presence_;
	// stationed (manned) military buildings nearby
	int16_t military_stationed_;

	std::vector<uint8_t> consumers_nearby_;
	std::vector<uint8_t> producers_nearby_;

	BuildableField(const Widelands::FCoords& fc)
	   : coords(fc),
	     next_update_due_(0),
	     reachable(false),
	     preferred_(false),
	     enemy_nearby_(0),
	     unowned_land_nearby_(0),
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
	     space_consumers_nearby_(0),
	     military_capacity_(0),
	     military_loneliness_(1000),
	     military_in_constr_nearby_(0),
	     military_presence_(0),
	     military_stationed_(0) {
	}
};

struct MineableField {
	Widelands::FCoords coords;

	int32_t next_update_due_;

	bool reachable;
	bool preferred_;

	int32_t mines_nearby_;

	MineableField(const Widelands::FCoords& fc)
	   : coords(fc), next_update_due_(0), reachable(false), preferred_(false), mines_nearby_(0) {
	}
};

struct EconomyObserver {
	Widelands::Economy& economy;
	std::list<Widelands::Flag const*> flags;
	int32_t next_connection_try;
	uint32_t failed_connection_tries;

	EconomyObserver(Widelands::Economy& e) : economy(e) {
		next_connection_try = 0;
		failed_connection_tries = 0;
	}
};

struct BuildingObserver {
	char const* name;
	Widelands::Building_Index id;
	Widelands::Building_Descr const* desc;

	enum {
		BORING,
		CONSTRUCTIONSITE,
		PRODUCTIONSITE,
		MILITARYSITE,
		WAREHOUSE,
		TRAININGSITE,
		MINE
	} type;

	bool is_basic_;  // is a "must" to have for the ai
	bool prod_build_material_;
	bool plants_trees_;
	bool recruitment_;  // is "producing" workers?
	bool is_buildable_;
	bool need_trees_;      // lumberjack = true
	bool need_stones_;     // quarry = true
	bool mines_marble_;    // need to distinquish mines_ that produce marbles
	bool need_water_;      // fisher, fish_breeder = true
	bool space_consumer_;  // farm, vineyard... = true

	bool unoccupied_;  //

	int32_t mines_;           // type of resource it mines_
	uint16_t mines_percent_;  // % of res it can mine

	uint32_t current_stats_;

	std::vector<int16_t> inputs_;
	std::vector<int16_t> outputs_;
	int16_t production_hint_;

	int32_t cnt_built_;
	int32_t cnt_under_construction_;
	int32_t cnt_target_;  // number of buildings as target

	// used to track amount of wares produced by building
	uint32_t stocklevel_;
	int32_t stocklevel_time;  // time when stocklevel_ was last time recalculated
	int32_t last_dismantle_time_;
	int32_t construction_decision_time_;

	int32_t total_count() const {
		return cnt_built_ + cnt_under_construction_;
	}
	bool buildable(Widelands::Player& player_) {
		return is_buildable_ && player_.is_building_type_allowed(id);
	}
};

struct ProductionSiteObserver {
	Widelands::ProductionSite* site;
	int32_t built_time_;
	uint8_t stats_zero_;
	BuildingObserver* bo;
};

struct MilitarySiteObserver {
	Widelands::MilitarySite* site;
	BuildingObserver* bo;
	uint8_t checks;
};

struct WareObserver {
	uint8_t producers_;
	uint8_t consumers_;
	uint8_t preciousness_;
};

#endif
