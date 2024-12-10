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

#ifndef WL_SCRIPTING_MAP_LUA_PINNED_NOTE_H
#define WL_SCRIPTING_MAP_LUA_PINNED_NOTE_H

#include "logic/map_objects/pinned_note.h"
#include "scripting/map/lua_bob.h"

namespace LuaMaps {

class LuaPinnedNote : public LuaBob {
public:
	LUNA_CLASS_HEAD(LuaPinnedNote);

	LuaPinnedNote() = default;
	explicit LuaPinnedNote(Widelands::PinnedNote& n) : LuaBob(n) {
	}
	explicit LuaPinnedNote(lua_State* L) : LuaBob(L) {
	}
	~LuaPinnedNote() override = default;

	/*
	 * Properties
	 */
	int get_owner(lua_State* L);
	int get_text(lua_State*);
	int set_text(lua_State*);
	int get_color(lua_State*);
	int set_color(lua_State*);

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */
	CASTED_GET(PinnedNote)
};

}  // namespace LuaMaps

#endif
