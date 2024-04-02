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

#ifndef WL_SCRIPTING_MAP_LUA_TRAINING_SITE_DESCRIPTION_H
#define WL_SCRIPTING_MAP_LUA_TRAINING_SITE_DESCRIPTION_H

#include "logic/map_objects/tribes/trainingsite.h"
#include "scripting/map/lua_production_site_description.h"

namespace LuaMaps {

class LuaTrainingSiteDescription : public LuaProductionSiteDescription {
public:
	LUNA_CLASS_HEAD(LuaTrainingSiteDescription);

	~LuaTrainingSiteDescription() override = default;

	LuaTrainingSiteDescription() = default;
	explicit LuaTrainingSiteDescription(const Widelands::TrainingSiteDescr* const trainingsitedescr)
	   : LuaProductionSiteDescription(trainingsitedescr) {
	}
	explicit LuaTrainingSiteDescription(lua_State* L) : LuaProductionSiteDescription(L) {
	}

	/*
	 * Properties
	 */
	int get_max_attack(lua_State*);
	int get_max_defense(lua_State*);
	int get_max_evade(lua_State*);
	int get_max_health(lua_State*);
	int get_max_number_of_soldiers(lua_State*);
	int get_min_attack(lua_State*);
	int get_min_defense(lua_State*);
	int get_min_evade(lua_State*);
	int get_min_health(lua_State*);

	/*
	 * Lua methods
	 */

	int trained_soldiers(lua_State*);

	/*
	 * C methods
	 */

private:
	CASTED_GET_DESCRIPTION(TrainingSiteDescr)
};

}  // namespace LuaMaps

#endif
