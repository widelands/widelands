/*
 * Copyright (C) 2006-2021 by the Widelands Development Team
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

#include "logic/message_id.h"
#include "scripting/lua.h"
#include "scripting/lua_bases.h"
#include "scripting/luna.h"

namespace Widelands {
class TribeDescr;
class Objective;
struct Message;
}  // namespace Widelands

namespace LuaGame {

/*
 * Base class for all classes in wl.game
 */
class LuaGameModuleClass : public LunaClass {
public:
	const char* get_modulename() override {
		return "game";
	}
};

class LuaPlayer : public LuaBases::LuaPlayerBase {
public:
	// Overwritten from LuaPlayerBase, avoid ambiguity when deriving from
	// LuaGameModuleClass and LuaPlayerBase
	const char* get_modulename() override {
		return "game";
	}

	LUNA_CLASS_HEAD(LuaPlayer);

	LuaPlayer() : LuaBases::LuaPlayerBase() {
	}
	explicit LuaPlayer(Widelands::PlayerNumber n) : LuaBases::LuaPlayerBase(n) {
	}
	explicit LuaPlayer(lua_State* L) {
		report_error(L, "Cannot instantiate a 'Player' directly!");
	}

	/*
	 * Properties
	 */
	int get_name(lua_State* L);
	int get_allowed_buildings(lua_State* L);
	int get_objectives(lua_State* L);
	int get_defeated(lua_State* L);
	int get_messages(lua_State* L);
	int get_inbox(lua_State* L);
	int get_color(lua_State* L);
	int get_team(lua_State* L);
	int get_tribe(lua_State* L);
	int set_team(lua_State* L);
	int get_see_all(lua_State* L);
	int set_see_all(lua_State* L);
	int get_allow_additional_expedition_items(lua_State* L);
	int set_allow_additional_expedition_items(lua_State* L);
	int get_hidden_from_general_statistics(lua_State* L);
	int set_hidden_from_general_statistics(lua_State* L);

	/*
	 * Lua methods
	 */
	int send_to_inbox(lua_State* L);
	int message_box(lua_State* L);
	int sees_field(lua_State* L);
	int seen_field(lua_State* L);
	int allow_buildings(lua_State* L);
	int forbid_buildings(lua_State* L);
	int add_objective(lua_State* L);
	int reveal_fields(lua_State* L);
	int hide_fields(lua_State* L);
	int mark_scenario_as_solved(lua_State* L);
#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
	int acquire_training_wheel_lock(lua_State* L);
	int release_training_wheel_lock(lua_State* L);
	int mark_training_wheel_as_solved(lua_State* L);
	int run_training_wheel(lua_State* L);
	int skip_training_wheel(lua_State* L);
#endif
	int get_ships(lua_State* L);
	int get_buildings(lua_State* L);
	int get_constructionsites(lua_State* L);
	int get_suitability(lua_State* L);
	int allow_workers(lua_State* L);
	int switchplayer(lua_State* L);
	int get_produced_wares_count(lua_State* L);
	int set_attack_forbidden(lua_State* L);
	int is_attack_forbidden(lua_State* L);

	/*
	 * C methods
	 */
private:
	void parse_building_list(lua_State*,
	                         const Widelands::TribeDescr&,
	                         std::vector<Widelands::DescriptionIndex>&);
	int allow_forbid_buildings(lua_State* L, bool);
	int do_get_buildings(lua_State* L, bool);
};

class LuaObjective : public LuaGameModuleClass {
	std::string name_;

public:
	LUNA_CLASS_HEAD(LuaObjective);

	~LuaObjective() override {
	}

	explicit LuaObjective(const Widelands::Objective& n);
	LuaObjective() = default;
	explicit LuaObjective(lua_State* L) {
		report_error(L, "Cannot instantiate a '%s' directly!", className);
	}

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State*) override;
	void __unpersist(lua_State*) override;
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * Properties
	 */
	int get_name(lua_State* L);
	int get_title(lua_State* L);
	int set_title(lua_State* L);
	int get_body(lua_State* L);
	int set_body(lua_State* L);
	int get_visible(lua_State* L);
	int set_visible(lua_State* L);
	int get_done(lua_State* L);
	int set_done(lua_State* L);

	/*
	 * Lua Methods
	 */
	int remove(lua_State* L);
	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	int __eq(lua_State* L);
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * C Methods
	 */
	Widelands::Objective& get(lua_State*, Widelands::Game&);
};

class LuaInboxMessage : public LuaGameModuleClass {
	Widelands::PlayerNumber player_number_;
	Widelands::MessageId message_id_;

public:
	LUNA_CLASS_HEAD(LuaInboxMessage);
	~LuaInboxMessage() override {
	}

	explicit LuaInboxMessage(uint8_t, Widelands::MessageId);
	LuaInboxMessage() : player_number_(0), message_id_(0) {
	}
	explicit LuaInboxMessage(lua_State* L) {
		report_error(L, "Cannot instantiate a '%s' directly!", className);
	}

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State*) override;
	void __unpersist(lua_State*) override;
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * Properties
	 */
	int get_sent(lua_State* L);
	int get_title(lua_State* L);
	int get_body(lua_State* L);
	int get_field(lua_State* L);
	int get_status(lua_State* L);
	int set_status(lua_State* L);
	int get_heading(lua_State* L);
	int get_icon_name(lua_State* L);

	/*
	 * Lua Methods
	 */
	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	int __eq(lua_State* L);
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * C Methods
	 */
	Widelands::Player& get_plr(lua_State* L, const Widelands::Game& game);
	const Widelands::Message& get(lua_State* L, Widelands::Game& game);
};

void luaopen_wlgame(lua_State*);

#endif  // end of include guard: WL_SCRIPTING_LUA_GAME_H
}
