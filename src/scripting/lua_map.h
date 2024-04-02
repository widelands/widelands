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

#ifndef WL_SCRIPTING_LUA_MAP_H
#define WL_SCRIPTING_LUA_MAP_H

#include <map>

#include "logic/map_objects/tribes/wareworker.h"
#include "logic/widelands.h"
#include "scripting/lua.h"
#include "scripting/luna.h"

namespace Widelands {
class MapObject;
class MapObjectDescr;
class TribeDescr;
}  // namespace Widelands

namespace LuaMaps {

// Used as return for parse_ware_workers_* functions
enum class RequestedWareWorker { kAll, kSingle, kList, kUndefined };

/*
 * Base class for all classes in wl.map
 */
class LuaMapModuleClass : public LunaClass {
public:
	const char* get_modulename() override {
		return "map";
	}
};

using InputMap =
   std::map<std::pair<Widelands::DescriptionIndex, Widelands::WareWorker>, Widelands::Quantity>;

int upcasted_map_object_descr_to_lua(lua_State* L, const Widelands::MapObjectDescr* descr);
int upcasted_map_object_to_lua(lua_State* L, Widelands::MapObject* mo);
RequestedWareWorker parse_wares_workers_list(lua_State*,
                                             const Widelands::TribeDescr&,
                                             Widelands::DescriptionIndex*,
                                             std::vector<Widelands::DescriptionIndex>*,
                                             bool is_ware);
RequestedWareWorker parse_wares_workers_counted(lua_State*,
                                                const Widelands::TribeDescr&,
                                                LuaMaps::InputMap*,
                                                bool is_ware);
void luaopen_wlmap(lua_State*);

}  // namespace LuaMaps

#endif  // end of include guard: WL_SCRIPTING_LUA_MAP_H
