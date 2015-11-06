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

#ifndef WL_SCRIPTING_LUA_GAME_H
#define WL_SCRIPTING_LUA_GAME_H

#include "logic/building.h"
#include "logic/message_id.h"
#include "scripting/lua.h"
#include "scripting/lua_bases.h"
#include "scripting/luna.h"

namespace Widelands {
	class TribeDescr;
	class Objective;
	struct Message;
}

namespace LuaGame {

/*
 * Base class for all classes in wl.game
 */
class LuaGameModuleClass : public LunaClass {
	public:
		const char * get_modulename() override {return "game";}
};

class LuaPlayer : public LuaBases::LuaPlayerBase {
public:
	// Overwritten from LuaPlayerBase, avoid ambiguity when deriving from
	// LuaGameModuleClass and LuaPlayerBase
	const char * get_modulename() override {return "game";}

	LUNA_CLASS_HEAD(LuaPlayer);

	LuaPlayer() : LuaBases::LuaPlayerBase() {}
	LuaPlayer(Widelands::PlayerNumber n) : LuaBases::LuaPlayerBase(n)  {}
	LuaPlayer(lua_State * L) {
		report_error(L, "Cannot instantiate a 'Player' directly!");
	}

	/*
	 * Properties
	 */
	int get_name(lua_State * L);
	int get_allowed_buildings(lua_State * L);
	int get_objectives(lua_State * L);
	int get_defeated(lua_State * L);
	int get_inbox(lua_State * L);
	int get_team(lua_State * L);
	int get_tribe(lua_State * L);
	int set_team(lua_State * L);
	int get_see_all(lua_State * L);
	int set_see_all(lua_State * L);

	/*
	 * Lua methods
	 */
	int send_message(lua_State * L);
	int message_box(lua_State * L);
	int sees_field(lua_State * L);
	int seen_field(lua_State * L);
	int allow_buildings(lua_State * L);
	int forbid_buildings(lua_State * L);
	int add_objective(lua_State * L);
	int reveal_fields(lua_State * L);
	int hide_fields(lua_State * L);
	int reveal_scenario(lua_State * L);
	int reveal_campaign(lua_State * L);
	int get_ships(lua_State * L);
	int get_buildings(lua_State * L);
	int get_suitability(lua_State * L);
	int allow_workers(lua_State * L);
	int switchplayer(lua_State * L);

	/*
	 * C methods
	 */
private:
	void m_parse_building_list
		(lua_State *, const Widelands::TribeDescr &,
		 std::vector<Widelands::BuildingIndex> &);
	int m_allow_forbid_buildings(lua_State * L, bool);

};

class LuaObjective : public LuaGameModuleClass {
	std::string m_name;

public:
	LUNA_CLASS_HEAD(LuaObjective);

	virtual ~LuaObjective() {}

	LuaObjective(const Widelands::Objective& n);
	LuaObjective() : m_name("") {}
	LuaObjective(lua_State * L) {
		report_error(L, "Cannot instantiate a '%s' directly!", className);
	}

	void __persist(lua_State *) override;
	void __unpersist(lua_State *) override;

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

class LuaMessage : public LuaGameModuleClass {
	uint32_t m_plr;
	Widelands::MessageId m_mid;

public:
	LUNA_CLASS_HEAD(LuaMessage);
	virtual ~LuaMessage() {}

	LuaMessage(uint8_t, Widelands::MessageId);
	LuaMessage() : m_plr(0), m_mid(0) {}
	LuaMessage(lua_State * L) {
		report_error(L, "Cannot instantiate a '%s' directly!", className);
	}

	void __persist(lua_State *) override;
	void __unpersist(lua_State *) override;

	/*
	 * Properties
	 */
	int get_sent(lua_State * L);
	int get_title(lua_State * L);
	int get_body(lua_State * L);
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

#endif  // end of include guard: WL_SCRIPTING_LUA_GAME_H
}
