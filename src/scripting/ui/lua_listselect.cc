/*
 * Copyright (C) 2006-2026 by the Widelands Development Team
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

#include "scripting/ui/lua_listselect.h"

#include "graphic/image_cache.h"
#include "scripting/ui/lua_dropdown.h"

namespace LuaUi {

/* RST
Listselect
----------

.. class:: Listselect

   .. versionadded:: 1.2

   This represents a selection list.

   Some attributes and functions are available only for listselects with a data type
   supported by the Lua interface. See :attr:`datatype`.
*/
const char LuaListselect::className[] = "Listselect";
const MethodType<LuaListselect> LuaListselect::Methods[] = {
   METHOD(LuaListselect, select),
   METHOD(LuaListselect, clear),
   METHOD(LuaListselect, add),
   METHOD(LuaListselect, get_value_at),
   METHOD(LuaListselect, get_label_at),
   METHOD(LuaListselect, get_tooltip_at),
   METHOD(LuaListselect, get_enable_at),
   METHOD(LuaListselect, get_indent_at),
   {nullptr, nullptr},
};
const PropertyType<LuaListselect> LuaListselect::Properties[] = {
   PROP_RO(LuaListselect, datatype),  PROP_RO(LuaListselect, no_of_items),
   PROP_RO(LuaListselect, selection), PROP_RO(LuaListselect, linked_dropdown),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: datatype

      (RO) The listselect's datatype as :class:`string` if supported,
      or ``nil`` for listselects with unsupported datatype.

      Currently only ``"string"`` is supported.
*/
int LuaListselect::get_datatype(lua_State* L) {
	if (dynamic_cast<const ListselectOfString*>(get()) != nullptr) {
		lua_pushstring(L, "string");
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: no_of_items

      (RO) The number of items this listselect has.
*/
int LuaListselect::get_no_of_items(lua_State* L) {
	lua_pushinteger(L, get()->size());
	return 1;
}

/* RST
   .. attribute:: selection

      (RO) The currently selected entry or ``nil`` if none is selected.
      For listselects with unsupported datatype, this is always ``nil``.
*/
int LuaListselect::get_selection(lua_State* L) {
	if (upcast(const ListselectOfString, list, get()); list != nullptr && list->has_selection()) {
		lua_pushstring(L, list->get_selected().c_str());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: linked_dropdown

      .. versionadded:: 1.3

      (RO) If this listselect represents the list component of a dropdown,
      this links to the :class:`Dropdown`. Otherwise this is ``nil``.
*/
int LuaListselect::get_linked_dropdown(lua_State* L) {
	if (UI::BaseDropdown* dd = get()->get_linked_dropdown(); dd != nullptr) {
		to_lua<LuaDropdown>(L, new LuaDropdown(dd));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/*
 * Lua Functions
 */
/* RST
   .. method:: select(index)

      .. versionadded:: 1.3

      Select the item with the given index.

      :arg index: The index to select, starting from ``1``.
      :type index: :class:`int`
*/
int LuaListselect::select(lua_State* L) {
	get()->select(luaL_checkuint32(L, 2) - 1);
	return 0;
}

/* RST
   .. method:: clear()

      .. versionadded:: 1.3

      Remove all entries from the listselect.
*/
int LuaListselect::clear(lua_State* /* L */) {
	get()->clear();
	return 0;
}

/* RST
   .. method:: add(label, value
      [, icon = nil, tooltip = "", select = false, indent = 0, enable = true, hotkey = nil])

      .. versionchanged:: 1.3
         Added ``enable`` and ``hotkey`` parameters.

      Add an entry to the list. Only allowed for lists with supported datatypes.

      :arg label: The label to display for the entry.
      :type label: :class:`string`
      :arg value: The internal value of the entry.
      :type value: This list's :attr:`datatype`
      :arg icon: The icon filepath for the entry (``nil`` for no icon).
      :type icon: :class:`string`
      :arg tooltip: The entry's tooltip text.
      :type tooltip: :class:`string`
      :arg select: Whether to select this entry.
      :type select: :class:`boolean`
      :arg indent: By how many levels to indent this entry.
      :type indent: :class:`int`
      :arg enable: Whether to enable this entry.
      :type enable: :class:`boolean`
      :arg hotkey: The internal name of the hotkey for this entry.
      :type hotkey: :class:`string`
*/
int LuaListselect::add(lua_State* L) {
	int top = lua_gettop(L);
	std::string label = luaL_checkstring(L, 2);
	std::string icon = (top >= 4 && !lua_isnil(L, 4)) ? luaL_checkstring(L, 4) : "";
	std::string tooltip = top >= 5 ? luaL_checkstring(L, 5) : "";
	bool select = top >= 6 && luaL_checkboolean(L, 6);
	uint32_t indent = top >= 7 ? luaL_checkuint32(L, 7) : 0;
	bool enable = top < 8 || luaL_checkboolean(L, 8);
	std::string hotkey = top >= 9 ? luaL_checkstring(L, 9) : "";

	if (upcast(ListselectOfString, list, get()); list != nullptr) {
		std::string value = luaL_checkstring(L, 3);
		list->add(label, value, icon.empty() ? nullptr : g_image_cache->get(icon), select, tooltip,
		          shortcut_string_if_set(hotkey, false), indent, enable);
	} else {
		report_error(L, "add() not allowed for listselect with unsupported datatype");
	}
	return 0;
}

/* RST
   .. method:: get_value_at(index)

      .. versionadded:: 1.3

      Get the internal value of the item at the specified position.
      Only allowed for lists with supported datatypes.

      :arg index: The index to query, starting from ``1``.
      :type index: :class:`integer`
      :returns: The entry's internal value.
      :rtype: This list's :attr:`datatype`
*/
int LuaListselect::get_value_at(lua_State* L) {
	const uint32_t index = luaL_checkuint32(L, 2);
	const uint32_t nritems = get()->size();
	if (index < 1 || index > nritems) {
		report_error(L, "Index %u out of range for list of size %u", index, nritems);
	}
	if (upcast(ListselectOfString, dd, get()); dd != nullptr) {
		lua_pushstring(L, (*dd)[index - 1]);
	} else {
		report_error(L, "get_value_at() not allowed for list with unsupported datatype");
	}
	return 1;
}

/* RST
   .. method:: get_label_at(index)

      .. versionadded:: 1.3

      Get the user-facing name of the item at the specified position.

      :arg index: The index to query, starting from ``1``.
      :type index: :class:`integer`
      :returns: The entry's display name.
      :rtype: :class:`string`
*/
int LuaListselect::get_label_at(lua_State* L) {
	const uint32_t index = luaL_checkuint32(L, 2);
	const uint32_t nritems = get()->size();
	if (index < 1 || index > nritems) {
		report_error(L, "Index %u out of range for list of size %u", index, nritems);
	}
	lua_pushstring(L, get()->at(index - 1).name);
	return 1;
}

/* RST
   .. method:: get_tooltip_at(index)

      .. versionadded:: 1.3

      Get the tooltip of the item at the specified position.

      :arg index: The index to query, starting from ``1``.
      :type index: :class:`integer`
      :returns: The entry's tooltip (may be empty if the item has no tooltip).
      :rtype: :class:`string`
*/
int LuaListselect::get_tooltip_at(lua_State* L) {
	const uint32_t index = luaL_checkuint32(L, 2);
	const uint32_t nritems = get()->size();
	if (index < 1 || index > nritems) {
		report_error(L, "Index %u out of range for list of size %u", index, nritems);
	}
	lua_pushstring(L, get()->at(index - 1).tooltip);
	return 1;
}

/* RST
   .. method:: get_enable_at(index)

      .. versionadded:: 1.3

      Get the enable/disable display state of the item at the specified position.

      :arg index: The index to query, starting from ``1``.
      :type index: :class:`integer`
      :returns: Whether the entry is marked as enabled.
      :rtype: :class:`boolean`
*/
int LuaListselect::get_enable_at(lua_State* L) {
	const uint32_t index = luaL_checkuint32(L, 2);
	const uint32_t nritems = get()->size();
	if (index < 1 || index > nritems) {
		report_error(L, "Index %u out of range for list of size %u", index, nritems);
	}
	lua_pushboolean(L, static_cast<int>(get()->at(index - 1).enable));
	return 1;
}

/* RST
   .. method:: get_indent_at(index)

      .. versionadded:: 1.3

      Get the indentation level of the item at the specified position.

      :arg index: The index to query, starting from ``1``.
      :type index: :class:`integer`
      :returns: The entry's indentation.
      :rtype: :class:`integer`
*/
int LuaListselect::get_indent_at(lua_State* L) {
	const uint32_t index = luaL_checkuint32(L, 2);
	const uint32_t nritems = get()->size();
	if (index < 1 || index > nritems) {
		report_error(L, "Index %u out of range for list of size %u", index, nritems);
	}
	lua_pushinteger(L, get()->at(index - 1).indent);
	return 1;
}

/*
 * C Functions
 */

}  // namespace LuaUi
