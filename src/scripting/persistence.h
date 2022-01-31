/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

#ifndef WL_SCRIPTING_PERSISTENCE_H
#define WL_SCRIPTING_PERSISTENCE_H

#include "scripting/lua.h"

class FileRead;
class FileWrite;

namespace Widelands {
class MapObjectLoader;
struct MapObjectSaver;
}  // namespace Widelands

/**
 * This persists the lua object at the stack position
 * 2 after populating the (empty) table at position 1
 * with the items given in globals.
 */
uint32_t persist_object(lua_State* L, FileWrite&, Widelands::MapObjectSaver&);

// Does all the unpersisting work. The unpersisted object is at the top of the
// stack after the function returns.
void unpersist_object(lua_State* L, FileRead& fr, Widelands::MapObjectLoader& mol, uint32_t size);

#endif  // end of include guard: WL_SCRIPTING_PERSISTENCE_H
