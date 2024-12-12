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

#include "scripting/map/lua_resource_description.h"

#include "logic/map_objects/descriptions.h"
#include "scripting/globals.h"

namespace LuaMaps {

/* RST
ResourceDescription
--------------------
.. class:: ResourceDescription

   A static description of a resource.
*/
const char LuaResourceDescription::className[] = "ResourceDescription";
const MethodType<LuaResourceDescription> LuaResourceDescription::Methods[] = {
   METHOD(LuaResourceDescription, editor_image),
   {nullptr, nullptr},
};
const PropertyType<LuaResourceDescription> LuaResourceDescription::Properties[] = {
   PROP_RO(LuaResourceDescription, name),
   PROP_RO(LuaResourceDescription, descname),
   PROP_RO(LuaResourceDescription, is_detectable),
   PROP_RO(LuaResourceDescription, max_amount),
   PROP_RO(LuaResourceDescription, representative_image),
   {nullptr, nullptr, nullptr},
};

void LuaResourceDescription::__persist(lua_State* L) {
	const Widelands::ResourceDescription* descr = get();
	PERS_STRING("name", descr->name());
}

void LuaResourceDescription::__unpersist(lua_State* L) {
	std::string name;
	UNPERS_STRING("name", name)
	const Widelands::Descriptions& descriptions = get_egbase(L).descriptions();
	const Widelands::ResourceDescription* descr =
	   descriptions.get_resource_descr(descriptions.safe_resource_index(name));
	set_description_pointer(descr);
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: name

      (RO) The internal name of this resource as :class:`string`.
*/

int LuaResourceDescription::get_name(lua_State* L) {
	lua_pushstring(L, get()->name());
	return 1;
}

/* RST
   .. attribute:: descname

      (RO) The display name of this resource as :class:`string`.
*/

int LuaResourceDescription::get_descname(lua_State* L) {
	lua_pushstring(L, get()->descname());
	return 1;
}

/* RST
   .. attribute:: is_detectable

      (RO) :const:`true` if geologists can find this resource.
*/

int LuaResourceDescription::get_is_detectable(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->detectable()));
	return 1;
}

/* RST
   .. attribute:: max_amount

      (RO) The maximum amount of this resource that a terrain can have.
*/

int LuaResourceDescription::get_max_amount(lua_State* L) {
	lua_pushinteger(L, get()->max_amount());
	return 1;
}

/* RST
   .. attribute:: representative_image

      (RO) The path to the image representing this resource in the GUI as :class:`string`.
*/
int LuaResourceDescription::get_representative_image(lua_State* L) {
	lua_pushstring(L, get()->representative_image());
	return 1;
}

/*
 ==========================================================
 METHODS
 ==========================================================
 */

/* RST
   .. method:: editor_image(amount)

      (RO) The path to the image representing the specified amount of this
      resource as :class:`string`.

      :arg amount: The amount of the resource what we want an overlay image for.
      :type amount: :class:`integer`

*/
int LuaResourceDescription::editor_image(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}
	const uint32_t amount = luaL_checkuint32(L, 2);
	lua_pushstring(L, get()->editor_image(amount));
	return 1;
}

}  // namespace LuaMaps
