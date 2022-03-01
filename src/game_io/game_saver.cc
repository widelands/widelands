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

#include "game_io/game_saver.h"

#include "base/log.h"
#include "base/scoped_timer.h"
#include "game_io/game_class_packet.h"
#include "game_io/game_cmd_queue_packet.h"
#include "game_io/game_interactive_player_packet.h"
#include "game_io/game_map_packet.h"
#include "game_io/game_player_ai_persistent_packet.h"
#include "game_io/game_player_economies_packet.h"
#include "game_io/game_player_info_packet.h"
#include "game_io/game_preload_packet.h"
#include "io/filesystem/filesystem.h"
#include "logic/game.h"
#include "ui_basic/progresswindow.h"

namespace Widelands {

GameSaver::GameSaver(FileSystem& fs, Game& game) : fs_(fs), game_(game) {
}

/*
 * The core save function
 */
void GameSaver::save() {
	ScopedTimer timer("GameSaver::save() took %ums", true);

	// We might not have a loader UI during emergency saves, so we don't assert that we have one.
	// We also don't want it for game objectives.
	auto set_progress_message = [](std::string text, int step) {
		Notifications::publish(UI::NoteLoadingMessage(
		   step < 0 ? text : format(_("Saving game: %1$s (%2$d/%3$d)"), text, step, 5)));
	};
	set_progress_message(_("Autosaving game…"), -1);

	fs_.ensure_directory_exists("binary");

	verb_log_info_time(game_.get_gametime(), "Game: Writing Preload Data ... ");
	set_progress_message(_("Elemental data"), 1);
	{
		GamePreloadPacket p;
		p.write(fs_, game_, nullptr);
	}

	verb_log_info_time(game_.get_gametime(), "Game: Writing Game Class Data ... ");
	{
		GameClassPacket p;
		p.write(fs_, game_);
	}

	verb_log_info_time(game_.get_gametime(), "Game: Writing Player Info ... ");
	{
		GamePlayerInfoPacket p;
		p.write(fs_, game_, nullptr);
	}

	verb_log_info_time(game_.get_gametime(), "Game: Writing Map Data!\n");
	GameMapPacket map_packet;
	map_packet.write(fs_, game_, nullptr);

	MapObjectSaver* const mos = map_packet.get_map_object_saver();

	verb_log_info_time(game_.get_gametime(), "Game: Writing Player Economies Info ... ");
	set_progress_message(_("Economies"), 2);
	{
		GamePlayerEconomiesPacket p;
		p.write(fs_, game_, mos);
	}

	verb_log_info_time(game_.get_gametime(), "Game: Writing ai persistent data ... ");
	set_progress_message(_("AI"), 3);
	{
		GamePlayerAiPersistentPacket p;
		p.write(fs_, game_, mos);
	}

	verb_log_info_time(game_.get_gametime(), "Game: Writing Command Queue Data ... ");
	set_progress_message(_("Command queue"), 4);
	{
		GameCmdQueuePacket p;
		p.write(fs_, game_, mos);
	}

	verb_log_info_time(game_.get_gametime(), "Game: Writing Interactive Player Data ... ");
	set_progress_message(_("Interactive player"), 5);
	{
		GameInteractivePlayerPacket p;
		p.write(fs_, game_, mos);
	}
}
}  // namespace Widelands
