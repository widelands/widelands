/*
 * Copyright (C) 2002-2009 by the Widelands Development Team
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

#include "cmd_lua.h"

#include "game.h"
#include "game_data_error.h"
#include "scripting/scripting.h"

namespace Widelands {

void Cmd_Lua::execute (Game & game) {
	try {
		game.lua().run_script(m_ns, m_script);
	} catch (LuaScriptNotExistingError & e) {
		if (!m_optional) // this is only critical if this is not optional
			throw wexception("%s", e.what());
	} catch (LuaError & e) {
		throw wexception("%s", e.what());
	}
}

#define CMD_LUA_VERSION 1
void Cmd_Lua::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CMD_LUA_VERSION) {
			GameLogicCommand::Read(fr, egbase, mol);
			m_ns = fr.String();
			m_script = fr.String();
			m_optional = fr.Unsigned8();
		} else
			throw game_data_error
				(_("unknown/unhandled version %u"), packet_version);
	} catch (_wexception const & e) {
		throw game_data_error(_("lua: %s"), e.what());
	}
}
void Cmd_Lua::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	fw.Unsigned16(CMD_LUA_VERSION);
	GameLogicCommand::Write(fw, egbase, mos);

	fw.String(m_ns);
	fw.String(m_script);
	fw.Unsigned8(m_optional);
}

}
