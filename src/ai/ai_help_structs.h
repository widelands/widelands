/*
 * Copyright (C) 2009 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "checkstep.h"
#include "computer_player_hints.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "findnode.h"
#include "map.h"

#include <list>

namespace Widelands {

struct ProductionSite;

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
	bool accept (const Map &, const FCoords fc) const {
		// when looking for unowned terrain to acquire, we are actually
		// only interested in fields we can walk on
		return
			fc.field->get_owned_by() == 0 && (fc.field->get_caps() & MOVECAPS_WALK);
	}
};


struct FindNodeWater {
	bool accept(Map const & map, FCoords const & coord) const {
		return
			(map.world().terrain_descr(coord.field->terrain_d()).get_is() & TERRAIN_WATER) ||
			(map.world().terrain_descr(coord.field->terrain_r()).get_is() & TERRAIN_WATER);
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

	NearFlag (Flag const & f, int32_t const c, int32_t const d) :
		flag(&f), cost(c), distance(d)
	{}

	bool operator< (NearFlag const & f) const {return cost > f.cost;}

	bool operator== (Flag const * const f) const {return flag == f;}
};


struct CompareDistance {
	bool operator() (NearFlag const & a, NearFlag const & b) const {
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


struct BuildableField {
	Widelands::FCoords coords;

	int32_t next_update_due;

	bool    reachable;
	bool    preferred;
	bool    avoid_military;

	uint8_t unowned_land_nearby;

	uint8_t trees_nearby;
	uint8_t stones_nearby;
	uint8_t tree_consumers_nearby;
	uint8_t stone_consumers_nearby;
	uint8_t water_nearby;

	int16_t military_influence;

	std::vector<uint8_t> consumers_nearby;
	std::vector<uint8_t> producers_nearby;

	BuildableField (Widelands::FCoords const & fc)
		:
		coords             (fc),
		next_update_due    (0),
		reachable          (false),
		preferred          (false),
		unowned_land_nearby(0),
		trees_nearby       (0),
		stones_nearby      (0)
	{}
};

struct MineableField {
	Widelands::FCoords coords;

	int32_t next_update_due;

	bool    reachable;
	bool    preferred;

	int32_t mines_nearby;

	MineableField (Widelands::FCoords const & fc)
		: coords(fc), next_update_due(0)
	{}
};

struct EconomyObserver {
	Widelands::Economy         & economy;
	std::list<Widelands::Flag const *> flags;

	EconomyObserver (Widelands::Economy & e) : economy(e) {}
};

struct BuildingObserver {
	char                      const * name;
	Widelands::Building_Index         id;
	Widelands::Building_Descr const * desc;
	BuildingHints             const * hints;

	enum {
		BORING,
		CONSTRUCTIONSITE,
		PRODUCTIONSITE,
		MILITARYSITE,
		MINE
	}                                 type;

	bool                              is_buildable;

	bool                              need_trees;
	bool                              need_stones;
	bool                              need_water;

	int32_t                           mines;

	std::vector<int16_t>              inputs;
	std::vector<int16_t>              outputs;
	int16_t                           production_hint;

	int32_t                           cnt_built;
	int32_t                           cnt_under_construction;

	int32_t get_total_count() {return cnt_built + cnt_under_construction;}
};

struct ProductionSiteObserver {
	Widelands::ProductionSite * site;
	BuildingObserver * bo;
};

struct WareObserver {
	uint8_t producers;
	uint8_t consumers;
	uint8_t preciousness;
};
