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

#include "game_interactive_player_data_packet.h"

#include "fileread.h"
#include "filewrite.h"
#include "game.h"
#include "interactive_player.h"
#include "mapview.h"
#include "overlay_manager.h"
#include "player.h"
#include <stdint.h>
#include "tribe.h"


#define CURRENT_PACKET_VERSION 2


/*
 * Destructor
 */
Game_Interactive_Player_Data_Packet::~Game_Interactive_Player_Data_Packet() {
}

/*
 * Read Function
 */
void Game_Interactive_Player_Data_Packet::Read
(FileSystem & fs, Game* game, Widelands_Map_Map_Object_Loader * const)
throw (_wexception)
{
	FileRead fr;
	fr.Open(fs, "binary/interactive_player");

	// read packet version
	const Uint16 packet_version = fr.Unsigned16();

	if (packet_version == CURRENT_PACKET_VERSION || packet_version == 1) {
		uint8_t player_number = fr.Unsigned8();
		int x = fr.Unsigned16();
		int y = fr.Unsigned16();
		uint32_t display_flags = fr.Unsigned32();

		if (packet_version == 1)
			game->m_last_stats_update = fr.Unsigned32();

		if (Interactive_Player* plr = game->get_ipl()) {
			plr->set_player_number(player_number);

			// Map Position
			plr->set_viewpoint(Point(x, y));

			plr->set_display_flags(display_flags);

			if (packet_version == 1) {

				plr->get_player()->ReadStatistics(fr, 0);
				game->ReadStatistics(fr, 0);
			}
		}

		// DONE
		return;
	} else
		throw wexception("Unknown version in Game_Interactive_Player_Data_Packet: %i\n", packet_version);

	assert(0); // never here
}

/*
 * Write Function
 */
void Game_Interactive_Player_Data_Packet::Write
(FileSystem & fs, Game* game, Widelands_Map_Map_Object_Saver * const)
throw (_wexception)
{
	FileWrite fw;

	// Now packet version
	fw.Unsigned16(CURRENT_PACKET_VERSION);

	Interactive_Player* plr = game->get_ipl();

	// Player number
	fw.Unsigned8(plr->get_player_number());

	// Map Position
	assert(0 <= plr->get_viewpoint().x);
	assert(0 <= plr->get_viewpoint().y);
	fw.Unsigned16(plr->get_viewpoint().x);
	fw.Unsigned16(plr->get_viewpoint().y);

	// Display flags
	fw.Unsigned32(plr->get_display_flags());

	fw.Write(fs, "binary/interactive_player");
}
