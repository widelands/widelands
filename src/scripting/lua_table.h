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

#ifndef WL_SCRIPTING_LUA_TABLE_H
#define WL_SCRIPTING_LUA_TABLE_H

#include <cstdlib>
#include <memory>
#include <set>

#include "base/multithreading.h"
#include "base/string.h"
#include "base/vector.h"
#include "scripting/lua.h"
#include "scripting/lua_coroutine.h"
#include "scripting/lua_errors.h"

class LuaTableKeyError : public LuaError {
public:
	explicit LuaTableKeyError(const std::string& wanted)
	   : LuaError(wanted + " is not a field in this table.") {
	}
};

/// Easy handling of return values from Wideland's Lua configurations scripts:
/// they return a Lua table with (string,value) pairs.
class LuaTable {
public:
	enum class DataType { kError, kBoolean, kFunction, kNumber, kString, kTable };

	explicit LuaTable(lua_State* L);

	~LuaTable();

	/// Disables warning about unused keys on destruction of this table. If this
	/// is not called before destruction, stderr will get a text with a warning
	/// if any key was unused in the dictionary.
	void do_not_warn_about_unaccessed_keys();

	/// Returns all keys. All keys must be of the given type.
	template <typename KeyType> std::set<KeyType> keys() const {
		lua_pushlightuserdata(L_, const_cast<LuaTable*>(this));  // S: this
		lua_rawget(L_, LUA_REGISTRYINDEX);                       // S: table

		std::set<KeyType> table_keys;
		lua_pushnil(L_);                 // S: table nil
		while (lua_next(L_, -2) != 0) {  // S: table key value
			lua_pop(L_, 1);               // S: table key
			table_keys.insert(get_value<KeyType>());
		}
		lua_pop(L_, 1);  // S:
		return table_keys;
	}

	/// Returns all integer entries starting at 1 till nil is found. All entries
	/// must be of the given type.
	template <typename ValueType> std::vector<ValueType> array_entries() const {
		lua_pushlightuserdata(L_, const_cast<LuaTable*>(this));  // S: this
		lua_rawget(L_, LUA_REGISTRYINDEX);                       // S: table

		std::vector<ValueType> values;
		int index = 1;
		for (;;) {
			lua_rawgeti(L_, -1, index);  // S: table value
			if (lua_isnil(L_, -1)) {
				lua_pop(L_, 1);  // S: table
				break;
			}
			values.emplace_back(get_value<ValueType>());
			accessed_keys_.insert(as_string(index));
			lua_pop(L_, 1);  // S: table
			++index;
		}
		lua_pop(L_, 1);  // S:
		return values;
	}

	/// Returns true if the key is in the table.
	template <typename KeyType> bool has_key(const KeyType& key) const {
		try {
			get_existing_table_value(key);
			lua_pop(L_, 1);
			return true;
		} catch (LuaTableKeyError&) {
			return false;
		}
	}

	/// Returns the value's data type if the key is in the table. Returns DataType::kError otherwise.
	template <typename KeyType> DataType get_datatype(const KeyType& key) const {
		DataType result = DataType::kError;
		try {
			get_existing_table_value(key);
			if (lua_isstring(L_, -1)) {
				result = DataType::kString;
			} else if (lua_istable(L_, -1)) {
				result = DataType::kTable;
			} else if (lua_isnumber(L_, -1)) {
				result = DataType::kNumber;
			} else if (lua_isboolean(L_, -1)) {
				result = DataType::kBoolean;
			} else if (lua_isfunction(L_, -1)) {
				result = DataType::kFunction;
			}
			lua_pop(L_, 1);
		} catch (LuaTableKeyError&) {
			return result;
		}
		return result;
	}

	/// Returns the corresponding value with the given key.
	template <typename KeyType> std::string get_string(const KeyType& key) const {
		get_existing_table_value(key);
		if (!lua_isstring(L_, -1)) {
			lua_pop(L_, 1);
			throw LuaError(as_string(key) + " is not a string value.");
		}
		const std::string rv = lua_tostring(L_, -1);
		lua_pop(L_, 1);
		return rv;
	}

