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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "game_player_info_data_packet.h"

#include "computer_player.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "wui/interactive_player.h"
#include "logic/player.h"
#include "logic/tribe.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 8


void Game_Player_Info_Data_Packet::Read
	(FileSystem & fs, Game & game, Map_Map_Object_Loader *)
{
	try {
		FileRead fr;
		fr.Open(fs, "binary/player_info");
		uint16_t const packet_version = fr.Unsigned16();
		if (1 <= packet_version and packet_version <= CURRENT_PACKET_VERSION) {
			uint32_t const max_players = fr.Unsigned16();
			for (uint32_t i = 1; i < max_players + 1; ++i) {
				game.remove_player(i);
				if (fr.Unsigned8()) {
					bool    const see_all = fr.Unsigned8();
					if (packet_version <= 2)
						// Used to be the player type, not needed anymore
						fr.Signed32();

					int32_t const plnum   =
						packet_version < 7 ? fr.Signed32() : fr.Player_Number8();
					if (plnum < 1 or MAX_PLAYERS < plnum)
						throw game_data_error
							(_("player number (%i) is out of range (1 .. %u)"),
							 plnum, MAX_PLAYERS);
					char const * const tribe_name = fr.CString();
					char const * const frontier_style_name =
						packet_version < 7 ? 0 : fr.CString();
					char const * const flag_style_name     =
						packet_version < 7 ? 0 : fr.CString();

					RGBColor rgb[4];

					for (uint32_t j = 0; j < 4; ++j) {
						uint8_t const r = fr.Unsigned8();
						uint8_t const g = fr.Unsigned8();
						uint8_t const b = fr.Unsigned8();
						rgb[j] = RGBColor(r, g, b);
					}

					std::string const name = fr.CString();

					game.add_player(plnum, 0, tribe_name, name);
					Player & player = game.player(plnum);
					{
						Tribe_Descr const & tribe = player.tribe();
						try {
							player.m_frontier_style_index =
								frontier_style_name ?
								tribe.frontier_style_index(frontier_style_name) : 0;
						} catch (Tribe_Descr::Nonexistent) {
							log
								("WARNING: player %u has frontier style index \"%s\", "
								 "which does not exist in his tribe %s; will use "
								 "default frontier style \"%s\" instead\n",
								 plnum, frontier_style_name, tribe.name().c_str(),
								 tribe.frontier_style_name(0).c_str());
						}
						try {
							player.m_flag_style_index =
								flag_style_name ?
								tribe.flag_style_index(flag_style_name) : 0;
						} catch (Tribe_Descr::Nonexistent) {
							log
								("WARNING: player %u has flag style index \"%s\", "
								 "which does not exist in his tribe %s; will use "
								 "default flag style \"%s\" instead\n",
								 plnum, flag_style_name, tribe.name().c_str(),
								 tribe.flag_style_name(0).c_str());
						}
					}
					player.set_see_all(see_all);

					if (packet_version >= 6)
						player.setAI(fr.CString());

					for (uint32_t j = 0; j < 4; ++j)
						player.m_playercolor[j] = rgb[j];

					if (packet_version >= 2) {
						player.ReadStatistics(fr, 0);
						if (packet_version >= 4) {
							player.m_casualties = fr.Unsigned32();
							player.m_kills      = fr.Unsigned32();
							if (packet_version >= 5) {
								player.m_msites_lost         = fr.Unsigned32();
								player.m_msites_defeated     = fr.Unsigned32();
								player.m_civil_blds_lost     = fr.Unsigned32();
								player.m_civil_blds_defeated = fr.Unsigned32();
							}
						}
					}

					if (packet_version >= 8) {
						player.allow_retreat_change(fr.Unsigned8());
						player.set_retreat_percentage(fr.Unsigned8());
					}
				}
			}

			if (packet_version >= 2)
				game.ReadStatistics
					(fr,
					 packet_version >= 5 ? 3 :
					 packet_version == 4 ? 2 :
					 1);
		} else
			throw game_data_error
				(_("unknown/unhandled version %u"), packet_version);
	} catch (_wexception const & e) {
		throw game_data_error(_("player info: %s"), e.what());
	}
}


void Game_Player_Info_Data_Packet::Write
	(FileSystem & fs, Game & game, Map_Map_Object_Saver *)
{
	FileWrite fw;

	// Now packet version
	fw.Unsigned16(CURRENT_PACKET_VERSION);

	// Number of (potential) players
	Player_Number const nr_players = game.map().get_nrplayers();
	fw.Unsigned16(nr_players);
	iterate_players_existing_const(p, nr_players, game, plr) {
		fw.Unsigned8(1); // Player is in game.

		fw.Unsigned8(plr->m_see_all);

		fw.Player_Number8(plr->m_plnum);

		{
			Tribe_Descr const & tribe = plr->tribe();
			fw.CString(tribe.name().c_str());
			fw.CString(tribe.frontier_style_name(plr->m_frontier_style_index));
			fw.CString(tribe.flag_style_name    (plr->m_flag_style_index));
		}

		for (uint32_t j = 0; j < 4; ++j) {
			fw.Unsigned8(plr->m_playercolor[j].r());
			fw.Unsigned8(plr->m_playercolor[j].g());
			fw.Unsigned8(plr->m_playercolor[j].b());
		}

		// Seen fields is in a map packet
		// Allowed buildings is in a map packet

		// Economies are in a packet after map loading

		fw.CString(plr->m_name.c_str());
		fw.CString(plr->m_ai.c_str());

		plr->WriteStatistics(fw);
		fw.Unsigned32(plr->casualties());
		fw.Unsigned32(plr->kills     ());
		fw.Unsigned32(plr->msites_lost        ());
		fw.Unsigned32(plr->msites_defeated    ());
		fw.Unsigned32(plr->civil_blds_lost    ());
		fw.Unsigned32(plr->civil_blds_defeated());
		fw.Unsigned8(plr->is_retreat_change_allowed());
		fw.Unsigned8(plr->get_retreat_percentage   ());
	} else
		fw.Unsigned8(0); //  Player is NOT in game.

	game.WriteStatistics(fw);

	fw.Write(fs, "binary/player_info");
}

}
