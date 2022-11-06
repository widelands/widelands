/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "scripting/lua_game.h"

#include <memory>

#include "economy/economy.h"
#include "economy/flag.h"
#include "logic/filesystem_constants.h"
#include "logic/game_controller.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/message.h"
#include "logic/objective.h"
#include "logic/path.h"
#include "logic/player.h"
#include "logic/player_end_result.h"
#include "logic/playersmanager.h"
#include "scripting/globals.h"
#include "scripting/lua_map.h"
#include "wlapplication_options.h"
#include "wui/interactive_player.h"
#include "wui/story_message_box.h"

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

   This class represents one of the players in the game. You can access
   information about this player or act on his behalf. Note that you cannot
   instantiate a class of this type directly. To get an object of a player index the
   :class:`array` of :attr:`~wl.bases.EditorGameBase.players`:

   .. code-block:: lua

      local plr = wl.Game().players[1]                            -- the first player (usually blue)
      local plr = wl.Game().players[2]                            -- the second player
      local plr = wl.Game().players[wl.Game().interactive_player] -- the interactive player

*/
const char LuaPlayer::className[] = "Player";
const MethodType<LuaPlayer> LuaPlayer::Methods[] = {
   METHOD(LuaPlayer, send_to_inbox),
   METHOD(LuaPlayer, message_box),
   METHOD(LuaPlayer, sees_field),
   METHOD(LuaPlayer, seen_field),
   METHOD(LuaPlayer, allow_buildings),
   METHOD(LuaPlayer, forbid_buildings),
   METHOD(LuaPlayer, add_objective),
   METHOD(LuaPlayer, reveal_fields),
   METHOD(LuaPlayer, hide_fields),
   METHOD(LuaPlayer, mark_scenario_as_solved),
#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
   METHOD(LuaPlayer, acquire_training_wheel_lock),
   METHOD(LuaPlayer, release_training_wheel_lock),
   METHOD(LuaPlayer, mark_training_wheel_as_solved),
   METHOD(LuaPlayer, run_training_wheel),
   METHOD(LuaPlayer, skip_training_wheel),
#endif
   METHOD(LuaPlayer, get_ships),
   METHOD(LuaPlayer, get_buildings),
   METHOD(LuaPlayer, get_constructionsites),
   METHOD(LuaPlayer, get_suitability),
   METHOD(LuaPlayer, allow_workers),
   METHOD(LuaPlayer, switchplayer),
   METHOD(LuaPlayer, get_produced_wares_count),
   METHOD(LuaPlayer, set_attack_forbidden),
   METHOD(LuaPlayer, is_attack_forbidden),
   {nullptr, nullptr},
};
const PropertyType<LuaPlayer> LuaPlayer::Properties[] = {
   PROP_RO(LuaPlayer, name),
   PROP_RO(LuaPlayer, allowed_buildings),
   PROP_RO(LuaPlayer, objectives),
   PROP_RO(LuaPlayer, defeated),
   PROP_RO(LuaPlayer, resigned),
   PROP_RO(LuaPlayer, messages),
   PROP_RO(LuaPlayer, inbox),
   PROP_RO(LuaPlayer, color),
   PROP_RW(LuaPlayer, team),
   PROP_RO(LuaPlayer, tribe),
   PROP_RW(LuaPlayer, see_all),
   PROP_RW(LuaPlayer, allow_additional_expedition_items),
   PROP_RW(LuaPlayer, hidden_from_general_statistics),
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
int LuaPlayer::get_name(lua_State* L) {
	Widelands::Game& game = get_game(L);
	const Widelands::Player& p = get(L, game);
	lua_pushstring(L, p.get_name());
	return 1;
}

/* RST
   .. attribute:: allowed_buildings

      (RO) An :class:`array` with :class:`boolean` values with all buildings
      that are currently allowed for this player. Note that
      you can not enable/forbid a building by setting the value. Use
      :meth:`allow_buildings` or :meth:`forbid_buildings` for that.
*/
int LuaPlayer::get_allowed_buildings(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	const Widelands::Player& player = get(L, egbase);

	lua_newtable(L);
	for (Widelands::DescriptionIndex i = 0; i < egbase.descriptions().nr_buildings(); ++i) {
		const Widelands::BuildingDescr* building_descr = egbase.descriptions().get_building_descr(i);
		lua_pushstring(L, building_descr->name().c_str());
		lua_pushboolean(L, static_cast<int>(player.is_building_type_allowed(i)));
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: objectives

      (RO) A :class:`table` of :class:`objectives <wl.game.Objective>` in form of
      ``{objectivename=objective}``. You can change the objectives in this :class:`table`
      and it will be reflected in the game. To add a new item, use :meth:`add_objective`.
*/
int LuaPlayer::get_objectives(lua_State* L) {
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
int LuaPlayer::get_defeated(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get(L, get_egbase(L)).is_defeated()));
	return 1;
}

/* RST
   .. attribute:: resigned

      .. versionadded:: 1.1

      (RO) :const:`true` if this player has resigned, :const:`false` otherwise
*/
int LuaPlayer::get_resigned(lua_State* L) {
	const Widelands::PlayerEndStatus* p =
	   get_egbase(L).player_manager()->get_player_end_status(player_number());
	lua_pushboolean(L, p != nullptr && p->result == Widelands::PlayerEndResult::kResigned ? 1 : 0);
	return 1;
}

/* RST
   .. attribute:: messages

      (RO) An :class:`array` of **all** the :class:`inbox messages <InboxMessage>` sent to the
      player. Note that you can't add messages to this :class:`array`, use
      :meth:`send_to_inbox` for that.
*/
int LuaPlayer::get_messages(lua_State* L) {
	Widelands::Player& p = get(L, get_egbase(L));

	lua_newtable(L);
	uint32_t cidx = 1;
	for (const auto& temp_message : p.messages()) {
		lua_pushuint32(L, cidx++);
		to_lua<LuaInboxMessage>(L, new LuaInboxMessage(player_number(), temp_message.first));
		lua_rawset(L, -3);
	}

	return 1;
}

/* RST
   .. attribute:: inbox

      (RO) An :class:`array` of the :class:`inbox messages <InboxMessage>` that are either read
      or new. Note that you can't add messages to this :class:`array`, use :meth:`send_to_inbox`
      for that.
*/
int LuaPlayer::get_inbox(lua_State* L) {
	Widelands::Player& p = get(L, get_egbase(L));

	lua_newtable(L);
	uint32_t cidx = 1;
	for (const auto& temp_message : p.messages()) {
		if (temp_message.second->status() == Widelands::Message::Status::kArchived) {
			continue;
		}

		lua_pushuint32(L, cidx++);
		to_lua<LuaInboxMessage>(L, new LuaInboxMessage(player_number(), temp_message.first));
		lua_rawset(L, -3);
	}

	return 1;
}

/* RST
   .. attribute:: color

      (RO) The playercolor assigned to this player, in hex notation.
*/
int LuaPlayer::get_color(lua_State* L) {
	const Widelands::PlayerNumber pnumber = get(L, get_egbase(L)).player_number();
	lua_pushstring(L, kPlayerColors[pnumber - 1].hex_value());
	return 1;
}

/* RST
   .. attribute:: team

      (RW) The team number of this player (``0`` means player is not in a team).

      Normally only reading should be enough, however it's a nice idea to have
      a modular scenario, where teams form during the game.
*/
int LuaPlayer::set_team(lua_State* L) {
	get(L, get_egbase(L)).set_team_number(luaL_checkinteger(L, -1));
	return 0;
}
int LuaPlayer::get_team(lua_State* L) {
	lua_pushinteger(L, get(L, get_egbase(L)).team_number());
	return 1;
}

/* RST
   .. attribute:: tribe

      (RO) The :class:`wl.map.TribeDescription` for this player.
*/
int LuaPlayer::get_tribe(lua_State* L) {
	return to_lua<LuaMaps::LuaTribeDescription>(
	   L, new LuaMaps::LuaTribeDescription(&get(L, get_egbase(L)).tribe()));
}

/* RST
   .. attribute:: see_all

      (RW) If you set this to :const:`true`, the map will be completely visible for this
      player. Note that the player for whom this is set to :const:`true` can't attack any
      buildings anymore.
*/
int LuaPlayer::set_see_all(lua_State* const L) {
	get(L, get_egbase(L)).set_see_all(luaL_checkboolean(L, -1));
	return 0;
}
int LuaPlayer::get_see_all(lua_State* const L) {
	lua_pushboolean(L, static_cast<int>(get(L, get_egbase(L)).see_all()));
	return 1;
}

/* RST
   .. attribute:: allow_additional_expedition_items

      (RW) Whether the player may take additional items onto expeditions.
*/
int LuaPlayer::get_allow_additional_expedition_items(lua_State* L) {
	lua_pushboolean(
	   L, static_cast<int>(get(L, get_egbase(L)).additional_expedition_items_allowed()));
	return 1;
}
int LuaPlayer::set_allow_additional_expedition_items(lua_State* L) {
	get(L, get_egbase(L)).set_allow_additional_expedition_items(luaL_checkboolean(L, -1));
	return 0;
}

/* RST
   .. attribute:: hidden_from_general_statistics

      (RW) Whether this player's existence and statistics are not
      disclosed to other players in the general statistics menu.
*/
int LuaPlayer::get_hidden_from_general_statistics(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get(L, get_egbase(L)).is_hidden_from_general_statistics()));
	return 1;
}
int LuaPlayer::set_hidden_from_general_statistics(lua_State* L) {
	get(L, get_egbase(L)).set_hidden_from_general_statistics(luaL_checkboolean(L, -1));
	return 0;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

/* RST
   .. method:: send_to_inbox(title, message[, opts])

      Send a message to the player's inbox. One should prefer
      :meth:`send_to_inbox` from *messages.lua*, which
      has an option to wait before sending the message until the player leaves
      the roadbuilding mode.
      Title or Message can be a formatted using widelands'
      :ref:`richtext functions <richtext.lua>`.

      :arg title: The title of the message.
      :type title: :class:`string`

      :arg message: The text of the message.
      :type message: :class:`string`

      Opts is a :class:`table` of optional arguments and can be omitted. If it
      exist it must contain string/value pairs of the following type:

      :arg field: The field connected to this message. Default:
         no field connected to message.
      :type field: :class:`wl.map.Field`

      :arg status: Status to attach to this message. Can be :const:`"new"`, :const:`"read"`
         or :const:`"archived"`. Default: :const:`"new"`
      :type status: :class:`string`

      :arg popup: If :const:`true` the message window popups immediately,
         defaults to :const:`false`.
      :type popup: :class:`boolean`

      :arg icon: A file path to an icon which is shown beside this message. Defaults to
         ``"images/wui/messages/menu_toggle_objectives_menu.png"``.
      :type icon: :class:`string`

      :arg heading: A longer message heading to be shown within the message.
         If this is not set, `title` is used instead.
         Default: :const:`""`
      :type building: :class:`string`

      :returns: The message created.
      :rtype: :class:`wl.game.InboxMessage`
*/
int LuaPlayer::send_to_inbox(lua_State* L) {
	uint32_t n = lua_gettop(L);
	const std::string title = luaL_checkstring(L, 2);
	std::string heading = title;
	const std::string body = luaL_checkstring(L, 3);
	std::string icon = "images/wui/messages/menu_toggle_objectives_menu.png";
	Widelands::Coords c = Widelands::Coords::null();
	Widelands::Message::Status st = Widelands::Message::Status::kNew;
	bool popup = false;
	std::string sub_type;

	Widelands::Game& game = get_game(L);
	Widelands::Player& plr = get(L, game);

	if (n == 4) {
		// Optional arguments
		lua_getfield(L, 4, "field");
		if (!lua_isnil(L, -1)) {
			c = (*get_user_class<LuaMaps::LuaField>(L, -1))->coords();
		}
		lua_pop(L, 1);

		lua_getfield(L, 4, "status");
		if (!lua_isnil(L, -1)) {
			const std::string s = luaL_checkstring(L, -1);
			if (s == "new") {
				st = Widelands::Message::Status::kNew;
			} else if (s == "read") {
				st = Widelands::Message::Status::kRead;
			} else if (s == "archived") {
				st = Widelands::Message::Status::kArchived;
			} else {
				report_error(L, "Unknown message status: %s", s.c_str());
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, 4, "popup");
		if (!lua_isnil(L, -1)) {
			popup = luaL_checkboolean(L, -1);
		}
		lua_pop(L, 1);

		lua_getfield(L, 4, "icon");
		if (!lua_isnil(L, -1)) {
			const std::string s = luaL_checkstring(L, -1);
			if (!s.empty()) {
				icon = s;
			}
		}
		lua_getfield(L, 4, "heading");
		if (!lua_isnil(L, -1)) {
			const std::string s = luaL_checkstring(L, -1);
			if (!s.empty()) {
				heading = s;
			}
		}
		lua_getfield(L, 4, "sub_type");
		if (!lua_isnil(L, -1)) {
			const std::string s = luaL_checkstring(L, -1);
			if (!s.empty()) {
				sub_type = s;
			}
		}
	}

	Widelands::MessageId const message =
	   plr.add_message(game,
	                   std::unique_ptr<Widelands::Message>(new Widelands::Message(
	                      Widelands::Message::Type::kScenario, game.get_gametime(), title, icon,
	                      heading, body, c, 0, sub_type, st)),
	                   popup);

	return to_lua<LuaInboxMessage>(L, new LuaInboxMessage(player_number(), message));
}

/* RST
   .. method:: message_box(title, message[, opts])

      Shows a message box to the player. While the message box is displayed the
      game is set to pause. Usually you want to use :meth:`campaign_message_box`
      which has more options, e.g. easier positioning of message boxes.

      :arg title: The title of the message.
      :type title: :class:`string`

      :arg message: The text of the message.
      :type message: :class:`string`

      Opts is a :class:`table` of optional arguments and can be omitted. If it
      exist it must contain string/value pairs of the following type:

      :arg field: The main view will be centered on this field when the box
         pops up. Default: No field attached to message.
      :type field: :class:`wl.map.Field`

      :arg modal: If this is :const:`false`, the game will not wait for the message window to
         close, but continue at once.
      :type modal: :class:`boolean`

      :arg w: The width of the message box in pixels. Default: 400.
      :type w: :class:`integer`
      :arg h: The height of the message box in pixels. Default: 300.
      :type h: :class:`integer`
      :arg posx: x position of window in pixels. Default: centered
      :type posx: :class:`integer`
      :arg posy: y position of window in pixels. Default: centered
      :type posy: :class:`integer`
      :arg allow_next_scenario: If set to ``true``, show a button that allows starting
                                the next scenario at once. Defaults to ``false``.
      :type allow_next_scenario: :class:`boolean`

      :returns: :const:`nil`
*/
// UNTESTED
int LuaPlayer::message_box(lua_State* L) {
	Widelands::Game& game = get_game(L);
	// don't show message boxes in replays, cause they crash the game
	if (game.game_controller()->get_game_type() == GameController::GameType::kReplay) {
		return 1;
	}

	uint32_t w = 400;
	uint32_t h = 300;
	int32_t posx = -1;
	int32_t posy = -1;
	Widelands::Coords coords = Widelands::Coords::null();
	bool is_modal = true;
	bool allow_next_scenario = false;

#define CHECK_UINT(var)                                                                            \
	lua_getfield(L, -1, #var);                                                                      \
	if (!lua_isnil(L, -1))                                                                          \
		var = luaL_checkuint32(L, -1);                                                               \
	lua_pop(L, 1);

	if (lua_gettop(L) == 4) {
		CHECK_UINT(posx)
		CHECK_UINT(posy)
		CHECK_UINT(w)
		CHECK_UINT(h)

		// If a field has been defined, read the coordinates to jump to.
		lua_getfield(L, 4, "field");
		if (!lua_isnil(L, -1)) {
			coords = (*get_user_class<LuaMaps::LuaField>(L, -1))->coords();
		}
		lua_pop(L, 1);

		// Check whether we want a modal window
		lua_getfield(L, 4, "modal");
		if (!lua_isnil(L, -1)) {
			is_modal = luaL_checkboolean(L, -1);
		}
		lua_pop(L, 1);

		lua_getfield(L, 4, "allow_next_scenario");
		if (!lua_isnil(L, -1)) {
			allow_next_scenario = luaL_checkboolean(L, -1);
		}
		lua_pop(L, 1);
	}
#undef CHECK_UINT

	if (is_modal) {
		std::unique_ptr<StoryMessageBox> mb(new StoryMessageBox(&game, coords, luaL_checkstring(L, 2),
		                                                        luaL_checkstring(L, 3), posx, posy, w,
		                                                        h, is_modal, allow_next_scenario));
		mb->run<UI::Panel::Returncodes>();
	} else {
		new StoryMessageBox(&game, coords, luaL_checkstring(L, 2), luaL_checkstring(L, 3), posx, posy,
		                    w, h, is_modal, allow_next_scenario);
	}

	return 1;
}

/* RST
   .. method:: sees_field(f)

      Returns :const:`true` if this field is currently seen by this player.

      :returns: :const:`true` or :const:`false`.
      :rtype: :class:`boolean`
*/
int LuaPlayer::sees_field(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);

	Widelands::MapIndex const i =
	   (*get_user_class<LuaMaps::LuaField>(L, 2))->fcoords(L).field - &egbase.map()[0];

	lua_pushboolean(L, static_cast<int>(get(L, egbase).is_seeing(i)));
	return 1;
}

/* RST
   .. method:: seen_field(f)

      Returns :const:`true` if this field has ever been seen by this player or
      is currently seen.

      :returns: :const:`true` or :const:`false`.
      :rtype: :class:`bool`
*/
int LuaPlayer::seen_field(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);

	Widelands::MapIndex const i =
	   (*get_user_class<LuaMaps::LuaField>(L, 2))->fcoords(L).field - &egbase.map()[0];

	lua_pushboolean(
	   L, static_cast<int>(get(L, egbase).get_vision(i) != Widelands::VisibleState::kUnexplored));
	return 1;
}

/* RST
   .. method:: allow_buildings(what)

      This method disables or enables buildings to build for the player. What
      can either be the single string "all" or a list of strings containing
      the names of the buildings that are allowed.

      :arg what: Either the string :const:`"all"` or an :class:`array` containing the names of
         the allowed buildings.
      :type what: :class:`string` or :class:`array`
      :returns: :const:`nil`

      The opposite function is :meth:`forbid_buildings`
*/
int LuaPlayer::allow_buildings(lua_State* L) {
	return allow_forbid_buildings(L, true);
}

/* RST
   .. method:: forbid_buildings(what)

      See :meth:`allow_buildings` for arguments. This is the opposite function.

      :arg what: Either the string :const:`"all"` or an :class:`array` containing the names
         of the allowed buildings.
      :type what: :class:`string` or :class:`array`
      :returns: :const:`nil`
*/
int LuaPlayer::forbid_buildings(lua_State* L) {
	return allow_forbid_buildings(L, false);
}

/* RST
   .. method:: add_objective(name, title, body)

      Add a new :class:`~wl.game.Objective` for this player. Will report an error, if an
      Objective with the same name is already registered - note that the names
      for the objectives are shared internally for all players, so not even
      another player can have an objective with the same name.

      :arg name: The name of the objective. Has to be unique.
      :type name: :class:`string`
      :arg title: The title of the objective that will be shown in the menu.
      :type title: :class:`string`
      :arg body: The full text of the objective.
      :type body: :class:`string`

      :returns: The objective created.
      :rtype: :class:`wl.game.Objective`
*/
int LuaPlayer::add_objective(lua_State* L) {
	Widelands::Game& game = get_game(L);

	Widelands::Map::Objectives* objectives = game.mutable_map()->mutable_objectives();

	const std::string name = luaL_checkstring(L, 2);
	if (objectives->count(name) != 0u) {
		report_error(L, "An objective with the name '%s' already exists!", name.c_str());
	}

	Widelands::Objective* o = new Widelands::Objective(name);
	o->set_done(false);
	o->set_descname(luaL_checkstring(L, 3));
	o->set_descr(luaL_checkstring(L, 4));
	o->set_visible(true);

	objectives->insert(std::make_pair(name, std::unique_ptr<Widelands::Objective>(o)));
	return to_lua<LuaObjective>(L, new LuaObjective(*o));
}

/* RST
   .. method:: reveal_fields(fields)

      Make these **fields** visible for the player. The fields will remain visible until they
      are hidden again by :meth:`hide_fields`, even if they are not in vision range of any
      buildings or workers.

      See also :ref:`field_animations` for animated revealing.

      :arg fields: The fields to reveal.
      :type fields: :class:`array` of :class:`fields <wl.map.Field>`.

      :returns: :const:`nil`
*/
int LuaPlayer::reveal_fields(lua_State* L) {
	Widelands::Game& game = get_game(L);
	Widelands::Player& p = get(L, game);

	luaL_checktype(L, 2, LUA_TTABLE);

	lua_pushnil(L); /* first key */
	while (lua_next(L, 2) != 0) {
		p.hide_or_reveal_field((*get_user_class<LuaMaps::LuaField>(L, -1))->coords(),
		                       Widelands::HideOrRevealFieldMode::kReveal);
		lua_pop(L, 1);
	}

	return 0;
}

/* RST
   .. method:: hide_fields(fields[, state = "seen"])

      Undo the effect of :meth:`reveal_fields` on these fields for the player and
      optionally completely hide them or reset them to unexplored.

      See also :ref:`field_animations` for animated hiding.

      .. note:: Passing a :class:`boolean` as the **state** argument is deprecated.
         Use :const:`"permanent"` instead of :const:`true` and :const:`"seen"` instead of
         :const:`false`.

      :arg fields: The fields to hide.
      :type fields: :class:`array` of :class:`fields <wl.map.Field>`.

      :arg state: (Optional) One of :const:`"permanent"`, :const:`"explorable"` or :const:`"seen"`.
         If :const:`"permanent"`, the fields will be marked as completely
         hidden and will not be seen by buildings or workers until they are revealed again
         by :meth:`reveal_fields`. If :const:`"explorable"`, they will no longer be visible,
         but can still be rediscovered by buildings, ships or workers (own or allied). If
         :const:`"seen"`, they will no longer be permanently visible (fading to foggy), but
         can still be seen by buildings or workers (own or allied), and the player will
         remember the last state that they had been seen. This is the default.
      :type state: :class:`string`

      :returns: :const:`nil`
*/
int LuaPlayer::hide_fields(lua_State* L) {
	Widelands::Game& game = get_game(L);
	Widelands::Player& p = get(L, game);

	luaL_checktype(L, 2, LUA_TTABLE);
	// TODO(hessenfarmer): Boolean check for compatibility. Remove after v1.0
	const std::string state = lua_isnone(L, 3)        ? "seen" :
	                          !lua_isboolean(L, 3)    ? luaL_checkstring(L, 3) :
	                          luaL_checkboolean(L, 3) ? "permanent" :
                                                       "seen";
	const Widelands::HideOrRevealFieldMode mode =
	   (state == "permanent")  ? Widelands::HideOrRevealFieldMode::kHide :
	   (state == "explorable") ? Widelands::HideOrRevealFieldMode::kUnexplore :
                                Widelands::HideOrRevealFieldMode::kUnreveal;
	if (mode == Widelands::HideOrRevealFieldMode::kUnreveal && state != "seen") {
		report_error(L, "'%s' is no valid parameter for hide_fields!", state.c_str());
	}

	lua_pushnil(L); /* first key */
	while (lua_next(L, 2) != 0) {
		p.hide_or_reveal_field((*get_user_class<LuaMaps::LuaField>(L, -1))->coords(), mode);
		lua_pop(L, 1);
	}

	return 0;
}

/* RST
   .. method:: mark_scenario_as_solved(name)

      Marks a campaign scenario as solved. Reads the scenario definition in
      *data/campaigns/campaigns.lua* to check which scenario and/or campaign should be
      revealed as a result. This only works for the interactive player and most likely
      also only in single player games.

      :arg name: The name of the scenario to be marked as solved.
      :type name: :class:`string`
*/
// UNTESTED
int LuaPlayer::mark_scenario_as_solved(lua_State* L) {
	if (get_game(L).get_ipl()->player_number() != player_number()) {
		report_error(L, "Can only be called for interactive player!");
	}

	Profile campvis(kCampVisFile.c_str());
	campvis.pull_section("scenarios").set_bool(luaL_checkstring(L, 2), true);
	campvis.write(kCampVisFile.c_str(), false);

	return 0;
}

#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
/* R#S#T
   .. method:: acquire_training_wheel_lock(name)

      Try to mark the given training wheel as the active one.

      :arg name: The name of the training wheel that wants to run.
      :type name: :class:`string`

      :returns: Whether the training wheel is allowed to run.
      :rtype: :class:`boolean`
*/
// UNTESTED
int LuaPlayer::acquire_training_wheel_lock(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "One argument is required for acquire_training_wheel_lock(string)");
	}
	const bool success = get_game(L).acquire_training_wheel_lock(luaL_checkstring(L, 2));
	lua_pushboolean(L, success);
	return 1;
}

