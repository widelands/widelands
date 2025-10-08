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

#ifndef WL_SCRIPTING_MAP_LUA_TRAINING_SITE_H
#define WL_SCRIPTING_MAP_LUA_TRAINING_SITE_H

#include "logic/map_objects/tribes/trainingsite.h"
#include "scripting/map/lua_production_site.h"

namespace LuaMaps {

class LuaTrainingSite : public LuaProductionSite {
public:
	LUNA_CLASS_HEAD(LuaTrainingSite);

	LuaTrainingSite() = default;
	explicit LuaTrainingSite(Widelands::TrainingSite& mo) : LuaProductionSite(mo) {
	}
	explicit LuaTrainingSite(lua_State* L) : LuaProductionSite(L) {
	}
	~LuaTrainingSite() override = default;

	/*
	 * Properties
	 */
	int get_max_soldiers(lua_State*);
	int get_capacity(lua_State*);
	int set_capacity(lua_State*);
	int get_build_heroes(lua_State*);
	int set_build_heroes(lua_State*);

	/*
	 * Lua Methods
	 */
	int set_soldiers(lua_State*);
	int get_soldiers(lua_State*);

	/*
	 * C Methods
	 */
	CASTED_GET(TrainingSite)
};

}  // namespace LuaMaps

#endif
