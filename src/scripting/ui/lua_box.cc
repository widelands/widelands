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

#include "scripting/ui/lua_box.h"

namespace LuaUi {

/* RST
Box
---

.. class:: Box

   .. versionadded:: 1.3

   This represents a box that dynamically layouts its child components.
*/
const char LuaBox::className[] = "Box";
const MethodType<LuaBox> LuaBox::Methods[] = {
   METHOD(LuaBox, clear),        METHOD(LuaBox, get_index), METHOD(LuaBox, is_space),
   METHOD(LuaBox, get_resizing), METHOD(LuaBox, get_align), {nullptr, nullptr},
};
const PropertyType<LuaBox> LuaBox::Properties[] = {
   PROP_RO(LuaBox, orientation),   PROP_RO(LuaBox, no_of_items),
   PROP_RW(LuaBox, scrolling),     PROP_RW(LuaBox, force_scrolling),
   PROP_RW(LuaBox, inner_spacing), PROP_RW(LuaBox, min_desired_breadth),
   PROP_RW(LuaBox, max_width),     PROP_RW(LuaBox, max_height),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: orientation

      (RO) The box's layouting direction: ``"vertical"`` or ``"horizontal"``.
*/
int LuaBox::get_orientation(lua_State* L) {
	lua_pushstring(L, get()->get_orientation() == UI::Box::Horizontal ? "horizontal" : "vertical");
	return 1;
}

/* RST
   .. attribute:: no_of_items

      (RO) The number of items currently layouted by this box.

      An item can be either a panel which is a direct child of the box, or a space.
      A child of the box is not necessarily represented by an item.
*/
int LuaBox::get_no_of_items(lua_State* L) {
	lua_pushinteger(L, get()->get_nritems());
	return 1;
}

/* RST
   .. attribute:: scrolling

      (RW) Whether the box may scroll when its content is larger than the box.
*/
int LuaBox::get_scrolling(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->is_scrolling()));
	return 1;
}
int LuaBox::set_scrolling(lua_State* L) {
	get()->set_scrolling(luaL_checkboolean(L, -1));
	return 0;
}

/* RST
   .. attribute:: force_scrolling

      (RW) Whether the box will always show a scrollbar even if its content fits in the box.
*/
int LuaBox::get_force_scrolling(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->is_force_scrolling()));
	return 1;
}
int LuaBox::set_force_scrolling(lua_State* L) {
	get()->set_force_scrolling(luaL_checkboolean(L, -1));
	return 0;
}

/* RST
   .. attribute:: inner_spacing

      (RW) The spacing between items.
*/
int LuaBox::get_inner_spacing(lua_State* L) {
	lua_pushinteger(L, get()->get_inner_spacing());
	return 1;
}
int LuaBox::set_inner_spacing(lua_State* L) {
	get()->set_inner_spacing(luaL_checkuint32(L, 2));
	return 0;
}

/* RST
   .. attribute:: min_desired_breadth

      (RW) The minimum size of the box in the direction
      orthogonal to the primary layouting direction.
*/
int LuaBox::get_min_desired_breadth(lua_State* L) {
	lua_pushinteger(L, get()->get_min_desired_breadth());
	return 1;
}
int LuaBox::set_min_desired_breadth(lua_State* L) {
	get()->set_min_desired_breadth(luaL_checkuint32(L, 2));
	return 0;
}

/* RST
   .. attribute:: max_width

      (RW) The maximum width of the box.
*/
int LuaBox::get_max_width(lua_State* L) {
	lua_pushinteger(L, get()->get_max_x());
	return 1;
}
int LuaBox::set_max_width(lua_State* L) {
	get()->set_max_size(luaL_checkuint32(L, 2), get()->get_max_y());
	return 0;
}

/* RST
   .. attribute:: max_height

      (RW) The maximum height of the box.
*/
int LuaBox::get_max_height(lua_State* L) {
	lua_pushinteger(L, get()->get_max_y());
	return 1;
}
int LuaBox::set_max_height(lua_State* L) {
	get()->set_max_size(get()->get_max_x(), luaL_checkuint32(L, 2));
	return 0;
}

