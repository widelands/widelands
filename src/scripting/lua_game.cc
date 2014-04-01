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

#include <boost/format.hpp>

#include "campvis.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "gamecontroller.h"
#include "i18n.h"
#include "logic/objective.h"
#include "logic/path.h"
#include "logic/player.h"
#include "logic/playersmanager.h"
#include "logic/tribe.h"
#include "scripting/c_utils.h"
#include "scripting/lua_map.h"
#include "scripting/scripting.h"
#include "wui/interactive_player.h"
#include "wui/story_message_box.h"

using namespace Widelands;
using namespace LuaMap;

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
const char L_Player::className[] = "Player";
const MethodType<L_Player> L_Player::Methods[] = {
	METHOD(L_Player, send_message),
	METHOD(L_Player, message_box),
	METHOD(L_Player, sees_field),
	METHOD(L_Player, seen_field),
	METHOD(L_Player, allow_buildings),
	METHOD(L_Player, forbid_buildings),
	METHOD(L_Player, add_objective),
	METHOD(L_Player, reveal_fields),
	METHOD(L_Player, hide_fields),
	METHOD(L_Player, reveal_scenario),
	METHOD(L_Player, reveal_campaign),
	METHOD(L_Player, get_buildings),
	METHOD(L_Player, set_flag_style),
	METHOD(L_Player, set_frontier_style),
	METHOD(L_Player, get_suitability),
	METHOD(L_Player, allow_workers),
	METHOD(L_Player, switchplayer),
	{nullptr, nullptr},
};
const PropertyType<L_Player> L_Player::Properties[] = {
	PROP_RO(L_Player, name),
	PROP_RO(L_Player, allowed_buildings),
	PROP_RO(L_Player, objectives),
	PROP_RO(L_Player, defeated),
	PROP_RW(L_Player, retreat_percentage),
	PROP_RW(L_Player, changing_retreat_percentage_allowed),
	PROP_RO(L_Player, inbox),
	PROP_RW(L_Player, team),
	PROP_RW(L_Player, see_all),
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
int L_Player::get_name(lua_State * L) {
	Game & game = get_game(L);
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
int L_Player::get_allowed_buildings(lua_State * L) {
	Player & p = get(L, get_egbase(L));
	const Tribe_Descr & t = p.tribe();

	lua_newtable(L);
	for
		(Building_Index i = Building_Index::First(); i < t.get_nrbuildings(); ++i)
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
int L_Player::get_objectives(lua_State * L) {
	const Manager<Objective> & mom = get_egbase(L).map().mom();

	lua_newtable(L);
	for (Manager<Objective>::Index i = 0; i < mom.size(); i++) {
		lua_pushstring(L, mom[i].name().c_str());
		to_lua<L_Objective>(L, new L_Objective(mom[i]));
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
	.. attribute:: defeated

		(RO) :const:`true` if this player was defeated, :const:`false` otherwise
*/
int L_Player::get_defeated(lua_State * L) {
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
	.. attribute:: retreat_percentage


		(RW) Soldiers that only have this amount of total hitpoints
		left will go home
*/
// UNTESTED
int L_Player::get_retreat_percentage(lua_State * L) {
	lua_pushuint32(L, get(L, get_egbase(L)).get_retreat_percentage());
	return 1;
}
int L_Player::set_retreat_percentage(lua_State * L) {
	uint32_t value = luaL_checkuint32(L, -1);
	if (value > 100)
		return report_error(L, "%i is not a valid percentage!", value);

	get(L, get_egbase(L)).set_retreat_percentage(value);
	return 0;
}

/* RST
	.. attribute:: changing_retreat_percentage_allowed

		(RW) A boolean value. :const:`true` if the player is allowed to change
		    the :attr:`retreat_percentage`, :const:`false` otherwise.
*/
// UNTESTED
int L_Player::get_changing_retreat_percentage_allowed(lua_State * L) {
	lua_pushuint32(L, get(L, get_egbase(L)).is_retreat_change_allowed());
	return 1;
}
int L_Player::set_changing_retreat_percentage_allowed(lua_State * L) {
	get(L, get_egbase(L)).allow_retreat_change(luaL_checkboolean(L, -1));
	return 0;
}

/* RST
	.. attribute:: inbox

		(RO) An array of the message that are either read or new. Note that you
		can't add messages to this array, use :meth:`send_message` for that.
*/
int L_Player::get_inbox(lua_State * L) {
	Player & p = get(L, get_egbase(L));

	lua_newtable(L);
	uint32_t cidx = 1;
	container_iterate_const(MessageQueue, p.messages(), m) {
		if (m.current->second->status() == Message::Archived)
			continue;

		lua_pushuint32(L, cidx ++);
		to_lua<L_Message>(L, new L_Message(player_number(), m.current->first));
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
int L_Player::set_team(lua_State * L) {
	get(L, get_egbase(L)).set_team_number(luaL_checkinteger(L, -1));
	return 0;
}
int L_Player::get_team(lua_State * L) {
	lua_pushinteger(L, get(L, get_egbase(L)).team_number());
	return 1;
}


/* RST
	.. attribute:: see_all

		(RW) If you set this to true, the map will be completely visible for this
		player.
*/
int L_Player::set_see_all(lua_State * const L) {
	get(L, get_egbase(L)).set_see_all(luaL_checkboolean(L, -1));
	return 0;
}
int L_Player::get_see_all(lua_State * const L) {
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

		:arg duration: if this is given, the message will be removed
			from the players inbox after this many ms. Default:
			message never expires.
		:type duration: :class:`integer`

		:arg field: the field connected to this message. Default:
			no field connected to message
		:type field: :class:`wl.map.Field`

		:arg status: status to attach to this message. can be 'new', 'read' or
			'archived'. Default: "new"
		:type status: :class:`string`

		:arg sender: sender name of this string. Default: "ScriptingEngine"
		:type sender: :class:`string`

		:arg popup: should the message window be opened for this message or not.
			Default: :const:`false`
		:type popup: :class:`boolean`

		:returns: the message created
		:rtype: :class:`wl.game.Message`
*/
int L_Player::send_message(lua_State * L) {
	uint32_t n = lua_gettop(L);
	std::string title = luaL_checkstring(L, 2);
	std::string body = luaL_checkstring(L, 3);
	Coords c = Coords::Null();
	Duration d = Forever();
	Message::Status st = Message::New;
	std::string sender = "ScriptingEngine";
	bool popup = false;

	if (n == 4) {
		// Optional arguments
		lua_getfield(L, 4, "duration");
		if (not lua_isnil(L, -1))
			d = luaL_checkuint32(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, 4, "field");
		if (not lua_isnil(L, -1))
			c = (*get_user_class<L_Field>(L, -1))->coords();
		lua_pop(L, 1);

		lua_getfield(L, 4, "status");
		if (not lua_isnil(L, -1)) {
			std::string s = luaL_checkstring(L, -1);
			if (s == "new") st = Message::New;
			else if (s == "read") st = Message::Read;
			else if (s == "archived") st = Message::Archived;
			else report_error(L, "Unknown message status: %s", s.c_str());
		}
		lua_pop(L, 1);

		lua_getfield(L, 4, "sender");
		if (not lua_isnil(L, -1))
			sender = luaL_checkstring(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, 4, "popup");
		if (not lua_isnil(L, -1))
			popup = luaL_checkboolean(L, -1);
		lua_pop(L, 1);
	}

	Game & game = get_game(L);
	Player & plr = get(L, game);

	Message_Id const message =
		plr.add_message
			(game,
			 *new Message
			 	(sender,
			 	 game.get_gametime(),
			 	 d,
			 	 title,
			 	 body,
				 c,
				 0,
				 st),
			popup);

	return to_lua<L_Message>(L, new L_Message(player_number(), message));
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
int L_Player::message_box(lua_State * L) {
	Game & game = get_game(L);
	// don't show message boxes in replays, cause they crash the game
	if (game.gameController()->getGameDescription() == "replay") {
		return 1;
	}

	uint32_t w = 400;
	uint32_t h = 300;
	int32_t posx = -1;
	int32_t posy = -1;
	std::string button_text = _("OK");

#define CHECK_ARG(var, type) \
	lua_getfield(L, -1, #var); \
	if (not lua_isnil(L, -1)) var = luaL_check ## type(L, -1); \
	lua_pop(L, 1);

	if (lua_gettop(L) == 4) {
		CHECK_ARG(posx, uint32);
		CHECK_ARG(posy, uint32);
		CHECK_ARG(w, uint32);
		CHECK_ARG(h, uint32);
		CHECK_ARG(button_text, string);

		// This must be done manually
		lua_getfield(L, 4, "field");
		if (not lua_isnil(L, -1)) {
			Coords c = (*get_user_class<L_Field>(L, -1))->coords();
			game.get_ipl()->move_view_to(c);
		}
		lua_pop(L, 1);
	}
#undef CHECK_ARG

	std::string title = luaL_checkstring(L, 2);
	std::string body =  luaL_checkstring(L, 3);

	uint32_t cspeed = game.gameController()->desiredSpeed();
	game.gameController()->setDesiredSpeed(0);

	game.save_handler().set_allow_saving(false);

	Story_Message_Box * mb =
		new Story_Message_Box
				(game.get_ipl(), luaL_checkstring(L, 2), luaL_checkstring(L, 3),
				 button_text, posx, posy, w, h);

	mb->run();
	delete mb;

	// Manually force the game to reevaluate it's current state,
	// especially time information.
	game.gameController()->think();

	game.gameController()->setDesiredSpeed(cspeed);

	game.save_handler().set_allow_saving(true);

	return 1;
}

/* RST
	.. method:: sees_field(f)

		Returns true if this field is currently seen by this player

		:returns: :const:`true` or :const:`false`
		:rtype: :class:`bool`
*/
int L_Player::sees_field(lua_State * L) {
	Editor_Game_Base & egbase = get_egbase(L);

	Widelands::Map_Index const i =
		(*get_user_class<L_Field>(L, 2))->fcoords(L).field - &egbase.map()[0];

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
int L_Player::seen_field(lua_State * L) {
	Editor_Game_Base & egbase = get_egbase(L);

	Widelands::Map_Index const i =
		(*get_user_class<L_Field>(L, 2))->fcoords(L).field - &egbase.map()[0];

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
int L_Player::allow_buildings(lua_State * L) {
	return m_allow_forbid_buildings(L, true);
}

/* RST
	.. method:: forbid_buildings(what)

		See :meth:`allow_buildings` for arguments. This is the opposite function.

		:arg what: either "all" or an array containing the names of the allowed
			buildings
		:returns: :const:`nil`
*/
int L_Player::forbid_buildings(lua_State * L) {
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
int L_Player::add_objective(lua_State * L) {
	Game & game = get_game(L);

	Map * map = game.get_map();
	Manager<Objective> & mom = map->mom();

	std::string name = luaL_checkstring(L, 2);
	if (mom[name] != nullptr)
		return
			report_error
				(L, "An objective with the name '%s' already exists!", name.c_str()
			);


	Objective & o = *new Objective;

	o.set_done(false);
	o.set_name(name);
	o.set_descname(luaL_checkstring(L, 3));
	o.set_descr(luaL_checkstring(L, 4));
	o.set_visible(true);

	game.get_map()->mom().register_new(o);

	return to_lua<L_Objective>(L, new L_Objective(o));
}

/* RST
	.. method:: reveal_fields(fields)

		Make these fields visible for the current player. The fields will remain
		visible until they are hidden again.

		:arg fields: The fields to show
		:type fields: :class:`array` of :class:`wl.map.Fields`

		:returns: :const:`nil`
*/
int L_Player::reveal_fields(lua_State * L) {
	Editor_Game_Base & egbase = get_egbase(L);
	Player & p = get(L, egbase);
	Map & m = egbase.map();

	luaL_checktype(L, 2, LUA_TTABLE);

	lua_pushnil(L);  /* first key */
	while (lua_next(L, 2) != 0) {
		p.see_node
			(m, m[0], (*get_user_class<L_Field>(L, -1))->fcoords(L),
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
int L_Player::hide_fields(lua_State * L) {
	Editor_Game_Base & egbase = get_egbase(L);
	Player & p = get(L, egbase);
	Map & m = egbase.map();

	luaL_checktype(L, 2, LUA_TTABLE);

	lua_pushnil(L);  /* first key */
	while (lua_next(L, 2) != 0) {
		p.unsee_node
			((*get_user_class<L_Field>(L, -1))->fcoords(L).field - &m[0],
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
int L_Player::reveal_scenario(lua_State * L) {
	if (get_game(L).get_ipl()->player_number() != player_number())
		return report_error(L, "Can only be called for interactive player!");

	Campaign_visibility_save cvs;
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
int L_Player::reveal_campaign(lua_State * L) {
	if (get_game(L).get_ipl()->player_number() != player_number())
		return report_error(L, "Can only be called for interactive player!");

	Campaign_visibility_save cvs;
	cvs.set_campaign_visibility(luaL_checkstring(L, 2), true);

	return 0;
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
int L_Player::get_buildings(lua_State * L) {
	Editor_Game_Base & egbase = get_egbase(L);
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

	std::vector<Building_Index> houses;
	m_parse_building_list(L, p.tribe(), houses);

	lua_newtable(L);

	uint32_t cidx = 1;
	container_iterate_const(std::vector<Building_Index>, houses, i) {
		const std::vector<Widelands::Player::Building_Stats> & vec =
			p.get_building_statistics(*i.current);

		if (return_array) {
			lua_pushstring(L, p.tribe().get_building_descr(*i.current)->name());
			lua_newtable(L);
			cidx = 1;
		}

		for (uint32_t l = 0; l < vec.size(); ++l) {
			if (vec[l].is_constructionsite)
				continue;

			lua_pushuint32(L, cidx++);
			upcasted_immovable_to_lua(L, (*map)[vec[l].pos].get_immovable());
			lua_rawset(L, -3);
		}

		if (return_array)
			lua_rawset(L, -3);
	}
	return 1;
}

/* RST
	.. method:: set_flag_style(name)

		Sets the appearance of the flags for this player to the given style.
		The style must be defined for the tribe.

		:arg name: name of style
		:type name: :class:`string`
*/
// UNTESTED, UNUSED so far
int L_Player::set_flag_style(lua_State * L) {
	Player & p = get(L, get_game(L));
	const char * name = luaL_checkstring(L, 2);

	try {
		p.set_flag_style(p.tribe().flag_style_index(name));
	} catch (Tribe_Descr::Nonexistent &) {
		return report_error(L, "Flag style <%s> does not exist!\n", name);
	}
	return 0;
}

/* RST
	.. method:: set_frontier_style(name)

		Sets the appearance of the frontiers for this player to the given style.
		The style must be defined for the tribe.

		:arg name: name of style
		:type name: :class:`string`
*/
// UNTESTED, UNUSED so far
int L_Player::set_frontier_style(lua_State * L) {
	Player & p = get(L, get_game(L));
	const char * name = luaL_checkstring(L, 2);

	try {
		p.set_frontier_style(p.tribe().frontier_style_index(name));
	} catch (Tribe_Descr::Nonexistent &) {
		return report_error(L, "Frontier style <%s> does not exist!\n", name);
	}
	return 0;
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
int L_Player::get_suitability(lua_State * L) {
	Game & game = get_game(L);
	const Tribe_Descr & tribe = get(L, game).tribe();

	const char * name = luaL_checkstring(L, 2);
	Building_Index i = tribe.building_index(name);
	if (i == Building_Index::Null())
		report_error(L, "Unknown building type: <%s>", name);

	lua_pushint32
		(L, tribe.get_building_descr(i)->suitability
		 (game.map(), (*get_user_class<L_Field>(L, 3))->fcoords(L))
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
int L_Player::allow_workers(lua_State * L) {

	if (luaL_checkstring(L, 2) != std::string("all"))
		return report_error(L, "Argument must be <all>!");

	Game & game = get_game(L);
	const Tribe_Descr & tribe = get(L, game).tribe();
	Player & player = get(L, game);

	const std::vector<Ware_Index> & worker_types_without_cost =
		tribe.worker_types_without_cost();

	for (Ware_Index i = Ware_Index::First(); i < tribe.get_nrworkers(); ++i) {
		const Worker_Descr & worker_descr = *tribe.get_worker_descr(i);
		if (not worker_descr.is_buildable())
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
				container_iterate_const
					(std::vector<Warehouse *>, economy.warehouses(), k)
					(*k.current)->enable_spawn
						(game, worker_types_without_cost_index);
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
int L_Player::switchplayer(lua_State * L) {
	Game & game = get_game(L);

	uint8_t newplayer = luaL_checkinteger(L, -1);
	Interactive_Player * ipl = game.get_ipl();
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
void L_Player::m_parse_building_list
	(lua_State * L, const Tribe_Descr & tribe, std::vector<Building_Index> & rv)
{
	if (lua_isstring(L, -1)) {
		std::string opt = luaL_checkstring(L, -1);
		if (opt != "all")
			report_error(L, "'%s' was not understood as argument!", opt.c_str());
		for
			(Building_Index i = Building_Index::First();
			 i < tribe.get_nrbuildings(); ++i)
				rv.push_back(i);
	} else {
		// array of strings argument
		luaL_checktype(L, -1, LUA_TTABLE);

		lua_pushnil(L);
		while (lua_next(L, -2) != 0) {
			const char * name = luaL_checkstring(L, -1);
			Building_Index i = tribe.building_index(name);
			if (i == Building_Index::Null())
				report_error(L, "Unknown building type: '%s'", name);

			rv.push_back(i);

			lua_pop(L, 1); // pop value
		}
	}
}
int L_Player::m_allow_forbid_buildings(lua_State * L, bool allow)
{
	Player & p = get(L, get_egbase(L));

	std::vector<Building_Index> houses;
	m_parse_building_list(L, p.tribe(), houses);

	container_iterate_const(std::vector<Building_Index>, houses, i)
		p.allow_building_type(*i.current, allow);

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
const char L_Objective::className[] = "Objective";
const MethodType<L_Objective> L_Objective::Methods[] = {
	METHOD(L_Objective, remove),
	METHOD(L_Objective, __eq),
	{nullptr, nullptr},
};
const PropertyType<L_Objective> L_Objective::Properties[] = {
	PROP_RO(L_Objective, name),
	PROP_RW(L_Objective, title),
	PROP_RW(L_Objective, body),
	PROP_RW(L_Objective, visible),
	PROP_RW(L_Objective, done),
	{nullptr, nullptr, nullptr},
};

L_Objective::L_Objective(Widelands::Objective o) {
	m_name = o.name();
}

void L_Objective::__persist(lua_State * L) {
	PERS_STRING("name", m_name);
}
void L_Objective::__unpersist(lua_State * L) {
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
int L_Objective::get_name(lua_State * L) {
	Objective & o = get(L, get_game(L));
	lua_pushstring(L, o.name().c_str());
	return 1;
}
/* RST
	.. attribute:: title

		(RW) The line that is shown in the objectives menu
*/
int L_Objective::get_title(lua_State * L) {
	Objective & o = get(L, get_game(L));
	lua_pushstring(L, o.descname().c_str());
	return 1;
}
int L_Objective::set_title(lua_State * L) {
	Objective & o = get(L, get_game(L));
	o.set_descname(luaL_checkstring(L, -1));
	return 0;
}
/* RST
	.. attribute:: body

		(RW) The complete text of this objective. Can be Widelands Richtext.
*/
int L_Objective::get_body(lua_State * L) {
	Objective & o = get(L, get_game(L));
	lua_pushstring(L, o.descr().c_str());
	return 1;
}
int L_Objective::set_body(lua_State * L) {
	Objective & o = get(L, get_game(L));
	o.set_descr(luaL_checkstring(L, -1));
	return 0;
}
/* RST
	.. attribute:: visible

		(RW) is this objective shown in the objectives menu
*/
int L_Objective::get_visible(lua_State * L) {
	Objective & o = get(L, get_game(L));
	lua_pushboolean(L, o.visible());
	return 1;
}
int L_Objective::set_visible(lua_State * L) {
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
int L_Objective::get_done(lua_State * L) {
	Objective & o = get(L, get_game(L));
	lua_pushboolean(L, o.done());
	return 1;
}
int L_Objective::set_done(lua_State * L) {
	Objective & o = get(L, get_game(L));
	o.set_done(luaL_checkboolean(L, -1));

	const int32_t autosave = g_options.pull_section("global").get_int("autosave", 0);
	if (autosave <= 0) {
		return 0;
	}

	if (o.done()) {
		/** TRANSLATORS: File name for saving objective achieved */
		/** TRANSLATORS: %1$s = map name. %2$s = achievement name */
		std::string filename = (boost::format
			(_("%1$s (achieved %2$s)"))
			% get_egbase(L).get_map()->get_name()
			% o.descname().c_str()
		).str();
		get_game(L).save_handler().request_save(filename);
	}
	return 0;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
int L_Objective::remove(lua_State * L) {
	Game & g = get_game(L);
	// The next call checks if the Objective still exists
	get(L, g);
	g.map().mom().remove(m_name);
	return 0;
}

int L_Objective::__eq(lua_State * L) {
	const Manager<Objective> & mom = get_game(L).map().mom();

	const Objective * me = mom[m_name];
	const Objective * you = mom[(*get_user_class<L_Objective>(L, 2))->m_name];

	lua_pushboolean(L, (me && you) and (me == you));
	return 1;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */
Objective & L_Objective::get(lua_State * L, Widelands::Game & g) {
	Objective * o = g.map().mom()[m_name];
	if (!o)
		report_error
			(L, "Objective with name '%s' doesn't exist!", m_name.c_str());
	assert(o != nullptr);  // report_error never returns.
	return *o;
}

/* RST
Message
---------

.. class:: Message

	This represents a message in the Message Box of a given user.
*/
const char L_Message::className[] = "Message";
const MethodType<L_Message> L_Message::Methods[] = {
	METHOD(L_Message, __eq),
	{nullptr, nullptr},
};
const PropertyType<L_Message> L_Message::Properties[] = {
	PROP_RO(L_Message, sender),
	PROP_RO(L_Message, title),
	PROP_RO(L_Message, body),
	PROP_RO(L_Message, sent),
	PROP_RO(L_Message, duration),
	PROP_RO(L_Message, field),
	PROP_RW(L_Message, status),
	{nullptr, nullptr, nullptr},
};

L_Message::L_Message(uint8_t plr, Message_Id id) {
	m_plr = plr;
	m_mid = id;
}

void L_Message::__persist(lua_State * L) {
	PERS_UINT32("player", m_plr);
	PERS_UINT32("msg_idx", get_mos(L)->message_savers[m_plr - 1][m_mid].value());
}
void L_Message::__unpersist(lua_State * L) {
	UNPERS_UINT32("player", m_plr);
	uint32_t midx = 0;
	UNPERS_UINT32("msg_idx", midx);
	m_mid = Message_Id(midx);
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
	.. attribute:: sender

		(RO) The name of the sender of this message
*/
int L_Message::get_sender(lua_State * L) {
	lua_pushstring(L, get(L, get_game(L)).sender());
	return 1;
}
/* RST
	.. attribute:: title

		(RO) The title of this message
*/
int L_Message::get_title(lua_State * L) {
	lua_pushstring(L, get(L, get_game(L)).title());
	return 1;
}
/* RST
	.. attribute:: body

		(RO) The body of this message
*/
int L_Message::get_body(lua_State * L) {
	lua_pushstring(L, get(L, get_game(L)).body());
	return 1;
}

/* RST
	.. attribute:: sent

		(RO) The game time in milliseconds when this message was sent
*/
int L_Message::get_sent(lua_State * L) {
	lua_pushuint32(L, get(L, get_game(L)).sent());
	return 1;
}

/* RST
	.. attribute:: duration

		(RO) The time in milliseconds before this message is invalidated or nil if
		this message has an endless duration.
*/
int L_Message::get_duration(lua_State * L) {
	uint32_t d = get(L, get_game(L)).duration();
	if (d == Forever())
		return 0;
	lua_pushuint32(L, d);
	return 1;
}

/* RST
	.. attribute:: field

		(RO) The field that corresponds to this Message.
*/
int L_Message::get_field(lua_State * L) {
	Coords c = get(L, get_game(L)).position();
	if (c == Coords::Null())
		return 0;
	return to_lua<L_Field>(L, new L_Field(c));
}

/* RST
	.. attribute:: status

		(RW) The status of the message. Can be either of

			* new
			* read
			* archived
*/
int L_Message::get_status(lua_State * L) {
	switch (get(L, get_game(L)).status()) {
		case Message::New: lua_pushstring(L, "new"); break;
		case Message::Read: lua_pushstring(L, "read"); break;
		case Message::Archived: lua_pushstring(L, "archived"); break;
		default: return report_error(L, "Unknown Message status encountered!");
	}
	return 1;
}
int L_Message::set_status(lua_State * L) {
	Message::Status status = Message::New;
	std::string s = luaL_checkstring(L, -1);
	if (s == "new") status = Message::New;
	else if (s == "read") status = Message::Read;
	else if (s == "archived") status = Message::Archived;
	else return report_error(L, "Invalid message status <%s>!", s.c_str());

	get_plr(L, get_game(L)).messages().set_message_status(m_mid, status);

	return 0;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
int L_Message::__eq(lua_State * L) {
	lua_pushboolean(L, m_mid == (*get_user_class<L_Message>(L, 2))->m_mid);
	return 1;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */
Player & L_Message::get_plr(lua_State * L, Widelands::Game & game) {
	if (m_plr > MAX_PLAYERS)
		report_error(L, "Illegal player number %i",  m_plr);
	Player * rv = game.get_player(m_plr);
	if (!rv)
		report_error(L, "Player with the number %i does not exist", m_plr);
	return *rv;
}
const Message & L_Message::get(lua_State * L, Widelands::Game & game) {
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

	get_game(L).gameController()->report_result
		((*get_user_class<L_Player>(L, 1))->get(L, get_game(L)).player_number(),
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

	register_class<L_Player>(L, "game", true);
	add_parent<L_Player, LuaBases::L_PlayerBase>(L);
	lua_pop(L, 1); // Pop the meta table

	register_class<L_Objective>(L, "game");
	register_class<L_Message>(L, "game");
}

};
