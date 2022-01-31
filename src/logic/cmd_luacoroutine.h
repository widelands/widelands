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

#ifndef WL_LOGIC_CMD_LUACOROUTINE_H
#define WL_LOGIC_CMD_LUACOROUTINE_H

#include <memory>

#include "logic/cmd_queue.h"
#include "scripting/lua_coroutine.h"

namespace Widelands {

struct CmdLuaCoroutine : public GameLogicCommand {
	CmdLuaCoroutine() : GameLogicCommand(Time(0)) {
	}  // For savegame loading
	CmdLuaCoroutine(const Time& init_duetime, std::unique_ptr<LuaCoroutine> cr)
	   : GameLogicCommand(init_duetime), cr_(std::move(cr)) {
	}

	~CmdLuaCoroutine() override = default;

	// Write these commands to a file (for savegames)
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kLuaCoroutine;
	}

	void execute(Game&) override;

private:
	std::unique_ptr<LuaCoroutine> cr_;
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_CMD_LUACOROUTINE_H