/* R#S#T
   .. method:: release_training_wheel_lock()

      Mark the current training wheel as no longer active without solving it.
*/
// UNTESTED
int LuaPlayer::release_training_wheel_lock(lua_State* L) {
	get_game(L).release_training_wheel_lock();
	return 0;
}

/* R#S#T
   .. method:: mark_training_wheel_as_solved(name)

      Marks a global training wheel objective as solved. Also releases the lock.

      :arg name: The name of the training wheel to be marked as solved.
      :type name: :class:`string`
*/
// UNTESTED
int LuaPlayer::mark_training_wheel_as_solved(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "One argument is required for mark_training_wheel_as_solved(string)");
	}

	get_game(L).mark_training_wheel_as_solved(luaL_checkstring(L, 2));
	return 0;
}

/* R#S#T
   .. method:: run_training_wheel(name[, force])

      Trigger running a training wheel. This function will skip the dependency check, so the given
      training wheel will run even if its preconditions haven't been met. No further training wheels
      will be triggered. Previously solved training wheels will not be run unless ``force == true``.

      .. note:: Intended for use in scenarios only.

      :arg name: The name of the training wheel to be run.
      :type name: :class:`string`

      :arg force: (Optional) Whether this training wheel should be run anyway if it was
         previously solved.
      :type force: :class:`boolean`
*/
// UNTESTED
int LuaPlayer::run_training_wheel(lua_State* L) {
	if (lua_gettop(L) < 2 || lua_gettop(L) > 3) {
		report_error(L, "1-2 arguments are required for run_training_wheel(string[, boolean])");
	}

	const bool force = lua_gettop(L) == 3 && luaL_checkboolean(L, 3);
	get_game(L).run_training_wheel(luaL_checkstring(L, 2), force);
	return 0;
}

