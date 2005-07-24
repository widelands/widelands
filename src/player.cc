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
#include "militarysite.h"
#include "player.h"
#include "transport.h"
#include "trainingsite.h"
#include "tribe.h"
#include "wexception.h"
#include "cmd_queue.h"
#include "sound_handler.h"


//
//
// class Player
//
//
Player::Player(Editor_Game_Base* g, int type, int plnum, Tribe_Descr* tribe, const char* name, const uchar *playercolor)
{
   m_see_all = false;
   m_type = type;
	m_plnum = plnum;
	m_tribe = tribe;
	m_egbase = g;
   
	for(int i = 0; i < 4; i++)
		m_playercolor[i] = RGBColor(playercolor[i*3 + 0], playercolor[i*3 + 1], playercolor[i*3 + 2]);

   set_name(name);
   
   // Allow all buildings per default
   int i; 
   m_allowed_buildings.resize(m_tribe->get_nrbuildings());
   for(i=0; i<m_tribe->get_nrbuildings(); i++) 
      m_allowed_buildings[i]=true;

   // Resize the visibility array, so that it is large enough
   // init() will shrink it again
   seen_fields.resize(1024*1024);
}

Player::~Player(void)
{
}

/*
===============
Player::init

Prepare the player for in-game action

we could use static cast to upcast our Editor_Game_Base object
but instead, we let us pass the object once again
===============
*/
void Player::init(Editor_Game_Base* game, bool hq)
{
	Map *map = game->get_map();

	seen_fields.resize(map->get_width()*map->get_height(), false); 

	// place the HQ
   if(hq) {
      const Coords &c = map->get_starting_pos(m_plnum);
      int idx = get_tribe()->get_building_index("headquarters");
      if (idx < 0)
         throw wexception("Tribe %s lacks headquarters", get_tribe()->get_name());
      Warehouse *wh = (Warehouse *)game->warp_building(c, m_plnum, idx);

      get_tribe()->load_warehouse_with_start_wares(game, wh);
   }
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

	sound_handler->play_fx("create_construction_site");
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
      
      if( b->has_soldiers() ) {
         const std::vector<Soldier*>* soldier =  ((ProductionSite*)b)->get_soldiers();
         m_soldiers = *soldier;
      }
      
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
	Editor_Game_Base* eg = get_game();

	if (!eg->is_game())
		return;

	Game* g = (Game*)eg;

	// Additional security check
	if (flag->get_owner() != this)
		return;

	switch(action) {
	case FLAGACTION_GEOLOGIST:
		{
		int id = get_tribe()->get_worker_index("geologist");

		if (id < 0) {
			log("Tribe defines no geologist\n");
			return;
		}

		flag->add_flag_job(g, id, "expedition");
		return;
		}

	default:
		log("Player sent bad flagaction = %i\n", action);
	}
}

/*
 * allow building
 *
 * Disable or enable a building for a player
 */
void Player::allow_building(int i, bool t) {
   assert(m_tribe && i<m_tribe->get_nrbuildings());
   m_allowed_buildings.resize(m_tribe->get_nrbuildings());

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
void Player::enemyflagaction(Flag* flag, int action, int attacker, int num, int type)
{
   Editor_Game_Base* eg = get_game();

   if (attacker != get_player_number())
      throw wexception ("Player (%d) is not the sender of an attack (%d)", attacker, get_player_number());
   
   if (!eg->is_game())
      return;

   assert (num >= 0);

      
   Game* g = (Game*)eg;
   Map* map = g->get_map();

log("++Player::EnemyFlagAction()\n");
   // Additional security check LOOK, if equal exit!!
   if (flag->get_owner() == this)
      return;
log("--Player::EnemyFlagAction() Checkpoint!\n");

   switch(action) {
      
      case ENEMYFLAGACTION_ATTACK:
         {
            int id = get_tribe()->get_worker_index("soldier");
            uint i;

            if (id < 0) 
            {
               log("Tribe defines no soldier\n");
               return;
            }
            int radius = 25;
            std::vector<ImmovableFound> list;
            std::vector<MilitarySite*> ms_list;
            CheckStepWalkOn cstep(MOVECAPS_WALK, false);
            
            map->find_reachable_immovables(flag->get_position(), radius, &list, &cstep);

            if (!list.size()) 
               return;
            
            /* Find all friendly MS */
            for (i = 0; i < list.size(); i++)
            {
               BaseImmovable* imm = list[i].object;

               if (imm->get_type() == Building::BUILDING &&
                  ((PlayerImmovable*)imm)->get_owner() == this && 
                  ((Building*)imm)->get_building_type() == Building::MILITARYSITE)
                  {
log("Player::EnemyFlagAction() MilitarySite %p found!\n", &list[i]);
                     MilitarySite* ms = static_cast<MilitarySite*>(imm);
                     ms_list.push_back (ms);
                 }
            }
            
            int launched = 0;
            for (i = 0; i < ms_list.size(); i++)
            {
               if (launched >= num)
                  break;
                  
               if (ms_list[i]->can_launch_soldiers())
               {
                  launched += ms_list[i]->launch_attack(flag, type);
               }
            }
            break;
         }
      
      default:
         log("Player sent bad enemyflagaction = %i\n", action);
   }
}

