/*
 * Copyright (C) 2006-2026 by the Widelands Development Team
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

#include "scripting/map/lua_construction_site_description.h"

namespace LuaMaps {

/* RST
ConstructionSiteDescription
---------------------------

.. class:: ConstructionSiteDescription

   A static description of a tribe's constructionsite. See the parent classes for more properties.
*/
const char LuaConstructionSiteDescription::className[] = "ConstructionSiteDescription";
const MethodType<LuaConstructionSiteDescription> LuaConstructionSiteDescription::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaConstructionSiteDescription> LuaConstructionSiteDescription::Properties[] = {
   {nullptr, nullptr, nullptr},
};

}  // namespace LuaMaps
