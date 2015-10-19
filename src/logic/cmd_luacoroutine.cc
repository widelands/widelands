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

#include "base/log.h"
#include "base/macros.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "scripting/logic.h"
#include "scripting/lua_coroutine.h"

namespace Widelands {

void CmdLuaCoroutine::execute (Game & game) {
	try {
		int rv = m_cr->resume();
		const uint32_t sleeptime = m_cr->pop_uint32();
		if (rv == LuaCoroutine::YIELDED) {
			game.enqueue_command(new Widelands::CmdLuaCoroutine(sleeptime, m_cr));
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
				(Message::Type::kGameLogic,
				 game.get_gametime(),
				 "pics/menu_help.png",
				 "Coroutine",
				 "Lua Coroutine Failed",
				 e.what());
			game.player(i).add_message(game, msg, true);
		}
		game.game_controller()->set_desired_speed(0);
	}
}

constexpr uint16_t kCurrentPacketVersion = 3;

void CmdLuaCoroutine::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			GameLogicCommand::read(fr, egbase, mol);

			// This function is only called when saving/loading savegames. So save
			// to cast here
			upcast(LuaGameInterface, lgi, &egbase.lua());
			assert(lgi); // If this is not true, this is not a game.

			m_cr = lgi->read_coroutine(fr);
		} else {
			throw UnhandledVersionError(packet_version, kCurrentPacketVersion);
		}
	} catch (const WException & e) {
		throw GameDataError("lua function: %s", e.what());
	}
}
void CmdLuaCoroutine::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	fw.unsigned_16(kCurrentPacketVersion);
	GameLogicCommand::write(fw, egbase, mos);

	// This function is only called when saving/loading savegames. So save to
	// cast here
	upcast(LuaGameInterface, lgi, &egbase.lua());
	assert(lgi); // If this is not true, this is not a game.

	lgi->write_coroutine(fw, m_cr);
}

}