/* R#S#T
   .. method:: skip_training_wheel(name)

      Skips the execution of a training wheel and activates the training wheels that depend on it.
      Also releases the lock.

      :arg name: The name of the training wheel to be skipped.
      :type name: :class:`string`
*/
// UNTESTED
int LuaPlayer::skip_training_wheel(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "One argument is required for skip_training_wheel(string)");
	}

	get_game(L).skip_training_wheel(luaL_checkstring(L, 2));
	return 0;
}
#endif

/* RST
   .. method:: get_ships()

      :returns: An :class:`array` of player's :class:`ships <wl.map.Ship>`.
      :rtype: :class:`array`
*/
int LuaPlayer::get_ships(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::PlayerNumber p = (get(L, egbase)).player_number();
	lua_newtable(L);
	uint32_t cidx = 1;
	for (const auto& serial : egbase.player(p).ships()) {
		Widelands::MapObject* obj = egbase.objects().get_object(serial);
		assert(obj->descr().type() == Widelands::MapObjectType::SHIP);
		upcast(Widelands::Ship, ship, obj);
		lua_pushuint32(L, cidx++);
		LuaMaps::upcasted_map_object_to_lua(L, ship);
		lua_rawset(L, -3);
	}
	return 1;
}

/* RST
   .. method:: get_buildings(which)

      **which** can be either a single name or an :class:`array` of names. In the first
      case, the method returns an :class:`array` of all Buildings that the player has of
      this kind. If **which** is an :class:`array`, the function returns a :class:`table` of
      ``{name=array_of_buildings}`` pairs.

      :arg which: The name of a building or an :class:`array` of building names.
      :type which: :class:`string` or :class:`array`
      :returns: Information about the player's buildings,
         see :class:`wl.map.Building`.
      :rtype: :class:`array` or :class:`table`
*/
int LuaPlayer::get_buildings(lua_State* L) {
	return do_get_buildings(L, false);
}

