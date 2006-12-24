/*
 * Copyright (C) 2002-2003, 2006 by the Widelands Development Team
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
#include "militarysite.h"
#include "player.h"
#include "transport.h"
#include "trainingsite.h"
#include "tribe.h"
#include "warehouse.h"
#include "wexception.h"
#include "cmd_queue.h"
#include "soldier.h"
#include "sound_handler.h"
#include "attack_controller.h"


//
//
// class Player
//
//
Player::Player
(Editor_Game_Base* g,
 const int type,
 const int plnum,
 const Tribe_Descr & tr,
 const std::string & name,
 const uchar * const playercolor)
:
m_see_all(false),
m_egbase (g),
m_type   (type),
m_plnum  (plnum),
m_tribe  (tr)
{

	for(int i = 0; i < 4; i++)
		m_playercolor[i] = RGBColor(playercolor[i*3 + 0], playercolor[i*3 + 1], playercolor[i*3 + 2]);

   set_name(name);

   // Allow all buildings per default
   int i;
   m_allowed_buildings.resize(m_tribe.get_nrbuildings());
   for(i=0; i<m_tribe.get_nrbuildings(); i++)
      m_allowed_buildings[i]=true;

   // Resize the visibility array, so that it is large enough
   // init() will shrink it again
   seen_fields.resize(1024*1024);
}

Player::~Player() {
}
/*
===============
Player::init

Prepare the player for in-game action
===============
*/
void Player::init(const bool place_headquarters) {
	const Map & map = *m_egbase->get_map();

	seen_fields.resize(map.max_index(), false);

	if (place_headquarters) {
		const Tribe_Descr & trdesc = m_tribe;
		const int plnum = m_plnum;
		Editor_Game_Base & game = *m_egbase;
		//try {
			trdesc.load_warehouse_with_start_wares
				(game,
				 *dynamic_cast<Warehouse * const>
				 (game.warp_building
				  (map.get_starting_pos(plnum),
				   plnum,
				   trdesc.get_building_index("headquarters"))));
		//} catch () {
			//throw wexception("Tribe %s lacks headquarters", tribe.get_name());
		//}
	}
}


/*
===============
Player::get_buildcaps

Return filtered buildcaps that take the player's territory into account.
===============
*/
FieldCaps Player::get_buildcaps(const Coords coords) const {
	const Map & map = *m_egbase->get_map();
	const FCoords fc = map.get_fcoords(coords);
	uchar buildcaps = fc.field->get_caps();
	const uchar player_number = m_plnum;

	if (not fc.field->is_interior(player_number)) buildcaps = 0;

	// Check if a building's flag can't be build due to ownership
	else if (buildcaps & BUILDCAPS_BUILDINGMASK) {
		FCoords flagcoords;
		map.get_brn(fc, &flagcoords);
		if (not flagcoords.field->is_interior(player_number))
			buildcaps &= ~BUILDCAPS_BUILDINGMASK;
	}

	return static_cast<const FieldCaps>(buildcaps);
}


