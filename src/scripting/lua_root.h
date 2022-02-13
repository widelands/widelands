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

#ifndef WL_SCRIPTING_LUA_ROOT_H
#define WL_SCRIPTING_LUA_ROOT_H

#include "scripting/lua.h"
#include "scripting/lua_bases.h"
#include "scripting/luna.h"

namespace LuaRoot {

/*
 * Base class for all classes in wl
 */
class LuaRootModuleClass : public LunaClass {
public:
	const char* get_modulename() override {
		return "";
	}
};

class LuaGame : public LuaBases::LuaEditorGameBase {
public:
	LUNA_CLASS_HEAD(LuaGame);
	const char* get_modulename() override {
		return "";
	}

	LuaGame() {
	}
	explicit LuaGame(lua_State* L);

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * Properties
	 */
	int get_real_speed(lua_State* L);
	int get_time(lua_State*);
	int get_desired_speed(lua_State*);
	int set_desired_speed(lua_State*);
	int get_allow_saving(lua_State*);
	int set_allow_saving(lua_State*);
	int get_last_save_time(lua_State* L);
	int get_type(lua_State*);
	int get_scenario_difficulty(lua_State*);
	int get_interactive_player(lua_State*);
	int get_win_condition(lua_State*);

	/*
	 * Lua methods
	 */
	int launch_coroutine(lua_State*);
	int save(lua_State*);

	/*
	 * C methods
	 */
};

class LuaEditor : public LuaBases::LuaEditorGameBase {
public:
	LUNA_CLASS_HEAD(LuaEditor);
	const char* get_modulename() override {
		return "";
	}

	LuaEditor() {
	}
	explicit LuaEditor(lua_State* L);
	~LuaEditor() override {
	}

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * Properties
	 */

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */
};

class LuaDescriptions : public LuaRootModuleClass {
public:
	LUNA_CLASS_HEAD(LuaDescriptions);
	const char* get_modulename() override {
		return "";
	}

	LuaDescriptions() {
	}
	explicit LuaDescriptions(lua_State* L);

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * Properties
	 */
	int get_tribes_descriptions(lua_State* L);
	int get_immovable_descriptions(lua_State* L);
	int get_terrain_descriptions(lua_State* L);
	int get_worker_descriptions(lua_State*);

	/*
	 * Lua methods
	 */
	int new_critter_type(lua_State* L);
	int new_immovable_type(lua_State* L);
	int new_resource_type(lua_State* L);
	int new_terrain_type(lua_State* L);
	int new_carrier_type(lua_State* L);
	int new_ferry_type(lua_State* L);
	int new_constructionsite_type(lua_State* L);
	int new_dismantlesite_type(lua_State* L);
	int new_market_type(lua_State* L);
	int new_militarysite_type(lua_State* L);
	int new_productionsite_type(lua_State* L);
	int new_ship_type(lua_State* L);
	int new_soldier_type(lua_State* L);
	int new_trainingsite_type(lua_State* L);
	int new_tribe(lua_State* L);
	int new_ware_type(lua_State* L);
	int new_warehouse_type(lua_State* L);
	int new_worker_type(lua_State* L);

	int modify_unit(lua_State* L);

	/*
	 * C methods
	 */

	void do_modify_tribe(lua_State* L, const std::string& name, const std::string& property);
	void do_modify_building(lua_State* L, const std::string&, const std::string&);
	void do_modify_trainingsite(lua_State* L, const std::string&, const std::string&);
	void do_modify_productionsite(lua_State* L, const std::string&, const std::string&);
	void do_modify_militarysite(lua_State* L, const std::string&, const std::string&);
	void do_modify_warehouse(lua_State* L, const std::string&, const std::string&);
	void do_modify_worker(lua_State* L, const std::string&, const std::string&);
	void do_modify_ware(lua_State* L, const std::string&, const std::string&);
	void do_modify_immovable(lua_State* L, const std::string&, const std::string&);
	void do_modify_resource(lua_State* L, const std::string&, const std::string&);
	void do_modify_terrain(lua_State* L, const std::string&, const std::string&);
	void do_modify_critter(lua_State* L, const std::string&, const std::string&);
	void do_modify_ship(lua_State* L, const std::string&, const std::string&);
};

// TODO(GunChleoc): This class is here for saveloading compatibility only. We'll get a SIGABRT from
// the scripting packet if it's not there.
class LuaWorld : public LuaRootModuleClass {
public:
	LUNA_CLASS_HEAD(LuaWorld);
	const char* get_modulename() override {
		return "";
	}
	LuaWorld() = default;
	explicit LuaWorld(lua_State*) {
	}

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State*) override {
	}
	void __unpersist(lua_State*) override {
	}
	CLANG_DIAG_RESERVED_IDENTIFIER_ON
};

// TODO(GunChleoc): This class is here for saveloading compatibility only. We'll get a SIGABRT from
// the scripting packet if it's not there.
class LuaTribes : public LuaRootModuleClass {
public:
	LUNA_CLASS_HEAD(LuaTribes);
	const char* get_modulename() override {
		return "";
	}
	LuaTribes() = default;
	explicit LuaTribes(lua_State*) {
	}

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State*) override {
	}
	void __unpersist(lua_State*) override {
	}
	CLANG_DIAG_RESERVED_IDENTIFIER_ON
};

void luaopen_wlroot(lua_State*, bool in_editor);

#endif  // end of include guard: WL_SCRIPTING_LUA_ROOT_H
}
