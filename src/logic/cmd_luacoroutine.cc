/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "logic/cmd_luacoroutine.h"

#include <memory>

#include "base/log.h"
#include "base/macros.h"
#include "graphic/text_layout.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "scripting/logic.h"
#include "scripting/lua_coroutine.h"

namespace Widelands {

void CmdLuaCoroutine::execute(Game& game) {
	try {
		int rv = cr_->resume();
		const uint32_t sleeptime = cr_->pop_uint32();
		if (rv == LuaCoroutine::YIELDED) {
			game.enqueue_command(new Widelands::CmdLuaCoroutine(Time(sleeptime), std::move(cr_)));
		} else if (rv == LuaCoroutine::DONE) {
			cr_.reset();
		}
	} catch (LuaError& e) {
		log_err_time(game.get_gametime(), "Error in Lua Coroutine\n");
		log_err_time(game.get_gametime(), "%s\n", e.what());

		if (g_fail_on_lua_error) {
			log_err_time(game.get_gametime(), "Terminating Widelands.");
			abort();
		}

		log_err_time(game.get_gametime(), "Send message to all players and pause game\n");
		const std::string error_message = richtext_escape(e.what());
		for (int i = 1; i <= game.map().get_nrplayers(); i++) {
			// Send message only to open player slots
			Player* recipient = game.get_player(i);
			if (recipient != nullptr) {
				std::unique_ptr<Message> msg(new Widelands::Message(
				   Message::Type::kGameLogic, game.get_gametime(), "Coroutine",
				   "images/ui_basic/menu_help.png", "Lua Coroutine Failed", error_message));

				recipient->add_message(game, std::move(msg), true);
			}
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
}  // namespace Widelands
