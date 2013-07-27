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
	CheckStepRoadAI(Player * const pl, uint8_t const mc, bool const oe)
		: player(pl), movecaps(mc), openend(oe)
	{}

	void set_openend (bool const oe) {openend = oe;}

	bool allowed
		(Map &, FCoords start, FCoords end, int32_t dir, CheckStep::StepId)
		const;
	bool reachabledest(Map &, FCoords dest) const;

	Player * player;
	uint8_t  movecaps;
	bool     openend;
};


struct FindNodeUnowned {
	bool accept (const Map &, const FCoords & fc) const {
		// when looking for unowned terrain to acquire, we are actually
		// only interested in fields we can walk on.
		// Fields should either be completely unowned or owned by an opposing player
		return
			(fc.field->nodecaps() & MOVECAPS_WALK)
			&&
			((fc.field->get_owned_by() == 0)
			 || player->is_hostile(*game.get_player(fc.field->get_owned_by())))
			&& (!onlyenemies || (fc.field->get_owned_by() != 0));
	}

	//int8_t playernum;
	Player * player;
	Game & game;
	bool onlyenemies;

	FindNodeUnowned(Player * p, Game & g, bool oe = false)
		: player(p), game(g), onlyenemies(oe)
	{}
};


struct FindNodeWater {
	bool accept(const Map & map, const FCoords & coord) const {
		return
			(map.world().terrain_descr(coord.field->terrain_d()).get_is()
			 & TERRAIN_WATER)
			||
			(map.world().terrain_descr(coord.field->terrain_r()).get_is()
			 & TERRAIN_WATER);
	}
};


struct FindNodeWithFlagOrRoad {
	Economy * economy;
	bool accept(const Map &, FCoords) const;
};


struct NearFlag {
	Flag const * flag;
	int32_t  cost;
	int32_t  distance;

	NearFlag (const Flag & f, int32_t const c, int32_t const d) :
		flag(&f), cost(c), distance(d)
	{}

	bool operator< (const NearFlag & f) const {return cost > f.cost;}

	bool operator== (Flag const * const f) const {return flag == f;}
};


struct CompareDistance {
	bool operator() (const NearFlag & a, const NearFlag & b) const {
		return a.distance < b.distance;
	}
};


struct WalkableSpot {
	Coords coords;
	bool   hasflag;

	int32_t    cost;
	void * eco;

	int16_t  from;
	int16_t  neighbours[6];
};

}

struct BlockedField {
	Widelands::FCoords coords;
	int32_t blocked_until;

	BlockedField(Widelands::FCoords c, int32_t until)
		:
		coords(c),
		blocked_until(until)
	{}
};

struct BuildableField {
	Widelands::FCoords coords;

	int32_t next_update_due;

	bool    reachable;
	bool    preferred;
	bool    avoid_military;
	bool    enemy_nearby;

	uint8_t unowned_land_nearby;

	uint8_t trees_nearby;
	uint8_t stones_nearby;
	uint8_t water_nearby;
	uint8_t space_consumers_nearby;

	int16_t military_influence;

	std::vector<uint8_t> consumers_nearby;
	std::vector<uint8_t> producers_nearby;

	BuildableField (const Widelands::FCoords & fc)
		:
		coords             (fc),
		next_update_due    (0),
		reachable          (false),
		preferred          (false),
		avoid_military(0),
		enemy_nearby(0),
		unowned_land_nearby(0),
		trees_nearby       (0),
		stones_nearby      (0),
		water_nearby(0),
		space_consumers_nearby(0),
		military_influence(0)
	{}
};

struct MineableField {
	Widelands::FCoords coords;

	int32_t next_update_due;

	bool    reachable;
	bool    preferred;

	int32_t mines_nearby;

	MineableField (const Widelands::FCoords & fc) :
		coords         (fc),
		next_update_due(0),
		reachable      (false),
		preferred      (false),
		mines_nearby(0)
	{}
};

struct EconomyObserver {
	Widelands::Economy               & economy;
	std::list<Widelands::Flag const *> flags;
	int32_t                            next_connection_try;
	uint32_t                           failed_connection_tries;

	EconomyObserver (Widelands::Economy & e) : economy(e) {
		next_connection_try = 0;
		failed_connection_tries = 0;
	}
};

struct BuildingObserver {
	char                      const * name;
	Widelands::Building_Index         id;
	Widelands::Building_Descr const * desc;

	enum {
		BORING,
		CONSTRUCTIONSITE,
		PRODUCTIONSITE,
		MILITARYSITE,
		WAREHOUSE,
		TRAININGSITE,
		MINE
	}                                 type;

	bool                              is_basic; // is a "must" to have for the ai
	bool                              prod_build_material;
	bool                              recruitment; // is "producing" workers?

	bool                              is_buildable;

	bool                              need_trees;  // lumberjack = true
	bool                              need_stones; // quarry = true
	bool                              need_water;  // fisher, fish_breeder = true
	bool                              space_consumer; // farm, vineyard... = true

	bool                              unoccupied;  // >= 1 building unoccupied ?

	int32_t                           mines;       // type of resource it mines
	uint16_t                          mines_percent; // % of res it can mine

	uint32_t                          current_stats;

	std::vector<int16_t>              inputs;
	std::vector<int16_t>              outputs;
	int16_t                           production_hint;

	int32_t                           cnt_built;
	int32_t                           cnt_under_construction;

	int32_t total_count() const {return cnt_built + cnt_under_construction;}
	bool buildable(Widelands::Player & player) {
		return is_buildable && player.is_building_type_allowed(id);
	}
};

struct ProductionSiteObserver {
	Widelands::ProductionSite * site;
	int32_t builttime;
	uint8_t statszero;
	BuildingObserver * bo;
};

struct MilitarySiteObserver {
	Widelands::MilitarySite * site;
	BuildingObserver * bo;
	uint8_t checks;
};

struct WareObserver {
	uint8_t producers;
	uint8_t consumers;
	uint8_t preciousness;
};

#endif
