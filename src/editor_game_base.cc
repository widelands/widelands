/*
 * Copyright (C) 2002-2004 by The Widelands Development Team
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

#include <set>
#include "battle.h"
#include "building.h"
#include "editor_game_base.h"
#include "game.h"
#include "graphic.h"
#include "instances.h"
#include "interactive_base.h"
#include "map.h"
#include "player.h"
#include "tribe.h"
#include "wexception.h"
#include "worker.h"
#include "world.h"
#include "error.h"

// hard-coded playercolors
const uchar g_playercolors[MAX_PLAYERS][12] = {
	{ // blue
		  2,   2,  74,
		  2,   2, 112,
		  2,   2, 149,
		  2,   2, 198
	},
	{ // red
		119,  19,   0,
		166,  27,   0,
		209,  34,   0,
		255,  41,   0
	},
	{ // yellow
		112, 103,   0,
		164, 150,   0,
		209, 191,   0,
		255, 232,   0
	},
	{ // green
		 26,  99,   1,
		 37, 143,   2,
		 48, 183,   3,
		 59, 223,   3
	},
	{ // black/dark gray
		  0,   0,   0,
		 19,  19,  19,
		 35,  35,  35,
		 57,  57,  57
	},
	{ // orange
		119,  80,   0,
		162, 109,   0,
		209, 141,   0,
		255, 172,   0,
	},
	{ // purple
		 91,   0,  93,
		139,   0, 141,
		176,   0, 179,
		215,   0, 218,
	},
	{ // white
		119, 119, 119,
		166, 166, 166,
		210, 210, 210,
		255, 255, 255
	}
};

/*
============
Editor_Game_Base::Editor_Game_Base()

initialization
============
*/
Editor_Game_Base::Editor_Game_Base() {
   m_map = 0;

   m_objects = new Object_Manager;
	memset(m_players, 0, sizeof(m_players));

   m_gametime=0;
   m_iabase=0;

	m_lasttrackserial = 0;
   memset (m_conquer_map, 0, sizeof (m_conquer_map));
}

/*
============
Editor_Game_Base::~Editor_Game_Base()

last few cleanups
============
*/
Editor_Game_Base::~Editor_Game_Base() {
   int i;

   delete m_objects;
   for(i = 1; i <= MAX_PLAYERS; i++)
      if (m_players[i-1])
         remove_player(i);

   if (m_map)
		delete m_map;

   for(i = 0; i < (int)m_tribes.size(); i++)
		delete m_tribes[i];
	m_tribes.resize(0);

   m_battle_serials.resize(0);
      
   memset (m_conquer_map, 0, sizeof (m_conquer_map));
}

