/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#include "widelands.h"
#include "bob.h"
#include "descr_maintainer.h"
#include "tribe.h"
#include "player.h"
#include "transport.h"

//
//
// class Player
//
//
Player::Player(Editor_Game_Base* g, int type, int plnum, Tribe_Descr* tribe, const uchar *playercolor)
{
   m_type = type; 
	m_plnum = plnum;
	m_tribe = tribe;
	m_egbase = g;
   seen_fields = 0;
	
	for(int i = 0; i < 4; i++)
		m_playercolor[i] = RGBColor(playercolor[i*3 + 0], playercolor[i*3 + 1], playercolor[i*3 + 2]);
}

Player::~Player(void)
{
   if(seen_fields) 
      delete seen_fields;
}

/*
===============
Player::init_for_game

Prepare the player for in-game action

we could use static cast to upcast our Editor_Game_Base object
but instead, we let us pass the object once again
===============
*/
void Player::init_for_game(Game* game) 
{
	Map *map = game->get_map();

	seen_fields = new std::vector<bool>(map->get_width()*map->get_height(), false);

	// place the HQ
	const Coords &c = map->get_starting_pos(m_plnum);
	int idx = get_tribe()->get_building_index("headquarters");
	if (idx < 0)
		throw wexception("Tribe %s lacks headquarters", get_tribe()->get_name());
	Warehouse *wh = (Warehouse *)game->warp_building(c.x, c.y, m_plnum, idx);

	//	Add starting wares
	wh->create_wares(game->get_safe_ware_id("carrier"), 20);
}

/*
===============
Player::get_buildcaps

Return filtered buildcaps that take the player's territory into account.
===============
*/
int Player::get_buildcaps(Coords coords)
{
	FCoords fc = m_egbase->get_map()->get_fcoords(coords);
	int buildcaps = fc.field->get_caps();
	
	if (fc.field->get_owned_by() != get_player_number())
		return 0;
	
	// If any neighbour belongs to somebody else, we can't build here
	FCoords neighb[6];
	
	for(int dir = 1; dir <= 6; dir++) {
		m_egbase->get_map()->get_neighbour(fc, dir, &neighb[dir-1]);
		
		if (neighb[dir-1].field->get_owned_by() != get_player_number())
			return 0;
	}
	
	// TODO: check if a building's flag can't be build due to ownership
	
	return buildcaps;
}

/*
===============
Player::set_area_seen

Mark the given area as (un)seen
===============
*/
void Player::set_area_seen(int x, int y, uint area, bool on)
{
	Map_Region_Coords r(Coords(x, y), area, m_egbase->get_map());
  
	while(r.next(&x, &y)) {
      set_field_seen(x, y, on);
   }
}

/*
===============
Player::build_flag

Build a flag, checking that it's legal to do so.
===============
*/
void Player::build_flag(Coords c)
{
	int buildcaps = get_buildcaps(c);
	
	if (buildcaps & BUILDCAPS_FLAG)
		Flag::create(m_egbase, this, c, 1);
}

/*
===============
Player::rip_flag

Rip out the flag at the given coordinates, if there is one that is owned
by the player.
===============
*/
void Player::rip_flag(Coords c)
{
	BaseImmovable *imm = m_egbase->get_map()->get_immovable(c);
	
	if (imm && imm->get_type() == Map_Object::FLAG) {
		if (((Flag *)imm)->get_owner() == this)
			imm->destroy(m_egbase);
	}
}

/*
===============
Player::build_road

Build a road along the given path.
Perform sanity checks (ownership, flags).

Note: the diagnostic log messages aren't exactly errors. They might happen
in some situations over the network.
===============
*/
void Player::build_road(const Path *path)
{
	Map *map = m_egbase->get_map();
	BaseImmovable *imm;
	Flag *start, *end;
	
	imm = map->get_immovable(path->get_start());
	if (!imm || imm->get_type() != Map_Object::FLAG) {
		log("%i: building road, missed start flag\n", get_player_number());
		return;
	}
	start = (Flag *)imm;
	
	imm = map->get_immovable(path->get_end());
	if (!imm || imm->get_type() != Map_Object::FLAG) {
		log("%i: building road, missed end flag\n", get_player_number());
		return;
	}
	end = (Flag *)imm;
	
	// Verify ownership of the path
	Coords coords = path->get_start();
	
	for(int i = 0; i < path->get_nsteps()-1; i++) {
		int dir = path->get_step(i);
		map->get_neighbour(coords, dir, &coords);
		
		imm = map->get_immovable(coords);
		if (imm && imm->get_size() >= BaseImmovable::SMALL) {
			log("%i: building road, small immovable in the way\n", get_player_number());
			return;
		}
		int caps = get_buildcaps(coords);
		if (!(caps & MOVECAPS_WALK)) {
			log("%i: building road, unwalkable\n", get_player_number());
			return;
		}
	}
	
	// fine, we can build the road
	Road::create(m_egbase, Road_Normal, start, end, *path, 1);
}

/*
===============
Player::remove_road

Remove that road, if it belongs to the player.
===============
*/
void Player::remove_road(Road *road)
{
	if (road->get_flag_start()->get_owner() != this)
		return;
	
	road->destroy(m_egbase);
}
