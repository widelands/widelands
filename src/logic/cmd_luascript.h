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

#ifndef WL_LOGIC_CMD_LUASCRIPT_H
#define WL_LOGIC_CMD_LUASCRIPT_H

#include <string>

#include "logic/cmd_queue.h"

namespace Widelands {

struct CmdLuaScript : public GameLogicCommand {
	CmdLuaScript() : GameLogicCommand(Time(0)) {
	}  // For savegame loading
	CmdLuaScript(const Time& init_duetime, const std::string& script)
	   : GameLogicCommand(init_duetime), script_(script) {
	}

	// Write these commands to a file (for savegames)
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kLuaScript;
	}

	void execute(Game&) override;

private:
	std::string script_;
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_CMD_LUASCRIPT_H