/*
===============
Editor_Game_Base::unconquer_area

This unconquers a area. This is only possible, when there
is a building placed on this field
===============
*/
void Editor_Game_Base::unconquer_area(uchar playernr, Coords coords) {
   assert(playernr);
   assert(get_map()->get_immovable(coords)->get_type() == Map_Object::BUILDING);

   uint i=0;
	int radius;

   while(i<m_conquer_info.size() && m_conquer_info[i].middle_point!=coords) ++i;
   assert(i<m_conquer_info.size());

   assert((static_cast<Building*>(get_map()->get_immovable(coords)))->get_conquers() == m_conquer_info[i].area);
   assert(playernr==m_conquer_info[i].player);


   // step 1: unconquer area of this building
	radius = m_conquer_info[i].area;
   do_conquer_area(playernr, coords, radius, false);

   // step 2: remove this building out ot m_conquer_info
   // std::vector erase doen't work on my system. I manually erase this
   m_conquer_info[i]=m_conquer_info[m_conquer_info.size()-1];
   m_conquer_info.pop_back();

   // step 3: recalculate for all claimed areas of this player with a building
// BEGIN : If you wanna to disable the dinamic conquer (by influences) , uncomment this block
/*   for(i=0; i<m_conquer_info.size(); i++) {
      if(m_conquer_info[i].player==playernr) {
           // Unconquer and reconquer the area
         do_conquer_area(playernr, m_conquer_info[i].middle_point, m_conquer_info[i].area, false);
         do_conquer_area(playernr, m_conquer_info[i].middle_point, m_conquer_info[i].area, true);
      }
   }

   // step 4: recalculate for all other players buildings
   for(uchar player=1; player<=MAX_PLAYERS; player++) {
      if(player==playernr) continue;

      for(i=0; i<m_conquer_info.size(); i++) {
         if(m_conquer_info[i].player==player) {
              // Unconquer and reconquer the area
            do_conquer_area(player, m_conquer_info[i].middle_point, m_conquer_info[i].area, false);
            do_conquer_area(player, m_conquer_info[i].middle_point, m_conquer_info[i].area, true);
         }
      }
   }
*/
// END : If you wanna to disable the dinamic conquer (by influences) , uncomment this block
// BEGIN : If you wanna to disable the dinamic conquer (by influences) , comment this block
   // step 3 and 4: Need to reconquer the full map, so it's easier and faster to do only one loop because
   // of the use of an influence map, where the sorting of the m_conquer_info isn't important
   for(i=0; i<m_conquer_info.size(); i++) 
   {
      do_conquer_area(m_conquer_info[i].player, m_conquer_info[i].middle_point, m_conquer_info[i].area, false);
      do_conquer_area(m_conquer_info[i].player, m_conquer_info[i].middle_point, m_conquer_info[i].area, true);
   }
// END : If you wanna to disable the dinamic conquer (by influences) , comment this block

	// step 5: deal with player immovables in the lost area
	cleanup_playerimmovables_area(coords, radius);
}

/*
===============
Editor_Game_Base::conquer_area

This conquers a given area because of a new (military) building
that is set there.
===============
*/
void Editor_Game_Base::conquer_area(uchar playernr, Coords coords, Building_Descr* b) {
   Conquer_Info ci;
   ci.player=playernr;
   ci.middle_point=coords;
   ci.area=b->get_conquers();
   m_conquer_info.push_back(ci);

   do_conquer_area(playernr, coords, b->get_conquers(), true);
   cleanup_playerimmovables_area(coords, b->get_conquers());
}


/*
===============
Editor_Game_Base::conquer_area_no_building

Conquers the given area for that player; assumes that there is no military building there
(so there is nothing on coords that could possibly be attacked!) and this area is not reclaimed
and might be consumed..
===============
*/
void Editor_Game_Base::conquer_area_no_building(uchar playernr, Coords coords, int radius)
{
   do_conquer_area(playernr, coords, radius, true);
}

/**
 * Editor_Game_Base::calc_influence
 *
 *    Returns the influence that a position with a given radius has to another point.
 * As the function is defined here, many functions can be tested without changing lots
 * of code, only by adding another "if" with method and setting method to proper value.
 *    If changing type of influence will be allowed at game, will be needed to add new
 * parameter at this function.
 */
#define MAX_RADIUS 32
int Editor_Game_Base::calc_influence (Coords a, Coords b, int radius)
{
   int w = m_map->get_width(),
       h = m_map->get_height(),
       influence = 0,
       method = 0;
   uint minx = std::min (std::min(abs(a.x - b.x), abs(a.x - b.x + w)), abs(a.x - b.x - w));
   uint miny = std::min (std::min(abs(a.y - b.y), abs(a.y - b.y + h)), abs(a.y - b.y - h));
    
    // Now "std::max (minx, miny)" is the distance between the points
   influence = std::max (minx, miny);
   
   if (method == 0)
   {
         // This method makes a "parabola" like x^4, but the maxium radius is MAX_RADIUS,
         // Now it works good, I've an stupid mistake ;)
      if (influence > radius)
         influence = 0;
      else if (influence == 0)
         influence = MAX_RADIUS;
      else
         influence = MAX_RADIUS - influence;
   
      influence = influence * influence * influence * influence;
   }
   else if (method == 1)
   {
      //    The function used here to calculate the influence is (d*(d-1))/2 + 1 and this
      // functions returns something like: 1, 2, 4, 7, 11, 15, 21, 27 ... 
      // Works well, but lacks with the bug of cleaning immovables ...
      influence = radius - influence + 1;

      if (influence < 1)
         influence = 0;
      else
         influence = ((influence*(influence-1))>>1) + 1;
   }
       
   return influence;
}