	// Parses a Lua subtable into a Vector2i or Vector2f
	template <typename KeyType, typename ValueType>
	Vector2<ValueType> get_vector(const KeyType& key) const {
		Vector2<ValueType> result = Vector2<ValueType>::zero();
		std::unique_ptr<LuaTable> table(get_table(key));
		std::vector<ValueType> pts = table->array_entries<ValueType>();
		if (pts.size() != 2) {
			throw LuaError(format("Expected 2 entries, but got %d.", pts.size()));
		}
		result.x = pts[0];
		result.y = pts[1];
		return result;
	}

	template <typename KeyType> std::unique_ptr<LuaTable> get_table(const KeyType& key) const {
		get_existing_table_value(key);
		if (!lua_istable(L_, -1)) {
			lua_pop(L_, 1);
			throw LuaError(as_string(key) + " is not a table value.");
		}
		std::unique_ptr<LuaTable> rv(new LuaTable(L_));
		lua_pop(L_, 1);
		return rv;
	}

	template <typename KeyType> double get_double(const KeyType& key) const {
		get_existing_table_value(key);
		if (!lua_isnumber(L_, -1)) {
			lua_pop(L_, 1);
			throw LuaError(as_string(key) + " is not a number value.");
		}
		const double rv = lua_tonumber(L_, -1);
		lua_pop(L_, 1);
		return rv;
	}

	template <typename KeyType> int get_int(const KeyType& key) const {
		const double value = get_double(key);

		if (std::abs(value - std::floor(value)) > 1e-7) {
			throw LuaError(as_string(key) + " is not a integer value.");
		}
		return static_cast<int>(value);
	}

	template <typename KeyType> bool get_bool(const KeyType& key) const {
		get_existing_table_value(key);
		if (!lua_isboolean(L_, -1)) {
			lua_pop(L_, 1);
			throw LuaError(as_string(key) + " is not a boolean value.");
		}
		const bool rv = lua_toboolean(L_, -1);
		lua_pop(L_, 1);
		return rv;
	}

	template <typename KeyType>
	std::unique_ptr<LuaCoroutine> get_coroutine(const KeyType& key) const {
		get_existing_table_value(key);

		if (lua_isfunction(L_, -1)) {
			// Oh well, a function, not a coroutine. Let's turn it into one
			lua_State* t = lua_newthread(L_);
			lua_pop(L_, 1);  // Immediately remove this thread again

			lua_xmove(L_, t, 1);  // Move function to coroutine
			lua_pushthread(t);    // Now, move thread object back
			lua_xmove(t, L_, 1);
		}

		if (!lua_isthread(L_, -1)) {
			lua_pop(L_, 1);
			throw LuaError(as_string(key) + " is not a function value.");
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
	template <typename T> T get_value() const;

	MutexLock mutex_lock_;
	lua_State* L_;
	mutable std::set<std::string> accessed_keys_;
	bool warn_about_unaccessed_keys_;
};

template <> std::string LuaTable::get_value<std::string>() const;
template <> int LuaTable::get_value<int>() const;
template <> std::unique_ptr<LuaTable> LuaTable::get_value<std::unique_ptr<LuaTable>>() const;

/// Return a positive integer from the 'table'.
template <typename KeyType> uint32_t get_uint(const LuaTable& table, const KeyType& key) {
	int value = table.get_int(key);
	if (value < 0) {
		throw LuaError(as_string(key) + " is not a positive value.");
	}
	return static_cast<uint32_t>(value);
}

/// Return an integer > 0 from the 'table'.
template <typename KeyType> uint32_t get_positive_int(const LuaTable& table, const KeyType& key) {
	int value = get_uint(table, key);
	if (value == 0) {
		throw LuaError(as_string(key) + " must be > 0.");
	}
	return static_cast<uint32_t>(value);
}

/// Uses 'key' to fetch a string value from 'table'. If table does not have an
/// entry for key, returns 'default_value' instead.
const std::string get_string_with_default(const LuaTable& table,
                                          const std::string& key,
                                          const std::string& default_value);

#endif  // end of include guard: WL_SCRIPTING_LUA_TABLE_H
