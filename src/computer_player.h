/*
 * Copyright (C) 2004 by The Widelands Development Team
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
#include <string>

class Player;
class PlayerImmovable;
class Tribe_Descr;
class Road;

class Computer_Player {
	public:
		Computer_Player (Game *g, uchar pln);
		~Computer_Player ();

		void think ();
		
		void gain_immovable (PlayerImmovable*);
		void lose_immovable (PlayerImmovable*);
		
		void gain_field (const FCoords&);
		void lose_field (const FCoords&);

		inline Game *get_game() { return game; }
		inline uchar get_player_number(void) { return player_number; }
		inline Player *get_player() { assert(game); return game->get_player(player_number) ; }

	private:
		void gain_building (Building*);
		void lose_building (Building*);
		
		bool construct_building ();
		
		bool connect_flag_to_another_economy (Flag*);
		bool improve_roads (Flag*);
		
		struct BuildableField:FCoords {
			long			next_update_due;
			
			bool			reachable;
			bool			preferred;
			
			unsigned char		unowned_land_nearby;
			
			unsigned char		trees_nearby;
			unsigned char		stones_nearby;
			unsigned char		tree_consumers_nearby;
			unsigned char		stone_consumers_nearby;
			
			short			military_influence;

			BuildableField (const FCoords& fc):FCoords (fc)
			{
			    next_update_due=0;
			    reachable=false;
			    preferred=false;
			    unowned_land_nearby=0;
			    trees_nearby=0;
			    stones_nearby=0;
			}
		};
		
		struct MineableField:FCoords {
			MineableField (const FCoords& fc):FCoords (fc) {}
		};
		
		struct EconomyObserver {
			Economy*		economy;
			std::list<Flag*>	flags;

			EconomyObserver (Economy* e) { economy=e; }
		};
		
		struct BuildingObserver {
			const char*		name;
			int			id;
			Building_Descr*		desc;

			enum {
				BORING,
				CONSTRUCTIONSITE,
				PRODUCTIONSITE,
				MILITARYSITE,
				MINE
			}			type;
			
			bool			is_buildable;
			
			bool			need_trees;
			bool			need_stones;
			
			std::vector<int>	outputs;
			
			int			cnt_built;
			int			cnt_under_construction;
			
			int get_total_count()
			{ return cnt_built + cnt_under_construction; }
		};
		
		Game*				game;
		Map*				map;
		
		uchar				player_number;
		Player*				player;
		Tribe_Descr*			tribe;
		
		std::list<BuildingObserver>	buildings;
		int				total_constructionsites;
		
		std::list<FCoords>		unusable_fields;
		std::list<BuildableField>	buildable_fields;
		std::list<MineableField>	mineable_fields;
		std::list<Flag*>		new_flags;
		std::list<Road*>		roads;
		std::list<EconomyObserver>	economies;
		
		EconomyObserver& get_economy_observer (Economy*);
		
		long				next_construction_due;
		long				inhibit_road_building;
		
		void update_buildable_field (BuildableField&);
		void consider_productionsite_influence (BuildableField&, const Coords&, const BuildingObserver&);
		
		BuildingObserver& get_building_observer(const char*);
};


#endif // __COMPPLAYER_H