/*
 * Lua Functions
 */
/* RST
   .. method:: clear()

      Remove all items from the box's layouting. This does not delete the child items.
*/
int LuaBox::clear(lua_State* /* L */) {
	get()->clear();
	return 0;
}

/* RST
   .. method:: get_index(panel)

      Return the index of the given panel in the box,
      or ``nil`` if the box does not layout this panel.

      :arg index: The panel to query.
      :type index: :class:`Panel`
      :returns: The item's index, starting from ``1``.
      :rtype: :class:`integer`
*/
int LuaBox::get_index(lua_State* L) {
	LuaPanel* panel = *get_base_user_class<LuaPanel>(L, 2);
	const UI::Box* box = get();

	for (int i = 0; i < box->get_nritems(); ++i) {
		if (box->at(i).type == UI::Box::Item::Type::ItemPanel &&
		    box->at(i).u.panel.panel == panel->get()) {
			lua_pushinteger(L, i + 1);
			return 1;
		}
	}

	lua_pushnil(L);
	return 1;
}

/* RST
   .. method:: is_space(index)

      Check whether the item at the given index is a spacer or a child panel.

      :arg index: The index to query, starting from ``1``.
      :type index: :class:`integer`
      :returns: Whether the item is a space.
      :rtype: :class:`boolean`
*/
int LuaBox::is_space(lua_State* L) {
	const int index = luaL_checkint32(L, 2);
	if (index < 1 || index > get()->get_nritems()) {
		report_error(L, "Index %d out of range 1..%d", index, get()->get_nritems());
	}
	lua_pushboolean(L, static_cast<int>(get()->at(index).type == UI::Box::Item::Type::ItemSpace));
	return 1;
}

/* RST
   .. method:: get_resizing(index)

      Get the resizing strategy for the item at the given index.

      The result is one of ``"align"``, ``"fullsize"``, ``"fillspace"``, and ``"expandboth"``.
      For fixed-size spaces, this is always ``"align"``.
      For infinite spaces, this is always ``"fillspace"``.

      :arg index: The index to query, starting from ``1``.
      :type index: :class:`integer`
      :returns: The resizing strategy.
      :rtype: :class:`string`
*/
int LuaBox::get_resizing(lua_State* L) {
	const int index = luaL_checkint32(L, 2);
	if (index < 1 || index > get()->get_nritems()) {
		report_error(L, "Index %d out of range 1..%d", index, get()->get_nritems());
	}

	const auto& item = get()->at(index - 1);
	if (item.type == UI::Box::Item::Type::ItemPanel && item.u.panel.fullsize) {
		lua_pushstring(L, item.fillspace ? "expandboth" : "fullsize");
	} else {
		lua_pushstring(L, item.fillspace ? "fillspace" : "align");
	}

	return 1;
}

/* RST
   .. method:: get_align(index)

      Get the alignment for the item at the given index.

      The result is one of ``"left"``, ``"center"``, and ``"right"``.
      This function may not be called for spacers.

      :arg index: The index to query, starting from ``1``.
      :type index: :class:`integer`
      :returns: The item alignment.
      :rtype: :class:`string`
*/
int LuaBox::get_align(lua_State* L) {
	const int index = luaL_checkint32(L, 2);
	if (index < 1 || index > get()->get_nritems()) {
		report_error(L, "Index %d out of range 1..%d", index, get()->get_nritems());
	}

	const auto& item = get()->at(index - 1);
	if (item.type != UI::Box::Item::Type::ItemPanel) {
		report_error(L, "Index %d is a space", index);
	}

	switch (item.u.panel.align) {
	case UI::Align::kCenter:
		lua_pushstring(L, "center");
		break;
	case UI::Align::kLeft:
		lua_pushstring(L, "left");
		break;
	case UI::Align::kRight:
		lua_pushstring(L, "right");
		break;
	default:
		report_error(
		   L, "Index %d: Invalid alignment %d", index, static_cast<int>(item.u.panel.align));
	}

	return 1;
}

/*
 * C Functions
 */

}  // namespace LuaUi