/**
===============
Editor_Game_Base::do_conquer_area [private]

Conquers the given area for that player; does the actual work
Additionally, it updates the visible area for that player.
===============
*/
void Editor_Game_Base::do_conquer_area(uchar playernr, Coords coords, int radius, bool conquer)
{
	MapRegion mr(m_map, coords, radius);
	Field* f;

   while((f = mr.next())) 
   {
      Coords c;
      int influence;
      
      c = (Coords) m_map->get_fcoords(f);

      influence = calc_influence(c, coords, radius);
      
         // This is for put a weight to every field, its equal to the next array:
         // 1, 2, 4, 7, 11, 16, 22, 29, 37, 46, 56, 67, 79, 92, 106, 121, 137, 154, 172 ... 1+(x*(x-1)/2)
         // This method will make harder to conquer an area already owner by any other player.
         // This don't have conflict with changing radius of conquer of an specific building, the only
         // that is needed to do is first: unconquer the area with inital values, second: reconquer the area
         // with new values. Will be usefull to save this values at the building. 
         // -- RFerriz
      if(conquer) 
      {
          // Adds the influence
         m_conquer_map[playernr][c.x][c.y] += influence;

            // Else, do the things that should be done
         if (f->get_owned_by() == playernr)
            continue;
         
         if (!f->get_owned_by() && m_conquer_map[0][c.x][c.y] == playernr) 
         {
            f->set_owned_by(playernr);
            m_conquer_map[0][c.x][c.y] = playernr;
            player_field_notification (m_map->get_fcoords(f), GAIN);
            continue;
         }
         
           // See if we can get the own
// BEGIN : If you wanna to disable the dinamic conquer (by influences) , comment this block
         if (m_conquer_map[playernr][c.x][c.y] > m_conquer_map[f->get_owned_by()][c.x][c.y])
         {
            player_field_notification (m_map->get_fcoords(f), LOSE);
            f->set_owned_by (playernr);
            m_conquer_map[0][c.x][c.y] = playernr;
            player_field_notification (m_map->get_fcoords(f), GAIN);
         }
// END : If you wanna to disable the dinamic conquer (by influences) , comment this block
      } 
      else
      {
         m_conquer_map[playernr][c.x][c.y] -= influence;

         if(f->get_owned_by() != playernr)
            continue;
         
         m_conquer_map[0][c.x][c.y] = 0;

            // ALWAYS set to 0. So is needed to call do_conquer_area with TRUE if you want to have the real
            // map of incluence
         player_field_notification (m_map->get_fcoords(f), LOSE);
         f->set_owned_by(0);
      }
   }
	Player *player = get_player(playernr);

	if(is_game()) // For editor all fields are visible and players don't manage view area at the moment (may change)
		player->set_area_seen(coords, radius+4, true);

	m_map->recalc_for_field_area(coords, radius);
}


/*
===============
Editor_Game_Base::cleanup_playerimmovables_area

Make sure that buildings cannot exist outside their owner's territory.
TODO: By now something goes wrong at unconquer_area and this function, because
the game crashes a bit time after unconquer an area with some buildings.
===============
*/
void Editor_Game_Base::cleanup_playerimmovables_area(Coords coords, int radius)
{
	std::vector<ImmovableFound> immovables;
	std::set<PlayerImmovable*> burnset;

	// Find all immovables that need fixing
	m_map->find_immovables(coords, radius, &immovables, FindImmovablePlayerImmovable());

	for(uint i = 0; i < immovables.size(); ++i) {
		PlayerImmovable* imm = (PlayerImmovable*)immovables[i].object;
		Coords f = immovables[i].coords;

		if (!imm->get_owner()->is_field_owned(f))
			burnset.insert(imm);
	}

	// Fix all immovables
	for(std::set<PlayerImmovable*>::iterator it = burnset.begin(); it != burnset.end(); ++it)
	{
		if (is_game())
			(*it)->schedule_destroy((Game*)this);
		else
			(*it)->remove(this);
	}
}


