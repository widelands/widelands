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

#ifndef WL_LOGIC_CMD_LUASCRIPT_H
#define WL_LOGIC_CMD_LUASCRIPT_H

#include <string>

#include "logic/cmd_queue.h"

namespace Widelands {

struct CmdLuaScript : public GameLogicCommand {
	CmdLuaScript() : GameLogicCommand(0) {} // For savegame loading
	CmdLuaScript
		(uint32_t const _duetime, const std::string& script) :
		GameLogicCommand(_duetime), script_(script) {}

	// Write these commands to a file (for savegames)
	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_LUASCRIPT;}

	void execute(Game &) override;

private:
	std::string script_;
};

}

#endif  // end of include guard: WL_LOGIC_CMD_LUASCRIPT_H
