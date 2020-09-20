/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "game_io/game_loader.h"

#include <memory>

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
#include "io/filesystem/layered_filesystem.h"
#include "logic/game.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "ui_basic/progresswindow.h"

namespace Widelands {

GameLoader::GameLoader(const std::string& path, Game& game)
   : fs_(*g_fs->make_sub_file_system(path)), game_(game) {
}

GameLoader::~GameLoader() {
	delete &fs_;
}

/*
 * This function preloads a game
 */
int32_t GameLoader::preload_game(GamePreloadPacket& mp) {
	// Load elemental data block
	mp.read(fs_, game_, nullptr);

	return 0;
}

/*
 * Load the complete file
 */
int32_t GameLoader::load_game(bool const multiplayer) {
	ScopedTimer timer("GameLoader::load() took %ums");

	assert(game_.has_loader_ui());
	auto set_progress_message = [](const std::string& text, unsigned step) {
		Notifications::publish(UI::NoteLoadingMessage(
		   (boost::format(_("Loading game: %1$s (%2$u/%3$d)")) % text % step % 6).str()));
	};
	set_progress_message(_("Elemental data"), 1);
	log_info("Game: Reading Preload Data ... ");
	{
		GamePreloadPacket p;
		p.read(fs_, game_);
	}

	log_info("Game: Reading Game Class Data ... ");
	{
		GameClassPacket p;
		p.read(fs_, game_);
	}

	log_info("Game: Reading Map Data ... ");
	GameMapPacket map_packet;
	map_packet.read(fs_, game_);

	// This has to be loaded after the map packet so that the map's filesystem will exist.
	// The custom tribe scripts are saved when the map scripting packet is saved, but we need
	// to load them as early as possible here.
	FileSystem* map_fs = game_.map().filesystem();
	if (map_fs->file_exists("scripting/tribes")) {
		log_info("Game: Reading Scenario Tribes ... ");
		game_.mutable_tribes()->register_scenario_tribes(map_fs);
	}

	// This also triggers loading the world and tribes, so we need a newline at the end of the log
	// output
	log_info("Game: Reading Player Info ...\n");
	{
		GamePlayerInfoPacket p;
		p.read(fs_, game_);
	}

	log_info("Game: Calling read_complete()\n");
	map_packet.read_complete(game_);
	log_info("Game: read_complete took: %ums\n", timer.ms_since_last_query());

	MapObjectLoader* const mol = map_packet.get_map_object_loader();

	log_info("Game: Reading Player Economies Info ... ");
	set_progress_message(_("Economies"), 2);
	{
		GamePlayerEconomiesPacket p;
		p.read(fs_, game_, mol);
	}

	log_info("Game: Reading ai persistent data ... ");
	set_progress_message(_("AI"), 3);
	{
		GamePlayerAiPersistentPacket p;
		p.read(fs_, game_, mol);
	}

	log_info("Game: Reading Command Queue Data ... ");
	set_progress_message(_("Command queue"), 4);
	{
		GameCmdQueuePacket p;
		p.read(fs_, game_, mol);
	}

	//  This must be after the command queue has been read.
	log_info("Game: Parsing messages ... ");
	set_progress_message(_("Messages"), 5);
	PlayerNumber const nr_players = game_.map().get_nrplayers();
	iterate_players_existing_const(p, nr_players, game_, player) {
		const MessageQueue& messages = player->messages();
		for (const auto& temp_message : messages) {
			const std::unique_ptr<Message>& message = temp_message.second;
			const MessageId message_id = temp_message.first;

			// Renew MapObject connections
			if (message->serial() > 0) {
				MapObject* mo = game_.objects().get_object(message->serial());
				mo->removed.connect(
				   [player, message_id](unsigned) { player->message_object_removed(message_id); });
			}
		}
	}

	set_progress_message(_("Finishing"), 6);
	// For compatibility hacks only
	mol->load_finish_game(game_);

	// Only read and use interactive player data, if we load a singleplayer game.
	// In multiplayer games every client needs to create a new interactive
	// player.
	if (!multiplayer) {
		log_info("Game: Reading Interactive Player Data ... ");
		{
			GameInteractivePlayerPacket p;
			p.read(fs_, game_, mol);
		}
	}

	return 0;
}
}  // namespace Widelands
