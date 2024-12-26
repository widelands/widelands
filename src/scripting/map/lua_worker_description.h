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

#ifndef WL_SCRIPTING_MAP_LUA_WORKER_DESCRIPTION_H
#define WL_SCRIPTING_MAP_LUA_WORKER_DESCRIPTION_H

#include "logic/map_objects/tribes/worker_descr.h"
#include "scripting/map/lua_map_object_description.h"

namespace LuaMaps {

class LuaWorkerDescription : public LuaMapObjectDescription {
public:
	LUNA_CLASS_HEAD(LuaWorkerDescription);

	~LuaWorkerDescription() override = default;

	LuaWorkerDescription() = default;
	explicit LuaWorkerDescription(const Widelands::WorkerDescr* const workerdescr)
	   : LuaMapObjectDescription(workerdescr) {
	}
	explicit LuaWorkerDescription(lua_State* L) : LuaMapObjectDescription(L) {
	}

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * Properties
	 */
	int get_becomes(lua_State*);
	int get_promoted(lua_State*);
	int get_promoted_from(lua_State*);
	int get_buildcost(lua_State*);
	int get_employers(lua_State*);
	int get_buildable(lua_State*);
	int get_needed_experience(lua_State*);

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */

private:
	CASTED_GET_DESCRIPTION(WorkerDescr)
};

}  // namespace LuaMaps

#endif