/*
===============
Player::set_area_seen

Mark the given area as (un)seen
===============
*/
void Player::set_area_seen(Coords center, uint area, bool on)
{
	const Map & map = m_egbase->map();
	const uint mapwidth = map.get_width();
	MapRegion mr(map, center, area);
	FCoords fc;
	while (mr.next(fc)) set_field_seen(Map::get_index(fc, mapwidth), on);

   m_view_changed = true;
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
			log("%i: building road, small immovable in the way, type=%d\n", get_player_number(), imm->get_type());
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

void Player::start_stop_building(PlayerImmovable* imm) {
	if (imm->get_owner() != this)
		return;
	if (imm->get_type() == Map_Object::BUILDING) {
		Building *bld = (Building*)imm;
		bld->set_stop(!bld->get_stop());
	}
}

/*
 * enhance this building, remove it, but give the constructionsite
 * an idea of enhancing
 */
void Player::enhance_building(PlayerImmovable* imm, int id) {
   if(imm->get_owner() != this)
      return;
   if(imm->get_type() == Map_Object::BUILDING) {
      Building* b=static_cast<Building*>(imm);
      int cur_id=get_tribe()->get_building_index(b->get_name());
      Coords c = b->get_position();
      assert(cur_id!=-1);

      // Get workers and soldiers
      const std::vector<Worker*>& workers =  b->get_workers();
      std::vector<Worker*> m_workers = workers;
      std::vector<Soldier*> m_soldiers;

		if (b->has_soldiers())
			m_soldiers = static_cast<ProductionSite * const>(b)->get_soldiers();

      b->remove(get_game()); // No fire or stuff

      get_game()->warp_constructionsite(c, m_plnum, id, cur_id);

      // Reassign the workers
      for( uint i = 0; i < m_workers.size(); i++) {
         Worker* w = m_workers[i];
         w->set_location( (Building*)(get_game()->get_map()->get_field(c)->get_immovable()));
         w->reset_tasks( static_cast<Game*>( get_game() ) );
      }
      // Reassign the soldier
      for( uint i = 0; i < m_soldiers.size(); i++) {
         Worker* w = m_soldiers[i];
         w->set_location( (Building*)(get_game()->get_map()->get_field(c)->get_immovable()));
         w->reset_tasks( static_cast<Game*>( get_game() ) );
      }
   }
}


/*
===============
Player::flagaction

Perform an action on the given flag.
===============
*/
void Player::flagaction(Flag* flag, int action)
{
	Game * const game = dynamic_cast<Game * const>(get_game());
	if (game and flag->get_owner() == this) {// Additional security check.
		switch (action) {
		case FLAGACTION_GEOLOGIST:
			//try {
				flag->add_flag_job
					(game, get_tribe()->get_worker_index("geologist"), "expedition");
			/*} catch (Descr_Maintainer<Worker_Descr>::Nonexistent) {
				log("Tribe defines no geologist\n");
			}*/
			break;
		default:
			log("Player sent bad flagaction = %i\n", action);
		}
	}
}

/*
 * allow building
 *
 * Disable or enable a building for a player
 */
void Player::allow_building(int i, bool t) {
	assert(i < m_tribe.get_nrbuildings());
	m_allowed_buildings.resize(m_tribe.get_nrbuildings());

   m_allowed_buildings[i]=t;
}

/*
 * Economy stuff below
 */
void Player::add_economy(Economy* eco) {
   if(has_economy(eco)) return;
   m_economies.push_back(eco);
}

void Player::remove_economy(Economy* eco) {
   if(!has_economy(eco)) return;
   std::vector<Economy*>::iterator i = m_economies.begin();
   while(i!=m_economies.end()) {
      if(*i == eco) {
         m_economies.erase(i);
         return;
      }
      ++i;
   }
   assert(0); // Never here
}

bool Player::has_economy(Economy* eco) {
   std::vector<Economy*>::iterator  i = m_economies.begin();
   while(i!=m_economies.end()) {
      if( *i == eco) return true;
      ++i;
   }
   return false;
}

int Player::get_economy_number(Economy* eco) {
   assert(has_economy(eco));

   std::vector<Economy*>::iterator  i = m_economies.begin();
   while(i!=m_economies.end()) {
      if( *i == eco) return (i - m_economies.begin());
      ++i;
   }
   assert(0); // never here
   return 0;
}

/************  Military stuff  **********/

/*
==========
Player::change_training_options

Change the training priotity values
==========
*/
void Player::change_training_options(PlayerImmovable* imm, int atr, int val) {
    if (imm->get_owner() != this)
        return;
    if (imm->get_type() == Map_Object::BUILDING) {
        TrainingSite* ts=static_cast<TrainingSite*>(imm);
        if (val>0)
            ts->add_pri((enum tAttribute) atr);
        else
            ts->sub_pri((enum tAttribute) atr);
    }
}

/*
===========
Player::drop_soldier

Forces the drop of given soldier at given house
===========
*/
void Player::drop_soldier(PlayerImmovable* imm, Soldier* soldier) {
    if (imm->get_owner() != this)
        return;
    if ((soldier->get_worker_type() == Worker_Descr::SOLDIER) &&
        (imm->get_type() >= Map_Object::BUILDING)) {
            Building* ms= static_cast<Building*>(imm);
            ms->drop_soldier (soldier->get_serial());
    }
}

//TODO val might (theoretically) be >1 or <-1, but there's always an inc/dec by one
void Player::change_soldier_capacity (PlayerImmovable* imm, int val) {
	if (imm->get_owner() != this)
		return;
	if (imm->get_type() == Map_Object::BUILDING) {
		//Building* ts=static_cast<TrainingSite*>(imm);
		if (val>0)
			((Building*) imm)->soldier_capacity_up();
		else
			((Building*)imm)->soldier_capacity_down();
	}
}

/*
===============
Player::enemyflagaction

Perform an action on the given enemy flag.
===============
*/
void Player::enemyflagaction(Flag* flag, int action, int attacker, int num, int)
{
   if (attacker != get_player_number())
      throw wexception ("Player (%d) is not the sender of an attack (%d)", attacker, get_player_number());

	Game * const game = dynamic_cast<Game * const>(get_game());
	if (not game) return;

   assert (num >= 0);

log("++Player::EnemyFlagAction()\n");
   // Additional security check LOOK, if equal exit!!
   if (flag->get_owner() == this)
      return;
log("--Player::EnemyFlagAction() Checkpoint!\n");

   switch(action) {

      case ENEMYFLAGACTION_ATTACK:
         {
            game->create_attack_controller(flag,attacker,flag->get_owner()->get_player_number(),(uint)num);
            break;
         }

      default:
         log("Player sent bad enemyflagaction = %i\n", action);
   }
}
