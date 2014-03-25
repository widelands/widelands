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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "logic/cmd_luascript.h"

#include "log.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "scripting/lua_table.h"
#include "scripting/scripting.h"

namespace Widelands {

void Cmd_LuaScript::execute (Game & game) {
	log("Trying to run: %s: ", script_.c_str());
	try {
		game.lua().run_script(script_);
	} catch (LuaScriptNotExistingError &) {
		// The script has not been found.
		log("not found.\n");
		return;
	} catch (LuaError & e) {
		throw game_data_error("lua: %s", e.what());
	}
	log("done\n");
	return;
}

#define CMD_LUASCRIPT_VERSION 1
void Cmd_LuaScript::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CMD_LUASCRIPT_VERSION) {
			GameLogicCommand::Read(fr, egbase, mol);
			script_ = fr.String();
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("lua: %s", e.what());
	}
}
void Cmd_LuaScript::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	fw.Unsigned16(CMD_LUASCRIPT_VERSION);
	GameLogicCommand::Write(fw, egbase, mos);

	fw.String(script_);
}

}
