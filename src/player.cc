/*
 * Copyright (C) 2002, 2003 by the Widelands Development Team
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

#include "error.h"
#include "game.h"
#include "player.h"
#include "transport.h"
#include "tribe.h"
#include "wexception.h"

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
	Warehouse *wh = (Warehouse *)game->warp_building(c, m_plnum, idx);

	get_tribe()->load_warehouse_with_start_wares(game, wh);
}


/*
===============
Player::is_field_owned

Returns true if the field is completely owned by us and _inside_ the player's
country. This function returns false for border fields.
===============
*/
bool Player::is_field_owned(Coords coords)
{
	Map* map = m_egbase->get_map();
	FCoords fc = map->get_fcoords(coords);

	if (fc.field->get_owned_by() != get_player_number())
		return 0;

	// Check the neighbours
	for(int dir = 1; dir <= 6; ++dir) {
		FCoords neighb;

		map->get_neighbour(fc, dir, &neighb);

		if (neighb.field->get_owned_by() != get_player_number())
			return false;
	}

	return true;
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

	if (!is_field_owned(coords))
		return 0;

	// Check if a building's flag can't be build due to ownership
	if (buildcaps & BUILDCAPS_BUILDINGMASK) {
		Coords flagcoords;

		m_egbase->get_map()->get_brn(coords, &flagcoords);

		if (!is_field_owned(flagcoords))
			buildcaps &= ~BUILDCAPS_BUILDINGMASK;
	}

	return buildcaps;
}


/*
===============
Player::set_area_seen

Mark the given area as (un)seen
===============
*/
void Player::set_area_seen(Coords center, uint area, bool on)
{
	MapRegion mr(m_egbase->get_map(), center, area);
	Coords c;

	while(mr.next(&c))
      set_field_seen(c, on);
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
		Flag::create(m_egbase, this, c);
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
	Road::create(m_egbase, Road_Normal, start, end, *path);
}


/*
===============
Player::build

Place a construction site, checking that it's legal to do so.
===============
*/
void Player::build(Coords c, int idx)
{
	int buildcaps;
	Building_Descr* descr;

	// Validate building type
	if (idx < 0 || idx >= get_tribe()->get_nrbuildings())
		return;
	descr = get_tribe()->get_building_descr(idx);

	if (!descr->get_buildable())
		return;


	// Validate build position
	get_game()->get_map()->normalize_coords(&c);
	buildcaps = get_buildcaps(c);

	if (descr->get_ismine())
		{
		if (!(buildcaps & BUILDCAPS_MINE))
			return;
		}
	else
		{
		if ((buildcaps & BUILDCAPS_SIZEMASK) < (descr->get_size() - BaseImmovable::SMALL + 1))
			return;
		}

	get_game()->warp_constructionsite(c, m_plnum, idx);
}


/*
===============
Player::bulldoze

Bulldoze the given road, flag or building.
===============
*/
void Player::bulldoze(PlayerImmovable* imm)
{
	Building* building;

	// General security check
	if (imm->get_owner() != this)
		return;

	// Extended security check
	switch(imm->get_type()) {
	case Map_Object::BUILDING:
		building = (Building*)imm;
		if (!(building->get_playercaps() & (1 << Building::PCap_Bulldoze)))
			return;
		break;

	case Map_Object::FLAG:
		building = ((Flag*)imm)->get_building();
		if (building && !(building->get_playercaps() & (1 << Building::PCap_Bulldoze))) {
			log("Player trying to rip flag (%u) with undestroyable building (%u)\n", imm->get_serial(),
					building->get_serial());
			return;
		}
		break;

	case Map_Object::ROAD:
		break; // no additional check

	default:
		throw wexception("Player::bulldoze(%u): bad immovable type %u", imm->get_serial(), imm->get_type());
	}

	// Now destroy it
	imm->destroy(get_game());
}

