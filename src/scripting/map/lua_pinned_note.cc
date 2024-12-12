/*
 * Copyright (C) 2006-2024 by the Widelands Development Team
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

#include "scripting/map/lua_pinned_note.h"

#include "logic/player.h"
#include "scripting/factory.h"
#include "scripting/globals.h"

namespace LuaMaps {

/* RST
PinnedNote
----------

.. class:: PinnedNote

   .. versionadded:: 1.2

   This represents a note pinned to a map field.

   More properties are available through this object's
   :class:`MapObjectDescription`, which you can access via :any:`MapObject.descr`.
*/

const char LuaPinnedNote::className[] = "PinnedNote";
const MethodType<LuaPinnedNote> LuaPinnedNote::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaPinnedNote> LuaPinnedNote::Properties[] = {
   PROP_RO(LuaPinnedNote, owner),
   PROP_RW(LuaPinnedNote, text),
   PROP_RW(LuaPinnedNote, color),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
   .. attribute:: owner

      (RO) The :class:`wl.game.Player` who owns this object.
*/
int LuaPinnedNote::get_owner(lua_State* L) {
	get_factory(L).push_player(L, get(L, get_egbase(L))->get_owner()->player_number());
	return 1;
}

/* RST
   .. attribute:: text

      (RW) The text of the note.
*/
int LuaPinnedNote::get_text(lua_State* L) {
	lua_pushstring(L, get(L, get_egbase(L))->get_text().c_str());
	return 1;
}
int LuaPinnedNote::set_text(lua_State* L) {
	get(L, get_egbase(L))->set_text(luaL_checkstring(L, -1));
	return 0;
}

/* RST
   .. attribute:: color

      (RW) The color of the note, as an :class:`array` of three integers representing R, G, and B.
*/
int LuaPinnedNote::get_color(lua_State* L) {
	const RGBColor& rgb = get(L, get_egbase(L))->get_rgb();
	lua_newtable(L);
	lua_pushuint32(L, 1);
	lua_pushuint32(L, rgb.r);
	lua_rawset(L, -3);
	lua_pushuint32(L, 2);
	lua_pushuint32(L, rgb.g);
	lua_rawset(L, -3);
	lua_pushuint32(L, 3);
	lua_pushuint32(L, rgb.b);
	lua_rawset(L, -3);
	return 1;
}
int LuaPinnedNote::set_color(lua_State* L) {
	luaL_checktype(L, -1, LUA_TTABLE);

	lua_geti(L, -1, 1);
	const unsigned r = luaL_checkuint32(L, -1);
	lua_pop(L, 1);

	lua_geti(L, -1, 2);
	const unsigned g = luaL_checkuint32(L, -1);
	lua_pop(L, 1);

	lua_geti(L, -1, 3);
	const unsigned b = luaL_checkuint32(L, -1);
	lua_pop(L, 1);

	get(L, get_egbase(L))->set_rgb(RGBColor(r, g, b));

	return 0;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

}  // namespace LuaMaps
