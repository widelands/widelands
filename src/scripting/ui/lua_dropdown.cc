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

#include "scripting/ui/lua_dropdown.h"

#include "scripting/ui/lua_listselect.h"

namespace LuaUi {

/* RST
Dropdown
--------

.. class:: Dropdown

   This represents a dropdown menu.

   Some attributes and functions are available only for dropdowns with a data type
   supported by the Lua interface. See :attr:`datatype`.
*/
const char LuaDropdown::className[] = "Dropdown";
const MethodType<LuaDropdown> LuaDropdown::Methods[] = {
   METHOD(LuaDropdown, open),
   METHOD(LuaDropdown, highlight_item),
#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
   METHOD(LuaDropdown, indicate_item),
#endif
   METHOD(LuaDropdown, select),
   METHOD(LuaDropdown, clear),
   METHOD(LuaDropdown, add),
   METHOD(LuaDropdown, get_value_at),
   METHOD(LuaDropdown, get_label_at),
   METHOD(LuaDropdown, get_tooltip_at),
   {nullptr, nullptr},
};
const PropertyType<LuaDropdown> LuaDropdown::Properties[] = {
   PROP_RO(LuaDropdown, datatype),    PROP_RO(LuaDropdown, expanded),
   PROP_RO(LuaDropdown, no_of_items), PROP_RO(LuaDropdown, selection),
   PROP_RO(LuaDropdown, listselect),  {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: datatype

      .. versionadded:: 1.2

      (RO) The dropdown's datatype as :class:`string` if supported,
      or ``nil`` for dropdowns with unsupported datatype.

      Currently only ``"string"`` is supported.
*/
int LuaDropdown::get_datatype(lua_State* L) {
	if (dynamic_cast<const DropdownOfString*>(get()) != nullptr) {
		lua_pushstring(L, "string");
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: expanded

      (RO) True if the dropdown's list is currently expanded.
*/
int LuaDropdown::get_expanded(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->is_expanded()));
	return 1;
}

/* RST
   .. attribute:: no_of_items

      (RO) The number of items this dropdown has.
*/
int LuaDropdown::get_no_of_items(lua_State* L) {
	lua_pushinteger(L, get()->size());
	return 1;
}

/* RST
   .. attribute:: selection

      .. versionadded:: 1.2

      (RO) The currently selected entry or ``nil`` if none is selected.
      For dropdowns with unsupported datatype, this is always ``nil``.
*/
int LuaDropdown::get_selection(lua_State* L) {
	if (upcast(const DropdownOfString, dd, get()); dd != nullptr && dd->has_selection()) {
		lua_pushstring(L, dd->get_selected().c_str());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: listselect

      .. versionadded:: 1.3

      (RO) This links to the :class:`Listselect` representing the dropdown's list of items.
*/
int LuaDropdown::get_listselect(lua_State* L) {
	to_lua<LuaListselect>(L, new LuaListselect(get()->get_list()));
	return 1;
}

/*
 * Lua Functions
 */
/* RST
   .. method:: open

      Open this dropdown menu.
*/
int LuaDropdown::open(lua_State* /* L */) {
	verb_log_info("Opening dropdown '%s'\n", get()->get_name().c_str());
	get()->set_list_visibility(true);
	return 0;
}

/* RST
   .. method:: highlight_item(index)

      :arg index: the index of the item to highlight, starting from ``1``
      :type index: :class:`integer`

      Highlights an item in this dropdown without triggering a selection.
*/
int LuaDropdown::highlight_item(lua_State* L) {
	unsigned int desired_item = luaL_checkuint32(L, -1);
	if (desired_item < 1 || desired_item > get()->size()) {
		report_error(L,
		             "Attempted to highlight item %u on dropdown '%s'. Available range for this "
		             "dropdown is 1-%u.",
		             desired_item, get()->get_name().c_str(), get()->size());
	}
	verb_log_info(
	   "Highlighting item %u in dropdown '%s'\n", desired_item, get()->get_name().c_str());
	// Open the dropdown
	get()->set_list_visibility(true);

	SDL_Keysym code;
	// Ensure that we're at the top
	code.sym = SDLK_UP;
	code.scancode = SDL_SCANCODE_UP;
	code.mod = KMOD_NONE;
	code.unused = 0;
	for (size_t i = 1; i < get()->size(); ++i) {
		get()->handle_key(true, code);
	}
	// Press arrow down until the desired item is highlighted
	code.sym = SDLK_DOWN;
	code.scancode = SDL_SCANCODE_DOWN;
	for (size_t i = 1; i < desired_item; ++i) {
		get()->handle_key(true, code);
	}
	return 0;
}

#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
/* R#S#T
   .. method:: indicate_item(index)

      :arg index: the index of the item to indicate, starting from ``1``
      :type index: :class:`integer`

      Show an arrow that points to an item in this dropdown. You can only point to 1 panel at the
      same time.
*/
int LuaDropdown::indicate_item(lua_State* L) {
	assert(panel_);
	if (lua_gettop(L) != 2) {
		report_error(L, "Expected 1 int");
	}

	InteractivePlayer* ipl = get_game(L).get_ipl();
	if (ipl == nullptr) {
		report_error(L, "This can only be called when there's an interactive player");
	}

	size_t desired_item = luaL_checkuint32(L, -1);
	if (desired_item < 1 || desired_item > get()->size()) {
		report_error(L,
		             "Attempted to indicate item %" PRIuS
		             " on dropdown '%s'. Available range for this "
		             "dropdown is 1-%d.",
		             desired_item, get()->get_name().c_str(), get()->size());
	}
	verb_log_info(
	   "Indicating item %" PRIuS " in dropdown '%s'\n", desired_item, get()->get_name().c_str());

	int x = panel_->get_x() + panel_->get_w();
	int y = panel_->get_y();
	UI::Panel* parent = panel_->get_parent();
	while (parent != nullptr) {
		x += parent->get_x() + parent->get_lborder();
		y += parent->get_y() + parent->get_tborder();
		parent = parent->get_parent();
	}

	// Open the dropdown
	get()->set_list_visibility(true);

	for (; desired_item <= get()->size(); ++desired_item) {
		y -= get()->lineheight();
	}

	ipl->set_training_wheel_indicator_pos(Vector2i(x, y));

	return 0;
}
#endif

/* RST
   .. method:: select()

      Selects the currently highlighted item in this dropdown.
*/
int LuaDropdown::select(lua_State* /* L */) {
	verb_log_info("Selecting current item in dropdown '%s'\n", get()->get_name().c_str());
	SDL_Keysym code;
	code.sym = SDLK_RETURN;
	code.scancode = SDL_SCANCODE_RETURN;
	code.mod = KMOD_NONE;
	code.unused = 0;
	get()->handle_key(true, code);
	return 0;
}

/* RST
   .. method:: clear()

      .. versionadded:: 1.3

      Remove all entries from the dropdown. Only allowed for dropdowns with supported datatypes.
*/
int LuaDropdown::clear(lua_State* L) {
	if (upcast(DropdownOfString, dd, get()); dd != nullptr) {
		dd->clear();
	} else {
		report_error(L, "clear() not allowed for dropdown with unsupported datatype");
	}
	return 0;
}

/* RST
   .. method:: add(label, value[, icon = nil, tooltip = "", select = false, hotkey = nil])

      .. versionadded:: 1.2

      .. versionchanged:: 1.3
         Added parameter ``hotkey``.

      Add an entry to the dropdown. Only allowed for dropdowns with supported datatypes.

      :arg label: The label to display for the entry.
      :type label: :class:`string`
      :arg value: The internal value of the entry.
      :type value: This dropdown's :attr:`datatype`
      :arg icon: The icon filepath for the entry (``nil`` for no icon).
      :type icon: :class:`string`
      :arg tooltip: The entry's tooltip text.
      :type tooltip: :class:`string`
      :arg select: Whether to select this entry.
      :type select: :class:`boolean`
      :arg hotkey: The internal name of the hotkey for this entry.
      :type hotkey: :class:`string`
*/
int LuaDropdown::add(lua_State* L) {
	int top = lua_gettop(L);
	std::string label = luaL_checkstring(L, 2);
	std::string icon = (top >= 4 && !lua_isnil(L, 4)) ? luaL_checkstring(L, 4) : "";
	std::string tooltip = top >= 5 ? luaL_checkstring(L, 5) : "";
	bool select = top >= 6 && luaL_checkboolean(L, 6);
	std::string hotkey = top >= 7 ? luaL_checkstring(L, 7) : "";

	if (upcast(DropdownOfString, dd, get()); dd != nullptr) {
		std::string value = luaL_checkstring(L, 3);
		dd->add(label, value, icon.empty() ? nullptr : g_image_cache->get(icon), select, tooltip,
		        shortcut_string_if_set(hotkey, false));
	} else {
		report_error(L, "add() not allowed for dropdown with unsupported datatype");
	}
	return 0;
}

/* RST
   .. method:: get_value_at(index)

      .. versionadded:: 1.3

      Get the internal value of the item at the specified position.
      Only allowed for dropdowns with supported datatypes.

      :arg index: The index to query, starting from ``1``.
      :type index: :class:`integer`
      :returns: The entry's internal value.
      :rtype: This list's :attr:`datatype`
*/
int LuaDropdown::get_value_at(lua_State* L) {
	const uint32_t index = luaL_checkuint32(L, 2);
	const uint32_t nritems = get()->size();
	if (index < 1 || index > nritems) {
		report_error(L, "Index %u out of range for dropdown of size %u", index, nritems);
	}
	if (upcast(DropdownOfString, dd, get()); dd != nullptr) {
		lua_pushstring(L, dd->at(index - 1).value);
	} else {
		report_error(L, "get_value_at() not allowed for dropdown with unsupported datatype");
	}
	return 1;
}

/* RST
   .. method:: get_label_at(index)

      .. versionadded:: 1.3

      Get the user-facing name of the item at the specified position.
      Only allowed for dropdowns with supported datatypes.

      :arg index: The index to query, starting from ``1``.
      :type index: :class:`integer`
      :returns: The entry's display name.
      :rtype: :class:`string`
*/
int LuaDropdown::get_label_at(lua_State* L) {
	const uint32_t index = luaL_checkuint32(L, 2);
	const uint32_t nritems = get()->size();
	if (index < 1 || index > nritems) {
		report_error(L, "Index %u out of range for dropdown of size %u", index, nritems);
	}
	if (upcast(DropdownOfString, dd, get()); dd != nullptr) {
		lua_pushstring(L, dd->at(index - 1).name);
	} else {
		report_error(L, "get_label_at() not allowed for dropdown with unsupported datatype");
	}
	return 1;
}

/* RST
   .. method:: get_tooltip_at(index)

      .. versionadded:: 1.3

      Get the tooltip of the item at the specified position.
      Only allowed for dropdowns with supported datatypes.

      :arg index: The index to query, starting from ``1``.
      :type index: :class:`integer`
      :returns: The entry's tooltip (may be empty if the item has no tooltip).
      :rtype: :class:`string`
*/
int LuaDropdown::get_tooltip_at(lua_State* L) {
	const uint32_t index = luaL_checkuint32(L, 2);
	const uint32_t nritems = get()->size();
	if (index < 1 || index > nritems) {
		report_error(L, "Index %u out of range for dropdown of size %u", index, nritems);
	}
	if (upcast(DropdownOfString, dd, get()); dd != nullptr) {
		lua_pushstring(L, dd->at(index - 1).tooltip);
	} else {
		report_error(L, "get_tooltip_at() not allowed for dropdown with unsupported datatype");
	}
	return 1;
}

/*
 * C Functions
 */

}  // namespace LuaUi
