/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#include "game.h"
#include "interactive_player.h"
#include "mapview.h"
#include "overlay_manager.h"
#include "player.h"
#include "tribe.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 2


void Game_Interactive_Player_Data_Packet::Read
(FileSystem & fs, Game * game, Map_Map_Object_Loader * const)
throw (_wexception)
{
	FileRead fr;
	try {
		fr.Open(fs, "binary/interactive_player");
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CURRENT_PACKET_VERSION || packet_version == 1) {
			Player_Number const player_number =
				fr.Player_Number8(game->map().get_nrplayers());
			int32_t       const x             = fr.Unsigned16();
			int32_t       const y             = fr.Unsigned16();
			uint32_t      const display_flags = fr.Unsigned32();

			if (packet_version == 1)
				game->m_last_stats_update = fr.Unsigned32();

			if (Interactive_Player* plr = game->get_ipl()) {
				plr->set_player_number(player_number);

				plr->set_viewpoint(Point(x, y));

				const uint32_t loaded_df = ~(Interactive_Base::dfDebug);
				uint32_t olddf = plr->get_display_flags();
				uint32_t realdf = (olddf & ~loaded_df) | (display_flags & loaded_df);
				plr->set_display_flags(realdf);

				if (packet_version == 1) {
					plr->get_player()->ReadStatistics(fr, 0);
					game->ReadStatistics(fr, 0);
				}
			}
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("interactive player: %s", e.what());
	}
}

/*
 * Write Function
 */
void Game_Interactive_Player_Data_Packet::Write
(FileSystem & fs, Game * game, Map_Map_Object_Saver * const)
throw (_wexception)
{
	FileWrite fw;

	// Now packet version
	fw.Unsigned16(CURRENT_PACKET_VERSION);

	Interactive_Player* plr = game->get_ipl();

	// Player number
	fw.Unsigned8(plr ? plr->get_player_number() : 1);

	// Map Position
	if (plr) {
		assert(0 <= plr->get_viewpoint().x);
		assert(0 <= plr->get_viewpoint().y);
	}
	fw.Unsigned16(plr ? plr->get_viewpoint().x : 0);
	fw.Unsigned16(plr ? plr->get_viewpoint().y : 0);

	// Display flags
	fw.Unsigned32(plr ? plr->get_display_flags() : 0);

	fw.Write(fs, "binary/interactive_player");
}

};
