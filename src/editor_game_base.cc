/*
 * Copyright (C) 2002, 2003 by The Widelands Development Team
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
#include "editor_game_base.h"
#include "map.h"
#include "IntPlayer.h"
#include "player.h"
   
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
}

/*
===============
Editor_Game_Base::recalc_for_field

Call this function whenever the field at fx/fy has changed in one of the ways:
 - height has changed
 - robust Map_Object has been added or removed
 
This performs the steps outlined in the comment above Map::recalc_brightness()
and recalcs the interactive player's overlay.
===============
*/
void Editor_Game_Base::recalc_for_field(Coords coords, int radius)
{
   Map_Region_Coords mrc;
	Coords c;
   Field *f;

   // First pass
   mrc.init(coords, 2+radius, m_map);

   while(mrc.next(&c)) {
      f = m_map->get_field(c);
      m_map->recalc_brightness(c.x, c.y, f);
      m_map->recalc_fieldcaps_pass1(c.x, c.y, f);
   }


   // Second pass
   mrc.init(coords, 2+radius, m_map);

   while(mrc.next(&c)) {
      f = m_map->get_field(c);
      m_map->recalc_fieldcaps_pass2(c.x, c.y, f);

      if (m_iabase) {
         m_iabase->recalc_overlay(FCoords(c.x, c.y, f));
      }
   }

}

/*
===============
Editor_Game_Base::conquer_area

Conquers the given area for that player.
Additionally, it updates the visible area for that player.
===============
*/
void Editor_Game_Base::conquer_area(uchar playernr, Coords coords, int radius)
{
	Map_Region m(coords, radius, m_map);
	Field* f;

	while((f = m.next()))
	{
		if (f->get_owned_by() == playernr)
			continue;
		if (!f->get_owned_by()) {
			f->set_owned_by(playernr);
			continue;
		}
		
      // TODO: add support here what to do if some fields are already
      // occupied by another player
		// Probably the best thing to just don't grab it. Players should fight
		// for their land.
      //cerr << "warning: already occupied field is claimed by another user!" << endl;
   }
	
	Player *player = get_player(playernr);
	
	player->set_area_seen(coords.x, coords.y, radius+4, true);
	
	recalc_for_field(coords, radius);
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
void Editor_Game_Base::add_player(int plnum, int type, const char* tribe, const uchar *playercolor)
{
	assert(plnum >= 1 && plnum <= MAX_PLAYERS);
	
	if (m_players[plnum-1])
		remove_player(plnum);

	// Get the player's tribe
	uint i;
	
	for(i = 0; i < m_tribes.size(); i++)
		if (!strcmp(m_tribes[i]->get_name(), tribe))
			break;
	
	if (i == m_tribes.size())
		m_tribes.push_back(new Tribe_Descr(tribe));
	
	m_players[plnum-1] = new Player(this, type, plnum, m_tribes[i], playercolor);
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
		
		if (pid <= MAX_PLAYERS) {
			// the tribe is used, postload it
			m_tribes[id]->postload(this);
			id++;
		} else {
			delete m_tribes[id]; // the tribe is no longer used, remove it
			m_tribes.erase(m_tribes.begin() + id);
		}
	}
	
	// TODO: postload players? (maybe)

	// Postload wares
	init_wares();
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
	
	for(i = 0; i < m_wares.get_nitems(); i++)
		m_wares.get(i)->load_graphics();
	
	g_gr->load_animations();
}

/*
===============
Editor_Game_Base::init_wares

Called during postload.
Collects all wares from world and tribes and puts them into a global list
===============
*/
void Editor_Game_Base::init_wares()
{
	World *world = m_map->get_world();
	
	world->parse_wares(&m_wares);
	
	for(int pid = 1; pid <= MAX_PLAYERS; pid++) {
		Player *plr = get_player(pid);
		if (!plr)
			continue;
		
		Tribe_Descr *tribe = plr->get_tribe();
		
		for(int i = 0; i < tribe->get_nrworkers(); i++) {
			Worker_Descr *worker = tribe->get_worker_descr(i);
			if (!worker)
				continue;
			
			int idx = m_wares.get_index(worker->get_name());
			if (idx < 0)
				idx = m_wares.add(new Worker_Ware_Descr(worker->get_name()));
			
			worker->set_ware_id(idx);
				
			Worker_Ware_Descr *descr = (Worker_Ware_Descr*)m_wares.get(idx);
			descr->add_worker(tribe, worker);
		}
	}
}


/** Editor_Game_Base::warp_building(int x, int y, char owner, int idx)
 *
 * Instantly create a building at the given x/y location. There is no build time.
 *
 * owner is the player number of the building's owner.
 * idx is the building type index.
 */
Building *Editor_Game_Base::warp_building(int x, int y, char owner, int idx)
{
	Building_Descr *descr;
	Player *player = get_player(owner);

	assert(player);

	descr = player->get_tribe()->get_building_descr(idx);
	assert(descr);

	return descr->create(this, get_player(owner), Coords(x, y), false);
}


/*
===============
Editor_Game_Base::warp_constructionsite

Create a building site at the given x/y location for the given building type.
===============
*/
Building* Editor_Game_Base::warp_constructionsite(int x, int y, char owner, int idx)
{
	Building_Descr* descr;
	Player *player = get_player(owner);

	assert(player);

	descr = player->get_tribe()->get_building_descr(idx);
	assert(descr);

	return descr->create(this, get_player(owner), Coords(x, y), true);
}


/** Editor_Game_Base::create_bob(int x, int y, int idx)
 *
 * Instantly create a bob at the given x/y location.
 *
 * idx is the bob type.
 */
Bob *Editor_Game_Base::create_bob(int x, int y, int idx)
{
	Bob_Descr *descr;

	descr = m_map->get_world()->get_bob_descr(idx);
	assert(descr);
	
	return descr->create(this, 0, Coords(x, y));
}

/*
===============
Editor_Game_Base::create_immovable

Create an immovable at the given location.
Does not perform any placability checks.
===============
*/
Immovable *Editor_Game_Base::create_immovable(int x, int y, int idx)
{
	Immovable_Descr *descr;

	descr = m_map->get_world()->get_immovable_descr(idx);
	assert(descr);
	
	return descr->create(this, Coords(x, y));
}

Immovable* Editor_Game_Base::create_immovable(Coords c, std::string name)
{
	int idx = m_map->get_world()->get_immovable_index(name.c_str());

	if (idx < 0)
		throw wexception("Editor_Game_Base::create_immovable(%i, %i): %s is not defined",
								c.x, c.y, name.c_str());

	return create_immovable(c.x, c.y, idx);
}


/*
===============
Editor_Game_Base::get_safe_ware_id

Return the corresponding ware id. Throws an exception if ware can't be found.
===============
*/
int Editor_Game_Base::get_safe_ware_id(const char *name)
{
	int id = m_wares.get_index(name);
	if (id < 0)
		throw wexception("Ware '%s' not found", name);
	return id;
}


