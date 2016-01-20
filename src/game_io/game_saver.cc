/*
 * Copyright (C) 2002-2004, 2007-2009 by the Widelands Development Team
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

namespace Widelands {

GameSaver::GameSaver(FileSystem & fs, Game & game) : m_fs(fs), m_game(game) {
}


/*
 * The core save function
 */
void GameSaver::save() {
	ScopedTimer timer("GameSaver::save() took %ums");

	m_fs.ensure_directory_exists("binary");

	log("Game: Writing Preload Data ... ");
	{GamePreloadPacket                    p; p.write(m_fs, m_game, nullptr);}
	log("took %ums\n", timer.ms_since_last_query());

	log("Game: Writing Game Class Data ... ");
	{GameClassPacket                 p; p.write(m_fs, m_game, nullptr);}
	log("took %ums\n", timer.ms_since_last_query());

	log("Game: Writing Player Info ... ");
	{GamePlayerInfoPacket                p; p.write(m_fs, m_game, nullptr);}
	log("took %ums\n", timer.ms_since_last_query());

	log("Game: Writing Map Data!\n");
	GameMapPacket                         M; M.write(m_fs, m_game, nullptr);
	log("Game: Writing Map Data took %ums\n", timer.ms_since_last_query());

	MapObjectSaver * const mos = M.get_map_object_saver();

	log("Game: Writing Player Economies Info ... ");
	{GamePlayerEconomiesPacket           p; p.write(m_fs, m_game, mos);}
	log("took %ums\n", timer.ms_since_last_query());

	log("Game: Writing ai persistent data ... ");
	{GamePlayerAiPersistentPacket           p; p.write(m_fs, m_game, mos);}
	log("took %ums\n", timer.ms_since_last_query());

	log("Game: Writing Command Queue Data ... ");
	{GameCmdQueuePacket                  p; p.write(m_fs, m_game, mos);}
	log("took %ums\n", timer.ms_since_last_query());

	log("Game: Writing Interactive Player Data ... ");
	{GameInteractivePlayerPacket         p; p.write(m_fs, m_game, mos);}
	log("took %ums\n", timer.ms_since_last_query());
}

}
