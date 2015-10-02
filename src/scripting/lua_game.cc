/*
 * Copyright (C) 2006-2010 by the Widelands Development Team
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

#include "scripting/lua_game.h"

#include <memory>

#include <boost/format.hpp>

#include "economy/economy.h"
#include "economy/flag.h"
#include "logic/campaign_visibility.h"
#include "logic/constants.h"
#include "logic/game_controller.h"
#include "logic/message.h"
#include "logic/objective.h"
#include "logic/path.h"
#include "logic/player.h"
#include "logic/playersmanager.h"
#include "logic/tribe.h"
#include "profile/profile.h"
#include "scripting/globals.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_map.h"
#include "wui/interactive_player.h"
#include "wui/story_message_box.h"

using namespace Widelands;
using namespace LuaMaps;

namespace LuaGame {

/* RST
:mod:`wl.game`
======================

.. module:: wl.game
   :synopsis: Provides access on game internals like Players

.. moduleauthor:: The Widelands development team

.. currentmodule:: wl.game
*/

/*
 * ========================================================================
 *                         MODULE CLASSES
 * ========================================================================
 */

/* RST
Module Classes
^^^^^^^^^^^^^^^^

*/


/* RST
Player
------

.. class:: Player

	Child of: :class:`wl.bases.PlayerBase`

	This class represents one of the players in the game. You can access
	information about this player or act on his behalf. Note that you cannot
	instantiate a class of this type directly, use the :attr:`wl.Game.players`
	insteadl
*/
const char LuaPlayer::className[] = "Player";
const MethodType<LuaPlayer> LuaPlayer::Methods[] = {
	METHOD(LuaPlayer, send_message),
	METHOD(LuaPlayer, message_box),
	METHOD(LuaPlayer, sees_field),
	METHOD(LuaPlayer, seen_field),
	METHOD(LuaPlayer, allow_buildings),
	METHOD(LuaPlayer, forbid_buildings),
	METHOD(LuaPlayer, add_objective),
	METHOD(LuaPlayer, reveal_fields),
	METHOD(LuaPlayer, hide_fields),
	METHOD(LuaPlayer, reveal_scenario),
	METHOD(LuaPlayer, reveal_campaign),
	METHOD(LuaPlayer, get_ships),
	METHOD(LuaPlayer, get_buildings),
	METHOD(LuaPlayer, get_suitability),
	METHOD(LuaPlayer, allow_workers),
	METHOD(LuaPlayer, switchplayer),
	{nullptr, nullptr},
};
const PropertyType<LuaPlayer> LuaPlayer::Properties[] = {
	PROP_RO(LuaPlayer, name),
	PROP_RO(LuaPlayer, allowed_buildings),
	PROP_RO(LuaPlayer, objectives),
	PROP_RO(LuaPlayer, defeated),
	PROP_RO(LuaPlayer, inbox),
	PROP_RW(LuaPlayer, team),
	PROP_RW(LuaPlayer, see_all),
	{nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
	.. attribute:: name

			(RO) The name of this Player.
*/
int LuaPlayer::get_name(lua_State * L) {
	Game& game = get_game(L);
	Player & p = get(L, game);
	lua_pushstring(L, p.get_name());
	return 1;
}

/* RST
	.. attribute:: allowed_buildings

		(RO) an array with name:bool values with all buildings
		that are currently allowed for this player. Note that
		you can not enable/forbid a building by setting the value. Use
		:meth:`allow_buildings` or :meth:`forbid_buildings` for that.
*/
int LuaPlayer::get_allowed_buildings(lua_State * L) {
	Player & p = get(L, get_egbase(L));
	const TribeDescr & t = p.tribe();

	lua_newtable(L);
	for
		(BuildingIndex i = 0; i < t.get_nrbuildings(); ++i)
	{
		lua_pushstring(L, t.get_building_descr(i)->name().c_str());
		lua_pushboolean(L, p.is_building_type_allowed(i));
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
	.. attribute:: objectives

		(RO) A table of name -> :class:`wl.game.Objective`. You can change
		the objectives in this table and it will be reflected in the game. To add
		a new item, use :meth:`add_objective`.
*/
int LuaPlayer::get_objectives(lua_State * L) {
	lua_newtable(L);
	for (const auto& pair : get_egbase(L).map().objectives()) {
		lua_pushstring(L, pair.second->name());
		to_lua<LuaObjective>(L, new LuaObjective(*pair.second));
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
	.. attribute:: defeated

		(RO) :const:`true` if this player was defeated, :const:`false` otherwise
*/
int LuaPlayer::get_defeated(lua_State * L) {
	Player & p = get(L, get_egbase(L));
	bool have_warehouses = false;

	for (uint32_t economy_nr = 0; economy_nr < p.get_nr_economies(); economy_nr++) {
		if (!p.get_economy_by_number(economy_nr)->warehouses().empty()) {
		  have_warehouses = true;
		  break;
		}
	}

	lua_pushboolean(L, !have_warehouses);
	return 1;
}

/* RST
	.. attribute:: inbox

		(RO) An array of the message that are either read or new. Note that you
		can't add messages to this array, use :meth:`send_message` for that.
*/
int LuaPlayer::get_inbox(lua_State * L) {
	Player & p = get(L, get_egbase(L));

	lua_newtable(L);
	uint32_t cidx = 1;
	for (const std::pair<MessageId, Message *>& temp_message : p.messages()) {
		if (temp_message.second->status() == Message::Status::kArchived)
			continue;

		lua_pushuint32(L, cidx ++);
		to_lua<LuaMessage>(L, new LuaMessage(player_number(), temp_message.first));
		lua_rawset(L, -3);
	}

	return 1;
}

/* RST
	.. attribute:: team

		(RW) The team number of this player (0 means player is not in a team)

		normally only reading should be enough, however it's a nice idea to have
		a modular scenario, where teams form during the game.
*/
int LuaPlayer::set_team(lua_State * L) {
	get(L, get_egbase(L)).set_team_number(luaL_checkinteger(L, -1));
	return 0;
}
int LuaPlayer::get_team(lua_State * L) {
	lua_pushinteger(L, get(L, get_egbase(L)).team_number());
	return 1;
}


/* RST
	.. attribute:: see_all

		(RW) If you set this to true, the map will be completely visible for this
		player.
*/
int LuaPlayer::set_see_all(lua_State * const L) {
	get(L, get_egbase(L)).set_see_all(luaL_checkboolean(L, -1));
	return 0;
}
int LuaPlayer::get_see_all(lua_State * const L) {
	lua_pushboolean(L, get(L, get_egbase(L)).see_all());
	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

/* RST
	.. method:: send_message(t, m[, opts])

		Send a message to the player, the message will
		appear in his inbox. Title or Message can be a
		formatted using wideland's rich text.

		:arg t: title of the message
		:type t: :class:`string`

		:arg m: text of the message
		:type m: :class:`string`

		Opts is a table of optional arguments and can be omitted. If it
		exist it must contain string/value pairs of the following type:

		:arg field: the field connected to this message. Default:
			no field connected to message
		:type field: :class:`wl.map.Field`

		:arg status: status to attach to this message. can be 'new', 'read' or
			'archived'. Default: "new"
		:type status: :class:`string`

		:arg popup: should the message window be opened for this message or not.
			Default: :const:`false`
		:type popup: :class:`boolean`

		:returns: the message created
		:rtype: :class:`wl.game.Message`
*/
int LuaPlayer::send_message(lua_State * L) {
	uint32_t n = lua_gettop(L);
	std::string title = luaL_checkstring(L, 2);
	std::string body = luaL_checkstring(L, 3);
	Coords c = Coords::null();
	Message::Status st = Message::Status::kNew;
	bool popup = false;

	if (n == 4) {
		// Optional arguments
		lua_getfield(L, 4, "field");
		if (!lua_isnil(L, -1))
			c = (*get_user_class<LuaField>(L, -1))->coords();
		lua_pop(L, 1);

		lua_getfield(L, 4, "status");
		if (!lua_isnil(L, -1)) {
			std::string s = luaL_checkstring(L, -1);
			if (s == "new") st = Message::Status::kNew;
			else if (s == "read") st = Message::Status::kRead;
			else if (s == "archived") st = Message::Status::kArchived;
			else report_error(L, "Unknown message status: %s", s.c_str());
		}
		lua_pop(L, 1);

		lua_getfield(L, 4, "popup");
		if (!lua_isnil(L, -1))
			popup = luaL_checkboolean(L, -1);
		lua_pop(L, 1);
	}

	Game & game = get_game(L);
	Player & plr = get(L, game);

	MessageId const message =
		plr.add_message
			(game,
			 *new Message
				(Message::Type::kScenario,
			 	 game.get_gametime(),
			 	 title,
			 	 body,
				 c,
				 0,
				 st),
			popup);

	return to_lua<LuaMessage>(L, new LuaMessage(player_number(), message));
}

/* RST
	.. method:: message_box(t, m[, opts])

		Shows a message box to the player. While the message box is displayed the
		game will not continue. Use this carefully and prefer
		:meth:`send_message` because it is less interruptive, but nevertheless
		for a set of narrative messages with map movements, this is still useful.

		:arg t: title of the message
		:type t: :class:`string`

		:arg m: text of the message
		:type m: :class:`string`

		Opts is a table of optional arguments and can be omitted. If it
		exist it must contain string/value pairs of the following type:

		:arg field: The main view will be centered on this field when the box
			pops up. Default: no field attached to message
		:type field: :class:`wl.map.Field`

		:arg w: width of message box in pixels. Default: 400.
		:type w: :class:`integer`
		:arg h: width of message box in pixels. Default: 300.
		:type h: :class:`integer`
		:arg posx: x position of window in pixels. Default: centered
		:type posx: :class:`integer`
		:arg posy: y position of window in pixels. Default: centered
		:type posy: :class:`integer`

		:arg button_text: Text on the button. Default: OK.
		:type button_text: :class:`string`

		:returns: :const:`nil`
*/
// UNTESTED
int LuaPlayer::message_box(lua_State * L) {
	Game & game = get_game(L);
	// don't show message boxes in replays, cause they crash the game
	if (game.game_controller()->get_game_type() == GameController::GameType::REPLAY) {
		return 1;
	}

	uint32_t w = 400;
	uint32_t h = 300;
	int32_t posx = -1;
	int32_t posy = -1;
	std::string button_text = _("OK");

#define CHECK_ARG(var, type) \
	lua_getfield(L, -1, #var); \
	if (!lua_isnil(L, -1)) var = luaL_check ## type(L, -1); \
	lua_pop(L, 1);

	if (lua_gettop(L) == 4) {
		CHECK_ARG(posx, uint32);
		CHECK_ARG(posy, uint32);
		CHECK_ARG(w, uint32);
		CHECK_ARG(h, uint32);
		CHECK_ARG(button_text, string);

		// This must be done manually
		lua_getfield(L, 4, "field");
		if (!lua_isnil(L, -1)) {
			Coords c = (*get_user_class<LuaField>(L, -1))->coords();
			game.get_ipl()->move_view_to(c);
		}
		lua_pop(L, 1);
	}
#undef CHECK_ARG

	std::string title = luaL_checkstring(L, 2);
	std::string body =  luaL_checkstring(L, 3);

	uint32_t cspeed = game.game_controller()->desired_speed();
	game.game_controller()->set_desired_speed(0);

	game.save_handler().set_allow_saving(false);

	StoryMessageBox * mb =
		new StoryMessageBox
				(game.get_ipl(), luaL_checkstring(L, 2), luaL_checkstring(L, 3),
				 button_text, posx, posy, w, h);

	mb->run<UI::Panel::Returncodes>();
	delete mb;

	// Manually force the game to reevaluate it's current state,
	// especially time information.
	game.game_controller()->think();

	game.game_controller()->set_desired_speed(cspeed);

	game.save_handler().set_allow_saving(true);

	return 1;
}

/* RST
	.. method:: sees_field(f)

		Returns true if this field is currently seen by this player

		:returns: :const:`true` or :const:`false`
		:rtype: :class:`bool`
*/
int LuaPlayer::sees_field(lua_State * L) {
	EditorGameBase & egbase = get_egbase(L);

	Widelands::MapIndex const i =
		(*get_user_class<LuaField>(L, 2))->fcoords(L).field - &egbase.map()[0];

	lua_pushboolean(L, get(L, egbase).vision(i) > 1);
	return 1;
}

/* RST
	.. method:: seen_field(f)

		Returns true if this field has ever been seen by this player or
		is currently seen

		:returns: :const:`true` or :const:`false`
		:rtype: :class:`bool`
*/
int LuaPlayer::seen_field(lua_State * L) {
	EditorGameBase & egbase = get_egbase(L);

	Widelands::MapIndex const i =
		(*get_user_class<LuaField>(L, 2))->fcoords(L).field - &egbase.map()[0];

	lua_pushboolean(L, get(L, egbase).vision(i) >= 1);
	return 1;
}

/* RST
	.. method:: allow_buildings(what)

		This method disables or enables buildings to build for the player. What
		can either be the single string "all" or a list of strings containing
		the names of the buildings that are allowed.

		:see: :meth:`forbid_buildings`

		:arg what: either "all" or an array containing the names of the allowed
			buildings
		:returns: :const:`nil`
*/
int LuaPlayer::allow_buildings(lua_State * L) {
	return m_allow_forbid_buildings(L, true);
}

/* RST
	.. method:: forbid_buildings(what)

		See :meth:`allow_buildings` for arguments. This is the opposite function.

		:arg what: either "all" or an array containing the names of the allowed
			buildings
		:returns: :const:`nil`
*/
int LuaPlayer::forbid_buildings(lua_State * L) {
	return m_allow_forbid_buildings(L, false);
}

/* RST
	.. method:: add_objective(name, title, descr)

		Add a new objective for this player. Will report an error, if an
		Objective with the same name is already registered - note that the names
		for the objectives are shared internally for all players, so not even
		another player can have an objective with the same name.

		:arg name: the name of the objective
		:type name: :class:`string`
		:arg title: the title of the objective that will be shown in the menu
		:type title: :class:`string`
		:arg body: the full text of the objective
		:type body: :class:`string`

		:returns: The objective class created
		:rtype: :class:`wl.game.Objective`
*/
int LuaPlayer::add_objective(lua_State * L) {
	Game & game = get_game(L);

	Map * map = game.get_map();
	Map::Objectives* objectives = map->mutable_objectives();

	const std::string name = luaL_checkstring(L, 2);
	if (objectives->count(name))
		report_error(L, "An objective with the name '%s' already exists!", name.c_str());

	Objective* o = new Objective(name);
	o->set_done(false);
	o->set_descname(luaL_checkstring(L, 3));
	o->set_descr(luaL_checkstring(L, 4));
	o->set_visible(true);

	objectives->insert(std::make_pair(name, std::unique_ptr<Objective>(o)));
	return to_lua<LuaObjective>(L, new LuaObjective(*o));
}

/* RST
	.. method:: reveal_fields(fields)

		Make these fields visible for the current player. The fields will remain
		visible until they are hidden again.

		:arg fields: The fields to show
		:type fields: :class:`array` of :class:`wl.map.Fields`

		:returns: :const:`nil`
*/
int LuaPlayer::reveal_fields(lua_State * L) {
	EditorGameBase & egbase = get_egbase(L);
	Player & p = get(L, egbase);
	Map & m = egbase.map();

	luaL_checktype(L, 2, LUA_TTABLE);

	lua_pushnil(L);  /* first key */
	while (lua_next(L, 2) != 0) {
		p.see_node
			(m, m[0], (*get_user_class<LuaField>(L, -1))->fcoords(L),
			egbase.get_gametime());
		lua_pop(L, 1);
	}

	return 0;
}

/* RST
	.. method:: hide_fields(fields)

		Make these fields hidden for the current player if they are not
		seen by a military building.

		:arg fields: The fields to hide
		:type fields: :class:`array` of :class:`wl.map.Fields`

		:returns: :const:`nil`
*/
int LuaPlayer::hide_fields(lua_State * L) {
	EditorGameBase & egbase = get_egbase(L);
	Player & p = get(L, egbase);
	Map & m = egbase.map();

	luaL_checktype(L, 2, LUA_TTABLE);

	lua_pushnil(L);  /* first key */
	while (lua_next(L, 2) != 0) {
		p.unsee_node
			((*get_user_class<LuaField>(L, -1))->fcoords(L).field - &m[0],
			egbase.get_gametime());
		lua_pop(L, 1);
	}

	return 0;
}

/* RST
	.. method:: reveal_scenario(name)

		This reveals a scenario inside a campaign. This only works for the
		interactive player and most likely also only in single player games.

		:arg name: name of the scenario to reveal
		:type name: :class:`string`
*/
// UNTESTED
int LuaPlayer::reveal_scenario(lua_State * L) {
	if (get_game(L).get_ipl()->player_number() != player_number())
		report_error(L, "Can only be called for interactive player!");

	CampaignVisibilitySave cvs;
	cvs.set_map_visibility(luaL_checkstring(L, 2), true);

	return 0;
}

/* RST
	.. method:: reveal_campaign(name)

		This reveals a campaign. This only works for the
		interactive player and most likely also only in single player games.

		:arg name: name of the campaign to reveal
		:type name: :class:`string`
*/
// UNTESTED
int LuaPlayer::reveal_campaign(lua_State * L) {
	if (get_game(L).get_ipl()->player_number() != player_number()) {
		report_error(L, "Can only be called for interactive player!");
	}

	CampaignVisibilitySave cvs;
	cvs.set_campaign_visibility(luaL_checkstring(L, 2), true);

	return 0;
}

/* RST
	.. method:: get_ships()

		:returns: array of player's ships
		:rtype: :class:`array` or :class:`table`
*/
int LuaPlayer::get_ships(lua_State * L) {
	EditorGameBase & egbase = get_egbase(L);
	Map * map = egbase.get_map();
	PlayerNumber p = (get(L, egbase)).player_number();
	lua_newtable(L);
	uint32_t cidx = 1;

	std::set<OPtr<Ship>> found_ships;
	for (int16_t y = 0; y < map->get_height(); ++y) {
		for (int16_t x = 0; x < map->get_width(); ++x) {
			FCoords f = map->get_fcoords(Coords(x, y));
			// there are too many bobs on the map so we investigate
			// only bobs on water
			if (f.field->nodecaps() & MOVECAPS_SWIM) {
				for (Bob* bob = f.field->get_first_bob(); bob; bob = bob->get_next_on_field()) {
					if (upcast(Ship, ship, bob)) {
						if (ship->get_owner()->player_number() == p && !found_ships.count(ship)) {
							found_ships.insert(ship);
							lua_pushuint32(L, cidx++);
							LuaMaps::upcasted_map_object_to_lua(L, ship);
							lua_rawset(L, -3);
						}
					}
				}
			}
		}
	}
	return 1;
}

/* RST
	.. method:: get_buildings(which)

		which can be either a single name or an array of names. In the first
		case, the method returns an array of all Buildings that the player has of
		this kind. If which is an array, the function returns a table of
		(name,array of buildings) pairs.

		:type which: name of building or array of building names
		:rtype which: :class:`string` or :class:`array`
		:returns: information about the players buildings
		:rtype: :class:`array` or :class:`table`
*/
int LuaPlayer::get_buildings(lua_State * L) {
	EditorGameBase & egbase = get_egbase(L);
	Map * map = egbase.get_map();
	Player & p = get(L, egbase);

	// if only one string, convert to array so that we can use
	// m_parse_building_list
	bool return_array = true;
	if (lua_isstring(L, -1)) {
		const char * name = luaL_checkstring(L, -1);
		lua_pop(L, 1);
		lua_newtable(L);
		lua_pushuint32(L, 1);
		lua_pushstring(L, name);
		lua_rawset(L, -3);
		return_array = false;
	}

	std::vector<BuildingIndex> houses;
	m_parse_building_list(L, p.tribe(), houses);

	lua_newtable(L);

	uint32_t cidx = 1;
	for (const BuildingIndex& house : houses) {
		const std::vector<Widelands::Player::BuildingStats> & vec =
			p.get_building_statistics(house);

		if (return_array) {
			lua_pushstring(L, p.tribe().get_building_descr(house)->name());
			lua_newtable(L);
			cidx = 1;
		}

		for (uint32_t l = 0; l < vec.size(); ++l) {
			if (vec[l].is_constructionsite)
				continue;

			lua_pushuint32(L, cidx++);
			upcasted_map_object_to_lua(L, (*map)[vec[l].pos].get_immovable());
			lua_rawset(L, -3);
		}

		if (return_array)
			lua_rawset(L, -3);
	}
	return 1;
}

/* RST
	.. method:: get_suitability(building, field)

		Returns the suitability that this building has for this field. This
		is mainly useful in initialization where buildings must be placed
		automatically.

		:arg building: name of the building to check for
		:type building: :class:`string`
		:arg field: where the suitability should be checked
		:type field: :class:`wl.map.Field`

		:returns: the suitability
		:rtype: :class:`integer`
*/
// UNTESTED
int LuaPlayer::get_suitability(lua_State * L) {
	Game & game = get_game(L);
	const TribeDescr & tribe = get(L, game).tribe();

	const char * name = luaL_checkstring(L, 2);
	BuildingIndex i = tribe.building_index(name);
	if (i == INVALID_INDEX)
		report_error(L, "Unknown building type: <%s>", name);

	lua_pushint32
		(L, tribe.get_building_descr(i)->suitability
		 (game.map(), (*get_user_class<LuaField>(L, 3))->fcoords(L))
		);
	return 1;
}


/* RST
	.. method:: allow_workers(what)

		This will become the corresponding function to :meth:`allow_buildings`,
		but at the moment this is only a stub that accepts only "all" as
		argument. It then activates all workers for the player, that means all
		workers are allowed to spawn in all warehouses.
*/
int LuaPlayer::allow_workers(lua_State * L) {

	if (luaL_checkstring(L, 2) != std::string("all"))
		report_error(L, "Argument must be <all>!");

	Game & game = get_game(L);
	const TribeDescr & tribe = get(L, game).tribe();
	Player & player = get(L, game);

	const std::vector<WareIndex> & worker_types_without_cost =
		tribe.worker_types_without_cost();

	for (WareIndex i = 0; i < tribe.get_nrworkers(); ++i) {
		const WorkerDescr & worker_descr = *tribe.get_worker_descr(i);
		if (!worker_descr.is_buildable())
			continue;

		player.allow_worker_type(i, true);

		if (worker_descr.buildcost().empty()) {
			//  Workers of this type can be spawned in warehouses. Start it.
			uint8_t worker_types_without_cost_index = 0;
			for (;; ++worker_types_without_cost_index) {
				assert
					(worker_types_without_cost_index
					 <
					 worker_types_without_cost.size());
				if
					(worker_types_without_cost.at
						(worker_types_without_cost_index) == i)
					break;
			}
			for (uint32_t j = player.get_nr_economies(); j;) {
				Economy & economy = *player.get_economy_by_number(--j);

				for (Warehouse * warehouse : economy.warehouses()) {
					warehouse->enable_spawn
						(game, worker_types_without_cost_index);
				}
			}
		}
	}
	return 0;
}


/* RST
	.. method:: switchplayer(playernumber)

		If *this* is the local player (the player set in interactive player)
		switch to the player with playernumber
*/
int LuaPlayer::switchplayer(lua_State * L) {
	Game & game = get_game(L);

	uint8_t newplayer = luaL_checkinteger(L, -1);
	InteractivePlayer * ipl = game.get_ipl();
	// only switch, if this is our player!
	if (ipl->player_number() == player_number()) {
		ipl->set_player_number(newplayer);
	}
	return 0;
}


/*
 ==========================================================
 C METHODS
 ==========================================================
 */
void LuaPlayer::m_parse_building_list
	(lua_State * L, const TribeDescr & tribe, std::vector<BuildingIndex> & rv)
{
	if (lua_isstring(L, -1)) {
		std::string opt = luaL_checkstring(L, -1);
		if (opt != "all")
			report_error(L, "'%s' was not understood as argument!", opt.c_str());
		for
			(BuildingIndex i = 0;
			 i < tribe.get_nrbuildings(); ++i)
				rv.push_back(i);
	} else {
		// array of strings argument
		luaL_checktype(L, -1, LUA_TTABLE);

		lua_pushnil(L);
		while (lua_next(L, -2) != 0) {
			const char * name = luaL_checkstring(L, -1);
			BuildingIndex i = tribe.building_index(name);
			if (i == INVALID_INDEX)
				report_error(L, "Unknown building type: '%s'", name);

			rv.push_back(i);

			lua_pop(L, 1); // pop value
		}
	}
}
int LuaPlayer::m_allow_forbid_buildings(lua_State * L, bool allow)
{
	Player & p = get(L, get_egbase(L));

	std::vector<BuildingIndex> houses;
	m_parse_building_list(L, p.tribe(), houses);

	for (const BuildingIndex& house : houses) {
		p.allow_building_type(house, allow);
	}
	return 0;
}

/* RST
Objective
---------

.. class:: Objective

	This represents an Objective, a goal for the player in the game. This is
	mainly for displaying to the user, but each objective also has a
	:attr:`done` which can be set by the scripter to define if this is done. Use
	:attr:`visible` to hide it from the user.
*/
const char LuaObjective::className[] = "Objective";
const MethodType<LuaObjective> LuaObjective::Methods[] = {
	METHOD(LuaObjective, remove),
	METHOD(LuaObjective, __eq),
	{nullptr, nullptr},
};
const PropertyType<LuaObjective> LuaObjective::Properties[] = {
	PROP_RO(LuaObjective, name),
	PROP_RW(LuaObjective, title),
	PROP_RW(LuaObjective, body),
	PROP_RW(LuaObjective, visible),
	PROP_RW(LuaObjective, done),
	{nullptr, nullptr, nullptr},
};

LuaObjective::LuaObjective(const Widelands::Objective& o) {
	m_name = o.name();
}

void LuaObjective::__persist(lua_State * L) {
	PERS_STRING("name", m_name);
}
void LuaObjective::__unpersist(lua_State * L) {
	UNPERS_STRING("name", m_name);
}


/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
	.. attribute:: name

		(RO) the internal name. You can reference this object via
		:attr:`wl.game.Player.objectives` with :attr:`name` as key.
*/
int LuaObjective::get_name(lua_State * L) {
	Objective & o = get(L, get_game(L));
	lua_pushstring(L, o.name().c_str());
	return 1;
}
/* RST
	.. attribute:: title

		(RW) The line that is shown in the objectives menu
*/
int LuaObjective::get_title(lua_State * L) {
	Objective & o = get(L, get_game(L));
	lua_pushstring(L, o.descname().c_str());
	return 1;
}
int LuaObjective::set_title(lua_State * L) {
	Objective & o = get(L, get_game(L));
	o.set_descname(luaL_checkstring(L, -1));
	return 0;
}
/* RST
	.. attribute:: body

		(RW) The complete text of this objective. Can be Widelands Richtext.
*/
int LuaObjective::get_body(lua_State * L) {
	Objective & o = get(L, get_game(L));
	lua_pushstring(L, o.descr().c_str());
	return 1;
}
int LuaObjective::set_body(lua_State * L) {
	Objective & o = get(L, get_game(L));
	o.set_descr(luaL_checkstring(L, -1));
	return 0;
}
/* RST
	.. attribute:: visible

		(RW) is this objective shown in the objectives menu
*/
int LuaObjective::get_visible(lua_State * L) {
	Objective & o = get(L, get_game(L));
	lua_pushboolean(L, o.visible());
	return 1;
}
int LuaObjective::set_visible(lua_State * L) {
	Objective & o = get(L, get_game(L));
	o.set_visible(luaL_checkboolean(L, -1));
	return 0;
}
/* RST
	.. attribute:: done

		(RW) defines if this objective is already fulfilled. If done is
		:const`true`, the objective will not be shown to the user, no matter what.
		:attr:`visible` is set to. A savegame will be created when this attribute
		is changed to :const`true`.

*/
int LuaObjective::get_done(lua_State * L) {
	Objective & o = get(L, get_game(L));
	lua_pushboolean(L, o.done());
	return 1;
}
int LuaObjective::set_done(lua_State * L) {
	Objective & o = get(L, get_game(L));
	o.set_done(luaL_checkboolean(L, -1));

	const int32_t autosave = g_options.pull_section("global").get_int("autosave", 0);
	if (autosave <= 0) {
		return 0;
	}

	if (o.done()) {
		/** TRANSLATORS: File name for saving objective achieved */
		/** TRANSLATORS: %1% = map name. %2% = achievement name */
		std::string filename = _("%1% (%2%)");
		i18n::Textdomain td("maps");
		filename = (boost::format(filename)
						% _(get_egbase(L).get_map()->get_name())
						% o.descname().c_str()).str();
		get_game(L).save_handler().request_save(filename);
	}
	return 0;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
int LuaObjective::remove(lua_State * L) {
	Game & g = get_game(L);
	// The next call checks if the Objective still exists
	get(L, g);
	g.map().mutable_objectives()->erase(m_name);
	return 0;
}

int LuaObjective::__eq(lua_State * L) {
	const Map::Objectives& objectives = get_game(L).map().objectives();

	const Map::Objectives::const_iterator me = objectives.find(m_name);
	const Map::Objectives::const_iterator other =
		objectives.find((*get_user_class<LuaObjective>(L, 2))->m_name);

	lua_pushboolean(L,
	                (me != objectives.end() && other != objectives.end()) &&
	                   (me->second->name() == other->second->name()));
	return 1;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */
Objective & LuaObjective::get(lua_State * L, Widelands::Game & g) {
	Map::Objectives* objectives = g.map().mutable_objectives();
	Map::Objectives::iterator i = objectives->find(m_name);
	if (i == objectives->end()) {
		report_error
			(L, "Objective with name '%s' doesn't exist!", m_name.c_str());
	}
	return *i->second;
}

/* RST
Message
---------

.. class:: Message

	This represents a message in the Message Box of a given user.
*/
const char LuaMessage::className[] = "Message";
const MethodType<LuaMessage> LuaMessage::Methods[] = {
	METHOD(LuaMessage, __eq),
	{nullptr, nullptr},
};
const PropertyType<LuaMessage> LuaMessage::Properties[] = {
	PROP_RO(LuaMessage, title),
	PROP_RO(LuaMessage, body),
	PROP_RO(LuaMessage, sent),
	PROP_RO(LuaMessage, field),
	PROP_RW(LuaMessage, status),
	{nullptr, nullptr, nullptr},
};

LuaMessage::LuaMessage(uint8_t plr, MessageId id) {
	m_plr = plr;
	m_mid = id;
}

void LuaMessage::__persist(lua_State * L) {
	PERS_UINT32("player", m_plr);
	PERS_UINT32("msg_idx", get_mos(L)->message_savers[m_plr - 1][m_mid].value());
}
void LuaMessage::__unpersist(lua_State * L) {
	UNPERS_UINT32("player", m_plr);
	uint32_t midx = 0;
	UNPERS_UINT32("msg_idx", midx);
	m_mid = MessageId(midx);
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
	.. attribute:: title

		(RO) The title of this message
*/
int LuaMessage::get_title(lua_State * L) {
	lua_pushstring(L, get(L, get_game(L)).title());
	return 1;
}
/* RST
	.. attribute:: body

		(RO) The body of this message
*/
int LuaMessage::get_body(lua_State * L) {
	lua_pushstring(L, get(L, get_game(L)).body());
	return 1;
}

/* RST
	.. attribute:: sent

		(RO) The game time in milliseconds when this message was sent
*/
int LuaMessage::get_sent(lua_State * L) {
	lua_pushuint32(L, get(L, get_game(L)).sent());
	return 1;
}


/* RST
	.. attribute:: field

		(RO) The field that corresponds to this Message.
*/
int LuaMessage::get_field(lua_State * L) {
	Coords c = get(L, get_game(L)).position();
	if (c == Coords::null())
		return 0;
	return to_lua<LuaField>(L, new LuaField(c));
}

/* RST
	.. attribute:: status

		(RW) The status of the message. Can be either of

			* new
			* read
			* archived
*/
int LuaMessage::get_status(lua_State * L) {
	switch (get(L, get_game(L)).status()) {
		case Message::Status::kNew: lua_pushstring(L, "new"); break;
		case Message::Status::kRead: lua_pushstring(L, "read"); break;
		case Message::Status::kArchived: lua_pushstring(L, "archived"); break;
		default: report_error(L, "Unknown Message status encountered!");
	}
	return 1;
}
int LuaMessage::set_status(lua_State * L) {
	Message::Status status = Message::Status::kNew;
	std::string s = luaL_checkstring(L, -1);
	if (s == "new") status = Message::Status::kNew;
	else if (s == "read") status = Message::Status::kRead;
	else if (s == "archived") status = Message::Status::kArchived;
	else report_error(L, "Invalid message status <%s>!", s.c_str());

	get_plr(L, get_game(L)).messages().set_message_status(m_mid, status);

	return 0;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
int LuaMessage::__eq(lua_State * L) {
	lua_pushboolean(L, m_mid == (*get_user_class<LuaMessage>(L, 2))->m_mid);
	return 1;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */
Player & LuaMessage::get_plr(lua_State * L, Widelands::Game & game) {
	if (m_plr > MAX_PLAYERS)
		report_error(L, "Illegal player number %i",  m_plr);
	Player * rv = game.get_player(m_plr);
	if (!rv)
		report_error(L, "Player with the number %i does not exist", m_plr);
	return *rv;
}
const Message & LuaMessage::get(lua_State * L, Widelands::Game & game) {
	const Message * rv = get_plr(L, game).messages()[m_mid];
	if (!rv)
		report_error(L, "This message has been deleted!");
	return *rv;
}


/* RST
.. function:: report_result(plr, result[, info = ""])

	Reports the game ending to the metaserver if this is an Internet
	network game. Otherwise, does nothing.

	:arg plr: The Player to report results for.
	:type plr: :class:`~wl.game.Player`
	:arg result: The player result (0: lost, 1: won, 2: resigned)
	:type result: :class:`number`
	:arg info: a string containing extra data for this particular win
		condition. This will vary from game type to game type. See
		:class:`PlayerEndStatus` for allowed values
	:type info: :class:`string`

*/
// TODO(sirver): this should be a method of wl.Game(). Fix for b19.
static int L_report_result(lua_State * L) {
	std::string info = "";
	if (lua_gettop(L) >= 3)
		info = luaL_checkstring(L, 3);

	Widelands::PlayerEndResult result = static_cast<Widelands::PlayerEndResult>
		(luaL_checknumber(L, 2));

	get_game(L).game_controller()->report_result
		((*get_user_class<LuaPlayer>(L, 1))->get(L, get_game(L)).player_number(),
		 result, info);
	return 0;
}

/*
 * ========================================================================
 *                            MODULE FUNCTIONS
 * ========================================================================
 */
const static struct luaL_Reg wlgame [] = {
	{"report_result", &L_report_result},
	{nullptr, nullptr}
};

void luaopen_wlgame(lua_State * L) {
	lua_getglobal(L, "wl");  // S: wl_table
	lua_pushstring(L, "game"); // S: wl_table "game"
	luaL_newlib(L, wlgame);  // S: wl_table "game" wl.game_table
	lua_settable(L, -3); // S: wl_table
	lua_pop(L, 1); // S:

	register_class<LuaPlayer>(L, "game", true);
	add_parent<LuaPlayer, LuaBases::LuaPlayerBase>(L);
	lua_pop(L, 1); // Pop the meta table

	register_class<LuaObjective>(L, "game");
	register_class<LuaMessage>(L, "game");
}

}
