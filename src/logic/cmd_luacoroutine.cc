/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include <boost/format.hpp>

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

CmdLuaCoroutine::~CmdLuaCoroutine() {
}

void CmdLuaCoroutine::execute(Game& game) {
	try {
		int rv = cr_->resume();
		const uint32_t sleeptime = cr_->pop_uint32();
		if (rv == LuaCoroutine::YIELDED) {
			game.enqueue_command(new Widelands::CmdLuaCoroutine(sleeptime, std::move(cr_)));
		} else if (rv == LuaCoroutine::DONE) {
			cr_.reset();
		}
	} catch (LuaError& e) {
		log("Error in Lua Coroutine\n");
		log("%s\n", e.what());
		log("Send message to all players and pause game\n");
		for (int i = 1; i <= game.map().get_nrplayers(); i++) {
			Widelands::Message& msg = *new Widelands::Message(
			   Message::Type::kGameLogic, game.get_gametime(), "Coroutine",
			   "images/ui_basic/menu_help.png", "Lua Coroutine Failed",
			   (boost::format("<rt><p font-size=12>%s</p></rt>") % e.what()).str());
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
			assert(lgi);  // If this is not true, this is not a game.

			cr_ = lgi->read_coroutine(fr);
		} else {
			throw UnhandledVersionError("CmdLuaCoroutine", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("lua function: %s", e.what());
	}
}
void CmdLuaCoroutine::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersion);
	GameLogicCommand::write(fw, egbase, mos);

	// This function is only called when saving/loading savegames. So save to
	// cast here
	upcast(LuaGameInterface, lgi, &egbase.lua());
	assert(lgi);  // If this is not true, this is not a game.

	lgi->write_coroutine(fw, *cr_);
}
}