/* RST
   .. method:: get_constructionsites(which)

      **which** can be either a single name or an :class:`array` of names. In the first
      case, the method returns an :class:`array` of all constructionsites that the
      player has of this kind. If which is an :class:`array`, the function returns a
      :class:`table` of ``{name=array_of_constructionsites}`` pairs.

      :arg which: The internal name of a constructionsites building or an :class:`array` of
         building names.
      :type which: :class:`string` or :class:`array`
      :returns: Information about the player's constructionsites,
         see :class:`wl.map.ConstructionSite`.
      :rtype: :class:`array` or :class:`table`
*/
int LuaPlayer::get_constructionsites(lua_State* L) {
	return do_get_buildings(L, true);
}

/* RST
   .. method:: get_suitability(building, field)

      Returns whether this building type can be placed on this field. This
      is mainly useful in initializations where buildings must be placed
      automatically.

      :arg building: The internal name of the building to check for.
      :type building: :class:`string`
      :arg field: The field to check for suitability.
      :type field: :class:`wl.map.Field`

      :returns: :const:`true` if the field has a suitable building plot for this building,
         :const:`false` otherwise.
      :rtype: :class:`boolean`
*/
// UNTESTED
int LuaPlayer::get_suitability(lua_State* L) {
	const Widelands::Game& game = get_game(L);
	const Widelands::Descriptions& descriptions = game.descriptions();

	const char* name = luaL_checkstring(L, 2);
	Widelands::DescriptionIndex i = descriptions.building_index(name);
	if (!descriptions.building_exists(i)) {
		report_error(L, "Unknown building type: <%s>", name);
	}

	lua_pushboolean(L, static_cast<int>(descriptions.get_building_descr(i)->suitability(
	                      game.map(), (*get_user_class<LuaMaps::LuaField>(L, 3))->fcoords(L))));
	return 1;
}

