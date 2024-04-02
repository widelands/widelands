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

#include "scripting/map/lua_dismantle_site.h"

#include "scripting/globals.h"

namespace LuaMaps {

/* RST
DismantleSite
-----------------

.. class:: DismantleSite

   A dismantle site as it appears in the game.

   More properties are available through this object's
   :class:`DismantleSiteDescription`, which you can access via :any:`MapObject.descr`.
*/
const char LuaDismantleSite::className[] = "DismantleSite";
const MethodType<LuaDismantleSite> LuaDismantleSite::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaDismantleSite> LuaDismantleSite::Properties[] = {
   PROP_RW(LuaDismantleSite, has_builder),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: has_builder

      (RW) :const:`true` if this dismantlesite has a builder. Changing this setting causes the
      worker to be instantly deleted, or to be created from thin air.
*/
int LuaDismantleSite::get_has_builder(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get(L, get_egbase(L))->builder_.is_set()));
	return 1;
}
int LuaDismantleSite::set_has_builder(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::PartiallyFinishedBuilding& cs = *get(L, egbase);
	const bool arg = luaL_checkboolean(L, -1);
	if (cs.builder_.is_set()) {
		if (!arg) {
			cs.builder_.get(egbase)->remove(egbase);
			cs.builder_ = nullptr;
			cs.set_seeing(false);
			if (upcast(Widelands::Game, g, &egbase)) {
				cs.request_builder(*g);
			}
		}
	} else if (arg) {
		assert(cs.builder_request_);
		delete cs.builder_request_;
		cs.builder_request_ = nullptr;
		Widelands::Worker& w = egbase.descriptions()
		                          .get_worker_descr(cs.owner().tribe().builder())
		                          ->create(egbase, cs.get_owner(), &cs, cs.get_position());
		cs.builder_ = &w;
		if (upcast(Widelands::Game, g, &egbase)) {
			w.start_task_buildingwork(*g);
		}
		cs.set_seeing(true);
	}
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