/*
===============
Editor_Game_Base::remove_player

Remove the player with the given number
===============
*/
void Editor_Game_Base::remove_player(int plnum)
{
   assert(plnum >= 1 && plnum <= MAX_PLAYERS);

	if (m_players[plnum-1]) {
		delete m_players[plnum-1];
		m_players[plnum-1] = 0;
	}
}


/*
===============
Editor_Game_Base::add_player

Create the player structure for the given plnum.
Note that AI player structures and the Interactive_Player are created when
the game starts. Similar for remote players.
===============
*/
void Editor_Game_Base::add_player(int plnum, int type, const char* tribe, const char* name)
{
   assert(plnum >= 1 && plnum <= MAX_PLAYERS);

   if (m_players[plnum-1])
      remove_player(plnum);

   // Get the player's tribe
   uint i;

   manually_load_tribe(tribe);

   for(i = 0; i < m_tribes.size(); i++)
      if (!strcmp(m_tribes[i]->get_name(), tribe))
         break;

   if (i == m_tribes.size())
      m_tribes.push_back(new Tribe_Descr(tribe));

   m_players[plnum-1] = new Player(this, type, plnum, m_tribes[i], name, g_playercolors[plnum-1]);
}

/*
 * Load the given tribe into structure 
 */
void Editor_Game_Base::manually_load_tribe(const char* tribe) {
	uint i;

	for(i = 0; i < m_tribes.size(); i++)
		if (!strcmp(m_tribes[i]->get_name(), tribe))
			break;

	if (i == m_tribes.size()) 
		m_tribes.push_back(new Tribe_Descr(tribe));
}

/*
 * Returns a tribe description from the internally loaded list
 */
Tribe_Descr* Editor_Game_Base::get_tribe(const char* tribe) {
	uint i;
   for(i = 0; i < m_tribes.size(); i++) {
		if (!strcmp(m_tribes[i]->get_name(), tribe))
			return m_tribes[i];
   }
   return 0;
}

/*
===============
Editor_Game_Base::set_map

Replaces the current map with the given one. Ownership of the map is transferred
to the Editor_Game_Base object.
===============
*/
void Editor_Game_Base::set_map(Map* map)
{
	if (m_map)
		delete m_map;

   m_map = map;

   // Register map_variable callback
   g_fh->register_variable_callback( g_MapVariableCallback, m_map);
}


/*
===============
Editor_Game_Base::postload

Load and prepare detailled game data.
This happens once just after the host has started the game and before the
graphics are loaded.
===============
*/
void Editor_Game_Base::postload()
{
	uint id;
	int pid;

	// Postload tribes
	id = 0;
	while(id < m_tribes.size()) {
		for(pid = 1; pid <= MAX_PLAYERS; pid++) {
			Player* plr = get_player(pid);

			if (plr && plr->get_tribe() == m_tribes[id])
				break;
		}

		if (pid <= MAX_PLAYERS || !is_game()) { // if this is editor, load the tribe anyways
			// the tribe is used, postload it
			m_tribes[id]->postload(this);
			id++;
		} else {
			delete m_tribes[id]; // the tribe is no longer used, remove it
			m_tribes.erase(m_tribes.begin() + id);
		}
	}

	// TODO: postload players? (maybe)
}


/*
===============
Editor_Game_Base::load_graphics

Load all graphics.
This function needs to be called once at startup when the graphics system
is ready.
If the graphics system is to be replaced at runtime, the function must be
called after that has happened.
===============
*/
void Editor_Game_Base::load_graphics()
{
	int i;

	m_map->load_graphics(); // especially loads world data

	for(i = 0; i < (int)m_tribes.size(); i++)
		m_tribes[i]->load_graphics();

	// TODO: load player graphics? (maybe)

	g_gr->load_animations();
}

