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

namespace LuaMaps {

// TODO(kaputtnik): Readd RST once this get fully implemented
/*
MarketDescription
-----------------

.. class:: MarketDescription

   A static description of a tribe's market. A Market is used for
   trading over land with other players. See the parent classes for more
   properties.
*/
// TODO(sirver,trading): Expose the properties of MarketDescription here once
// the interface settles.
const char LuaMarketDescription::className[] = "MarketDescription";
const MethodType<LuaMarketDescription> LuaMarketDescription::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaMarketDescription> LuaMarketDescription::Properties[] = {
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

}  // namespace LuaMaps
