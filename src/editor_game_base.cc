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
#include "building.h"
#include "editor_game_base.h"
#include "graphic.h"
#include "instances.h"
#include "interactive_base.h"
#include "map.h"
#include "player.h"
#include "tribe.h"
#include "wexception.h"
#include "worker.h"
#include "world.h"

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
   MapRegion mr;
	FCoords c;

   // First pass
   mr.init(m_map, coords, radius+2);

   while(mr.next(&c)) {
      m_map->recalc_brightness(c);
      m_map->recalc_fieldcaps_pass1(c);
   }


   // Second pass
   mr.init(m_map, coords, radius+2);

   while(mr.next(&c)) {
      m_map->recalc_fieldcaps_pass2(c);

      if (m_iabase)
         m_iabase->recalc_overlay(c);
   }
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
   for(i=0; i<m_conquer_info.size(); i++) {
      if(m_conquer_info[i].player==playernr) {
         do_conquer_area(playernr, m_conquer_info[i].middle_point, m_conquer_info[i].area, true);
      }
   }

   // step 4: recalculate for all other players buildings
   for(uchar player=1; player<=MAX_PLAYERS; player++) {
      if(player==playernr) continue;

      for(i=0; i<m_conquer_info.size(); i++) {
         if(m_conquer_info[i].player==player) {
            do_conquer_area(player, m_conquer_info[i].middle_point, m_conquer_info[i].area, true);
         }
      }
   }

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

/*
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
      if(conquer) {
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
         // Too simple. What should be done when too HQ are too close and the area interact.
         // Also, when one user gets close to another, he might not be able to see a military building
         // when his land doesn't increase
         //cerr << "warning: already occupied field is claimed by another user!" << endl;
      } else {
         if(f->get_owned_by() != playernr) continue;
         f->set_owned_by(0);
      }
   }

	Player *player = get_player(playernr);

	player->set_area_seen(coords, radius+4, true);

	recalc_for_field(coords, radius);
}


/*
===============
Editor_Game_Base::cleanup_playerimmovables_area

Make sure that buildings cannot exist outside their owner's territory.
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
	for(int pid = 1; pid <= MAX_PLAYERS; pid++) {
		Player *plr = get_player(pid);
		if (!plr)
			continue;

      // TODO: BUG different wares with same name (different tribes)
      // do not get inserted ok
		Tribe_Descr *tribe = plr->get_tribe();
	   for(int i=0; i < tribe->get_nrwares(); i++) {
         Ware_Descr* ware = tribe->get_ware_descr(i);
         if(!ware) 
            continue;

         int idx = m_wares.get_index(ware->get_name());
         Ware_Descr* m_ware=new Item_Ware_Descr(*(static_cast<Item_Ware_Descr*>(ware)));
         if(idx < 0)
            idx = m_wares.add(m_ware);
      }

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
===============
*/
Building* Editor_Game_Base::warp_constructionsite(Coords c, char owner, int idx)
{
	Building_Descr* descr;
	Player *player = get_player(owner);

	assert(player);

	descr = player->get_tribe()->get_building_descr(idx);
	assert(descr);

	return descr->create(this, get_player(owner), c, true);
}


/*
===============
Editor_Game_Base::create_bob

Instantly create a bob at the given x/y location.

idx is the bob type.
===============
*/
Bob *Editor_Game_Base::create_bob(Coords c, int idx)
{
	Bob_Descr *descr;

	descr = m_map->get_world()->get_bob_descr(idx);
	assert(descr);

	return descr->create(this, 0, c);
}


/*
===============
Editor_Game_Base::create_immovable

Create an immovable at the given location.
Does not perform any placability checks.
===============
*/
Immovable *Editor_Game_Base::create_immovable(Coords c, int idx)
{
	Immovable_Descr *descr;

	descr = m_map->get_world()->get_immovable_descr(idx);
	assert(descr);

	return descr->create(this, c);
}

Immovable* Editor_Game_Base::create_immovable(Coords c, std::string name)
{
	int idx = m_map->get_world()->get_immovable_index(name.c_str());

	if (idx < 0)
		throw wexception("Editor_Game_Base::create_immovable(%i, %i): %s is not defined",
								c.x, c.y, name.c_str());

	return create_immovable(c, idx);
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