/*
===============
Editor_Game_Base::warp_building

Instantly create a building at the given x/y location. There is no build time.

owner is the player number of the building's owner.
idx is the building type index.
===============
*/
Building *Editor_Game_Base::warp_building(Coords c, char owner, int idx)
{
	Building_Descr *descr;
	Player *player = get_player(owner);

	assert(player);

	descr = player->get_tribe()->get_building_descr(idx);
	assert(descr);

	return descr->create(this, get_player(owner), c, false);
}


/*
===============
Editor_Game_Base::warp_constructionsite

Create a building site at the given x/y location for the given building type.

if oldi != -1 this is a constructionsite comming from an enhancing action
===============
*/
Building* Editor_Game_Base::warp_constructionsite(Coords c, char owner, int idx, int old_id)
{
	Building_Descr* descr, *old_descr=0;
	Player *player = get_player(owner);

	assert(player);

	descr = player->get_tribe()->get_building_descr(idx);
   if(old_id!=-1)
      old_descr= player->get_tribe()->get_building_descr(old_id);
	assert(descr);

	return descr->create(this, get_player(owner), c, true, old_descr);
}


/*
===============
Editor_Game_Base::create_bob

Instantly create a bob at the given x/y location.

idx is the bob type.
===============
*/
Bob *Editor_Game_Base::create_bob(Coords c, int idx, Tribe_Descr* tribe)
{
	Bob_Descr *descr;

   if(!tribe)
      descr = m_map->get_world()->get_bob_descr(idx);
   else
      descr=tribe->get_bob_descr(idx);
	assert(descr);

	return descr->create(this, 0, c); // The bob knows for itself it is a world or a tribe bob
}


/*
===============
Editor_Game_Base::create_immovable

Create an immovable at the given location.
If tribe is not zero, create a immovable of a player (not a PlayerImmovable
but an immovable defined by the players tribe)
Does not perform any placability checks.
===============
*/
Immovable *Editor_Game_Base::create_immovable(Coords c, int idx, Tribe_Descr* tribe)
{
	Immovable_Descr *descr;

   if(!tribe) 
      descr = m_map->get_world()->get_immovable_descr(idx);
   else
      descr = tribe->get_immovable_descr(idx);
	assert(descr);

	return descr->create(this, c);
}

Immovable* Editor_Game_Base::create_immovable(Coords c, std::string name, Tribe_Descr* tribe)
{
	int idx;
   
   if(!tribe)
      idx = m_map->get_world()->get_immovable_index(name.c_str());
   else { 
      idx = tribe->get_immovable_index(name.c_str());
   }

	if (idx < 0)
		throw wexception("Editor_Game_Base::create_immovable(%i, %i): %s is not defined for %s",
								c.x, c.y, name.c_str(), tribe ? tribe->get_name() : "world");

	return create_immovable(c, idx, tribe);
}
      
Battle* Editor_Game_Base::create_battle ()
{
   Battle* b = new Battle ();
   b->init (this);
   m_battle_serials.push_back (b->get_serial());
   return b;
}

/*
================
Editor_Game_Base::get_save_player()

Returns the correct player, creates it
with the scenario data when he is not yet created
This should only happen in the editor. 
In the game, this is the same as get_player(). If it returns
zero it means that this player is disabled in the game.
================
*/
Player* Editor_Game_Base::get_safe_player(int n) {
   Player* plr=get_player(n);
   if(plr || is_game()) return plr;

   if(!plr) {
      // Create this player, but check that 
      // we are in the editor. In the game, all 
      // players are known from the beginning. In the
      // case of savegames, players must be set up
      // before this is ever called. Only in the editor
      // players are not always initialized 
     
      assert(!is_game()); 
      add_player(n, Player::playerLocal, get_map()->get_scenario_player_tribe(n).c_str(), get_map()->get_scenario_player_name(n).c_str());
      get_player(n)->init(this, false);
   }
   return get_player(n);
}

/*
===============
Editor_Game_Base::add_trackpointer

Add a registered pointer.
Returns the serial number that can be used to retrieve or remove the pointer.
===============
*/
uint Editor_Game_Base::add_trackpointer(void* ptr)
{
	m_lasttrackserial++;

	if (!m_lasttrackserial)
		throw wexception("Dude, you play too long. Track serials exceeded.");

	m_trackpointers[m_lasttrackserial] = ptr;
	return m_lasttrackserial;
}


