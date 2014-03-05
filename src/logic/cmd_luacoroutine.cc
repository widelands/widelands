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

#include "logic/cmd_luacoroutine.h"

#include "gamecontroller.h"
#include "log.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "scripting/lua_coroutine.h"
#include "scripting/scripting.h"
#include "upcast.h"

namespace Widelands {

void Cmd_LuaCoroutine::execute (Game & game) {
	try {
		uint32_t sleeptime;
		int rv = m_cr->resume(&sleeptime);
		if (rv == LuaCoroutine::YIELDED) {
			game.enqueue_command(new Widelands::Cmd_LuaCoroutine(sleeptime, m_cr));
			m_cr = nullptr;  // Remove our ownership so we don't delete.
		} else if (rv == LuaCoroutine::DONE) {
			delete m_cr;
			m_cr = nullptr;
		}
	} catch (LuaError & e) {
		log("Error in Lua Coroutine\n");
		log("%s\n", e.what());
		log("Send message to all players and pause game");
		for (int i = 1; i <= game.map().get_nrplayers(); i++) {
			Widelands::Message & msg =
				*new Widelands::Message
				("Game Logic", game.get_gametime(),
				 Forever(), "Lua Coroutine Failed", e.what());
			game.player(i).add_message(game, msg, true);
		}
		game.gameController()->setDesiredSpeed(0);
	}
}

#define CMD_LUACOROUTINE_VERSION 2
void Cmd_LuaCoroutine::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CMD_LUACOROUTINE_VERSION) {
			GameLogicCommand::Read(fr, egbase, mol);

			// This function is only called when saving/loading savegames. So save
			// to cast here
			upcast(LuaGameInterface, lgi, &egbase.lua());
			assert(lgi); // If this is not true, this is not a game.

			m_cr = lgi->read_coroutine(fr, mol, fr.Unsigned32());
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("lua function: %s", e.what());
	}
}
void Cmd_LuaCoroutine::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	fw.Unsigned16(CMD_LUACOROUTINE_VERSION);
	GameLogicCommand::Write(fw, egbase, mos);

	FileWrite::Pos p = fw.GetPos();
	fw.Unsigned32(0); // N bytes written, follows below

	// This function is only called when saving/loading savegames. So save
	// to cast here
	upcast(LuaGameInterface, lgi, &egbase.lua());
	assert(lgi); // If this is not true, this is not a game.

	uint32_t nwritten = Little32(lgi->write_coroutine(fw, mos, m_cr));
	fw.Data(&nwritten, 4, p);
}

}
