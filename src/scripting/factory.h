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

#ifndef FACTORY_H
#define FACTORY_H

#include <lua.hpp>

#include "logic/widelands.h"

/*
 * Class to create the correct type for types shared between Editor and Game.
 */
class Factory {
public:
	virtual void push_player(lua_State * L, Widelands::Player_Number) = 0;
	virtual ~Factory() {}

};

class GameFactory : public Factory {
public:
	virtual ~GameFactory() {}

	virtual void push_player(lua_State * L, Widelands::Player_Number plr);
};

class EditorFactory  : public Factory{
public:
	virtual ~EditorFactory() {}

	virtual void push_player(lua_State * L, Widelands::Player_Number plr);
};

#endif /* end of include guard: FACTORY_H */

