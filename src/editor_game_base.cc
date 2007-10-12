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

#include "editor_game_base.h"

#include "areawatcher.h"
#include "attack_controller.h"
#include "battle.h"
#include "building.h"
#include "font_handler.h"
#include "game.h"
#include "graphic.h"
#include "i18n.h"
#include "instances.h"
#include "interactive_base.h"
#include "mapregion.h"
#include "player.h"
#include "tribe.h"
#include "wexception.h"
#include "worker.h"
#include "world.h"

#include "ui_progresswindow.h"

#include <set>

extern Map_Object_Descr g_road_descr;

// hard-coded playercolors
const uint8_t g_playercolors[MAX_PLAYERS][12] = {
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
Editor_Game_Base::Editor_Game_Base() :
m_gametime          (0),
m_iabase            (0),
m_map               (0),
m_lasttrackserial   (0)
{
	memset(m_players, 0, sizeof(m_players));
}

/*
============
Editor_Game_Base::~Editor_Game_Base()

last few cleanups
============
*/
Editor_Game_Base::~Editor_Game_Base() {
	const Player * const * const players_end = m_players + MAX_PLAYERS;
	for (Player * * p = m_players; p < players_end; ++p)
		delete *p;

		delete m_map;

	const std::vector<Tribe_Descr*>::const_iterator tribes_end = m_tribes.end();
	for
		(std::vector<Tribe_Descr*>::const_iterator it = m_tribes.begin();
		 it != tribes_end;
		 ++it)
		delete *it;
}

void Editor_Game_Base::think()
{
	//TODO: Get rid of this; replace by a function that just advances gametime
	// by a given number of milliseconds
}

void Editor_Game_Base::player_immovable_notification (PlayerImmovable*, losegain_t)
{
	//TODO: Get rid of this; replace by general notification system
}

void Editor_Game_Base::player_field_notification (const FCoords&, losegain_t)
{
	//TODO: Get rid of this; replace by general notification system
}

/*
===============
This unconquers a area. This is only possible, when there
is a building placed on this field
===============
*/
void Editor_Game_Base::unconquer_area
(Player_Area<Area<FCoords> > player_area, const Player_Number destroying_player)
{
	assert(0 <= player_area.x);
	assert     (player_area.x < map().get_width());
	assert(0 <= player_area.y);
	assert     (player_area.y < map().get_height());
	const Field & first_field = map()[0];
	assert(&first_field <= player_area.field);
	assert                (player_area.field < &first_field + map().max_index());
	assert(0 < player_area.player_number);
	assert    (player_area.player_number <= map().get_nrplayers());

	//  Here must be a building.
	const Building & building =
		dynamic_cast<const Building &>(*map().get_immovable(player_area));
	assert(building.owner().get_player_number() == player_area.player_number);

   // step 1: unconquer area of this building
	do_conquer_area(player_area, false, destroying_player);

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
void Editor_Game_Base::conquer_area(Player_Area<Area<FCoords> > player_area) {
	assert(0 <= player_area.x);
	assert(player_area.x < map().get_width());
	assert(0 <= player_area.y);
	assert(player_area.y < map().get_height());
	const Field & first_field = map()[0];
	assert(&first_field <= player_area.field);
	assert                (player_area.field < &first_field + map().max_index());
	assert(0 < player_area.player_number);
	assert    (player_area.player_number <= map().get_nrplayers());

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
void Editor_Game_Base::conquer_area_no_building
(const Player_Area<Area<FCoords> > player_area)
{do_conquer_area(player_area, true);}


/**
===============
Conquers the given area for that player; does the actual work
Additionally, it updates the visible area for that player.
===============
*/
void Editor_Game_Base::do_conquer_area
(Player_Area<Area<FCoords> > player_area,
 const bool conquer,
 const Player_Number preferred_player,
 const bool neutral_when_no_influence,
 const bool neutral_when_competing_influence,
 const bool conquer_guarded_location_by_superior_influence)
{
	assert(0 <= player_area.x);
	assert(player_area.x < map().get_width());
	assert(0 <= player_area.y);
	assert(player_area.y < map().get_height());
	const Field & first_field = map()[0];
	assert(&first_field <= player_area.field);
	assert                (player_area.field < &first_field + map().max_index());
	assert(0 < player_area.player_number);
	assert    (player_area.player_number <= map().get_nrplayers());
	assert    (preferred_player          <= map().get_nrplayers());
	assert(preferred_player != player_area.player_number);
	assert(not conquer or not preferred_player);
	Player & conquering_player = player(player_area.player_number);
	MapRegion<Area<FCoords> > mr(map(), player_area);
	do {
		const Map::Index index = mr.location().field - &first_field;
		const Military_Influence influence =
			map().calc_influence
			(mr.location(), Area<>(player_area, player_area.radius));

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
			Military_Influence new_influence_modified =
				conquering_player.military_influence(index) += influence;
			if (owner and not conquer_guarded_location_by_superior_influence)
				new_influence_modified = 1;
			if
				(not owner
				 or
				 player(owner).military_influence(index) < new_influence_modified)
			{
				if (owner) player_field_notification(mr.location(), LOSE);
				mr.location().field->set_owned_by(player_area.player_number);
				inform_players_about_ownership(index, player_area.player_number);
				player_field_notification (mr.location(), GAIN);
			}
		} else if
			(not (conquering_player.military_influence(index) -= influence)
			 and
			 owner == player_area.player_number)
		{
			//  The player completely lost influence over the location, which he
			//  owned. Now we must see if some other player has influence and if
			//  so, transfer the ownership to that player.
			Player_Number best_player;
			if
				(preferred_player
				 and
				 player(preferred_player).military_influence(index))
				best_player = preferred_player;
			else {
				best_player =
					neutral_when_no_influence ? 0 : player_area.player_number;
				Military_Influence highest_military_influence = 0;
				const Player_Number nr_players = map().get_nrplayers();
				iterate_players_existing_const(p, nr_players, *this, plr) {
					if
						(const Military_Influence value =
						 plr->military_influence(index))
						if        (value >  highest_military_influence) {
							highest_military_influence = value;
							best_player = p;
						} else if (value == highest_military_influence) {
							const Coords c = map().get_fcoords(map()[index]);
							best_player = neutral_when_competing_influence ?
								0 : player_area.player_number;
						}
				}
			}
			if (best_player != player_area.player_number) {
				player_field_notification (mr.location(), LOSE);
				mr.location().field->set_owned_by (best_player);
				inform_players_about_ownership(index, best_player);
				if (best_player) player_field_notification (mr.location(), GAIN);
			}
		}
	} while (mr.advance(map()));

	//  This must reach one step beyond the conquered area to adjust the borders
	//  of neighbour players.
	++player_area.radius;
	map().recalc_for_field_area(player_area);
}


/*
===============
Editor_Game_Base::cleanup_playerimmovables_area

Make sure that buildings cannot exist outside their owner's territory.
===============
*/
void Editor_Game_Base::cleanup_playerimmovables_area(const Area<FCoords> area)
{
	std::vector<ImmovableFound> immovables;
	std::vector<PlayerImmovable*> burnlist;
	Map & m = *m_map;

	// Find all immovables that need fixing
	m.find_immovables(area, &immovables, FindImmovablePlayerImmovable());

	for
		(std::vector<ImmovableFound>::const_iterator it = immovables.begin();
		 it != immovables.end(); ++it)
	{
		PlayerImmovable & imm = dynamic_cast<PlayerImmovable &>(*it->object);
		if
			(not m.get_field(it->coords)->is_interior
			 (imm.get_owner()->get_player_number()))
		{
			if (std::find(burnlist.begin(), burnlist.end(), &imm) == burnlist.end())
				burnlist.push_back(&imm);
		}
	}

	// Fix all immovables
	if (Game * const game = dynamic_cast<Game *>(this))
		for
			(std::vector<PlayerImmovable *>::const_iterator it = burnlist.begin();
			 it != burnlist.end();
			 ++it)
			(*it)->schedule_destroy(game);
	else
		for
			(std::vector<PlayerImmovable *>::const_iterator it = burnlist.begin();
			 it != burnlist.end();
			 ++it)
			(*it)->remove(this);
}


/*
===============
Editor_Game_Base::remove_player

Remove the player with the given number
===============
*/
void Editor_Game_Base::remove_player(int32_t plnum)
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
 const int32_t type,
 const std::string & tribe,
 const std::string & name)
{
	assert(1 <= player_number);
	assert(player_number <= MAX_PLAYERS);

   if (m_players[player_number - 1]) remove_player(player_number);

   // Get the player's tribe
   uint32_t i;

	manually_load_tribe(tribe.c_str());

	for (i = 0; i < m_tribes.size(); ++i)
		if (m_tribes[i]->name() == tribe) break;

   if (i == m_tribes.size())
      m_tribes.push_back(new Tribe_Descr(tribe, map().world()));

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
	uint32_t i;

	for (i = 0; i < m_tribes.size(); ++i)
		if (m_tribes[i]->name() == tribe) break;

	if (i == m_tribes.size())
		m_tribes.push_back(new Tribe_Descr(tribe, map().world()));
}

/*
 * Returns a tribe description from the internally loaded list
 */
Tribe_Descr * Editor_Game_Base::get_tribe(const char * const tribe) const {
	uint32_t i;
   for (i = 0; i < m_tribes.size(); ++i) {
		if (not strcmp(m_tribes[i]->name().c_str(), tribe))
			return m_tribes[i];
	}
   return 0;
}

void Editor_Game_Base::inform_players_about_ownership
(const Map::Index i, const Player_Number new_owner)
{
	for (Player_Number plnum = 0; plnum < MAX_PLAYERS; ++plnum)
		if (Player * const p = m_players[plnum]) {
			Player::Field & player_field = p->m_fields[i];
			if (1 < player_field.vision) player_field.owner = new_owner;
		}
}
void Editor_Game_Base::inform_players_about_immovable
(const Map::Index i, const Map_Object_Descr * const descr)
{
	if (descr != &g_road_descr)
		for (Player_Number plnum = 0; plnum < MAX_PLAYERS; ++plnum)
			if (Player * const p = m_players[plnum]) {
				Player::Field & player_field = p->m_fields[i];
				if (1 < player_field.vision)
					player_field.map_object_descr[TCoords<>::None] = descr;
			}
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
   g_fh->register_variable_callback(g_MapVariableCallback, m_map);
}


void Editor_Game_Base::allocate_player_maps() {
	for (Player_Number i = 0; i < MAX_PLAYERS; ++i)
		if (m_players[i]) m_players[i]->allocate_map();
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
	uint32_t id;
	int32_t pid;

	// Postload tribes
	id = 0;
	while (id < m_tribes.size()) {
		for (pid = 1; pid <= MAX_PLAYERS; ++pid)
			if (const Player * const plr = get_player(pid))
				if (&plr->tribe() == m_tribes[id])
					break;

		if
			(pid <= MAX_PLAYERS
			 or
			 not dynamic_cast<const Game *>(this))
		{ // if this is editor, load the tribe anyways
			// the tribe is used, postload it
			m_tribes[id]->postload(this);
			++id;
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
void Editor_Game_Base::load_graphics(UI::ProgressWindow & loader_ui) {
	loader_ui.step(_("Loading world data"));
	m_map->load_graphics(); // especially loads world data

	const std::vector<Tribe_Descr*>::const_iterator tribes_end = m_tribes.end();
	for
		(std::vector<Tribe_Descr*>::const_iterator it = m_tribes.begin();
		 it != tribes_end;
		 ++it)
	{
		loader_ui.stepf(_("Loading tribe: %s"), (*it)->name().c_str());
		(*it)->load_graphics();
	}

	// TODO: load player graphics? (maybe)

	g_gr->load_animations(loader_ui);
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
Building* Editor_Game_Base::warp_constructionsite(Coords c, int8_t owner, int32_t idx, int32_t old_id)
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
(const Coords c, const Bob::Descr::Index idx, const Tribe_Descr * const tribe)
{
	Bob::Descr *descr;

   if (!tribe)
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
(const Coords c, int32_t idx, const Tribe_Descr* tribe)
{
	Immovable_Descr *descr;

   if (!tribe)
      descr = m_map->get_world()->get_immovable_descr(idx);
   else
      descr = tribe->get_immovable_descr(idx);
	assert(descr);

	inform_players_about_immovable(Map::get_index(c, map().get_width()), descr);
	return descr->create(this, c);
}

Immovable* Editor_Game_Base::create_immovable
(const Coords c, const std::string & name, const Tribe_Descr* tribe)
{
	const int32_t idx =
		tribe ?
		tribe->get_immovable_index(name.c_str())
		:
		m_map->get_world()->get_immovable_index(name.c_str());
	if (idx < 0)
		throw wexception
			("Editor_Game_Base::create_immovable(%i, %i): %s is not defined for "
			 "%s",
			 c.x, c.y, name.c_str(), tribe ? tribe->name().c_str() : "world");

	return create_immovable(c, idx, tribe);
}

Battle* Editor_Game_Base::create_battle ()
{
   Battle* b = new Battle ();
   b->init (this);
   m_battle_serials.push_back (b->get_serial());
   return b;
}

AttackController* Editor_Game_Base::create_attack_controller
		(Flag* flag, int32_t attacker, int32_t defender, uint32_t num)
{
	for (uint32_t i = 0; i < m_attack_serials.size(); ++i) {
		AttackController* curCtrl =
			dynamic_cast<AttackController *>
			(objects().get_object(m_attack_serials[i]));
		if (curCtrl->getFlag() == flag) {
			curCtrl->launchAttack(num);
			return curCtrl;
		}
	}

	AttackController* ctrl = new AttackController((Game*)this, flag, attacker, defender);
	ctrl->init(this);
	ctrl->launchAttack(num);
	m_attack_serials.push_back(ctrl->get_serial());
	return ctrl;
}

AttackController* Editor_Game_Base::create_attack_controller()
{
	AttackController * const ctrl =
		new AttackController(static_cast<Game &>(*this));
	ctrl->init(this);
	m_attack_serials.push_back(ctrl->get_serial());
	return ctrl;
}

void Editor_Game_Base::remove_attack_controller(uint32_t serial)
{
	for (uint32_t i = 0; i < m_attack_serials.size(); ++i) {
		if (m_attack_serials[i] == serial) {
			dynamic_cast<AttackController *>(objects().get_object(serial))
				->destroy(this);

			if (i < m_attack_serials.size() - 1)
				m_attack_serials[i] = m_attack_serials[m_attack_serials.size() - 1];
			m_attack_serials.pop_back();
			return;
		}
	}
}

void Editor_Game_Base::register_attack_controller(AttackController* ctrl)
{
	assert
		(std::find
		 (m_attack_serials.begin(), m_attack_serials.end(), ctrl->get_serial())
			==
		 m_attack_serials.end());

	m_attack_serials.push_back(ctrl->get_serial());
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
Player * Editor_Game_Base::get_safe_player(const int32_t n)
{
	return get_player(n);
}

/*
===============
Editor_Game_Base::add_trackpointer

Add a registered pointer.
Returns the serial number that can be used to retrieve or remove the pointer.
===============
*/
uint32_t Editor_Game_Base::add_trackpointer(void* ptr)
{
	++m_lasttrackserial;

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
void* Editor_Game_Base::get_trackpointer(uint32_t serial)
{
	std::map<uint32_t, void*>::iterator it = m_trackpointers.find(serial);

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
void Editor_Game_Base::remove_trackpointer(uint32_t serial)
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
	cleanup_objects();

	m_battle_serials.clear();
	m_attack_serials.clear();

	// We do not flush the animations in the editor since all tribes are loaded and
	// animations can not change a lot, or?
	// TODO: check this when another world is needed
	if (flush_animations)
		g_anim.flush();
	if (flush_graphics)
		g_gr->flush(0);

	const Player * const * const players_end = m_players + MAX_PLAYERS;
	for (Player * * p = m_players; p < players_end; ++p) {
		delete *p;
		*p = 0;
	}

	m_map->cleanup();
}


void Editor_Game_Base::set_road
		(const FCoords f, const Uint8 direction, const Uint8 roadtype)
{
	const Map & m = map();
	const Field & first_field = m[0];
	assert(0 <= f.x);
	assert(f.x < m.get_width());
	assert(0 <= f.y);
	assert(f.y < m.get_height());
	assert(&first_field <= f.field);
	assert                (f.field < &first_field + m.max_index());
	assert
		(direction == Road_SouthWest or
		 direction == Road_SouthEast or
		 direction == Road_East);
	assert
		(roadtype == Road_None or roadtype == Road_Normal or
		 roadtype == Road_Busy or roadtype == Road_Water);

	if (f.field->get_road(direction) == roadtype) return;
	f.field->set_road(direction, roadtype);

	FCoords neighbour;
	Uint8 mask;
	switch (direction) {
	case Road_SouthWest:
		neighbour = m.bl_n(f);
		mask = Road_Mask << Road_SouthWest;
		break;
	case Road_SouthEast:
		neighbour = m.br_n(f);
		mask = Road_Mask << Road_SouthEast;
		break;
	case Road_East:
		neighbour = m. r_n(f);
		mask = Road_Mask << Road_East;
		break;
	default: assert(false);
	}
	const Uint8 road = f.field->get_roads() & mask;
	const Map::Index           i = f        .field - &first_field;
	const Map::Index neighbour_i = neighbour.field - &first_field;
	for (Player_Number plnum = 0; plnum < MAX_PLAYERS; ++plnum) {
		if (Player * const p = m_players[plnum]) {
			Player::Field & first_player_field = *p->m_fields;
			Player::Field & player_field = (&first_player_field)[i];
			if
				(1 < player_field                      .vision
				 |
				 1 < (&first_player_field)[neighbour_i].vision)
			{
				player_field.roads &= ~mask;
				player_field.roads |= road;
			}
		}
	}
}