/* RST
   .. method:: allow_workers(what)

      This will become the corresponding function to :meth:`allow_buildings`,
      but at the moment this is only a stub that accepts only :const:`"all"` as
      argument. It then activates all workers for the player, that means all
      workers are allowed to spawn in all warehouses.
*/
int LuaPlayer::allow_workers(lua_State* L) {

	if (luaL_checkstring(L, 2) != std::string("all")) {
		report_error(L, "Argument must be <all>!");
	}

	Widelands::Game& game = get_game(L);
	const Widelands::TribeDescr& tribe = get(L, game).tribe();
	Widelands::Player& player = get(L, game);

	const std::vector<Widelands::DescriptionIndex>& worker_types_without_cost =
	   tribe.worker_types_without_cost();

	for (const Widelands::DescriptionIndex& worker_index : tribe.workers()) {
		const Widelands::WorkerDescr* worker_descr =
		   game.descriptions().get_worker_descr(worker_index);
		if (!worker_descr->is_buildable()) {
			continue;
		}
		player.allow_worker_type(worker_index, true);

		if (worker_descr->buildcost().empty()) {
			//  Workers of this type can be spawned in warehouses. Start it.
			uint8_t worker_types_without_cost_index = 0;
			for (;; ++worker_types_without_cost_index) {
				assert(worker_types_without_cost_index < worker_types_without_cost.size());
				if (worker_types_without_cost.at(worker_types_without_cost_index) == worker_index) {
					break;
				}
			}
			for (const auto& economy : player.economies()) {
				for (Widelands::Warehouse* warehouse : economy.second->warehouses()) {
					warehouse->enable_spawn(game, worker_types_without_cost_index);
				}
			}
		}
	}
	return 0;
}

