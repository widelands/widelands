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

#include "game_io/game_cmd_queue_data_packet.h"
#include "game_io/game_game_class_data_packet.h"
#include "game_io/game_interactive_player_data_packet.h"
#include "game_io/game_map_data_packet.h"
#include "game_io/game_player_economies_data_packet.h"
#include "game_io/game_player_info_data_packet.h"
#include "game_io/game_preload_data_packet.h"
#include "io/filesystem/filesystem.h"
#include "log.h"
#include "logic/game.h"
#include "scoped_timer.h"

namespace Widelands {

Game_Saver::Game_Saver(FileSystem & fs, Game & game) : m_fs(fs), m_game(game) {
}


/*
 * The core save function
 */
void Game_Saver::save() {
	ScopedTimer timer("Game_Saver::save() took %ums");

	m_fs.EnsureDirectoryExists("binary");

	log("Game: Writing Preload Data ... ");
	{Game_Preload_Data_Packet                    p; p.Write(m_fs, m_game, nullptr);}
	log("took %ums\n", timer.ms_since_last_query());

	log("Game: Writing Game Class Data ... ");
	{Game_Game_Class_Data_Packet                 p; p.Write(m_fs, m_game, nullptr);}
	log("took %ums\n", timer.ms_since_last_query());

	log("Game: Writing Player Info ... ");
	{Game_Player_Info_Data_Packet                p; p.Write(m_fs, m_game, nullptr);}
	log("took %ums\n", timer.ms_since_last_query());

	log("Game: Writing Map Data!\n");
	Game_Map_Data_Packet                         M; M.Write(m_fs, m_game, nullptr);
	log("Game: Writing Map Data took %ums\n", timer.ms_since_last_query());

	Map_Map_Object_Saver * const mos = M.get_map_object_saver();

	log("Game: Writing Player Economies Info ... ");
	{Game_Player_Economies_Data_Packet           p; p.Write(m_fs, m_game, mos);}
	log("took %ums\n", timer.ms_since_last_query());

	log("Game: Writing Command Queue Data ... ");
	{Game_Cmd_Queue_Data_Packet                  p; p.Write(m_fs, m_game, mos);}
	log("took %ums\n", timer.ms_since_last_query());

	log("Game: Writing Interactive Player Data ... ");
	{Game_Interactive_Player_Data_Packet         p; p.Write(m_fs, m_game, mos);}
	log("took %ums\n", timer.ms_since_last_query());
}

}
