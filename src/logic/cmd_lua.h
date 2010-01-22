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

#ifndef CMD_LUA_H
#define CMD_LUA_H

#include <string>

#include "cmd_queue.h"

namespace Widelands {

struct Cmd_Lua : public GameLogicCommand {
	Cmd_Lua() : GameLogicCommand(0) {} // For savegame loading
	Cmd_Lua
		(int32_t const _duetime, std::string ns, std::string script,
		 bool optional) :
		GameLogicCommand(_duetime), m_ns(ns),
		m_script(script), m_optional(optional)
	{}

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &);
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &);

	virtual uint8_t id() const {return QUEUE_CMD_LUA;}

	virtual void execute(Game &);

private:
	std::string m_ns;
	std::string m_script;
	bool m_optional;
};

}

#endif /* end of include guard: CMD_LUA_H */

