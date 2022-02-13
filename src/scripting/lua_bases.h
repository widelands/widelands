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

#ifndef WL_SCRIPTING_LUA_BASES_H
#define WL_SCRIPTING_LUA_BASES_H

#include "logic/editor_game_base.h"
#include "logic/player.h"
#include "scripting/lua.h"
#include "scripting/luna.h"

namespace LuaBases {

/*
 * Base class for all classes in wl.base
 */
class LuaBasesModuleClass : public LunaClass {
public:
	const char* get_modulename() override {
		return "bases";
	}
};

class LuaEditorGameBase : public LuaBasesModuleClass {
public:
	LUNA_CLASS_HEAD(LuaEditorGameBase);

	LuaEditorGameBase() {
	}
	explicit LuaEditorGameBase(lua_State* L) {
		report_error(L, "Cannot instantiate a 'EditorGameBase' directly!");
	}
	~LuaEditorGameBase() override {
	}

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * Properties
	 */
	int get_map(lua_State*);
	int get_players(lua_State*);

	/*
	 * Lua methods
	 */
	int get_immovable_description(lua_State* L);
	int immovable_exists(lua_State* L);
	int get_building_description(lua_State* L);
	int get_ship_description(lua_State* L);
	int get_tribe_description(lua_State* L);
	int get_ware_description(lua_State* L);
	int get_worker_description(lua_State* L);
	int get_resource_description(lua_State* L);
	int get_terrain_description(lua_State* L);
	int save_campaign_data(lua_State* L);
	int read_campaign_data(lua_State* L);
	int set_loading_message(lua_State*);

	/*
	 * C methods
	 */
};

class LuaPlayerBase : public LuaBasesModuleClass {
	Widelands::PlayerNumber player_number_;
	enum { NONE = -1 };

public:
	LUNA_CLASS_HEAD(LuaPlayerBase);

	LuaPlayerBase() : player_number_(NONE) {
	}
	explicit LuaPlayerBase(lua_State* L) : player_number_(NONE) {
		report_error(L, "Cannot instantiate a 'PlayerBase' directly!");
	}
	explicit LuaPlayerBase(Widelands::PlayerNumber n) {
		player_number_ = n;
	}
	~LuaPlayerBase() override {
	}

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * Properties
	 */
	int get_number(lua_State* L);
	int get_tribe_name(lua_State* L);

	/*
	 * Lua methods
	 */
	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	int __eq(lua_State* L);
	int __tostring(lua_State* L);
	CLANG_DIAG_RESERVED_IDENTIFIER_ON
	int place_flag(lua_State* L);
	int place_road(lua_State* L);
	int place_building(lua_State* L);
	int place_ship(lua_State* L);
	int conquer(lua_State* L);
	int get_workers(lua_State* L);
	int get_wares(lua_State* L);

	/*
	 * C methods
	 */
	Widelands::Player& get(lua_State* L, const Widelands::EditorGameBase&) const;

protected:
	inline Widelands::PlayerNumber player_number() {
		return player_number_;
	}
};

void luaopen_wlbases(lua_State*);
}  // namespace LuaBases

#endif  // end of include guard: WL_SCRIPTING_LUA_BASES_H
