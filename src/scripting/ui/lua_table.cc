/*
 * Copyright (C) 2006-2025 by the Widelands Development Team
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

#include "scripting/ui/lua_table.h"

#include "graphic/image_cache.h"

namespace LuaUi {

/* RST
Table
-----

.. class:: Table

   .. versionadded:: 1.2

   This represents a table.

   Some attributes and functions are available only for tables with a data type
   supported by the Lua interface. See :attr:`datatype`.
*/
const char LuaTable::className[] = "Table";
const MethodType<LuaTable> LuaTable::Methods[] = {
   METHOD(LuaTable, get),          METHOD(LuaTable, add), METHOD(LuaTable, remove_row),
   METHOD(LuaTable, remove_entry), {nullptr, nullptr},
};
const PropertyType<LuaTable> LuaTable::Properties[] = {
   PROP_RO(LuaTable, datatype),        PROP_RO(LuaTable, no_of_rows),
   PROP_RW(LuaTable, selection_index), PROP_RO(LuaTable, selections),
   PROP_RW(LuaTable, sort_column),     PROP_RW(LuaTable, sort_descending),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: datatype

      (RO) The table's datatype as :class:`string` if supported,
      or ``nil`` for tables with unsupported datatype.

      Currently only ``"int"`` is supported.
*/
int LuaTable::get_datatype(lua_State* L) {
	if (dynamic_cast<const TableOfInt*>(get()) != nullptr) {
		lua_pushstring(L, "int");
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: no_of_rows

      (RO) The number of rows this table has.
*/
int LuaTable::get_no_of_rows(lua_State* L) {
	lua_pushinteger(L, get()->size());
	return 1;
}

/* RST
   .. attribute:: selection_index

      (RW) The index of the currently selected row or ``nil`` if none is selected.
      Setting this to ``nil`` clears the selection.
*/
int LuaTable::get_selection_index(lua_State* L) {
	if (!get()->has_selection()) {
		lua_pushnil(L);
	} else {
		lua_pushinteger(L, get()->selection_index());
	}
	return 1;
}
int LuaTable::set_selection_index(lua_State* L) {
	if (lua_isnil(L, -1)) {
		get()->clear_selections();
	} else {
		get()->select(luaL_checkuint32(L, -1));
	}
	return 0;
}

/* RST
   .. attribute:: selections

      (RO) An :class:`array` with all currently selected row indices (may be empty).
*/
int LuaTable::get_selections(lua_State* L) {
	lua_newtable(L);
	int index = 0;
	for (uint32_t value : get()->selections()) {
		lua_pushinteger(L, ++index);
		lua_pushinteger(L, value);
		lua_rawset(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: sort_column

      (RW) The index of the column by which the table is sorted.
*/
int LuaTable::get_sort_column(lua_State* L) {
	lua_pushinteger(L, get()->get_sort_column());
	return 1;
}
int LuaTable::set_sort_column(lua_State* L) {
	get()->set_sort_column(luaL_checkuint32(L, -1));
	return 0;
}

/* RST
   .. attribute:: sort_descending

      (RW) Whether the table is sorted in reverse order.
*/
int LuaTable::get_sort_descending(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->get_sort_descending()));
	return 1;
}
int LuaTable::set_sort_descending(lua_State* L) {
	get()->set_sort_descending(luaL_checkboolean(L, -1));
	return 0;
}

/*
 * Lua Functions
 */

/* RST
   .. method:: get(row)

      Lookup the internal value associated with a given row index.
      Only allowed for tables with supported datatypes.

      :arg row: The row index to look up.
      :type row: :class:`int`
      :returns: The row's internal value.
      :rtype: :class:`int`
*/
int LuaTable::get(lua_State* L) {
	if (upcast(TableOfInt, table, get()); table != nullptr) {
		lua_pushinteger(L, (*table)[luaL_checkuint32(L, 2)]);
	} else {
		report_error(L, "get() not allowed for table with unsupported datatype");
	}
	return 1;
}

/* RST
   .. method:: add(value, select, disable, columns)

      Add a row to the table. Only allowed for tables with supported datatypes.

      The named arguments are followed by an array of as many entries as the table has columns.
      Each array entry is a :class:`table` describing the content of one table cell.
      Valid keys are:

         * ``"text"``: **Optional**. The text to show in the cell.
         * ``"icon"``: **Optional**. The icon filepath in the cell.

      :arg value: The internal value of the entry.
      :type value: This table's :attr:`datatype`
      :arg select: Whether to select this entry.
      :type select: :class:`boolean`
      :arg disable: Whether to disable this row.
      :type disable: :class:`boolean`
      :arg columns: The cell descriptors.
      :type columns: :class:`array` of :class:`table`
*/
int LuaTable::add(lua_State* L) {
	bool select = luaL_checkboolean(L, 3);
	bool disable = luaL_checkboolean(L, 4);

	if (upcast(TableOfInt, table, get()); table != nullptr) {
		uintptr_t value = luaL_checkuint32(L, 2);
		TableOfInt::EntryRecord& record = table->add(value, select);
		record.set_disabled(disable);

		int column = 0;
		lua_pushnil(L);
		while (lua_next(L, 5) != 0) {
			std::string text = get_table_string(L, "text", false);
			std::string icon = get_table_string(L, "icon", false);

			if (icon.empty()) {
				record.set_string(column, text);
			} else {
				record.set_picture(column, g_image_cache->get(icon), text);
			}

			lua_pop(L, 1);
			++column;
		}
	} else {
		report_error(L, "add() not allowed for table with unsupported datatype");
	}

	return 0;
}

/* RST
   .. method:: remove_row(row)

      Delete the row at the specified index from the table.

      :arg row: The row index to delete.
      :type row: :class:`int`
*/
int LuaTable::remove_row(lua_State* L) {
	get()->remove(luaL_checkuint32(L, 2));
	return 0;
}

/* RST
   .. method:: remove_entry(entry)

      Delete the row with the specified internal value from the table.
      Only allowed for tables with supported datatypes.

      :arg entry: The entry value to delete.
      :type entry: :class:`int`
*/
int LuaTable::remove_entry(lua_State* L) {
	if (upcast(TableOfInt, table, get()); table != nullptr) {
		table->remove_entry(luaL_checkuint32(L, 2));
	} else {
		report_error(L, "get() not allowed for table with unsupported datatype");
	}
	return 0;
}

/*
 * C Functions
 */

}  // namespace LuaUi
