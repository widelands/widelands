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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "scripting/lua_table.h"

#include <memory>

#include "base/log.h"

LuaTable::LuaTable(lua_State* L) : L_(L), warn_about_unaccessed_keys_(true) {
	// S: <table>
	lua_pushlightuserdata(L_, const_cast<LuaTable*>(this));  // S: this
	lua_pushvalue(L, -2);                                    // S: <table> this <table>
	lua_rawset(L, LUA_REGISTRYINDEX);
}

LuaTable::~LuaTable() {
	if (warn_about_unaccessed_keys_) {
		std::vector<std::string> unused_keys;
		std::set<std::string> all_keys = keys<std::string>();
		std::set_difference(all_keys.begin(), all_keys.end(), accessed_keys_.begin(),
		                    accessed_keys_.end(), std::back_inserter(unused_keys));

		for (const std::string& unused_key : unused_keys) {
			// We must not throw in destructors as this can shadow other errors.
			log_warn("Unused key \"%s\" in LuaTable. Please report as a bug.\n", unused_key.c_str());
		}
	}

	lua_pushlightuserdata(L_, const_cast<LuaTable*>(this));  // S: this
	lua_pushnil(L_);                                         // S: this nil
	lua_rawset(L_, LUA_REGISTRYINDEX);
}

void LuaTable::do_not_warn_about_unaccessed_keys() {
	warn_about_unaccessed_keys_ = false;
}

void LuaTable::get_existing_table_value(const std::string& key) const {
	lua_pushstring(L_, key);
	check_if_key_was_in_table(key);
}

void LuaTable::get_existing_table_value(const int key) const {
	const std::string key_as_string = as_string(key);
	lua_pushint32(L_, key);
	check_if_key_was_in_table(key_as_string);
}

void LuaTable::check_if_key_was_in_table(const std::string& key) const {
	// S: key
	lua_pushlightuserdata(L_, const_cast<LuaTable*>(this));  // S: this
	lua_rawget(L_, LUA_REGISTRYINDEX);                       // S: key table
	lua_pushvalue(L_, -2);                                   // S: key table key

	lua_rawget(L_, -2);  // S: key table value
	lua_remove(L_, -2);  // S: key value
	lua_remove(L_, -2);  // S: value

	if (lua_isnil(L_, -1)) {
		lua_pop(L_, 1);
		throw LuaTableKeyError(key);
	}
	accessed_keys_.insert(key);
}

template <> std::unique_ptr<LuaTable> LuaTable::get_value() const {
	lua_pushvalue(L_, -1);
	if (!lua_istable(L_, -1)) {
		lua_pop(L_, 1);
		throw LuaError("Could not convert value at the top of the stack to table value.");
	}

	std::unique_ptr<LuaTable> rv(new LuaTable(L_));
	lua_pop(L_, 1);
	return rv;
}

template <> std::string LuaTable::get_value() const {
	lua_pushvalue(L_, -1);
	const char* str = lua_tostring(L_, -1);
	lua_pop(L_, 1);
	if (str == nullptr) {
		throw LuaError("Could not convert value at top of the stack to string.");
	}
	return str;
}

template <> int LuaTable::get_value() const {
	lua_pushvalue(L_, -1);
	int is_num;
	int return_value = lua_tointegerx(L_, -1, &is_num);
	lua_pop(L_, 1);
	if (!is_num) {
		throw LuaError("Could not convert value at top of the stack to integer.");
	}
	return return_value;
}

const std::string get_string_with_default(const LuaTable& table,
                                          const std::string& key,
                                          const std::string& default_value) {
	if (table.has_key(key)) {
		return table.get_string(key);
	} else {
		return default_value;
	}
}
