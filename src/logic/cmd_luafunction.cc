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

#include "cmd_luafunction.h"

#include "game.h"
#include "game_data_error.h"
#include "scripting/scripting.h"

// TODO: SirVer, Lua: this should not be here
#include "scripting/coroutine_impl.h"

namespace Widelands {

void Cmd_LuaFunction::execute (Game & game) {
	try {
		uint32_t sleeptime;
		int rv = m_cr->resume(&sleeptime);
		if (rv == LuaCoroutine::YIELDED) {
			game.enqueue_command(new Widelands::Cmd_LuaFunction(sleeptime, m_cr));
		} else if (rv == LuaCoroutine::DONE) {
			delete m_cr;
		}
	} catch (LuaError & e) {
		throw wexception("%s", e.what());
	}
}

#define CMD_LUAFUNCTION_VERSION 1
void Cmd_LuaFunction::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CMD_LUAFUNCTION_VERSION) {
			GameLogicCommand::Read(fr, egbase, mol);

			m_cr = new LuaCoroutine_Impl(egbase.lua().get_lua_state(), 0);
			m_cr->unfreeze(egbase.lua().get_lua_state(), fr, fr.Unsigned32());
		} else
			throw game_data_error
				(_("unknown/unhandled version %u"), packet_version);
	} catch (_wexception const & e) {
		throw game_data_error(_("lua function: %s"), e.what());
	}
}
void Cmd_LuaFunction::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	fw.Unsigned16(CMD_LUAFUNCTION_VERSION);
	GameLogicCommand::Write(fw, egbase, mos);

	FileWrite::Pos p = fw.GetPos();
	fw.Unsigned32(0); // N bytes written, follows below

	uint32_t nwritten = Little32(m_cr->freeze(fw));
	fw.Data(&nwritten, 4, p);
}

}
