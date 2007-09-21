/*
 * Copyright (C) 2004, 2006-2007 by the Widelands Development Team
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

#ifndef __COMPPLAYER_H
#define __COMPPLAYER_H

#include "game.h"

#include <list>
#include <stdint.h>

class BuildingHints;
class Economy;
class Player;
class PlayerImmovable;
class Tribe_Descr;
class Road;
class ProductionSite;

struct Computer_Player {
	Computer_Player(Game &, const Player_Number);
		~Computer_Player ();

		void think ();

		void gain_immovable (PlayerImmovable*);
		void lose_immovable (PlayerImmovable*);

		void gain_field (const FCoords&);
		void lose_field (const FCoords&);

	const Game & game() const throw () {return m_game;}
	Game       & game()       throw () {return m_game;}
		inline uint8_t get_player_number() {return player_number;}
	Player * get_player() const {return game().get_player(player_number);}

private:
		void gain_building (Building*);
		void lose_building (Building*);

		bool construct_building ();
		void construct_roads ();

		bool connect_flag_to_another_economy (Flag*);
		bool improve_roads (Flag*);

		struct BuildableField {
		FCoords       coords;

		long          next_update_due;

		bool          reachable;
		bool          preferred;
		bool          avoid_military;

		uint8_t unowned_land_nearby;

		uint8_t trees_nearby;
		uint8_t stones_nearby;
		uint8_t tree_consumers_nearby;
		uint8_t stone_consumers_nearby;

		short         military_influence;

			BuildableField (const FCoords& fc)
			{
			    coords=fc;
			    next_update_due=0;
			    reachable=false;
			    preferred=false;
			    unowned_land_nearby=0;
			    trees_nearby=0;
			    stones_nearby=0;
			}
		};

		struct MineableField {
		FCoords coords;

		long    next_update_due;

		bool    reachable;
		bool    preferred;

		int     mines_nearby;

			MineableField (const FCoords& fc)
			{
			    coords=fc;
			    next_update_due=0;
			}
		};

		struct EconomyObserver {
		Economy         * economy;
		std::list<Flag *> flags;

			EconomyObserver (Economy* e) {economy=e;}
		};

		struct BuildingObserver {
		const char                        * name;
		int                                 id;
		const Building_Descr              * desc;
		const BuildingHints               * hints;

		enum {
				BORING,
				CONSTRUCTIONSITE,
				PRODUCTIONSITE,
				MILITARYSITE,
				MINE
		}                                   type;

		bool                                is_buildable;

		bool                                need_trees;
		bool                                need_stones;

		std::vector<short>                  inputs;
		std::vector<short>                  outputs;
		short                               production_hint;

		int                                 cnt_built;
		int                                 cnt_under_construction;

			int get_total_count()
			{return cnt_built + cnt_under_construction;}
		};

		struct ProductionSiteObserver {
		ProductionSite   * site;
		BuildingObserver * bo;
		};

		struct WareObserver {
		uint8_t producers;
		uint8_t consumers;
		uint8_t preciousness;
		};

	Game                            & m_game;
	const Player_Number               player_number;
	Player                          * player;
	const Tribe_Descr               * tribe;

	std::list<BuildingObserver>       buildings;
	int                               total_constructionsites;

	std::list<FCoords>                unusable_fields;
	std::list<BuildableField *>       buildable_fields;
	std::list<MineableField *>        mineable_fields;
	std::list<Flag *>                 new_flags;
	std::list<Road *>                 roads;
	std::list<EconomyObserver *>      economies;
	std::list<ProductionSiteObserver> productionsites;

	WareObserver*         wares;

		EconomyObserver* get_economy_observer (Economy*);

	long                              next_road_due;
	long                              next_construction_due;
	long                              next_productionsite_check_due;
	long                              inhibit_road_building;

		void late_initialization ();

		void update_buildable_field (BuildableField*);
		void update_mineable_field (MineableField*);
		void consider_productionsite_influence (BuildableField*, const Coords&, const BuildingObserver&);
		void check_productionsite (ProductionSiteObserver&);

		BuildingObserver& get_building_observer(const char*);
};


#endif // __COMPPLAYER_H
