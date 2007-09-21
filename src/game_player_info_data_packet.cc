/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "game_player_info_data_packet.h"

#include "computer_player.h"
#include "fileread.h"
#include "filewrite.h"
#include "game.h"
#include "interactive_player.h"
#include "player.h"
#include <stdint.h>
#include "tribe.h"


#define CURRENT_PACKET_VERSION 2


Game_Player_Info_Data_Packet::~Game_Player_Info_Data_Packet() {
}


void Game_Player_Info_Data_Packet::Read
(FileSystem & fs, Game* game, Widelands_Map_Map_Object_Loader * const)
throw (_wexception)
{
	FileRead fr;
	fr.Open(fs, "binary/player_info");

	const uint16_t packet_version = fr.Unsigned16();
	if (1 <= packet_version and packet_version <= CURRENT_PACKET_VERSION) {
		uint32_t max_players = fr.Unsigned16();
		for (uint32_t i=1; i<=max_players; i++) {
			game->remove_player(i);
			if (fr.Unsigned8()) {
				bool see_all = fr.Unsigned8();
				int type = fr.Signed32();
				int plnum = fr.Signed32();
				std::string tribe = fr.CString();

				RGBColor rgb[4];

				for (uint32_t j=0; j<4; j++) {
					uint8_t r = fr.Unsigned8();
					uint8_t g = fr.Unsigned8();
					uint8_t b = fr.Unsigned8();
					rgb[j] = RGBColor(r, g, b);
				}

				std::string name = fr.CString();

				game->add_player(plnum, type, tribe, name);
				Player* plr = game->get_player(plnum);
				plr->set_see_all(see_all);

				for (uint32_t j=0; j<4; j++)
					plr->m_playercolor[j] = rgb[j];

				if (packet_version >= 2)
					plr->ReadStatistics(fr, 0);
			}
		}

		if (packet_version >= 2)
			game->ReadStatistics(fr, 1);
	} else
		throw wexception
			("Unknown version in Game_Player_Info_Data_Packet: %u",
			 packet_version);
}


void Game_Player_Info_Data_Packet::Write
(FileSystem & fs, Game* game, Widelands_Map_Map_Object_Saver * const)
throw (_wexception)
{
	FileWrite fw;

	// Now packet version
	fw.Unsigned16(CURRENT_PACKET_VERSION);

	// Number of (potential) players
	const Player_Number nr_players = game->map().get_nrplayers();
	fw.Unsigned16(nr_players);
	iterate_players_existing_const(p, nr_players, *game, plr) {
		// Player is in game
		fw.Unsigned8(1);

		fw.Unsigned8(plr->m_see_all);


		fw.Signed32(plr->m_type);
		fw.Signed32(plr->m_plnum);

		fw.CString(plr->m_tribe.name().c_str());

		for (uint32_t j=0; j<4; j++) {
			fw.Unsigned8(plr->m_playercolor[j].r());
			fw.Unsigned8(plr->m_playercolor[j].g());
			fw.Unsigned8(plr->m_playercolor[j].b());
		}

		// Seen fields is in a map packet
		// Allowed buildings is in a map packet

		// Economies are in a packet after map loading

		fw.CString(plr->m_name.c_str());

		plr->WriteStatistics(fw);
	} else fw.Unsigned8(0); //  Player is NOT in game.

	game->WriteStatistics(fw);

	fw.Write(fs, "binary/player_info");
}