/* RST
   .. method:: switchplayer(playernumber)

      Switch the :attr:`~wl.Game.interactive_player` to the player with **playernumber** gaining
      full control over the player given by **playernumber** and loosing control over the
      formerly interactive player.

      :arg playernumber: An index in the :class:`array` of :attr:`~wl.bases.EditorGameBase.players`.
      :type playernumber: :class:`integer`
*/
int LuaPlayer::switchplayer(lua_State* L) {
	Widelands::Game& game = get_game(L);

	uint8_t newplayer = luaL_checkinteger(L, -1);
	InteractivePlayer* ipl = game.get_ipl();
	// only switch, if this is our player!
	if (ipl->player_number() == player_number()) {
		ipl->set_player_number(newplayer);
	}
	return 0;
}

/* RST
   .. method:: produced_wares_count(what)

      Returns count of wares produced by the player up to now.

      :arg what: This can be either :const:`"all"` or a single name of a ware or an :class`array`
         of ware names.
      :type what: :class:`string` or :class:`array`
      :returns: If a single ware name is given, integer is returned, otherwise a :class:`table`
         is returned.
      :rtype: :class:`integer` or :class:`table`
*/
int LuaPlayer::get_produced_wares_count(lua_State* L) {
	Widelands::Player& p = get(L, get_egbase(L));
	const Widelands::TribeDescr& tribe = p.tribe();
	int32_t nargs = lua_gettop(L);
	if (nargs != 2) {
		report_error(L, "One argument is required for produced_wares_count()");
	}
	std::vector<Widelands::DescriptionIndex> requested_wares;
	Widelands::DescriptionIndex single_ware = Widelands::INVALID_INDEX;

	LuaMaps::parse_wares_workers_list(L, tribe, &single_ware, &requested_wares, true);

	if (single_ware != Widelands::INVALID_INDEX) {
		// We return single number
		lua_pushuint32(L, p.get_current_produced_statistics(single_ware));
	} else {
		// We return array of ware:quantity
		assert(!requested_wares.empty());
		lua_newtable(L);
		for (const Widelands::DescriptionIndex& idx : requested_wares) {
			lua_pushstring(L, tribe.get_ware_descr(idx)->name());
			lua_pushuint32(L, p.get_current_produced_statistics(idx));
			lua_settable(L, -3);
		}
	}

	return 1;
}

/* RST
   .. method:: is_attack_forbidden(playernumber)

      Returns :const:`true` if this player is currently forbidden to attack the player with
      the specified **playernumber**. Note that the return value :const:`false` does not
      necessarily mean that this player *can* attack the other player, as they might for
      example be in the same team.

      :arg playernumber: An index in the :class:`array` of :attr:`~wl.bases.EditorGameBase.players`.
      :type playernumber: :class:`int`
      :rtype: :class:`boolean`
*/
int LuaPlayer::is_attack_forbidden(lua_State* L) {
	lua_pushboolean(
	   L, static_cast<int>(get(L, get_egbase(L)).is_attack_forbidden(luaL_checkinteger(L, 2))));
	return 1;
}

/* RST
   .. method:: set_attack_forbidden(playernumber, forbid)

      Sets whether this player is forbidden to attack the player with the specified
      **playernumber**. Note that setting this to :const:`false` does not necessarily mean that this
      player *can* attack the other player, as they might for example be in the same team.

      :arg playernumber: An index in the :class:`array` of :attr:`~wl.bases.EditorGameBase.players`.
      :type playernumber: :class:`int`
      :arg forbid: If this is :const:`true` forbids attacking, :const:`false` allows
         attacking (if the player is not in the same team).
      :type forbid: :class:`boolean`
*/
int LuaPlayer::set_attack_forbidden(lua_State* L) {
	get(L, get_egbase(L)).set_attack_forbidden(luaL_checkinteger(L, 2), luaL_checkboolean(L, 3));
	return 0;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */
void LuaPlayer::parse_building_list(lua_State* L,
                                    const Widelands::TribeDescr& tribe,
                                    std::vector<Widelands::DescriptionIndex>& rv) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	const Widelands::Descriptions& descriptions = egbase.descriptions();
	if (lua_isstring(L, -1) != 0) {
		std::string opt = luaL_checkstring(L, -1);
		if (opt != "all") {
			report_error(L, "'%s' was not understood as argument!", opt.c_str());
		}
		// Only act on buildings that the tribe has or could conquer
		const Widelands::TribeDescr& tribe_descr = get(L, egbase).tribe();
		for (size_t i = 0; i < descriptions.nr_buildings(); ++i) {
			const Widelands::DescriptionIndex& building_index =
			   static_cast<Widelands::DescriptionIndex>(i);
			const Widelands::BuildingDescr& descr = *tribe_descr.get_building_descr(building_index);
			if (tribe_descr.has_building(building_index) ||
			    descr.type() == Widelands::MapObjectType::MILITARYSITE) {
				rv.push_back(building_index);
			}
		}
	} else {
		// array of strings argument
		luaL_checktype(L, -1, LUA_TTABLE);

		lua_pushnil(L);
		while (lua_next(L, -2) != 0) {
			const char* name = luaL_checkstring(L, -1);
			Widelands::DescriptionIndex i = tribe.building_index(name);
			if (!descriptions.building_exists(i)) {
				report_error(L, "Unknown building type: '%s'", name);
			}

			rv.push_back(i);

			lua_pop(L, 1);  // pop value
		}
	}
}

int LuaPlayer::allow_forbid_buildings(lua_State* L, bool allow) {
	Widelands::Player& p = get(L, get_egbase(L));

	std::vector<Widelands::DescriptionIndex> houses;
	parse_building_list(L, p.tribe(), houses);

	for (const Widelands::DescriptionIndex& house : houses) {
		p.allow_building_type(house, allow);
	}
	return 0;
}

