/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#include "game_io/game_interactive_player_data_packet.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "logic/tribe.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"
#include "wui/interactive_player.h"
#include "wui/mapview.h"
#include "wui/overlay_manager.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 2


void Game_Interactive_Player_Data_Packet::Read
	(FileSystem & fs, Game & game, Map_Map_Object_Loader *)
{
	try {
		FileRead fr;
		fr.Open(fs, "binary/interactive_player");
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CURRENT_PACKET_VERSION) {
			Player_Number player_number =
				fr.Player_Number8(game.map().get_nrplayers());
			if (not game.get_player(player_number)) {
				// This happens if the player, that saved the game, was a spectator
				// and the slot for player 1 was not used in the game.
				// So now we try to create an InteractivePlayer object for another
				// player instead.
				const Player_Number max = game.map().get_nrplayers();
				for (player_number = 1; player_number <= max; ++player_number)
					if (game.get_player(player_number))
						break;
				if (player_number > max)
					throw game_data_error("The game has no players!");
			}
			int32_t       const x             = fr.Unsigned16();
			int32_t       const y             = fr.Unsigned16();
			uint32_t      const display_flags = fr.Unsigned32();

			if (Interactive_Base * const ibase = game.get_ibase()) {
				ibase->set_viewpoint(Point(x, y), true);

				uint32_t const loaded_df =
					Interactive_Base::dfShowCensus |
					Interactive_Base::dfShowStatistics;
				uint32_t const olddf = ibase->get_display_flags();
				uint32_t const realdf =
					(olddf & ~loaded_df) | (display_flags & loaded_df);
				ibase->set_display_flags(realdf);
			}
			if (Interactive_Player * const ipl = game.get_ipl()) {
				ipl->set_player_number(player_number);
			}
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("interactive player: %s", e.what());
	}
}

/*
 * Write Function
 */
void Game_Interactive_Player_Data_Packet::Write
	(FileSystem & fs, Game & game, Map_Map_Object_Saver * const)
{
	FileWrite fw;

	// Now packet version
	fw.Unsigned16(CURRENT_PACKET_VERSION);

	Interactive_Base * const ibase = game.get_ibase();
	Interactive_Player * const iplayer = game.get_ipl();

	// Player number
	fw.Unsigned8(iplayer ? iplayer->player_number() : 1);

	// Map Position
	if (ibase) {
		assert(0 <= ibase->get_viewpoint().x);
		assert(0 <= ibase->get_viewpoint().y);
	}
	fw.Unsigned16(ibase ? ibase->get_viewpoint().x : 0);
	fw.Unsigned16(ibase ? ibase->get_viewpoint().y : 0);

	// Display flags
	fw.Unsigned32(ibase ? ibase->get_display_flags() : 0);

	fw.Write(fs, "binary/interactive_player");
}

}
