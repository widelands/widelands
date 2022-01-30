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
	explicit LuaEditorInterface(Widelands::EditorGameBase* g);
	~LuaEditorInterface() override = default;

	std::unique_ptr<LuaTable> run_script(const std::string& script) override;

private:
	std::unique_ptr<EditorFactory> factory_;
};

class LuaGameInterface : public LuaInterface {
public:
	explicit LuaGameInterface(Widelands::Game* g);
	~LuaGameInterface() override = default;

	// Returns a given hook if one is defined, otherwise returns 0
	std::unique_ptr<LuaTable> get_hook(const std::string& name);

	std::unique_ptr<LuaTable> run_script(const std::string& script) override;

	// Input/output for coroutines.
	std::unique_ptr<LuaCoroutine> read_coroutine(FileRead&);
	void write_coroutine(FileWrite&, const LuaCoroutine&);

	// Input output for the global game state.
	void read_global_env(FileRead&, Widelands::MapObjectLoader&, uint32_t);
	uint32_t write_global_env(FileWrite&, Widelands::MapObjectSaver&);
	void read_textdomain_stack(FileRead&);
	void write_textdomain_stack(FileWrite&);

private:
	std::unique_ptr<GameFactory> factory_;
};

#endif  // end of include guard: WL_SCRIPTING_LOGIC_H
