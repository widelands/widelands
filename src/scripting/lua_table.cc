/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#include <boost/format.hpp>

LuaTable::LuaTable(lua_State* L) : L_(L), index_(lua_gettop(L)), warn_about_unaccessed_keys_(true) {
}

LuaTable::~LuaTable() {
	if (warn_about_unaccessed_keys_) {
		std::vector<std::string> unused_keys;
		std::set<std::string> all_keys = keys<std::string>();
		std::set_difference(all_keys.begin(),
		                    all_keys.end(),
		                    accessed_keys_.begin(),
		                    accessed_keys_.end(),
		                    std::back_inserter(unused_keys));

		for (const std::string& unused_key : unused_keys) {
			// We must not throw in destructors as this can shadow other errors.
			log("ERROR: Unused key \"%s\" in LuaTable. Please report as a bug.\n", unused_key.c_str());
		}
	}

	lua_remove(L_, index_);
}

void LuaTable::do_not_warn_about_unaccessed_keys() {
	warn_about_unaccessed_keys_ = false;
}

void LuaTable::get_existing_table_value(const std::string& key) const {
	lua_pushstring(L_, key);
	check_if_key_was_in_table(key);
}

void LuaTable::get_existing_table_value(const int key) const {
	const std::string key_as_string = boost::lexical_cast<std::string>(key);
	lua_pushint32(L_, key);
	check_if_key_was_in_table(key_as_string);
}

void LuaTable::check_if_key_was_in_table(const std::string& key) const {
	lua_rawget(L_, index_);
	if (lua_isnil(L_, -1)) {
		lua_pop(L_, 1);
		throw LuaTableKeyError(key);
	}
	accessed_keys_.insert(key);
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
