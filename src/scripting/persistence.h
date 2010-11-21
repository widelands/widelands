/*
 * Copyright (C) 2006-2010 by the Widelands Development Team
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <string>

#include "logic/widelands_filewrite.h"
#include "logic/widelands_fileread.h"

namespace Widelands {
	struct Map_Map_Object_Loader;
	struct Map_Map_Object_Saver;
	struct Editor_Game_Base;
	struct Game;
}

#include <lua.hpp>

/**
 * This persists the lua object at the stack position
 * 2 after populating the (empty) table at position 1
 * with the items given in globals.
 */
uint32_t persist_object
	(lua_State * L,
	 Widelands::FileWrite &, Widelands::Map_Map_Object_Saver &);
uint32_t unpersist_object
	(lua_State * L,
	 Widelands::FileRead & fr, Widelands::Map_Map_Object_Loader & mol,
	 uint32_t size);

#endif /* end of include guard: PERSISTENCE_H */

