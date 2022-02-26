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
#include "logic/addons.h"
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
	ScopedTimer timer("GameLoader::load() took %ums", true);

	assert(game_.has_loader_ui());
	auto set_progress_message = [](const std::string& text, unsigned step) {
		Notifications::publish(
		   UI::NoteLoadingMessage(format(_("Loading game: %1$s (%2$u/%3$d)"), text, step, 6)));
	};
	set_progress_message(_("Elemental data"), 1);
	verb_log_info("Game: Reading Preload Data ... ");
	GamePreloadPacket preload;
	preload.read(fs_, game_);

	// Now that the preload data was read, we apply the add-ons.
	// Note: Only world- and tribes-type add-ons are saved in savegames because those are the
	// only ones where it makes a difference whether they are enabled during loading or not.
	{
		game_.enabled_addons().clear();
		for (const auto& requirement : preload.required_addons()) {
			bool found = false;
			for (auto& pair : AddOns::g_addons) {
				if (pair.first->internal_name == requirement.first) {
					found = true;
					if (pair.first->version != requirement.second) {
						log_warn(
						   "Savegame requires add-on '%s' at version %s but version %s is installed. "
						   "They might be compatible, but this is not necessarily the case.\n",
						   requirement.first.c_str(),
						   AddOns::version_to_string(requirement.second).c_str(),
						   AddOns::version_to_string(pair.first->version).c_str());
					}
					assert(pair.first->category == AddOns::AddOnCategory::kWorld ||
					       pair.first->category == AddOns::AddOnCategory::kTribes ||
					       pair.first->category == AddOns::AddOnCategory::kScript);
					game_.enabled_addons().push_back(pair.first);
					break;
				}
			}
			if (!found) {
				throw GameDataError("Add-on '%s' (version %s) required but not installed",
				                    requirement.first.c_str(),
				                    AddOns::version_to_string(requirement.second).c_str());
			}
		}
	}

	// This creates the map filesystem, giving us access to scenario-specific registries.
	verb_log_info("Game: Reading Map Data ... ");
	GameMapPacket map_packet;
	map_packet.read(fs_, game_);

	// This will load the descriptions in the correct order.
	// By now, all units must have been registered.
	// Do not request a description to be loaded before this call.
	verb_log_info("Game: Reading Game Class Data ... ");
	{
		GameClassPacket p;
		p.read(fs_, game_);
	}

	verb_log_info("Game: Reading Player Info ...\n");
	{
		GamePlayerInfoPacket p;
		p.read(fs_, game_);
	}

	verb_log_info("Game: Calling read_complete()\n");
	map_packet.read_complete(game_);

	MapObjectLoader* const mol = map_packet.get_map_object_loader();

	verb_log_info("Game: Reading Player Economies Info ... ");
	set_progress_message(_("Economies"), 2);
	{
		GamePlayerEconomiesPacket p;
		p.read(fs_, game_, mol);
	}

	verb_log_info("Game: Reading ai persistent data ... ");
	set_progress_message(_("AI"), 3);
	{
		GamePlayerAiPersistentPacket p;
		p.read(fs_, game_, mol);
	}

	verb_log_info("Game: Reading Command Queue Data ... ");
	set_progress_message(_("Command queue"), 4);
	{
		GameCmdQueuePacket p;
		p.read(fs_, game_, mol);
	}

	//  This must be after the command queue has been read.
	verb_log_info("Game: Parsing messages ... ");
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
				mo->removed.connect([player, message_id](unsigned /* serial */) {
					player->message_object_removed(message_id);
				});
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
		verb_log_info("Game: Reading Interactive Player Data ... ");
		GameInteractivePlayerPacket p;
		p.read(fs_, game_, mol);
	}

	return 0;
}
}  // namespace Widelands
