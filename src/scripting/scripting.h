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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef SCRIPTING_H
#define SCRIPTING_H

#include <map>
#include <string>

#include <stdint.h>

#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"
#include "scripting/eris/lua.hpp"
#include "scripting/lua_errors.h"

namespace Widelands {
	class Editor_Game_Base;
	class Game;
	class Map_Map_Object_Loader;
	struct Map_Map_Object_Saver;
	class Player;
}

class EditorFactory;
class GameFactory;
class LuaCoroutine;
class LuaTable;

// Provides an interface to call and execute Lua Code.
class LuaInterface {
public:
	LuaInterface();
	virtual ~LuaInterface();

	// Interpret the given string, will throw 'LuaError' on any error.
	void interpret_string(const std::string&);

	std::unique_ptr<LuaTable> run_script(const std::string& script);
	std::unique_ptr<LuaTable> get_hook(const std::string& name);

protected:
	lua_State * m_L;
};

class LuaEditorInterface : public LuaInterface {
public:
	LuaEditorInterface(Widelands::Editor_Game_Base * g);
	virtual ~LuaEditorInterface();

private:
	std::unique_ptr<EditorFactory> m_factory;
};

class LuaGameInterface : public LuaInterface {
public:
	LuaGameInterface(Widelands::Game * g);
	virtual ~LuaGameInterface();

	// Input/output for coroutines.
	LuaCoroutine * read_coroutine
		(Widelands::FileRead &, Widelands::Map_Map_Object_Loader &, uint32_t);
	uint32_t write_coroutine
		(Widelands::FileWrite &, Widelands::Map_Map_Object_Saver &, LuaCoroutine *);

	// Input output for the global game state.
	void read_global_env
		(Widelands::FileRead &, Widelands::Map_Map_Object_Loader &, uint32_t);
	uint32_t write_global_env
		(Widelands::FileWrite &, Widelands::Map_Map_Object_Saver &);

private:
	std::unique_ptr<GameFactory> m_factory;
};

#endif
