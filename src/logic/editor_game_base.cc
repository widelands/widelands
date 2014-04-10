/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "logic/editor_game_base.h"

#include <algorithm>
#include <set>

#include "economy/flag.h"
#include "economy/road.h"
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "logic/battle.h"
#include "logic/building.h"
#include "logic/dismantlesite.h"
#include "logic/findimmovable.h"
#include "logic/game.h"
#include "logic/instances.h"
#include "logic/mapregion.h"
#include "logic/player.h"
#include "logic/playersmanager.h"
#include "logic/roadtype.h"
#include "logic/tribe.h"
#include "logic/ware_descr.h"
#include "logic/worker.h"
#include "logic/world.h"
#include "rgbcolor.h"
#include "scripting/scripting.h"
#include "sound/sound_handler.h"
#include "ui_basic/progresswindow.h"
#include "upcast.h"
#include "wexception.h"
#include "wui/interactive_base.h"
#include "wui/interactive_gamebase.h"

namespace Widelands {

/*
============
Editor_Game_Base::Editor_Game_Base()

initialization
============
*/
Editor_Game_Base::Editor_Game_Base(LuaInterface * lua_interface) :
m_gametime          (0),
m_lua               (lua_interface),
m_player_manager    (new Players_Manager(*this)),
m_ibase             (nullptr),
m_map               (nullptr),
m_lasttrackserial   (0)
{
	if (not m_lua) // TODO SirVer: this is sooo ugly, I can't say
		m_lua = new LuaEditorInterface(this);

	g_sound_handler.m_egbase = this;

}


Editor_Game_Base::~Editor_Game_Base() {
	delete m_map;
	delete m_player_manager.release();

	container_iterate_const(Tribe_Vector, m_tribes, i)
		delete *i.current;

	delete m_lua;

	if (g_gr) { // dedicated does not use the sound_handler
		assert(this == g_sound_handler.m_egbase);
		g_sound_handler.m_egbase = nullptr;
	}
}

void Editor_Game_Base::think()
{
	//TODO: Get rid of this; replace by a function that just advances gametime
	// by a given number of milliseconds
}

void Editor_Game_Base::receive(const NoteImmovable & note)
{
	note.pi->owner().receive(note);
}

void Editor_Game_Base::receive(const NoteFieldPossession & note)
{
	get_player(note.fc.field->get_owned_by())->receive(note);
}

void Editor_Game_Base::receive(const NoteFieldTransformed & note)
{
	Widelands::Map_Index const i = note.fc.field - &(*m_map)[0];

	iterate_players_existing(p, m_map->get_nrplayers(), *this, plr)
		if (plr->vision(i) > 1) // player currently sees field?
			plr->rediscover_node(*m_map, (*m_map)[0], note.fc);
}

Interactive_GameBase* Editor_Game_Base::get_igbase()
{
	return dynamic_cast<Interactive_GameBase *>(get_ibase());
}

/// @see PlayerManager class
void Editor_Game_Base::remove_player(Player_Number plnum) {
	m_player_manager->remove_player(plnum);
}

/// @see PlayerManager class
Player * Editor_Game_Base::add_player
	(Player_Number       const player_number,
	 uint8_t             const initialization_index,
	 const std::string &       tribe,
	 const std::string &       name,
	 TeamNumber                team)
{
	return
		m_player_manager->add_player
			(player_number, initialization_index, tribe,
			name, team);
}

/// Load the given tribe into structure
const Tribe_Descr & Editor_Game_Base::manually_load_tribe
	(const std::string & tribe)
{
	container_iterate_const(Tribe_Vector, m_tribes, i)
		if ((*i.current)->name() == tribe)
			return **i.current;

	if (not map().get_world())
		map().load_world();
	assert(map().get_world());
	Tribe_Descr & result = *new Tribe_Descr(tribe, *this);
	//resize the configuration of our wares if they won't fit in the current window (12 = info label size)
	int number = (g_gr->get_yres() - 270) / (WARE_MENU_PIC_HEIGHT + WARE_MENU_PIC_PAD_Y + 12);
	result.resize_ware_orders(number);
	m_tribes.push_back(&result);
	return result;
}

Player* Editor_Game_Base::get_player(const int32_t n) const
{
	return m_player_manager->get_player(n);
}

Player& Editor_Game_Base::player(const int32_t n) const
{
	return m_player_manager->player(n);
}



/// Returns a tribe description from the internally loaded list
const Tribe_Descr * Editor_Game_Base::get_tribe(const char * const tribe) const
{
	container_iterate_const(Tribe_Vector, m_tribes, i)
		if (not strcmp((*i.current)->name().c_str(), tribe))
			return *i.current;
	return nullptr;
}

void Editor_Game_Base::inform_players_about_ownership
	(Map_Index const i, Player_Number const new_owner)
{
	iterate_players_existing_const(plnum, MAX_PLAYERS, *this, p) {
		Player::Field & player_field = p->m_fields[i];
		if (1 < player_field.vision) {
			player_field.owner = new_owner;
		}
	}
}
void Editor_Game_Base::inform_players_about_immovable
	(Map_Index const i, Map_Object_Descr const * const descr)
{
	if (not Road::IsRoadDescr(descr))
		iterate_players_existing_const(plnum, MAX_PLAYERS, *this, p) {
			Player::Field & player_field = p->m_fields[i];
			if (1 < player_field.vision) {
				player_field.map_object_descr[TCoords<>::None] = descr;
			}
		}
}

/**
 * Replaces the current map with the given one. Ownership of the map is transferred
 * to the Editor_Game_Base object.
 */
void Editor_Game_Base::set_map(Map * const new_map) {
	assert(new_map != m_map);
	assert(new_map);

	delete m_map;

	m_map = new_map;

	// if this map is already completely loaded, we better inform g_gr about the change of the world
	// to (re)load the correct road textures.
	if (g_gr && strcmp(m_map->get_world_name(), ""))
		g_gr->set_world(m_map->get_world_name());

	NoteReceiver<NoteFieldTransformed>::connect(*m_map);
}


void Editor_Game_Base::allocate_player_maps() {
	iterate_players_existing(plnum, MAX_PLAYERS, *this, p) {
		p->allocate_map();
	}
}


/**
 * Load and prepare detailled game data.
 * This happens once just after the host has started the game and before the
 * graphics are loaded.
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
			m_tribes[id]->postload(*this);
			++id;
		} else {
			delete m_tribes[id]; // the tribe is no longer used, remove it
			m_tribes.erase(m_tribes.begin() + id);
		}
	}

	// TODO: postload players? (maybe)
}


/**
 * Load all graphics.
 * This function needs to be called once at startup when the graphics system is ready.
 * If the graphics system is to be replaced at runtime, the function must be called after that has happened.
 */
void Editor_Game_Base::load_graphics(UI::ProgressWindow & loader_ui)
{
	loader_ui.step(_("Loading world data"));

	g_gr->set_world(m_map->get_world_name());
	m_map->load_graphics(); // especially loads world data

	container_iterate_const(Tribe_Vector, m_tribes, i) {
		loader_ui.stepf(_("Loading tribes"));
		(*i.current)->load_graphics();
	}

	// TODO: load player graphics? (maybe)
}

/**
 * Instantly create a building at the given x/y location. There is no build time.
 * \li owner  is the player number of the building's owner.
 * \li idx is the building type index.
 * \li former_buildings is the list of former buildings
 */
Building & Editor_Game_Base::warp_building
	(Coords const c, Player_Number const owner, Building_Index const idx,
		Building::FormerBuildings former_buildings)
{
	Player & plr = player(owner);
	const Tribe_Descr & tribe = plr.tribe();
	return
		tribe.get_building_descr(idx)->create
			(*this, plr, c, false, true, former_buildings);
}


/**
 * Create a building site at the given x/y location for the given building type.
 *
 * \li idx : the building index of the building in construction
 * \li former_buildings : the former buildings. If it is not empty, this is
 * an enhancement.
 */
Building & Editor_Game_Base::warp_constructionsite
	(Coords const c, Player_Number const owner,
	 Building_Index idx, bool loading,
	 Building::FormerBuildings former_buildings)
{
	Player            & plr   = player(owner);
	const Tribe_Descr & tribe = plr.tribe();
	return
		tribe.get_building_descr(idx)->create
			(*this, plr, c, true, loading, former_buildings);
}

/**
 * Create a dismantle site
 * \li former_buildings : the former buildings list. This should not be empty,
 * except during loading.
 */
Building & Editor_Game_Base::warp_dismantlesite
	(Coords const c, Player_Number const owner,
	 bool loading, Building::FormerBuildings former_buildings)
{
	Player            & plr   = player(owner);
	const Tribe_Descr & tribe = plr.tribe();

	Building_Descr const * const descr =
		tribe.get_building_descr
			(tribe.safe_building_index("dismantlesite"));

	upcast(const DismantleSite_Descr, ds_descr, descr);

	return
		*new DismantleSite
			(*ds_descr, *this, c, *get_player(owner), loading, former_buildings);
}


/**
 * Instantly create a bob at the given x/y location.
 *
 * idx is the bob type.
 */
Bob & Editor_Game_Base::create_bob(Coords c, const BobDescr & descr, Player * owner)
{
	return descr.create(*this, owner, c);
}


Bob & Editor_Game_Base::create_bob
	(Coords const c,
	 BobDescr::Index const idx, Tribe_Descr const * const tribe, Player * owner)
{
	const BobDescr & descr =
		*
		(tribe ?
		 tribe->get_bob_descr(idx)
		 :
		 m_map->get_world()->get_bob_descr(idx));

	return create_bob(c, descr, owner);
}

Bob & Editor_Game_Base::create_bob
	(Coords c, const std::string & name, const Widelands::Tribe_Descr * const tribe,
	 Player * owner)
{
	const BobDescr * descr =
		tribe ?
		tribe->get_bob_descr(name) :
		m_map->get_world()->get_bob_descr(name);

	if (!descr)
		throw wexception
			("create_bob(%i,%i,%s,%s): bob not found",
			 c.x, c.y, name.c_str(), tribe ? tribe->name().c_str() : "world");

	return create_bob(c, *descr, owner);
}


/*
===============
Create an immovable at the given location.
If tribe is not zero, create a immovable of a player (not a PlayerImmovable
but an immovable defined by the players tribe)
Does not perform any placability checks.
===============
*/
Immovable & Editor_Game_Base::create_immovable
	(Coords const c, uint32_t const idx, Tribe_Descr const * const tribe)
{
	const Immovable_Descr & descr =
		*
		(tribe ?
		 tribe->get_immovable_descr(idx)
		 :
		 m_map->world().get_immovable_descr(idx));
	assert(&descr);
	inform_players_about_immovable
		(Map::get_index(c, map().get_width()), &descr);
	return descr.create(*this, c);
}

Immovable & Editor_Game_Base::create_immovable
	(Coords const c, const std::string & name, Tribe_Descr const * const tribe)
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

/*
================
Returns the correct player, creates it
with the scenario data when he is not yet created
This should only happen in the editor.
In the game, this is the same as get_player(). If it returns
zero it means that this player is disabled in the game.
================
*/
Player * Editor_Game_Base::get_safe_player(Player_Number const n) {
	return get_player(n);
}

/*
===============
Add a registered pointer.
Returns the serial number that can be used to retrieve or remove the pointer.
===============
*/
uint32_t Editor_Game_Base::add_trackpointer(void * const ptr)
{
	++m_lasttrackserial;

	if (!m_lasttrackserial)
		throw wexception("Dude, you play too long. Track serials exceeded.");

	m_trackpointers[m_lasttrackserial] = ptr;
	return m_lasttrackserial;
}


/*
===============
Retrieve a previously stored pointer using the serial number.
Returns 0 if the pointer has been removed.
===============
*/
void * Editor_Game_Base::get_trackpointer(uint32_t const serial)
{
	std::map<uint32_t, void *>::iterator it = m_trackpointers.find(serial);

	if (it != m_trackpointers.end())
		return it->second;

	return nullptr;
}


/*
===============
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
void Editor_Game_Base::cleanup_for_load()
{
	cleanup_objects(); /// Clean all the stuff up, so we can load.
	m_player_manager->cleanup();

	if (m_map)
		m_map->cleanup();
}


void Editor_Game_Base::set_road
	(FCoords const f, uint8_t const direction, uint8_t const roadtype)
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

	if (f.field->get_road(direction) == roadtype)
		return;
	f.field->set_road(direction, roadtype);

	FCoords neighbour;
	uint8_t mask;
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
	default:
		assert(false);
		break;
	}
	uint8_t const road = f.field->get_roads() & mask;
	Map_Index const           i = f        .field - &first_field;
	Map_Index const neighbour_i = neighbour.field - &first_field;
	iterate_players_existing_const(plnum, MAX_PLAYERS, *this, p) {
		Player::Field & first_player_field = *p->m_fields;
		Player::Field & player_field = (&first_player_field)[i];
		if
			(1 < player_field                      .vision
				or
				1 < (&first_player_field)[neighbour_i].vision)
		{
			player_field.roads &= ~mask;
			player_field.roads |= road;
		}
	}
}

/// This unconquers an area. This is only possible, when there is a building
/// placed on this node.
void Editor_Game_Base::unconquer_area
	(Player_Area<Area<FCoords> > player_area,
	 Player_Number         const destroying_player)
{
	assert(0 <= player_area.x);
	assert     (player_area.x < map().get_width());
	assert(0 <= player_area.y);
	assert     (player_area.y < map().get_height());
	assert(&map()[0] <= player_area.field);
	assert             (player_area.field < &map()[map().max_index()]);
	assert(0 < player_area.player_number);
	assert    (player_area.player_number <= map().get_nrplayers());

	//  Here must be a building.
	assert
		(dynamic_cast<const Building &>(*map().get_immovable(player_area))
		 .owner().player_number()
		 ==
		 player_area.player_number);

	//  step 1: unconquer area of this building
	do_conquer_area(player_area, false, destroying_player);

	//  step 5: deal with player immovables in the lost area
	//  Players are not allowed to have their immovables on their borders.
	//  Therefore the area must be enlarged before calling
	//  cleanup_playerimmovables_area, so that those new border locations are
	//  covered.
	// SirVer, TODO: In the editor, no buildings should burn down when a military
	// building is removed. Check this again though
	if (is_a(Game, this)) {
		++player_area.radius;
		player_area.player_number = destroying_player;
		cleanup_playerimmovables_area(player_area);
	}
}

/// This conquers a given area because of a new (military) building that is set
/// there.
void Editor_Game_Base::conquer_area(Player_Area<Area<FCoords> > player_area) {
	assert(0 <= player_area.x);
	assert     (player_area.x < map().get_width());
	assert(0 <= player_area.y);
	assert     (player_area.y < map().get_height());
	assert(&map()[0] <= player_area.field);
	assert             (player_area.field < &map()[map().max_index()]);
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


void Editor_Game_Base::conquer_area_no_building
	(Player_Area<Area<FCoords> > player_area)
{
	assert(0 <= player_area.x);
	assert     (player_area.x < map().get_width());
	assert(0 <= player_area.y);
	assert     (player_area.y < map().get_height());
	const Field & first_field = map()[0];
	assert(&first_field <= player_area.field);
	assert(player_area.field < &first_field + map().max_index());
	assert(0 < player_area.player_number);
	assert    (player_area.player_number <= map().get_nrplayers());
	MapRegion<Area<FCoords> > mr(map(), player_area);
	do {
		Player_Number const owner = mr.location().field->get_owned_by();
		if (owner != player_area.player_number) {
			if (owner)
				receive(NoteFieldPossession(mr.location(), LOSE));
			mr.location().field->set_owned_by(player_area.player_number);
			inform_players_about_ownership
				(mr.location().field - &first_field, player_area.player_number);
			receive (NoteFieldPossession(mr.location(), GAIN));
		}
	} while (mr.advance(map()));

	//  This must reach one step beyond the conquered area to adjust the borders
	//  of neighbour players.
	++player_area.radius;
	map().recalc_for_field_area(player_area);
}


/// Conquers the given area for that player; does the actual work.
/// Additionally, it updates the visible area for that player.
// TODO: this needs a more fine grained refactoring
// for example scripts will want to (un)conquer area of non oval shape
// or give area back to the neutral player (this is very important for the Lua
// testsuite).
void Editor_Game_Base::do_conquer_area
	(Player_Area<Area<FCoords> > player_area,
	 bool          const conquer,
	 Player_Number const preferred_player,
	 bool          const neutral_when_no_influence,
	 bool          const neutral_when_competing_influence,
	 bool          const conquer_guarded_location_by_superior_influence)
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
		Map_Index const index = mr.location().field - &first_field;
		Military_Influence const influence =
			map().calc_influence
				(mr.location(), Area<>(player_area, player_area.radius));

		Player_Number const owner = mr.location().field->get_owned_by();
		if (conquer) {
			//  adds the influence
			Military_Influence new_influence_modified =
				conquering_player.military_influence(index) += influence;
			if (owner and not conquer_guarded_location_by_superior_influence)
				new_influence_modified = 1;
			if
				(not owner
				 or
				 player(owner).military_influence(index) < new_influence_modified)
			{
				if (owner)
					receive(NoteFieldPossession(mr.location(), LOSE));
				mr.location().field->set_owned_by(player_area.player_number);
				inform_players_about_ownership(index, player_area.player_number);
				receive (NoteFieldPossession(mr.location(), GAIN));
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
				Player_Number const nr_players = map().get_nrplayers();
				iterate_players_existing_const(p, nr_players, *this, plr) {
					if
						(Military_Influence const value =
						 	plr->military_influence(index))
					{
						if        (value >  highest_military_influence) {
							highest_military_influence = value;
							best_player = p;
						} else if (value == highest_military_influence) {
							best_player = neutral_when_competing_influence ?
								0 : player_area.player_number;
						}
					}
				}
			}
			if (best_player != player_area.player_number) {
				receive (NoteFieldPossession(mr.location(), LOSE));
				mr.location().field->set_owned_by (best_player);
				inform_players_about_ownership(index, best_player);
				if (best_player)
					receive (NoteFieldPossession(mr.location(), GAIN));
			}
		}
	} while (mr.advance(map()));

	//  This must reach one step beyond the conquered area to adjust the borders
	//  of neighbour players.
	++player_area.radius;
	map().recalc_for_field_area(player_area);
}

/// Makes sure that buildings cannot exist outside their owner's territory.
void Editor_Game_Base::cleanup_playerimmovables_area
	(Player_Area<Area<FCoords> > const area)
{
	std::vector<ImmovableFound> immovables;
	std::vector<PlayerImmovable *> burnlist;
	Map & m = map();

	//  find all immovables that need fixing
	m.find_immovables(area, &immovables, FindImmovablePlayerImmovable());

	container_iterate_const(std::vector<ImmovableFound>, immovables, i) {
		PlayerImmovable & imm =
			ref_cast<PlayerImmovable, BaseImmovable>(*i.current->object);
		if
			(not
			 m[i.current->coords].is_interior(imm.owner().player_number()))
			if
				(std::find(burnlist.begin(), burnlist.end(), &imm)
				 ==
				 burnlist.end())
				burnlist.push_back(&imm);
	}

	//  fix all immovables
	upcast(Game, game, this);
	container_iterate_const(std::vector<PlayerImmovable *>, burnlist, i) {
		if (upcast(Building, building, *i.current))
			building->set_defeating_player(area.player_number);
		else if (upcast(Flag,     flag,     *i.current))
			if (Building * const flag_building = flag->get_building())
				flag_building->set_defeating_player(area.player_number);
		if (game)
			(*i.current)->schedule_destroy(*game);
		else
			(*i.current)->remove(*this);
	}
}


}
