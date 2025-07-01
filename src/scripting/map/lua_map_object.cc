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

#include "scripting/map/lua_map_object.h"

#include "scripting/globals.h"

namespace LuaMaps {

/* RST
MapObject
---------

.. class:: MapObject

   This is the base class for all objects in Widelands, including
   :class:`immovables <BaseImmovable>` and :class:`bobs <Bob>`. This
   class can't be instantiated directly, but provides the base for all
   others.
*/
const char LuaMapObject::className[] = "MapObject";
const MethodType<LuaMapObject> LuaMapObject::Methods[] = {
   METHOD(LuaMapObject, remove), METHOD(LuaMapObject, destroy),
   METHOD(LuaMapObject, __eq),   METHOD(LuaMapObject, has_attribute),
   {nullptr, nullptr},
};
const PropertyType<LuaMapObject> LuaMapObject::Properties[] = {
   PROP_RO(LuaMapObject, __hash), PROP_RO(LuaMapObject, descr), PROP_RO(LuaMapObject, serial),
   PROP_RO(LuaMapObject, exists), {nullptr, nullptr, nullptr},
};

void LuaMapObject::__persist(lua_State* L) {
	Widelands::MapObjectSaver& mos = *get_mos(L);
	Widelands::Game& game = get_game(L);

	uint32_t idx = 0;
	if (Widelands::MapObject* obj = ptr_.get(game)) {
		idx = mos.get_object_file_index(*obj);
	}

	PERS_UINT32("file_index", idx);
}
void LuaMapObject::__unpersist(lua_State* L) {
	uint32_t idx;
	UNPERS_UINT32("file_index", idx)

	if (idx == 0u) {
		ptr_ = nullptr;
	} else {
		Widelands::MapObjectLoader& mol = *get_mol(L);
		ptr_ = &mol.get<Widelands::MapObject>(idx);
	}
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
   .. attribute:: __hash

      (RO) The map object's serial. Used to identify a class in a Set.
*/
int LuaMapObject::get___hash(lua_State* L) {
	lua_pushuint32(L, get(L, get_egbase(L))->serial());
	return 1;
}

/* RST
   .. attribute:: serial

      (RO) The serial number of this object. Note that this value does not stay
      constant after saving/loading.
*/
int LuaMapObject::get_serial(lua_State* L) {
	lua_pushuint32(L, get(L, get_egbase(L))->serial());
	return 1;
}

/* RST
   .. attribute:: descr

      (RO) The :class:`MapObjectDescription` for this immovable.

      .. code-block:: lua

         local immovable = wl.Game().map:get_field(20,31).immovable

         -- always check if the immovable was found on the field
         if immovable then
            if immovable.descr.type_name == "warehouse"  -- access MapObjectDescription
               immovable:set_wares("log", 5)
            end
         end

*/
int LuaMapObject::get_descr(lua_State* L) {
	const Widelands::MapObjectDescr* desc = &get(L, get_egbase(L))->descr();
	assert(desc != nullptr);

	return upcasted_map_object_descr_to_lua(L, desc);
}

/* RST
   .. attribute:: exists

      .. versionadded:: 1.2

      (RO) Whether the map object represented by this Lua object still exists.

      If it does not exist, no other attributes or functions of this object may be accessed.
*/
int LuaMapObject::get_exists(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get_or_zero(get_egbase(L)) != nullptr));
	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
int LuaMapObject::__eq(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	LuaMapObject* other = *get_base_user_class<LuaMapObject>(L, -1);

	Widelands::MapObject* me = get_or_zero(egbase);
	Widelands::MapObject* you = other->get_or_zero(egbase);

	// Both objects are destroyed (nullptr) or equal: they are equal
	if (me == you) {
		lua_pushboolean(L, 1);
	} else if (me == nullptr ||
	           you == nullptr) {  // One of the objects is destroyed: they are distinct
		lua_pushboolean(L, 0);
	} else {  // Compare their serial number.
		lua_pushboolean(
		   L, static_cast<int>(other->get(L, egbase)->serial() == get(L, egbase)->serial()));
	}

	return 1;
}

/* RST
   .. method:: remove()

      Removes this object immediately. If you want to destroy an
      object as if the player had see :func:`destroy`.
*/
int LuaMapObject::remove(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::MapObject* o = get(L, egbase);
	if (o == nullptr) {
		return 0;
	}

	o->remove(egbase);
	return 0;
}

/* RST
   .. method:: destroy()

      Removes this object immediately. Might do special actions (like leaving a
      burning fire). If you want to remove an object without side effects, see
      :func:`remove`.
*/
int LuaMapObject::destroy(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::MapObject* o = get(L, egbase);
	if (o == nullptr) {
		return 0;
	}

	o->destroy(egbase);
	return 0;
}

/* RST
   .. method:: has_attribute(attribute)

      Returns :const:`true` if the map object has this attribute, :const:`false` otherwise.

      :arg attribute: The attribute to check for.
      :type attribute: :class:`string`
*/
int LuaMapObject::has_attribute(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::MapObject* obj = get_or_zero(egbase);
	if (obj == nullptr) {
		lua_pushboolean(L, 0);
		return 1;
	}

	// Check if object has the attribute
	std::string attrib = luaL_checkstring(L, 2);
	if (obj->has_attribute(Widelands::MapObjectDescr::get_attribute_id(attrib))) {
		lua_pushboolean(L, 1);
	} else {
		lua_pushboolean(L, 0);
	}
	return 1;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */
Widelands::MapObject*
LuaMapObject::get(lua_State* L, Widelands::EditorGameBase& egbase, const std::string& name) {
	Widelands::MapObject* o = get_or_zero(egbase);
	if (o == nullptr) {
		report_error(L, "%s no longer exists!", name.c_str());
	}
	return o;
}
Widelands::MapObject* LuaMapObject::get_or_zero(const Widelands::EditorGameBase& egbase) {
	return ptr_.get(egbase);
}

}  // namespace LuaMaps
