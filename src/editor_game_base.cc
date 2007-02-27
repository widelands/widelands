/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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
#include "font_handler.h"
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
#include "attack_controller.h"

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

		delete m_map;

	const std::vector<Tribe_Descr*>::const_iterator tribes_end = m_tribes.end();
	for
		(std::vector<Tribe_Descr*>::const_iterator it = m_tribes.begin();
		 it != tribes_end;
		 ++it)
		delete *it;
}

/*
===============
Editor_Game_Base::unconquer_area

This unconquers a area. This is only possible, when there
is a building placed on this field
===============
*/
void Editor_Game_Base::unconquer_area
(Player_Area player_area, const Player_Number destroying_player)
{
	assert(0 <= player_area.x);
	assert     (player_area.x < map().get_width());
	assert(0 <= player_area.y);
	assert     (player_area.y < map().get_height());
	assert(0 < player_area.player_number);
	assert    (player_area.player_number <= map().get_nrplayers());

	//  Here must be a building.
	const Building & building =
		dynamic_cast<const Building &>(*map().get_immovable(player_area));
	assert(building.owner().get_player_number() == player_area.player_number);

	std::vector<Player_Area>::iterator this_conquer_info =
		m_conquer_info.begin();
	log
		("Editor_Game_Base::unconquer_area: (%i, %i) radius %u, player %u:\n",
		 player_area.x, player_area.y, player_area.radius,
		 player_area.player_number);
	for (;;) {
		assert(this_conquer_info != m_conquer_info.end());
		log
			("\tcomparing with (%i, %i)\n",
			 this_conquer_info->x, this_conquer_info->y);
		if (this_conquer_info->Coords::operator==(player_area)) break;
		++this_conquer_info;
	}
	assert(this_conquer_info->player_number == player_area.player_number);
	assert(this_conquer_info->radius        == player_area.radius);

   // step 1: unconquer area of this building
	do_conquer_area(player_area, false, destroying_player);

   // step 2: remove this building out ot m_conquer_info
	*this_conquer_info = m_conquer_info.back();
   m_conquer_info.pop_back();

	// step 5: deal with player immovables in the lost area
	//  Players are not allowed to have their immovables on their borders.
	//  Therefore the area must be enlarged before calling
	//  cleanup_playerimmovables_area, so that those new border locations are
	//  covered.
	++player_area.radius;
	cleanup_playerimmovables_area(player_area);
}

/*
===============
Editor_Game_Base::conquer_area

This conquers a given area because of a new (military) building
that is set there.
===============
*/
void Editor_Game_Base::conquer_area(Player_Area player_area) {
	assert(0 <= player_area.x);
	assert(player_area.x < map().get_width());
	assert(0 <= player_area.y);
	assert(player_area.y < map().get_height());
	assert(0 < player_area.player_number);
	assert    (player_area.player_number <= map().get_nrplayers());

	m_conquer_info.push_back(player_area);
	do_conquer_area(player_area, true);

	//  Players are not allowed to have their immovables on their borders.
	//  Therefore the area must be enlarged before calling
	//  cleanup_playerimmovables_area, so that those new border locations are
	//  covered.
	++player_area.radius;
	cleanup_playerimmovables_area(player_area);
}