int LuaPlayer::do_get_buildings(lua_State* L, const bool csites) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::Player& p = get(L, egbase);

	// if only one string, convert to array so that we can use
	// parse_building_list
	bool return_array = true;
	if (lua_isstring(L, -1) != 0) {
		const char* name = luaL_checkstring(L, -1);
		lua_pop(L, 1);
		lua_newtable(L);
		lua_pushuint32(L, 1);
		lua_pushstring(L, name);
		lua_rawset(L, -3);
		return_array = false;
	}

	std::vector<Widelands::DescriptionIndex> houses;
	parse_building_list(L, p.tribe(), houses);

	lua_newtable(L);

	uint32_t cidx = 1;
	for (const Widelands::DescriptionIndex& house : houses) {
		const std::vector<Widelands::Player::BuildingStats>& vec = p.get_building_statistics(house);

		if (return_array) {
			lua_pushstring(L, p.tribe().get_building_descr(house)->name());
			lua_newtable(L);
			cidx = 1;
		}

		for (const auto& stats : vec) {
			if (csites ^ stats.is_constructionsite) {
				continue;
			}

			lua_pushuint32(L, cidx++);
			LuaMaps::upcasted_map_object_to_lua(L, egbase.map()[stats.pos].get_immovable());
			lua_rawset(L, -3);
		}

		if (return_array) {
			lua_rawset(L, -3);
		}
	}
	return 1;
}

