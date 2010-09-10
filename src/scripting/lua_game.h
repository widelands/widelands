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

#include "luna.h"

namespace Widelands {
	struct Tribe_Descr;
	struct Objective;
	struct Message;
};

/*
 * Base class for all classes in wl.game
 */
class L_GameModuleClass : public LunaClass {
	public:
		const char * get_modulename() {return "game";}
};

class L_Player : public L_GameModuleClass {
	Widelands::Player_Number m_pl;
	enum {NONE = -1};

public:
	LUNA_CLASS_HEAD(L_Player);

	L_Player() : m_pl(NONE) {}
	L_Player(Widelands::Player_Number n) {
		m_pl = n;
	}
	L_Player(lua_State * L);

	virtual void __persist(lua_State * L);
	virtual void __unpersist(lua_State * L);

	/*
	 * Properties
	 */
	int get_number(lua_State * L);
	int get_name(lua_State * L);
	int get_allowed_buildings(lua_State * L);
	int get_objectives(lua_State * L);
	int get_defeated(lua_State * L);
	int get_starting_field(lua_State * L);
	int get_retreat_percentage(lua_State * L);
	int set_retreat_percentage(lua_State * L);
	int get_changing_retreat_percentage_allowed(lua_State * L);
	int set_changing_retreat_percentage_allowed(lua_State * L);
	int get_inbox(lua_State * L);
	int get_team(lua_State * L);
	int set_team(lua_State * L);
	int get_tribe(lua_State * L);
	int get_see_all(lua_State * L);
	int set_see_all(lua_State * L);

	/*
	 * Lua methods
	 */
	int __eq(lua_State * L);
	int place_flag(lua_State * L);
	int place_building(lua_State * L);
	int send_message(lua_State * L);
	int message_box(lua_State * L);
	int sees_field(lua_State * L);
	int seen_field(lua_State * L);
	int allow_buildings(lua_State * L);
	int forbid_buildings(lua_State * L);
	int add_objective(lua_State * L);
	int conquer(lua_State * L);
	int reveal_fields(lua_State * L);
	int hide_fields(lua_State * L);
	int reveal_scenario(lua_State * L);
	int reveal_campaign(lua_State * L);
	int place_road(lua_State * L);
	int get_buildings(lua_State * L);
	int set_flag_style(lua_State * L);
	int set_frontier_style(lua_State * L);
	int get_suitability(lua_State * L);
	int allow_workers(lua_State * L);



	/*
	 * C methods
	 */
	Widelands::Player & get(lua_State * L, Widelands::Editor_Game_Base &);
private:
	void m_parse_building_list
		(lua_State *, const Widelands::Tribe_Descr &,
		 std::vector<Widelands::Building_Index> &);
	int m_allow_forbid_buildings(lua_State * L, bool);
};

class L_Objective : public L_GameModuleClass {
	std::string m_name;

public:
	LUNA_CLASS_HEAD(L_Objective);

	L_Objective(Widelands::Objective n);
	L_Objective() : m_name("") {}
	L_Objective(lua_State * L) {
		report_error(L, "Cannot instantiate a '%s' directly!", className);
	}

	virtual void __persist(lua_State*);
	virtual void __unpersist(lua_State*);

	/*
	 * Properties
	 */
	int get_name(lua_State * L);
	int get_title(lua_State * L);
	int set_title(lua_State * L);
	int get_body(lua_State * L);
	int set_body(lua_State * L);
	int get_visible(lua_State * L);
	int set_visible(lua_State * L);
	int get_done(lua_State * L);
	int set_done(lua_State * L);

	/*
	 * Lua Methods
	 */
	int remove(lua_State * L);
	int __eq(lua_State * L);

	/*
	 * C Methods
	 */
	Widelands::Objective & get(lua_State *, Widelands::Game &);
};

class L_Message : public L_GameModuleClass {
	uint32_t m_plr;
	Widelands::Message_Id m_mid;

public:
	LUNA_CLASS_HEAD(L_Message);

	L_Message(uint8_t, Widelands::Message_Id);
	L_Message() : m_plr(0), m_mid(0) {}
	L_Message(lua_State * L) {
		report_error(L, "Cannot instantiate a '%s' directly!", className);
	}

	virtual void __persist(lua_State*);
	virtual void __unpersist(lua_State*);

	/*
	 * Properties
	 */
	int get_sender(lua_State * L);
	int get_sent(lua_State * L);
	int get_title(lua_State * L);
	int get_body(lua_State * L);
	int get_duration(lua_State * L);
	int get_field(lua_State * L);
	int get_status(lua_State * L);
	int set_status(lua_State * L);

	/*
	 * Lua Methods
	 */
	int __eq(lua_State * L);

	/*
	 * C Methods
	 */
	Widelands::Player & get_plr(lua_State * L, Widelands::Game & game);
	const Widelands::Message & get(lua_State * L, Widelands::Game & game);
};

void luaopen_wlgame(lua_State *);

#endif