/*
===============
Editor_Game_Base::get_trackpointer

Retrieve a previously stored pointer using the serial number.
Returns 0 if the pointer has been removed.
===============
*/
void* Editor_Game_Base::get_trackpointer(uint serial)
{
	std::map<uint, void*>::iterator it = m_trackpointers.find(serial);

	if (it != m_trackpointers.end())
		return it->second;

	return 0;
}


/*
===============
Editor_Game_Base::remove_trackpointer

Remove the registered track pointer. Subsequent calls to get_trackpointer()
using this serial number will return 0.
===============
*/
void Editor_Game_Base::remove_trackpointer(uint serial)
{
	m_trackpointers.erase(serial);
}

/**
 * Cleanup for load
 *
 * make this object ready to load new data
 */
void Editor_Game_Base::cleanup_for_load(bool flush_graphics, bool flush_animations) {
   // Clean all the stuff up, so we can load
   get_objects()->cleanup(this);

   // We do not flush the animations in the editor since all tribes are loaded and
   // animations can not change a lot, or?
   // TODO: check this when another world is needed
   if(flush_animations) 
      g_anim.flush();
   if(flush_graphics)
      g_gr->flush(0);

   int i;
   for(i=1; i<=MAX_PLAYERS; i++)
      if (m_players[i-1]) {
         remove_player(i);
         m_players[i-1] = 0;
      }

   m_map->cleanup();

   m_conquer_info.resize(0);

   if(is_game()) {
      for(uint i=0; i<m_tribes.size(); i++) {
         delete m_tribes[i];
      }
      m_tribes.resize(0);

      static_cast<Game*>(this)->get_cmdqueue()->flush();
   }
}

/**
 * Editor_Game_Base::get_attack_points
 *
 *    Returns all the positions of MilitarySites (buildings that can be attacked) of
 * a given player. If the player has no buildings that can be attacked, rare, then
 * returns 0
 */   
std::vector<Coords>* 
Editor_Game_Base::get_attack_points(uchar player)
{
   std::vector<Coords>* tmp = 0;
   
   for (uint i = 0; i < m_conquer_info.size(); ++i)
   {
      if (m_conquer_info[i].player == player)
      {
         // First initialization
         if (!tmp)
            tmp = new std::vector<Coords>;
         
         tmp->push_back (m_conquer_info[i].middle_point);

      }
   }

   return tmp;
}

// BEGIN Support for influece map in load/saved games
/**
 * Editor_Game_Base::make_influence_map
 *
 *    This method rebuilds the influence map of the full game. It should be called just after
 * load a game, and it can be called after every MilitarySite creation/remove, but may not
 * be efficient enought.
 */
void Editor_Game_Base::make_influence_map ()
{
   log("Making influence map\n");
   
   Coords c;
   int influence;
            
      // Clean influce maps
   memset (m_conquer_map, 0, sizeof (m_conquer_map));
      
   for(uint i=0; i<m_conquer_info.size(); i++) 
   {
         // First, update influence map of the player
      MapRegion mr(m_map,  m_conquer_info[i].middle_point, m_conquer_info[i].area);
      Field* f;
         
      while((f = mr.next())) 
      {
         c = (Coords) m_map->get_fcoords(f);
         influence = calc_influence(c, m_conquer_info[i].middle_point, m_conquer_info[i].area);
         m_conquer_map[m_conquer_info[i].player][c.x][c.y] += influence;
      }
   }

   // Now create the real influence map !
   for (int x = 0; x < MAX_X; x++)
   {
      for (int y = 0; y < MAX_Y; y++)
      {
         int best_player = 0;
         int best_value = 0;
         int npl = 1;
            // Find the most player influence over this position
         while (npl < MAX_PLAYERS)
         {
            if (m_conquer_map[npl][x][y] > best_value)
            {
               best_value = m_conquer_map[npl][x][y];
               best_player = npl;
            }
            npl++;
         }
         m_conquer_map[0][x][y] = best_player;
      }
   }
}
// END Support for influece map in load/saved games
