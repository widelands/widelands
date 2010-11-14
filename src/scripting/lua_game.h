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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef LUA_GAME_H
#define LUA_GAME_H

#include <lua.hpp>

#include "logic/building.h"
#include "logic/message_id.h"

#include "lua_bases.h"
#include "luna.h"

namespace Widelands {
	struct Tribe_Descr;
	struct Objective;
	struct Message;
};

namespace LuaGame {

/*
 * Base class for all classes in wl.game
 */
struct L_GameModuleClass : public LunaClass {
	char const * get_modulename() {return "game";}
};

struct L_Player : public LuaBases::L_PlayerBase {
	// Overwritten from L_PlayerBase, avoid ambiguity when deriving from
	// L_GameModuleClass and L_PlayerBase
	const char * get_modulename() {return "game";}

	LUNA_CLASS_HEAD(L_Player);

	L_Player() : LuaBases::L_PlayerBase() {}
	L_Player(Widelands::Player_Number n) : LuaBases::L_PlayerBase(n)  {}
	L_Player(lua_State * const L) {
		report_error(L, "Cannot instantiate a 'Player' directly!");
	}

	/*
	 * Properties
	 */
	int get_name(lua_State *);
	int get_allowed_buildings(lua_State *);
	int get_objectives(lua_State *);
	int get_defeated(lua_State *);
	int get_retreat_percentage(lua_State *);
	int set_retreat_percentage(lua_State *);
	int get_changing_retreat_percentage_allowed(lua_State *);
	int set_changing_retreat_percentage_allowed(lua_State *);
	int get_inbox(lua_State *);
	int get_team(lua_State *);
	int set_team(lua_State *);
	int get_see_all(lua_State *);
	int set_see_all(lua_State *);

	/*
	 * Lua methods
	 */
	int send_message(lua_State *);
	int message_box(lua_State *);
	int sees_field(lua_State *);
	int seen_field(lua_State *);
	int allow_buildings(lua_State *);
	int forbid_buildings(lua_State *);
	int add_objective(lua_State *);
	int reveal_fields(lua_State *);
	int hide_fields(lua_State *);
	int reveal_scenario(lua_State *);
	int reveal_campaign(lua_State *);
	int get_buildings(lua_State *);
	int set_flag_style(lua_State *);
	int set_frontier_style(lua_State *);
	int get_suitability(lua_State *);
	int allow_workers(lua_State *);
	int switchplayer(lua_State *);

	/*
	 * C methods
	 */
private:
	void m_parse_building_list
		(lua_State *, const Widelands::Tribe_Descr &,
		 std::vector<Widelands::Building_Index> &);
	int m_allow_forbid_buildings(lua_State *, bool);
};

struct L_Objective : public L_GameModuleClass {
	LUNA_CLASS_HEAD(L_Objective);

	L_Objective(Widelands::Objective n);
	L_Objective() : m_name("") {}
	L_Objective(lua_State * const L) {
		report_error(L, "Cannot instantiate a '%s' directly!", className);
	}

	virtual void __persist(lua_State *);
	virtual void __unpersist(lua_State *);

	/*
	 * Properties
	 */
	int get_name(lua_State *);
	int get_title(lua_State *);
	int set_title(lua_State *);
	int get_body(lua_State *);
	int set_body(lua_State *);
	int get_visible(lua_State *);
	int set_visible(lua_State *);
	int get_done(lua_State *);
	int set_done(lua_State *);

	/*
	 * Lua Methods
	 */
	int remove(lua_State *);
	int __eq(lua_State *);

	/*
	 * C Methods
	 */
	Widelands::Objective & get(lua_State *, Widelands::Game &);

private:
	std::string m_name;
};

struct L_Message : public L_GameModuleClass {
	LUNA_CLASS_HEAD(L_Message);

	L_Message(uint8_t, Widelands::Message_Id);
	L_Message() : m_plr(0), m_mid(0) {}
	L_Message(lua_State * const L) {
		report_error(L, "Cannot instantiate a '%s' directly!", className);
	}

	virtual void __persist(lua_State *);
	virtual void __unpersist(lua_State *);

	/*
	 * Properties
	 */
	int get_sender(lua_State *);
	int get_sent(lua_State *);
	int get_title(lua_State *);
	int get_body(lua_State *);
	int get_duration(lua_State *);
	int get_field(lua_State *);
	int get_status(lua_State *);
	int set_status(lua_State *);

	/*
	 * Lua Methods
	 */
	int __eq(lua_State *);

	/*
	 * C Methods
	 */
	Widelands::Player & get_plr(lua_State *, Widelands::Game & game);
	Widelands::Message const & get(lua_State *, Widelands::Game & game);

private:
	uint32_t m_plr;
	Widelands::Message_Id m_mid;
};

void luaopen_wlgame(lua_State *);

}

#endif
