/*
 * Copyright (C) 2024 by the Widelands Development Team
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

#include "scripting/map/lua_training_site.h"

#include "logic/player.h"
#include "scripting/globals.h"

namespace LuaMaps {

/* RST
TrainingSite
--------------

.. class:: TrainingSite

   A specialized production site for training soldiers.

   See also: :ref:`has_soldiers`

   More properties are available through this object's
   :class:`TrainingSiteDescription`, which you can access via :any:`MapObject.descr`.
*/
const char LuaTrainingSite::className[] = "TrainingSite";
const MethodType<LuaTrainingSite> LuaTrainingSite::Methods[] = {
   METHOD(LuaTrainingSite, get_soldiers),
   METHOD(LuaTrainingSite, set_soldiers),
   {nullptr, nullptr},
};
const PropertyType<LuaTrainingSite> LuaTrainingSite::Properties[] = {
   PROP_RO(LuaTrainingSite, max_soldiers),
   PROP_RW(LuaTrainingSite, capacity),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

// documented in parent class
int LuaTrainingSite::get_max_soldiers(lua_State* L) {
	lua_pushuint32(L, get(L, get_egbase(L))->soldier_control()->soldier_capacity());
	return 1;
}

/* RST
   .. attribute:: capacity

      (RW) The number of soldiers meant to be stationed here.
*/
int LuaTrainingSite::set_capacity(lua_State* L) {
	get(L, get_egbase(L))->mutable_soldier_control()->set_soldier_capacity(luaL_checkuint32(L, -1));
	return 0;
}
int LuaTrainingSite::get_capacity(lua_State* L) {
	lua_pushuint32(L, get(L, get_egbase(L))->soldier_control()->soldier_capacity());
	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

// documented in parent class
int LuaTrainingSite::get_soldiers(lua_State* L) {
	Widelands::TrainingSite* ts = get(L, get_egbase(L));
	return do_get_soldiers(L, *ts->soldier_control(), ts->owner().tribe());
}

// documented in parent class
int LuaTrainingSite::set_soldiers(lua_State* L) {
	Widelands::TrainingSite* ts = get(L, get_egbase(L));
	return do_set_soldiers(L, ts->get_position(), ts->mutable_soldier_control(), ts->get_owner());
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

}  // namespace LuaMaps
