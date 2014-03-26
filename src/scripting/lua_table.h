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

#ifndef LUA_TABLE_H
#define LUA_TABLE_H

#include <boost/lexical_cast.hpp>

#include "scripting/c_utils.h"
#include "scripting/eris/lua.hpp"
#include "scripting/lua_coroutine.h"
#include "scripting/lua_errors.h"

class LuaTableKeyError : public LuaError {
public:
	LuaTableKeyError(const std::string& wanted)
	   : LuaError(wanted + " is not a field in this table.") {
	}
};

// Easy handling of return values from Wideland's Lua configurations scripts:
// they return a Lua table with (string,value) pairs.
class LuaTable {
public:
	explicit LuaTable(lua_State* L);

	~LuaTable();
	// Returns all keys in sorted order (got by iterating using pair).
	// All keys must be of the given type.
	template <typename KeyType> std::vector<KeyType> keys() const {
		std::vector<KeyType> keys;
		lua_pushnil(L_);                      // S: table nil
		while (lua_next(L_, index_) != 0) {  // S: table key value
			lua_pop(L_, 1);                    // S: table key
			keys.emplace_back(get_value<KeyType>());
		}
		std::sort(keys.begin(), keys.end());
		return keys;
	}

	// Returns all integer entries starting at 1 till nil is found. All entries
	// must be of the given type.
	template <typename ValueType> std::vector<ValueType> array_entries() const {
		std::vector<ValueType> values;
		int index = 1;
		for (;;) {
			lua_rawgeti(L_, index_, index);
			if (lua_isnil(L_, -1)) {
				lua_pop(L_, 1);
				break;
			}
			values.emplace_back(get_value<ValueType>());
			lua_pop(L_, 1);
			++index;
		}
		return values;
	}

	// Returns the corresponding value with the given key.
	template <typename KeyType> std::string get_string(const KeyType& key) const {
		get_existing_table_value(key);
		if (!lua_isstring(L_, -1)) {
			lua_pop(L_, 1);
			throw LuaError(boost::lexical_cast<std::string>(key) + " is not a string value.");
		}
		const std::string rv = lua_tostring(L_, -1);
		lua_pop(L_, 1);
		return rv;
	}

	template <typename KeyType> std::unique_ptr<LuaTable> get_table(const KeyType& key) const {
		get_existing_table_value(key);
		if (!lua_istable(L_, -1)) {
			lua_pop(L_, 1);
			throw LuaError(boost::lexical_cast<std::string>(key) + " is not a table value.");
		}
		std::unique_ptr<LuaTable> rv(new LuaTable(L_));
		return rv;
	}

	template <typename KeyType> double get_double(const KeyType& key) const {
		get_existing_table_value(key);
		if (!lua_isnumber(L_, -1)) {
			lua_pop(L_, 1);
			throw LuaError(boost::lexical_cast<std::string>(key) + " is not a number value.");
		}
		const double rv = lua_tonumber(L_, -1);
		lua_pop(L_, 1);
		return rv;
	}

	template <typename KeyType> int get_int(const KeyType& key) const {
		const double value = get_double(key);
		const int integer = static_cast<int>(value);

		if (value != integer) {
			throw LuaError(boost::lexical_cast<std::string>(key) + " is not a integer value.");
		}
		return integer;
	}

	template <typename KeyType> uint32_t get_uint(const KeyType& key) const {
		int value = get_int(key);
		if (value < 0) {
			throw LuaError(boost::lexical_cast<std::string>(key) + " is not a positive value.");
		}
		return static_cast<uint32_t>(value);
	}

	template <typename KeyType> bool get_bool(const KeyType& key) const {
		get_existing_table_value(key);
		if (!lua_isboolean(L_, -1)) {
			lua_pop(L_, 1);
			throw LuaError(boost::lexical_cast<std::string>(key) + " is not a boolean value.");
		}
		const bool rv = lua_tonumber(L_, -1);
		lua_pop(L_, 1);
		return rv;
	}

	template <typename KeyType> std::unique_ptr<LuaCoroutine> get_coroutine(const KeyType& key) const {
		get_existing_table_value(key);

		if (lua_isfunction(L_, -1)) {
			// Oh well, a function, not a coroutine. Let's turn it into one
			lua_State* t = lua_newthread(L_);
			lua_pop(L_, 1);  // Immediately remove this thread again

			lua_xmove(L_, t, 1);  // Move function to coroutine
			lua_pushthread(t);     // Now, move thread object back
			lua_xmove(t, L_, 1);
		}

		if (not lua_isthread(L_, -1)) {
			lua_pop(L_, 1);
			throw LuaError(boost::lexical_cast<std::string>(key) + " is not a function value.");
		}
		std::unique_ptr<LuaCoroutine> cr(new LuaCoroutine(luaL_checkthread(L_, -1)));
		lua_pop(L_, 1);  // Remove coroutine from stack
		return cr;
	}

private:
	void get_existing_table_value(const std::string& key) const;
	void get_existing_table_value(int key) const;
	void check_if_key_was_in_table(const std::string& key) const;

	// Get a lua value of the specific type. See template specializations.
	template <typename T> T get_value() const {
		assert(false);
	}

	lua_State* L_;
	const int index_;
};

template <> std::string LuaTable::get_value<std::string>() const;
template <> int LuaTable::get_value<int>() const;

#endif /* end of include guard: LUA_TABLE_H */
