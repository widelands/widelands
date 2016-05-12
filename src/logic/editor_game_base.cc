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
rnrnrn * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "logic/editor_game_base.h"

#include <algorithm>
#include <memory>
#include <set>

#include "base/i18n.h"
#include "base/macros.h"
#include "base/scoped_timer.h"
#include "base/wexception.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "graphic/color.h"
#include "graphic/graphic.h"
#include "logic/constants.h"
#include "logic/findimmovable.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/map_object.h"
#include "logic/map_objects/tribes/battle.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/dismantlesite.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/tribes.h"
#include "logic/map_objects/tribes/ware_descr.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/map_objects/world/world.h"
#include "logic/mapregion.h"
#include "logic/player.h"
#include "logic/playersmanager.h"
#include "logic/roadtype.h"
#include "scripting/logic.h"
#include "scripting/lua_table.h"
#include "sound/sound_handler.h"
#include "ui_basic/progresswindow.h"
#include "wui/interactive_base.h"
#include "wui/interactive_gamebase.h"

namespace Widelands {

/*
============
EditorGameBase::EditorGameBase()

initialization
============
*/
EditorGameBase::EditorGameBase(LuaInterface * lua_interface) :
gametime_          (0),
lua_               (lua_interface),
player_manager_    (new PlayersManager(*this)),
ibase_             (nullptr),
map_               (nullptr),
lasttrackserial_   (0)
{
	if (!lua_) // TODO(SirVer): this is sooo ugly, I can't say
		lua_.reset(new LuaEditorInterface(this));

	g_sound_handler.egbase_ = this;

}


EditorGameBase::~EditorGameBase() {
	delete map_;
	delete player_manager_.release();
	g_sound_handler.egbase_ = nullptr;
}

void EditorGameBase::think()
{
	// TODO(unknown): Get rid of this; replace by a function that just advances gametime
	// by a given number of milliseconds
}

const World& EditorGameBase::world() const {
	// Const casts are evil, but this is essentially lazy evaluation and the
	// caller should really not modify this.
	return *const_cast<EditorGameBase*>(this)->mutable_world();
}

World* EditorGameBase::mutable_world() {
	if (!world_) {
		// Lazy initialization of World. We need to create the pointer to the
		// world immediately though, because the lua scripts need to have access
		// to world through this method already.
		ScopedTimer timer("Loading the world took %ums");
		world_.reset(new World());

		try {
			lua_->run_script("world/init.lua");
		} catch (const WException& e) {
			log("Could not read world information: %s", e.what());
			throw;
		}

		world_->load_graphics();
	}
	return world_.get();
}

const Tribes& EditorGameBase::tribes() const {
	// Const casts are evil, but this is essentially lazy evaluation and the
	// caller should really not modify this.
	return *const_cast<EditorGameBase*>(this)->mutable_tribes();
}

Tribes* EditorGameBase::mutable_tribes() {
	if (!tribes_) {
		// We need to make sure that the world is loaded first for some attribute checks in the worker programs.
		world();

		// Lazy initialization of Tribes. We need to create the pointer to the
		// tribe immediately though, because the lua scripts need to have access
		// to tribes through this method already.
		ScopedTimer timer("Loading the tribes took %ums");
		tribes_.reset(new Tribes());

		try {
			lua_->run_script("tribes/init.lua");
		} catch (const WException& e) {
			log("Could not read tribes information: %s", e.what());
			throw;
		}
	}
	return tribes_.get();
}

void EditorGameBase::set_ibase(InteractiveBase* const b) {
	ibase_.reset(b);
}

InteractiveGameBase* EditorGameBase::get_igbase()
{
	return dynamic_cast<InteractiveGameBase *>(get_ibase());
}

/// @see PlayerManager class
void EditorGameBase::remove_player(PlayerNumber plnum) {
	player_manager_->remove_player(plnum);
}

/// @see PlayerManager class
Player * EditorGameBase::add_player
	(PlayerNumber       const player_number,
	 uint8_t             const initialization_index,
	 const std::string &       tribe,
	 const std::string &       name,
	 TeamNumber                team)
{
	return
		player_manager_->add_player
			(player_number, initialization_index, tribe,
			name, team);
}

Player* EditorGameBase::get_player(const int32_t n) const
{
	return player_manager_->get_player(n);
}

Player& EditorGameBase::player(const int32_t n) const
{
	return player_manager_->player(n);
}

void EditorGameBase::inform_players_about_ownership
	(MapIndex const i, PlayerNumber const new_owner)
{
	iterate_players_existing_const(plnum, MAX_PLAYERS, *this, p) {
		Player::Field & player_field = p->fields_[i];
		if (1 < player_field.vision) {
			player_field.owner = new_owner;
		}
	}
}
void EditorGameBase::inform_players_about_immovable
	(MapIndex const i, MapObjectDescr const * const descr)
{
	if (!Road::is_road_descr(descr))
		iterate_players_existing_const(plnum, MAX_PLAYERS, *this, p) {
			Player::Field & player_field = p->fields_[i];
			if (1 < player_field.vision) {
				player_field.map_object_descr[TCoords<>::None] = descr;
			}
		}
}

/**
 * Replaces the current map with the given one. Ownership of the map is transferred
 * to the EditorGameBase object.
 */
void EditorGameBase::set_map(Map * const new_map) {
	assert(new_map != map_);
	assert(new_map);

	delete map_;

	map_ = new_map;
}


void EditorGameBase::allocate_player_maps() {
	iterate_players_existing(plnum, MAX_PLAYERS, *this, p) {
		p->allocate_map();
	}
}


/**
 * Load and prepare detailed game data.
 * This happens once just after the host has started the game and before the
 * graphics are loaded.
 */
void EditorGameBase::postload()
{
	// Postload tribes
	assert(tribes_);
	tribes_->postload();

	// TODO(unknown): postload players? (maybe)
}


/**
 * Load all graphics.
 * This function needs to be called once at startup when the graphics system is ready.
 * If the graphics system is to be replaced at runtime, the function must be called after that has happened.
 */
void EditorGameBase::load_graphics(UI::ProgressWindow& loader_ui)
{
	assert(tribes_);
	loader_ui.step(_("Loading graphics"));
	tribes_->load_graphics();
}

/**
 * Instantly create a building at the given x/y location. There is no build time.
 * \li owner  is the player number of the building's owner.
 * \li idx is the building type index.
 * \li former_buildings is the list of former buildings
 */
Building & EditorGameBase::warp_building
	(Coords const c, PlayerNumber const owner, DescriptionIndex const idx,
		Building::FormerBuildings former_buildings)
{
	Player & plr = player(owner);
	const TribeDescr & tribe = plr.tribe();
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
Building & EditorGameBase::warp_constructionsite
	(Coords const c, PlayerNumber const owner,
	 DescriptionIndex idx, bool loading,
	 Building::FormerBuildings former_buildings)
{
	Player            & plr   = player(owner);
	const TribeDescr & tribe = plr.tribe();
	return
		tribe.get_building_descr(idx)->create
			(*this, plr, c, true, loading, former_buildings);
}

/**
 * Create a dismantle site
 * \li former_buildings : the former buildings list. This should not be empty,
 * except during loading.
 */
Building & EditorGameBase::warp_dismantlesite
	(Coords const c, PlayerNumber const owner,
	 bool loading, Building::FormerBuildings former_buildings)
{
	Player            & plr   = player(owner);
	const TribeDescr & tribe = plr.tribe();

	BuildingDescr const * const descr =
		tribe.get_building_descr
			(tribe.safe_building_index("dismantlesite"));

	upcast(const DismantleSiteDescr, ds_descr, descr);

	return
		*new DismantleSite
			(*ds_descr, *this, c, *get_player(owner), loading, former_buildings);
}


/**
 * Instantly create a bob at the given x/y location.
 */
Bob & EditorGameBase::create_bob(Coords c, const BobDescr & descr, Player * owner)
{
	return descr.create(*this, owner, c);
}

/**
 * Instantly create a critter at the given x/y location.
 *
 */

Bob& EditorGameBase::create_critter(Coords const c, DescriptionIndex const bob_type_idx, Player* owner) {
	return create_bob(c, *world().get_bob_descr(bob_type_idx), owner);
}

Bob& EditorGameBase::create_critter(Coords c, const std::string& name, Player* owner) {
	const BobDescr* descr = world().get_bob_descr(name);
	if (descr == nullptr)
		throw GameDataError("create_critter(%i,%i,%s,%s): critter not found", c.x, c.y, name.c_str(),
		                    owner->get_name().c_str());
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
Immovable & EditorGameBase::create_immovable
	(Coords const c, DescriptionIndex const idx, MapObjectDescr::OwnerType type)
{
	const ImmovableDescr & descr =
		*
		(type == MapObjectDescr::OwnerType::kTribe ?
		 tribes().get_immovable_descr(idx)
		 :
		 world().get_immovable_descr(idx));
	assert(&descr);
	inform_players_about_immovable
		(Map::get_index(c, map().get_width()), &descr);
	return descr.create(*this, c);
}

Immovable & EditorGameBase::create_immovable
	(Coords const c, const std::string & name, MapObjectDescr::OwnerType type)
{
	DescriptionIndex idx;
	if (type == MapObjectDescr::OwnerType::kTribe) {
		idx = tribes().immovable_index(name.c_str());
		if (!tribes().immovable_exists(idx)) {
			throw wexception
				("EditorGameBase::create_immovable(%i, %i): %s is not defined for the tribes",
				 c.x, c.y, name.c_str());
		}
	} else {
		idx =  world().get_immovable_index(name.c_str());
		if (idx == INVALID_INDEX) {
			throw wexception
				("EditorGameBase::create_immovable(%i, %i): %s is not defined for the world",
				 c.x, c.y, name.c_str());
		}
	}
	return create_immovable(c, idx, type);
}

/**
 * Instantly create a ship at the given x/y location.
 *
 * idx is the bob type.
 */

Bob& EditorGameBase::create_ship(Coords const c, int const ship_type_idx, Player* owner) {
	const BobDescr* descr = dynamic_cast<const BobDescr*>(tribes().get_ship_descr(ship_type_idx));
	return create_bob(c, *descr, owner);
}

Bob& EditorGameBase::create_ship(Coords c, const std::string& name, Player* owner) {
	try {
		int idx = tribes().safe_ship_index(name);
		return create_ship(c, idx, owner);
	} catch (const GameDataError& e) {
		throw GameDataError("create_ship(%i,%i,%s,%s): ship not found: %s", c.x, c.y, name.c_str(),
		                    owner->get_name().c_str(), e.what());
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
Player * EditorGameBase::get_safe_player(PlayerNumber const n) {
	return get_player(n);
}

/*
===============
Add a registered pointer.
Returns the serial number that can be used to retrieve or remove the pointer.
===============
*/
uint32_t EditorGameBase::add_trackpointer(void * const ptr)
{
	++lasttrackserial_;

	if (!lasttrackserial_)
		throw wexception("Dude, you play too long. Track serials exceeded.");

	trackpointers_[lasttrackserial_] = ptr;
	return lasttrackserial_;
}


/*
===============
Retrieve a previously stored pointer using the serial number.
Returns 0 if the pointer has been removed.
===============
*/
void * EditorGameBase::get_trackpointer(uint32_t const serial)
{
	std::map<uint32_t, void *>::iterator it = trackpointers_.find(serial);

	if (it != trackpointers_.end())
		return it->second;

	return nullptr;
}


/*
===============
Remove the registered track pointer. Subsequent calls to get_trackpointer()
using this serial number will return 0.
===============
*/
void EditorGameBase::remove_trackpointer(uint32_t serial)
{
	trackpointers_.erase(serial);
}

/**
 * Cleanup for load
 *
 * make this object ready to load new data
 */
void EditorGameBase::cleanup_for_load()
{
	cleanup_objects(); /// Clean all the stuff up, so we can load.

	player_manager_->cleanup();

	if (map_)
		map_->cleanup();
}


void EditorGameBase::set_road
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
		(direction == RoadType::kSouthWest ||
		 direction == RoadType::kSouthEast ||
		 direction == RoadType::kEast);
	assert
		(roadtype == RoadType::kNone || roadtype == RoadType::kNormal ||
		 roadtype == RoadType::kBusy || roadtype == RoadType::kWater);

	if (f.field->get_road(direction) == roadtype)
		return;
	f.field->set_road(direction, roadtype);

	FCoords neighbour;
	uint8_t mask = 0;
	switch (direction) {
	case RoadType::kSouthWest:
		neighbour = m.bl_n(f);
		mask = RoadType::kMask << RoadType::kSouthWest;
		break;
	case RoadType::kSouthEast:
		neighbour = m.br_n(f);
		mask = RoadType::kMask << RoadType::kSouthEast;
		break;
	case RoadType::kEast:
		neighbour = m. r_n(f);
		mask = RoadType::kMask << RoadType::kEast;
		break;
	default:
		NEVER_HERE();
	}
	uint8_t const road = f.field->get_roads() & mask;
	MapIndex const           i = f        .field - &first_field;
	MapIndex const neighbour_i = neighbour.field - &first_field;
	iterate_players_existing_const(plnum, MAX_PLAYERS, *this, p) {
		Player::Field & first_player_field = *p->fields_;
		Player::Field & player_field = (&first_player_field)[i];
		if
			(1 < player_field                      .vision
				||
				1 < (&first_player_field)[neighbour_i].vision)
		{
			player_field.roads &= ~mask;
			player_field.roads |= road;
		}
	}
}

/// This unconquers an area. This is only possible, when there is a building
/// placed on this node.
void EditorGameBase::unconquer_area
	(PlayerArea<Area<FCoords> > player_area,
	 PlayerNumber         const destroying_player)
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
	// TODO(SirVer): In the editor, no buildings should burn down when a military
	// building is removed. Check this again though
	if (is_a(Game, this)) {
		++player_area.radius;
		player_area.player_number = destroying_player;
		cleanup_playerimmovables_area(player_area);
	}
}

/// This conquers a given area because of a new (military) building that is set
/// there.
void EditorGameBase::conquer_area(PlayerArea<Area<FCoords> > player_area) {
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

void EditorGameBase::change_field_owner(const FCoords& fc, PlayerNumber const new_owner) {
	const Field & first_field = map()[0];

	PlayerNumber const old_owner = fc.field->get_owned_by();
	if (old_owner == new_owner) {
		return;
	}

	if (old_owner) {
		Notifications::publish(
		   NoteFieldPossession(fc, NoteFieldPossession::Ownership::LOST, get_player(old_owner)));
	}

	fc.field->set_owned_by(new_owner);

	// TODO(unknown): the player should do this when it gets the NoteFieldPossession.
	// This means also sending a note when new_player = 0, i.e. the field is no
	// longer owned.
	inform_players_about_ownership(fc.field - &first_field, new_owner);

	if (new_owner) {
		Notifications::publish(
		   NoteFieldPossession(fc, NoteFieldPossession::Ownership::GAINED, get_player(new_owner)));
	}
}

void EditorGameBase::conquer_area_no_building
	(PlayerArea<Area<FCoords> > player_area)
{
	assert(0 <= player_area.x);
	assert     (player_area.x < map().get_width());
	assert(0 <= player_area.y);
	assert     (player_area.y < map().get_height());
	assert(&map()[0] <= player_area.field);
	assert(player_area.field < &map()[0] + map().max_index());
	assert(0 < player_area.player_number);
	assert    (player_area.player_number <= map().get_nrplayers());
	MapRegion<Area<FCoords> > mr(map(), player_area);
	do {
		change_field_owner(mr.location(), player_area.player_number);
	} while (mr.advance(map()));

	//  This must reach one step beyond the conquered area to adjust the borders
	//  of neighbour players.
	++player_area.radius;
	map().recalc_for_field_area(world(), player_area);
}


/// Conquers the given area for that player; does the actual work.
/// Additionally, it updates the visible area for that player.
// TODO(unknown): this needs a more fine grained refactoring
// for example scripts will want to (un)conquer area of non oval shape
// or give area back to the neutral player (this is very important for the Lua
// testsuite).
void EditorGameBase::do_conquer_area
	(PlayerArea<Area<FCoords> > player_area,
	 bool          const conquer,
	 PlayerNumber const preferred_player,
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
	assert(!conquer || !preferred_player);
	Player & conquering_player = player(player_area.player_number);
	MapRegion<Area<FCoords> > mr(map(), player_area);
	do {
		MapIndex const index = mr.location().field - &first_field;
		MilitaryInfluence const influence =
			map().calc_influence
				(mr.location(), Area<>(player_area, player_area.radius));

		PlayerNumber const owner = mr.location().field->get_owned_by();
		if (conquer) {
			//  adds the influence
			MilitaryInfluence new_influence_modified = conquering_player.military_influence(index) +=
			   influence;
			if (owner && !conquer_guarded_location_by_superior_influence)
				new_influence_modified = 1;
			if (!owner || player(owner).military_influence(index) < new_influence_modified) {
				change_field_owner(mr.location(), player_area.player_number);
			}
		} else if (!(conquering_player.military_influence(index) -= influence) &&
		           owner == player_area.player_number) {
			//  The player completely lost influence over the location, which he
			//  owned. Now we must see if some other player has influence and if
			//  so, transfer the ownership to that player.
			PlayerNumber best_player;
			if
				(preferred_player
				 &&
				 player(preferred_player).military_influence(index))
				best_player = preferred_player;
			else {
				best_player =
					neutral_when_no_influence ? 0 : player_area.player_number;
				MilitaryInfluence highest_military_influence = 0;
				PlayerNumber const nr_players = map().get_nrplayers();
				iterate_players_existing_const(p, nr_players, *this, plr) {
					if
						(MilitaryInfluence const value =
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
				change_field_owner(mr.location(), best_player);
			}
		}
	} while (mr.advance(map()));

	// This must reach one step beyond the conquered area to adjust the borders
	// of neighbour players.
	++player_area.radius;
	map().recalc_for_field_area(world(), player_area);
}

/// Makes sure that buildings cannot exist outside their owner's territory.
void EditorGameBase::cleanup_playerimmovables_area
	(PlayerArea<Area<FCoords> > const area)
{
	std::vector<ImmovableFound> immovables;
	std::vector<PlayerImmovable *> burnlist;
	Map & m = map();

	//  find all immovables that need fixing
	m.find_immovables(area, &immovables, FindImmovablePlayerImmovable());

	for (const ImmovableFound& temp_imm : immovables) {
		upcast(PlayerImmovable, imm, temp_imm.object);
		if
			(!m[temp_imm.coords].is_interior(imm->owner().player_number())) {
			if
				(std::find(burnlist.begin(), burnlist.end(), imm)
				 ==
				 burnlist.end()) {
				burnlist.push_back(imm);
			}
		}
	}

	//  fix all immovables
	upcast(Game, game, this);
	for (PlayerImmovable * temp_imm : burnlist) {
		if (upcast(Building, building, temp_imm))
			building->set_defeating_player(area.player_number);
		else if (upcast(Flag, flag, temp_imm))
			if (Building * const flag_building = flag->get_building())
				flag_building->set_defeating_player(area.player_number);
		if (game)
			temp_imm->schedule_destroy(*game);
		else
			temp_imm->remove(*this);
	}
}


}
