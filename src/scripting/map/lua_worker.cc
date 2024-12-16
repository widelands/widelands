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

#include "scripting/map/lua_worker.h"

#include "logic/player.h"
#include "scripting/factory.h"
#include "scripting/globals.h"

namespace LuaMaps {

/* RST
Worker
------

.. class:: Worker

   All workers that are visible on the map are of this kind.

   More properties are available through this object's
   :class:`WorkerDescription`, which you can access via :any:`MapObject.descr`.
*/

const char LuaWorker::className[] = "Worker";
const MethodType<LuaWorker> LuaWorker::Methods[] = {
   METHOD(LuaWorker, evict),
   {nullptr, nullptr},
};
const PropertyType<LuaWorker> LuaWorker::Properties[] = {
   PROP_RO(LuaWorker, owner),
   PROP_RO(LuaWorker, location),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
   .. attribute:: owner

      (RO) The :class:`wl.game.Player` who owns this worker.
*/
// UNTESTED
int LuaWorker::get_owner(lua_State* L) {
	get_factory(L).push_player(L, get(L, get_egbase(L))->get_owner()->player_number());
	return 1;
}

/* RST
   .. attribute:: location

      (RO) The location where this worker is situated. This will be either a
      :class:`Building`, :class:`Road`, :class:`Flag` or :const:`nil`. Note
      that a worker that is stored in a warehouse has a location :const:`nil`.
      A worker that is out working (e.g. hunter) has as a location his
      building. A stationed soldier has his military building as location.
      Workers on transit usually have the Road they are currently on as
      location.
*/
// UNTESTED
int LuaWorker::get_location(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	return upcasted_map_object_to_lua(
	   L, dynamic_cast<Widelands::BaseImmovable*>(get(L, egbase)->get_location(egbase)));
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

/* RST
   .. method:: evict()

      .. versionadded:: 1.2

      Evict this worker from his current workplace.
*/
int LuaWorker::evict(lua_State* L) {
	Widelands::Game& game = get_game(L);
	get(L, game)->evict(game);
	return 0;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

}  // namespace LuaMaps
