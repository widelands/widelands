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

#include "scripting/map/lua_market_description.h"

#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/tribes/worker_descr.h"
#include "scripting/globals.h"

namespace LuaMaps {

/* RST
MarketDescription
-----------------

.. class:: MarketDescription

   .. versionadded:: 1.3

   A static description of a tribe's market. A Market is used for
   trading over land with other players. See the parent classes for more
   properties.
*/
const char LuaMarketDescription::className[] = "MarketDescription";
const MethodType<LuaMarketDescription> LuaMarketDescription::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaMarketDescription> LuaMarketDescription::Properties[] = {
   PROP_RO(LuaMarketDescription, local_carrier),
   PROP_RO(LuaMarketDescription, trade_carrier),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: local_carrier

      (RO) The name of the worker that works in the market.
*/
int LuaMarketDescription::get_local_carrier(lua_State* L) {
	const Widelands::WorkerDescr* wd =
	   get_egbase(L).descriptions().get_worker_descr(get()->local_carrier);
	lua_pushstring(L, wd->name().c_str());
	return 1;
}

/* RST
   .. attribute:: trade_carrier

      (RO) The name of the worker that carries wares across the map to other markets.
*/
int LuaMarketDescription::get_trade_carrier(lua_State* L) {
	const Widelands::WorkerDescr* wd =
	   get_egbase(L).descriptions().get_worker_descr(get()->trade_carrier);
	lua_pushstring(L, wd->name().c_str());
	return 1;
}

}  // namespace LuaMaps