/* RST
Objective
---------

.. class:: Objective

   This represents an Objective, a goal for the player in the game. This is
   mainly for displaying to the user, but each objective also has an attribute
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
   PROP_RO(LuaObjective, name),    PROP_RW(LuaObjective, title), PROP_RW(LuaObjective, body),
   PROP_RW(LuaObjective, visible), PROP_RW(LuaObjective, done),  {nullptr, nullptr, nullptr},
};

LuaObjective::LuaObjective(const Widelands::Objective& o) : name_(o.name()) {
}

void LuaObjective::__persist(lua_State* L) {
	PERS_STRING("name", name_);
}
void LuaObjective::__unpersist(lua_State* L) {
	UNPERS_STRING("name", name_)
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
   .. attribute:: name

      (RO) The internal name. You can reference this object via
      :attr:`wl.game.Player.objectives` with :attr:`name` as key.
*/
int LuaObjective::get_name(lua_State* L) {
	const Widelands::Objective& o = get(L, get_game(L));
	lua_pushstring(L, o.name().c_str());
	return 1;
}
/* RST
   .. attribute:: title

      (RW) The line that is shown in the objectives menu.
*/
int LuaObjective::get_title(lua_State* L) {
	const Widelands::Objective& o = get(L, get_game(L));
	lua_pushstring(L, o.descname().c_str());
	return 1;
}
int LuaObjective::set_title(lua_State* L) {
	Widelands::Objective& o = get(L, get_game(L));
	o.set_descname(luaL_checkstring(L, -1));
	return 0;
}
/* RST
   .. attribute:: body

      (RW) The complete text of this objective. Can be formatted via
      Widelands :ref:`richtext <richtext.lua>`.
*/
int LuaObjective::get_body(lua_State* L) {
	const Widelands::Objective& o = get(L, get_game(L));
	lua_pushstring(L, o.descr().c_str());
	return 1;
}
int LuaObjective::set_body(lua_State* L) {
	Widelands::Objective& o = get(L, get_game(L));
	o.set_descr(luaL_checkstring(L, -1));
	return 0;
}
/* RST
   .. attribute:: visible

      (RW) This is :const:`true` if this objective is shown in the objectives menu,
      :const:`false` otherwisae.
*/
int LuaObjective::get_visible(lua_State* L) {
	const Widelands::Objective& o = get(L, get_game(L));
	lua_pushboolean(L, static_cast<int>(o.visible()));
	return 1;
}
int LuaObjective::set_visible(lua_State* L) {
	Widelands::Objective& o = get(L, get_game(L));
	o.set_visible(luaL_checkboolean(L, -1));
	return 0;
}
/* RST
   .. attribute:: done

      (RW) Defines if this objective is already fulfilled. If done is
      :const:`true`, the objective will not be shown to the user, no matter what
      :attr:`visible` is set to. A savegame will be created when this attribute
      is changed to :const:`true`.

*/
int LuaObjective::get_done(lua_State* L) {
	const Widelands::Objective& o = get(L, get_game(L));
	lua_pushboolean(L, static_cast<int>(o.done()));
	return 1;
}
int LuaObjective::set_done(lua_State* L) {
	Widelands::Objective& o = get(L, get_game(L));
	o.set_done(luaL_checkboolean(L, -1));

	const int32_t autosave = get_config_int("autosave", 0);
	if (autosave <= 0) {
		return 0;
	}

	if (o.done()) {
		/** TRANSLATORS: File name for saving objective achieved */
		/** TRANSLATORS: %1% = map name. %2% = achievement name */
		std::string filename = _("%1% (%2%)");
		i18n::Textdomain td("maps");  // TODO(Nordfriese): What if the scenario is in an add-on?
		filename = format(filename, _(get_egbase(L).map().get_name()), o.descname());
		get_game(L).save_handler().request_save(filename);
	}
	return 0;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
int LuaObjective::remove(lua_State* L) {
	Widelands::Game& g = get_game(L);
	// The next call checks if the Objective still exists
	get(L, g);
	g.mutable_map()->mutable_objectives()->erase(name_);
	return 0;
}

int LuaObjective::__eq(lua_State* L) {
	const Widelands::Map::Objectives& objectives = get_game(L).map().objectives();

	const Widelands::Map::Objectives::const_iterator me = objectives.find(name_);
	const Widelands::Map::Objectives::const_iterator other =
	   objectives.find((*get_user_class<LuaObjective>(L, 2))->name_);

	lua_pushboolean(L, static_cast<int>((me != objectives.end() && other != objectives.end()) &&
	                                    (me->second->name() == other->second->name())));
	return 1;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */
Widelands::Objective& LuaObjective::get(lua_State* L, Widelands::Game& g) {
	Widelands::Map::Objectives* objectives = g.mutable_map()->mutable_objectives();
	Widelands::Map::Objectives::iterator i = objectives->find(name_);
	if (i == objectives->end()) {
		report_error(L, "Objective with name '%s' doesn't exist!", name_.c_str());
	}
	return *i->second;
}

/* RST
InboxMessage
------------

.. class:: InboxMessage

   This represents a message in the inbox of a player.
*/
const char LuaInboxMessage::className[] = "InboxMessage";
const MethodType<LuaInboxMessage> LuaInboxMessage::Methods[] = {
   METHOD(LuaInboxMessage, __eq),
   {nullptr, nullptr},
};
const PropertyType<LuaInboxMessage> LuaInboxMessage::Properties[] = {
   PROP_RO(LuaInboxMessage, title),     PROP_RO(LuaInboxMessage, body),
   PROP_RO(LuaInboxMessage, sent),      PROP_RO(LuaInboxMessage, field),
   PROP_RW(LuaInboxMessage, status),    PROP_RO(LuaInboxMessage, heading),
   PROP_RO(LuaInboxMessage, icon_name), {nullptr, nullptr, nullptr},
};

LuaInboxMessage::LuaInboxMessage(uint8_t plr, Widelands::MessageId id) {
	player_number_ = plr;
	message_id_ = id;
}

void LuaInboxMessage::__persist(lua_State* L) {
	PERS_UINT32("player", player_number_);
	PERS_UINT32("msg_idx", get_mos(L)->message_savers[player_number_ - 1][message_id_].value());
}
void LuaInboxMessage::__unpersist(lua_State* L) {
	UNPERS_UINT32("player", player_number_)
	uint32_t midx = 0;
	UNPERS_UINT32("msg_idx", midx)
	message_id_ = Widelands::MessageId(midx);
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: title

      (RO) The title of this message.
*/
int LuaInboxMessage::get_title(lua_State* L) {
	lua_pushstring(L, get(L, get_game(L)).title());
	return 1;
}
/* RST
   .. attribute:: body

      (RO) The body of this message.
*/
int LuaInboxMessage::get_body(lua_State* L) {
	lua_pushstring(L, get(L, get_game(L)).body());
	return 1;
}

/* RST
   .. attribute:: sent

      (RO) The game time in milliseconds when this message was sent.
*/
int LuaInboxMessage::get_sent(lua_State* L) {
	lua_pushuint32(L, get(L, get_game(L)).sent().get());
	return 1;
}

/* RST
   .. attribute:: field

      (RO) The field that corresponds to this Message.
*/
int LuaInboxMessage::get_field(lua_State* L) {
	Widelands::Coords c = get(L, get_game(L)).position();
	if (c == Widelands::Coords::null()) {
		return 0;
	}
	return to_lua<LuaMaps::LuaField>(L, new LuaMaps::LuaField(c));
}

/* RST
   .. attribute:: status

      (RW) The status of the message. Can be either of

         * :const:`"new"`
         * :const:`"read"`
         * :const:`"archived"`
*/
int LuaInboxMessage::get_status(lua_State* L) {
	switch (get(L, get_game(L)).status()) {
	case Widelands::Message::Status::kNew:
		lua_pushstring(L, "new");
		break;
	case Widelands::Message::Status::kRead:
		lua_pushstring(L, "read");
		break;
	case Widelands::Message::Status::kArchived:
		lua_pushstring(L, "archived");
		break;
	}
	return 1;
}
int LuaInboxMessage::set_status(lua_State* L) {
	Widelands::Message::Status status = Widelands::Message::Status::kNew;
	std::string s = luaL_checkstring(L, -1);
	if (s == "new") {
		status = Widelands::Message::Status::kNew;
	} else if (s == "read") {
		status = Widelands::Message::Status::kRead;
	} else if (s == "archived") {
		status = Widelands::Message::Status::kArchived;
	} else {
		report_error(L, "Invalid message status <%s>!", s.c_str());
	}

	get_plr(L, get_game(L)).get_messages()->set_message_status(message_id_, status);

	return 0;
}

/* RST
   .. attribute:: heading

      (RO) The long heading of this message that is shown in the body.
*/
int LuaInboxMessage::get_heading(lua_State* L) {
	lua_pushstring(L, get(L, get_game(L)).heading());
	return 1;
}

/* RST
   .. attribute:: icon_name

      (RO) The filename for the icon that is shown with the message title.
*/
int LuaInboxMessage::get_icon_name(lua_State* L) {
	lua_pushstring(L, get(L, get_game(L)).icon_filename());
	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
int LuaInboxMessage::__eq(lua_State* L) {
	lua_pushboolean(
	   L, static_cast<int>(message_id_ == (*get_user_class<LuaInboxMessage>(L, 2))->message_id_));
	return 1;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */
Widelands::Player& LuaInboxMessage::get_plr(lua_State* L, const Widelands::Game& game) const {
	if (player_number_ > kMaxPlayers) {
		report_error(L, "Illegal player number %i", player_number_);
	}
	Widelands::Player* rv = game.get_player(player_number_);
	if (rv == nullptr) {
		report_error(L, "Player with the number %i does not exist", player_number_);
	}
	return *rv;
}
const Widelands::Message& LuaInboxMessage::get(lua_State* L, Widelands::Game& game) {
	const Widelands::Message* rv = get_plr(L, game).messages()[message_id_];
	if (rv == nullptr) {
		report_error(L, "This message has been deleted!");
	}
	return *rv;
}

/* RST
.. function:: report_result(plr, result[, info = ""])

   Reports the game ending of a player. The player get prompted with a window
   containing a :class:`table` showing the results of all players and teams
   (if there are any). For mutliplayer games this reports the game ending also
   to the metaserver if this is an internet network game.

   :arg plr: The Player to report results for.
   :type plr: :class:`~wl.game.Player`
   :arg result: The player result (0: lost, 1: won, 2: resigned)
   :type result: :class:`integer`
   :arg info: A string containing extra data for this particular win
      condition. Likely one wants to use :meth:`make_extra_data` for this.
      The string will be shown beside the :class:`table` as "Player information: info".
   :type info: :class:`string`

   Example reporting a won game:

   .. code-block:: lua

      wl.game.report_result(plr, 1, make_extra_data(
            plr, wc_name, wc_version, {score = "Score for this player"}))

*/
// TODO(sirver): this should be a method of wl.Game(). Fix for b19.
static int L_report_result(lua_State* L) {
	std::string info;
	if (lua_gettop(L) >= 3) {
		info = luaL_checkstring(L, 3);
	}

	Widelands::PlayerEndResult result =
	   static_cast<Widelands::PlayerEndResult>(luaL_checknumber(L, 2));

	get_game(L).game_controller()->report_result(
	   (*get_user_class<LuaPlayer>(L, 1))->get(L, get_game(L)).player_number(), result, info);
	return 0;
}

/*
 * ========================================================================
 *                            MODULE FUNCTIONS
 * ========================================================================
 */
const static struct luaL_Reg wlgame[] = {{"report_result", &L_report_result}, {nullptr, nullptr}};

void luaopen_wlgame(lua_State* L) {
	lua_getglobal(L, "wl");     // S: wl_table
	lua_pushstring(L, "game");  // S: wl_table "game"
	luaL_newlib(L, wlgame);     // S: wl_table "game" wl.game_table
	lua_settable(L, -3);        // S: wl_table
	lua_pop(L, 1);              // S:

	register_class<LuaPlayer>(L, "game", true);
	add_parent<LuaPlayer, LuaBases::LuaPlayerBase>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaObjective>(L, "game");
	register_class<LuaInboxMessage>(L, "game");
}
}  // namespace LuaGame
