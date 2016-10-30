/*
 * Copyright (C) 2006-2016 by the Widelands Development Team
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

#ifndef WL_SCRIPTING_LOGIC_H
#define WL_SCRIPTING_LOGIC_H

#include <memory>

#include "logic/editor_game_base.h"
#include "scripting/lua_coroutine.h"
#include "scripting/lua_interface.h"

class EditorFactory;
class GameFactory;

class LuaEditorInterface : public LuaInterface {
public:
	LuaEditorInterface(Widelands::EditorGameBase* g);
	virtual ~LuaEditorInterface();

	std::unique_ptr<LuaTable> run_script(const std::string& script) override;

private:
	std::unique_ptr<EditorFactory> factory_;
};

class LuaGameInterface : public LuaInterface {
public:
	LuaGameInterface(Widelands::Game* g);
	virtual ~LuaGameInterface();

	// Returns a given hook if one is defined, otherwise returns 0
	std::unique_ptr<LuaTable> get_hook(const std::string& name);

	std::unique_ptr<LuaTable> run_script(const std::string& script) override;

	// Input/output for coroutines.
	std::unique_ptr<LuaCoroutine> read_coroutine(FileRead&);
	void write_coroutine(FileWrite&, LuaCoroutine*);

	// Input output for the global game state.
	void read_global_env(FileRead&, Widelands::MapObjectLoader&, uint32_t);
	uint32_t write_global_env(FileWrite&, Widelands::MapObjectSaver&);

private:
	std::unique_ptr<GameFactory> factory_;
};

#endif  // end of include guard: WL_SCRIPTING_LOGIC_H
