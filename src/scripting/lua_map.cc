/*
 * Copyright (C) 2006-2009 by the Widelands Development Team
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


extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}

#include "log.h"
#include "logic/game.h"
#include "logic/widelands_geometry.h"
#include "logic/immovable.h"
#include "logic/game.h"
#include "c_utils.h"

#include "lua_map.h"

// LUAMODULE wl.map

using namespace Widelands;

/*
 * Intern definitions of Lua Functions
 */
/*
 * Create a World immovable object immediately
 *
 * name: name ob object to create
 * posx: int, x position
 * posy: int, y position
 *
 */
static int L_create_immovable(lua_State * l) {
	const char * objname = luaL_checkstring(l, 1);
	uint32_t x = luaL_checkint(l, 2);
	uint32_t y = luaL_checkint(l, 3);

	Game * game = get_game(l);
	Coords pos(x, y);

	// Check if the map is still free here
	// TODO: this exact code is duplicated in worker.cc
	if (BaseImmovable const * const imm = game->map()[pos].get_immovable())
		if (imm->get_size() >= BaseImmovable::SMALL)
			return report_error(l, "Field is no longer free!");

	int32_t imm_idx = game->map().world().get_immovable_index(objname);
	if (imm_idx < 0)
		return report_error(l, "Unknown immovable <%s>", objname);

	game->create_immovable (pos, imm_idx, 0);

	return 0;
}

const static struct luaL_reg wlmap [] = {
	{"create_immovable", &L_create_immovable},
	{NULL, NULL}
};


void luaopen_wlmap(lua_State * l) {
	luaL_register(l, "wl.map", wlmap);
}


