/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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
	ScopedTimer timer("GameSaver::save() took %ums");

	// We might not have a loader UI during emergency saves, so we don't assert that we have one.
	// We also don't want it for game objectives.
	auto set_progress_message = [this](std::string text, int step) {
		game_.step_loader_ui(
		   step < 0 ? text :
		              (boost::format(_("Saving game: %1$s (%2$d/%3$d)")) % text % step % 5).str());
	};
	set_progress_message(_("Autosaving game…"), -1);

	fs_.ensure_directory_exists("binary");

	log("Game: Writing Preload Data ... ");
	set_progress_message(_("Elemental data"), 1);
	{
		GamePreloadPacket p;
		p.write(fs_, game_, nullptr);
	}
	log("took %ums\n", timer.ms_since_last_query());

	log("Game: Writing Game Class Data ... ");
	{
		GameClassPacket p;
		p.write(fs_, game_, nullptr);
	}
	log("took %ums\n", timer.ms_since_last_query());

	log("Game: Writing Player Info ... ");
	{
		GamePlayerInfoPacket p;
		p.write(fs_, game_, nullptr);
	}
	log("took %ums\n", timer.ms_since_last_query());

	log("Game: Writing Map Data!\n");
	GameMapPacket M;
	M.write(fs_, game_, nullptr);
	log("Game: Writing Map Data took %ums\n", timer.ms_since_last_query());

	MapObjectSaver* const mos = M.get_map_object_saver();

	log("Game: Writing Player Economies Info ... ");
	set_progress_message(_("Economies"), 2);
	{
		GamePlayerEconomiesPacket p;
		p.write(fs_, game_, mos);
	}
	log("took %ums\n", timer.ms_since_last_query());

	log("Game: Writing ai persistent data ... ");
	set_progress_message(_("AI"), 3);
	{
		GamePlayerAiPersistentPacket p;
		p.write(fs_, game_, mos);
	}
	log("took %ums\n", timer.ms_since_last_query());

	log("Game: Writing Command Queue Data ... ");
	set_progress_message(_("Command queue"), 4);
	{
		GameCmdQueuePacket p;
		p.write(fs_, game_, mos);
	}
	log("took %ums\n", timer.ms_since_last_query());

	log("Game: Writing Interactive Player Data ... ");
	set_progress_message(_("Interactive player"), 5);
	{
		GameInteractivePlayerPacket p;
		p.write(fs_, game_, mos);
	}
	log("took %ums\n", timer.ms_since_last_query());
}
}  // namespace Widelands
