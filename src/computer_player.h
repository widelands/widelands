/*
 * Copyright (C) 2004, 2006-2008 by the Widelands Development Team
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

#ifndef COMPUTER_PLAYER_H
#define COMPUTER_PLAYER_H

#include "game.h"
#include "notification.h"

#include <list>

class BuildingHints;

namespace Widelands {
class Economy;
class Player;
class PlayerImmovable;
class Tribe_Descr;
class Road;
class ProductionSite;
};

struct Computer_Player : Widelands::NoteReceiver<Widelands::NoteImmovable>, Widelands::NoteReceiver<Widelands::NoteField> {
	Computer_Player(Widelands::Game &, const Widelands::Player_Number);
	~Computer_Player ();

	void think ();

	void receive(const Widelands::NoteImmovable& note);
	void receive(const Widelands::NoteField& note);

	Widelands::Game & game() const throw () {return m_game;}
	Widelands::Player_Number get_player_number() {return player_number;}

private:
	void gain_immovable (Widelands::PlayerImmovable* pi);
	void lose_immovable (Widelands::PlayerImmovable* pi);
	void gain_building (Widelands::Building *);
	void lose_building (Widelands::Building *);

	bool construct_building ();
	void construct_roads    ();

	bool connect_flag_to_another_economy (Widelands::Flag *);
	bool improve_roads                   (Widelands::Flag *);

	struct BuildableField {
		Widelands::FCoords coords;

		int32_t          next_update_due;

		bool          reachable;
		bool          preferred;
		bool          avoid_military;

		uint8_t unowned_land_nearby;

		uint8_t trees_nearby;
		uint8_t stones_nearby;
		uint8_t tree_consumers_nearby;
		uint8_t stone_consumers_nearby;

		int16_t         military_influence;

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

		int32_t    next_update_due;

		bool    reachable;
		bool    preferred;

		int32_t     mines_nearby;

		MineableField (Widelands::FCoords const & fc)
			: coords(fc), next_update_due(0)
		{}
	};

	struct EconomyObserver {
		Widelands::Economy         * economy;
		std::list<Widelands::Flag *> flags;

		EconomyObserver (Widelands::Economy * e) {economy = e;}
	};

	struct BuildingObserver {
		char                      const * name;
		int32_t                           id;
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

	Widelands::Game                 & m_game;
	Widelands::Player_Number const    player_number;
	Widelands::Player               * player;
	Widelands::Tribe_Descr const    * tribe;

	std::list<BuildingObserver>       buildings;
	int32_t                               total_constructionsites;

	std::list<Widelands::FCoords>     unusable_fields;
	std::list<BuildableField *>       buildable_fields;
	std::list<MineableField *>        mineable_fields;
	std::list<Widelands::Flag *>      new_flags;
	std::list<Widelands::Road *>      roads;
	std::list<EconomyObserver *>      economies;
	std::list<ProductionSiteObserver> productionsites;

	WareObserver*         wares;

	EconomyObserver * get_economy_observer (Widelands::Economy *);

	int32_t                              next_road_due;
	int32_t                              next_construction_due;
	int32_t                              next_productionsite_check_due;
	int32_t                              inhibit_road_building;

	void late_initialization ();

	void update_buildable_field (BuildableField *);
	void update_mineable_field (MineableField*);
	void consider_productionsite_influence
		(BuildableField *, Widelands::Coords, BuildingObserver const &);
	void check_productionsite (ProductionSiteObserver &);

	BuildingObserver& get_building_observer(char const *);
};

#endif
