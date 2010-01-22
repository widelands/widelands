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

#ifndef CMD_LUAFUNCTION_H
#define CMD_LUAFUNCTION_H

#include <string>

#include "cmd_queue.h"
#include "scripting/scripting.h"

namespace Widelands {

struct Cmd_LuaFunction : public GameLogicCommand {
	Cmd_LuaFunction() : GameLogicCommand(0) {} // For savegame loading
	Cmd_LuaFunction(int32_t const _duetime, LuaCoroutine* cr) : 
		GameLogicCommand(_duetime), m_cr(cr) {}

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &);
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &);

	virtual uint8_t id() const {return QUEUE_CMD_LUAFUNCTION;}

	virtual void execute(Game &);

private:
	LuaCoroutine* m_cr;
};

}

#endif /* end of include guard: CMD_LUAFUNCTION_H */