/*
===============
Editor_Game_Base::conquer_area_no_building

Conquers the given area for that player; assumes that there is no military building there
(so there is nothing on coords that could possibly be attacked!) and this area is not reclaimed
and might be consumed..
===============
*/
void Editor_Game_Base::conquer_area_no_building(const Player_Area player_area) {
	assert(0 <= player_area.x);
	assert(player_area.x < map().get_width());
	assert(0 <= player_area.y);
	assert(player_area.y < map().get_height());
	assert(0 < player_area.player_number);
	assert    (player_area.player_number <= map().get_nrplayers());

	do_conquer_area(player_area, true);
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
Editor_Game_Base::Influence Editor_Game_Base::calc_influence
(const Coords a, const Area area)
{
   int w = m_map->get_width(),
       h = m_map->get_height(),
       method = 0;

    // Now "std::max (minx, miny)" is the distance between the points
	Influence influence = std::max
		(std::min
		 (std::min
		  (abs(a.x - area.x), abs(a.x - area.x + w)), abs(a.x - area.x - w)),
		 std::min
		 (std::min
		  (abs(a.y - area.y), abs(a.y - area.y + h)), abs(a.y - area.y - h)));

	if (method == 0) {
         // This method makes a "parabola" like x^4, but the maxium radius is MAX_RADIUS,
         // Now it works good, I've an stupid mistake ;)
		if (influence >= 0 and static_cast<const uint>(influence) > area.radius)
         influence = 0;
      else if (influence == 0)
         influence = MAX_RADIUS;
      else
         influence = MAX_RADIUS - influence;

      influence = influence * influence * influence * influence;
   }
	else if (method == 1) {
      //    The function used here to calculate the influence is (d*(d-1))/2 + 1 and this
      // functions returns something like: 1, 2, 4, 7, 11, 15, 21, 27 ...
      // Works well, but lacks with the bug of cleaning immovables ...
		influence = area.radius - influence + 1;

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
void Editor_Game_Base::do_conquer_area
(const Player_Area player_area,
 const bool conquer,
 const Player_Number preferred_player,
 const Uint8 vision_range,
 const bool neutral_when_no_influence,
 const bool neutral_when_competing_influence,
 const bool conquer_guarded_location_by_superior_influence)
{
	assert(0 <= player_area.x);
	assert(player_area.x < map().get_width());
	assert(0 <= player_area.y);
	assert(player_area.y < map().get_height());
	assert(0 < player_area.player_number);
	assert    (player_area.player_number <= map().get_nrplayers());
	assert    (preferred_player          <= map().get_nrplayers());
	assert(preferred_player != player_area.player_number);
	assert(not conquer or not preferred_player);
	MapRegion mr(*m_map, player_area);
	do {
		const Map::Index index = m_map->get_index(mr.location());
		const int influence = calc_influence(mr.location(), player_area);

         // This is for put a weight to every field, its equal to the next array:
         // 1, 2, 4, 7, 11, 16, 22, 29, 37, 46, 56, 67, 79, 92, 106, 121, 137, 154, 172 ... 1+(x*(x-1)/2)
         // This method will make harder to conquer an area already owner by any other player.
         // This don't have conflict with changing radius of conquer of an specific building, the only
         // that is needed to do is first: unconquer the area with inital values, second: reconquer the area
         // with new values. Will be usefull to save this values at the building.
         // -- RFerriz
		const Player_Number owner = mr.location().field->get_owned_by();
		if (conquer) {
          // Adds the influence
			Influence new_influence_modified =
				m_conquer_map[player_area.player_number][index] += influence;
			if (owner and not conquer_guarded_location_by_superior_influence)
				new_influence_modified = 1;
			if (m_conquer_map[owner][index] < new_influence_modified) {
				if (owner) player_field_notification(mr.location(), LOSE);
				mr.location().field->set_owned_by(player_area.player_number);
				player_field_notification (mr.location(), GAIN);
         }
		} else if
			((m_conquer_map[player_area.player_number][index] -= influence) == 0
			 and
			 owner == player_area.player_number)
		{
			//  The player completely lost influence over the location, which he
			//  owned. Now we must see if some other player has influence and if
			//  so, transfer the ownership to that player.
			const Player_Number nr_players = map().get_nrplayers();
			Player_Number best_player =
				neutral_when_no_influence ? 0 : player_area.player_number;
			Influence best_value = 0;
			for (Player_Number plnum = 1; plnum <= nr_players; ++plnum)
				if (const Influence value = m_conquer_map[plnum][index])
					if        (value >  best_value) {
						best_value = value;
						best_player = plnum;
					} else if (value == best_value) {
						const Coords c = map().get_fcoords(map()[index]);
						log
							("Editor_Game_Base::do_conquer_area: Player %u completely "
							 "lost influence over (%i, %i). Players %u and %u, both "
							 "have influence %i there. Depending on game rules, the "
							 "location will not change ownership or it will become "
							 "neutral (unless another player has higher influence).\n",
							 player_area.player_number, c.x, c.y, best_player, plnum,
							 value);
						best_player = neutral_when_competing_influence ?
							0 : player_area.player_number;
					}
			m_conquer_map[0][index] = best_value;
			if (preferred_player and m_conquer_map[preferred_player][index])
				best_player = preferred_player;
			if (best_player != player_area.player_number) {
				player_field_notification (mr.location(), LOSE);
				mr.location().field->set_owned_by (best_player);
				if (best_player) player_field_notification (mr.location(), GAIN);
         }
      }
	} while (mr.advance(*m_map));

	//  This must reach one step beyond the conquered area to adjust the borders
	//  of neighbour players.
	m_map->recalc_for_field_area(Area(player_area, player_area.radius + 1));

	player(player_area.player_number).set_area_seen
		(Area(player_area, player_area.radius + vision_range), true);
}


/*
===============
Editor_Game_Base::cleanup_playerimmovables_area

Make sure that buildings cannot exist outside their owner's territory.
TODO: By now something goes wrong at unconquer_area and this function, because
the game crashes a bit time after unconquer an area with some buildings.
TODO: Document why there are 2 loops instead of destroying in the 1st.
===============
*/
void Editor_Game_Base::cleanup_playerimmovables_area(const Area area) {
	std::vector<ImmovableFound> immovables;
	std::set<PlayerImmovable*> burnset;
	Map & m = *m_map;

	// Find all immovables that need fixing
	m.find_immovables(area, area.radius, &immovables, FindImmovablePlayerImmovable());

	for
		(std::vector<ImmovableFound>::const_iterator it = immovables.begin();
		 it != immovables.end(); ++it) {
			 PlayerImmovable & imm =
				 *static_cast<PlayerImmovable * const>(it->object);
		if
			(not m.get_field(it->coords)->is_interior
			 (imm.get_owner()->get_player_number()))
			burnset.insert(&imm);
	}

	// Fix all immovables
	Game * const game = dynamic_cast<Game * const>(this);
	for(std::set<PlayerImmovable*>::iterator it = burnset.begin(); it != burnset.end(); ++it)
	{
		if (game) (*it)->schedule_destroy(game);
		else      (*it)->remove(this);
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
	assert(1 <= plnum);
	assert     (plnum <= MAX_PLAYERS);

		delete m_players[plnum-1];
		m_players[plnum-1] = 0;
}


/*
===============
Editor_Game_Base::add_player

Create the player structure for the given plnum.
Note that AI player structures and the Interactive_Player are created when
the game starts. Similar for remote players.
===============
*/
Player * Editor_Game_Base::add_player
(const Player_Number player_number,
 const int type,
 const std::string & tribe,
 const std::string & name)
{
	assert(1 <= player_number);
	assert(player_number <= MAX_PLAYERS);

   if (m_players[player_number - 1]) remove_player(player_number);

   // Get the player's tribe
   uint i;

	manually_load_tribe(tribe.c_str());

   for(i = 0; i < m_tribes.size(); i++)
		if (m_tribes[i]->get_name() == tribe) break;

   if (i == m_tribes.size())
      m_tribes.push_back(new Tribe_Descr(tribe));

   return
		m_players[player_number - 1]
		=
		new Player
		(*this,
		 type,
		 player_number,
		 *m_tribes[i],
		 name,
		 g_playercolors[player_number - 1]);
}

/*
 * Load the given tribe into structure
 */
void Editor_Game_Base::manually_load_tribe(const std::string & tribe) {
	uint i;

	for(i = 0; i < m_tribes.size(); i++)
		if (m_tribes[i]->get_name() == tribe) break;

	if (i == m_tribes.size())
		m_tribes.push_back(new Tribe_Descr(tribe));
}

/*
 * Returns a tribe description from the internally loaded list
 */
Tribe_Descr * Editor_Game_Base::get_tribe(const char * const tribe) const {
	uint i;
   for(i = 0; i < m_tribes.size(); i++) {
		if (not strcmp(m_tribes[i]->get_name().c_str(), tribe))
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
void Editor_Game_Base::set_map(Map * const new_map) {
	assert(new_map != m_map);
		delete m_map;

	m_map = new_map;

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

		if
			(pid <= MAX_PLAYERS
			 or
			 not dynamic_cast<const Game * const>(this))
		{ // if this is editor, load the tribe anyways
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
	m_map->load_graphics(); // especially loads world data

	const std::vector<Tribe_Descr*>::const_iterator tribes_end = m_tribes.end();
	for
		(std::vector<Tribe_Descr*>::const_iterator it = m_tribes.begin();
		 it != tribes_end;
		 ++it)
		(*it)->load_graphics();

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
Building * Editor_Game_Base::warp_building
(const Coords c,
 const Player_Number owner,
 const Building_Descr::Index i)
{
	Player & plr = player(owner);
	return plr.tribe().get_building_descr(i)->create(*this, plr, c, false);
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
	Player & plr = player(owner);

	descr = plr.tribe().get_building_descr(idx);
   if (old_id!=-1) old_descr = plr.tribe().get_building_descr(old_id);
	assert(descr);

	return descr->create(*this, plr, c, true, old_descr);
}


/*
===============
Editor_Game_Base::create_bob

Instantly create a bob at the given x/y location.

idx is the bob type.
===============
*/
Bob * Editor_Game_Base::create_bob
(const Coords c, const Bob_Descr::Index idx, const Tribe_Descr * const tribe)
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
Immovable *Editor_Game_Base::create_immovable
(const Coords c, int idx, const Tribe_Descr* tribe)
{
	Immovable_Descr *descr;

   if(!tribe)
      descr = m_map->get_world()->get_immovable_descr(idx);
   else
      descr = tribe->get_immovable_descr(idx);
	assert(descr);

	return descr->create(this, c);
}

Immovable* Editor_Game_Base::create_immovable
(const Coords c, const std::string & name, const Tribe_Descr* tribe)
{
	int idx;

   if(!tribe)
      idx = m_map->get_world()->get_immovable_index(name.c_str());
   else {
      idx = tribe->get_immovable_index(name.c_str());
   }

	if (idx < 0)
		throw wexception("Editor_Game_Base::create_immovable(%i, %i): %s is not defined for %s",
								c.x, c.y, name.c_str(), tribe ? tribe->get_name().c_str() : "world");

	return create_immovable(c, idx, tribe);
}

Battle* Editor_Game_Base::create_battle ()
{
   Battle* b = new Battle ();
   b->init (this);
   m_battle_serials.push_back (b->get_serial());
   return b;
}

AttackController* Editor_Game_Base::create_attack_controller(Flag* flag,int attacker, int defender, uint num) {
   uint i;
   for (i=0;i<m_attack_serials.size();i++) {
      AttackController* curCtrl = (AttackController*)this->get_objects()->get_object(m_attack_serials[i]);
      if (curCtrl->getFlag() == flag) {
         curCtrl->launchAttack(num);
         return curCtrl;
      }
   }

   AttackController* ctrl = new AttackController((Game*)this,flag,attacker,defender);
   ctrl->launchAttack(num);
   m_attack_serials.push_back(ctrl->get_serial());
   return ctrl;
}

AttackController* Editor_Game_Base::create_attack_controller() {
	AttackController * const ctrl =
		new AttackController(static_cast<Game &>(*this));
   m_attack_serials.push_back(ctrl->get_serial());
   return ctrl;
}

void Editor_Game_Base::remove_attack_controller(uint serial) {
   for(uint i=0;i<m_attack_serials.size();i++) {
      if (m_attack_serials[i] == serial) {
         log("Editor_Game_Base: Destroying battle with serial %i \n",serial);
			static_cast<AttackController * const>(this->get_objects()->get_object(serial))->destroy(this);

			if (i < m_attack_serials.size() - 1)
				m_attack_serials[i] = m_attack_serials[m_attack_serials.size() - 1];
			m_attack_serials.pop_back();
			return;
      }
   }
}

/*
================
Returns the correct player, creates it
with the scenario data when he is not yet created
This should only happen in the editor.
In the game, this is the same as get_player(). If it returns
zero it means that this player is disabled in the game.
================
*/
Player * Editor_Game_Base::get_safe_player(const int n) {return get_player(n);}

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
void Editor_Game_Base::cleanup_for_load
(const bool flush_graphics, const bool flush_animations)
{
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
}

/**
 * Editor_Game_Base::get_attack_points
 *
 *    Returns all the positions of MilitarySites (buildings that can be attacked) of
 * a given player. If the player has no buildings that can be attacked, rare, then
 * returns 0
 */
std::vector<Coords>*
Editor_Game_Base::get_attack_points(const Player_Number player_number)
{
   std::vector<Coords>* tmp = 0;

	const std::vector<Player_Area>::const_iterator  conquer_info_end =
		m_conquer_info.end();
	for
		(std::vector<Player_Area>::const_iterator it = m_conquer_info.begin();
		 it != conquer_info_end;
		 ++it)
	{
		if (it->player_number == player_number) { // First initialization
         if (!tmp)
            tmp = new std::vector<Coords>;

			tmp->push_back (*it);

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

      // Clean influce maps
   memset (m_conquer_map, 0, sizeof (m_conquer_map));

	const std::vector<Player_Area>::const_iterator conquer_info_end =
		m_conquer_info.end();
	for
		(std::vector<Player_Area>::const_iterator it = m_conquer_info.begin();
		 it != conquer_info_end;
		 ++it)
	{
         // First, update influence map of the player
		MapRegion mr(*m_map, *it);
		do m_conquer_map[it->player_number][m_map->get_index(mr.location())] +=
			calc_influence(mr.location(), *it);
		while (mr.advance(*m_map));
	}

   // Now create the real influence map !
	const Player_Number nr_players = map().get_nrplayers();
	const Map::Index max_index = map().max_index();
	for (Map::Index index = 0; index < max_index; ++index) {
		Player_Number best_player = 0;
		Influence best_value = 0;
            // Find the most player influence over this position
		for (Player_Number plnum = 1; plnum <= nr_players; ++plnum)
			if (const Influence value = m_conquer_map[plnum][index])
				if        (value >  best_value) {
					best_value = value;
					best_player = plnum;
				} else if (value == best_value) {
					best_player = 0;
					const Coords c = map().get_fcoords(map()[index]);
					log
						("Editor_Game_Base::make_influence_map: Players %u and %u "
						 "both have influence %i at (%i, %i), location will be "
						 "neutral (unless another player has higher influence).\n",
						 best_player, plnum, value, c.x, c.y);
				}
         m_conquer_map[0][index] = best_value;
   }
}
// END Support for influece map in load/saved games
