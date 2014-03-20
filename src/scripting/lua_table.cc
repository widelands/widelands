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

void LuaTable::get_existing_table_value(const std::string& key) const {
	lua_pushstring(m_L, key);
	check_if_key_was_in_table(key);
}

void LuaTable::get_existing_table_value(const int key) const {
	lua_pushint32(m_L, key);
	check_if_key_was_in_table(boost::lexical_cast<std::string>(key));
}

void LuaTable::check_if_key_was_in_table(const std::string& key) const {
	lua_rawget(m_L, m_index);
	if (lua_isnil(m_L, -1)) {
		lua_pop(m_L, 1);
		throw LuaTableKeyError(key);
	}
}

template <> std::string LuaTable::get_value() const {
	if (!lua_isstring(m_L, -1)) {
		lua_pop(m_L, 1);
		throw LuaError("No string on top of stack.");
	}
	return lua_tostring(m_L, -1);
}

template <> int LuaTable::get_value() const {
	if (!lua_isnumber(m_L, -1)) {
		lua_pop(m_L, 1);
		throw LuaError("No integer on top of stack.");
	}
	return lua_tointeger(m_L, -1);
}
