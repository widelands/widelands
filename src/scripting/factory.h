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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_SCRIPTING_FACTORY_H
#define WL_SCRIPTING_FACTORY_H

#include "logic/widelands.h"
#include "scripting/lua.h"

/*
 * Class to create the correct type for types shared between Editor and Game.
 */
class Factory {
public:
	virtual void push_player(lua_State* L, Widelands::PlayerNumber) = 0;
	virtual ~Factory() {
	}
};

class GameFactory : public Factory {
public:
	~GameFactory() override {
	}

	void push_player(lua_State* L, Widelands::PlayerNumber plr) override;
};

class EditorFactory : public Factory {
public:
	~EditorFactory() override {
	}

	void push_player(lua_State* L, Widelands::PlayerNumber plr) override;
};

Factory& get_factory(lua_State* const L);

#endif  // end of include guard: WL_SCRIPTING_FACTORY_H
