/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#include <boost/bind.hpp>
#include <boost/signals2.hpp>

#include "game_io/game_cmd_queue_data_packet.h"
#include "game_io/game_game_class_data_packet.h"
#include "game_io/game_interactive_player_data_packet.h"
#include "game_io/game_map_data_packet.h"
#include "game_io/game_player_economies_data_packet.h"
#include "game_io/game_player_info_data_packet.h"
#include "game_io/game_preload_data_packet.h"
#include "io/filesystem/layered_filesystem.h"
#include "log.h"
#include "logic/cmd_expire_message.h"
#include "logic/game.h"
#include "logic/player.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "scoped_timer.h"

namespace Widelands {

Game_Loader::Game_Loader(const std::string & path, Game & game) :
	m_fs(*g_fs->MakeSubFileSystem(path)), m_game(game)
{}


Game_Loader::~Game_Loader() {
	delete &m_fs;
}

/*
 * This function preloads a game
 */
int32_t Game_Loader::preload_game(Game_Preload_Data_Packet & mp) {
	// Load elemental data block
	mp.Read(m_fs, m_game, nullptr);

	return 0;
}

/*
 * Load the complete file
 */
int32_t Game_Loader::load_game(bool const multiplayer) {
	ScopedTimer timer("Game_Loader::load() took %ums");

	log("Game: Reading Preload Data ... ");
	{Game_Preload_Data_Packet                     p; p.Read(m_fs, m_game);}
	log("took %ums\n", timer.ms_since_last_query());

	log("Game: Reading Game Class Data ... ");
	{Game_Game_Class_Data_Packet                  p; p.Read(m_fs, m_game);}
	log("took %ums\n", timer.ms_since_last_query());

	log("Game: Reading Map Data ... ");
	Game_Map_Data_Packet M;                          M.Read(m_fs, m_game);
	log("Game: Reading Map Data took %ums\n", timer.ms_since_last_query());

	log("Game: Reading Player Info ... ");
	{Game_Player_Info_Data_Packet                 p; p.Read(m_fs, m_game);}
	log("Game: Reading Player Info took %ums\n", timer.ms_since_last_query());

	log("Game: Calling Read_Complete()\n");
	M.Read_Complete(m_game);
	log("Game: Read_Complete took: %ums\n", timer.ms_since_last_query());

	Map_Map_Object_Loader * const mol = M.get_map_object_loader();

	log("Game: Reading Player Economies Info ... ");
	{Game_Player_Economies_Data_Packet            p; p.Read(m_fs, m_game, mol);}
	log("took %ums\n", timer.ms_since_last_query());

	log("Game: Reading Command Queue Data ... ");
	{Game_Cmd_Queue_Data_Packet                   p; p.Read(m_fs, m_game, mol);}
	log("took %ums\n", timer.ms_since_last_query());

	//  This must be after the command queue has been read.
	log("Game: Parsing messages ... ");
	Player_Number const nr_players = m_game.map().get_nrplayers();
	iterate_players_existing_const(p, nr_players, m_game, player) {
		const MessageQueue & messages = player->messages();
		container_iterate_const(MessageQueue, messages, i) {
			Message* m = i.current->second;
			Message_Id m_id = i.current->first;

			// Renew expire commands
			Duration const duration = m->duration();
			if (duration != Forever()) {
				m_game.cmdqueue().enqueue
					(new Cmd_ExpireMessage
					 	(m->sent() + duration, p, m_id));
			}
			// Renew Map_Object connections
			if (m->serial() > 0) {
				Map_Object* mo = m_game.objects().get_object(m->serial());
				mo->removed.connect
					(boost::bind(&Player::message_object_removed, player, m_id));
			}
		}
	}
	log("took %ums\n", timer.ms_since_last_query());

	// For compatibility hacks only
	mol->load_finish_game(m_game);

	// Only read and use interactive player data, if we load a singleplayer game.
	// In multiplayer games every client needs to create a new interactive
	// player.
	if (!multiplayer) {
		log("Game: Reading Interactive Player Data ... ");
		{Game_Interactive_Player_Data_Packet       p; p.Read(m_fs, m_game, mol);}
		log("took %ums\n", timer.ms_since_last_query());
	}

	return 0;
}

}
