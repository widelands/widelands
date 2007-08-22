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
#include "tribe.h"


#define CURRENT_PACKET_VERSION 1

// Forward declaration. Defined in interactive_player.cc
int Int_Player_overlay_callback_function(const TCoords<FCoords>, void *, int);

/*
 * Destructor
 */
Game_Interactive_Player_Data_Packet::~Game_Interactive_Player_Data_Packet(void) {
}

/*
 * Read Function
 */
void Game_Interactive_Player_Data_Packet::Read
(FileSystem & fs, Game* game, Widelands_Map_Map_Object_Loader * const)
throw (_wexception)
{
	FileRead fr;
	fr.Open( fs, "binary/interactive_player" );

	// read packet version
	const Uint16 packet_version = fr.Unsigned16();

	// Resize the IPLs statistic stuff
	game->get_ipl()->m_current_statistics.resize(0);
	game->get_ipl()->m_ware_productions.resize(0);
	game->get_ipl()->m_last_stats_update = 0;
	game->get_ipl()->m_general_stats.resize(0);

	if (packet_version == CURRENT_PACKET_VERSION) {
		Interactive_Player* plr = game->get_ipl();

		plr->m_player_number = fr.Unsigned8();

		// Main Menu is not closed
		delete plr->m_fieldaction.window;
		plr->m_fieldaction.window = 0;

		// Map Position
		int x = fr.Unsigned16();
		int y = fr.Unsigned16();
		plr->set_viewpoint(Point(x, y));

		plr->m_display_flags = fr.Unsigned32();

		delete plr->m_minimap.window;
		plr->m_minimap.window = 0;

		// Now only restore the callback functions. assumes, map is already loaded
		game->get_map()->get_overlay_manager()->show_buildhelp(false);
		game->get_map()->get_overlay_manager()->register_overlay_callback_function(&Int_Player_overlay_callback_function, static_cast<void*>(plr));

		game->get_map()->recalc_whole_map();


		// Load statistics stuff
		plr->m_last_stats_update = fr.Unsigned32();
		ushort nr_wares = fr.Unsigned16();
		ushort nr_entries = fr.Unsigned16();

		assert //  FIXME NEVER USE assert TO VALIDATE INPUT!!!
			(nr_wares
			 ==
			 game->player(game->get_ipl()->get_player_number())
			 .tribe().get_nrwares());

		plr->m_current_statistics.resize( nr_wares );
		plr->m_ware_productions.resize( nr_wares );

		for( uint i = 0; i < plr->m_current_statistics.size(); i++) {
			plr->m_current_statistics[i] = fr.Unsigned32();
			plr->m_ware_productions[i].resize( nr_entries );

			for( uint j = 0; j < plr->m_ware_productions[i].size(); j++)
			plr->m_ware_productions[i][j] = fr.Unsigned32();
		}

		// Read general statistics
		uint entries = fr.Unsigned16();
		plr->m_general_stats.resize( game->get_map()->get_nrplayers() );

		for( uint i =0; i < game->get_map()->get_nrplayers(); i++)
			if( game->get_player(i+1)) {
				plr->m_general_stats[i].land_size.resize(entries);
				plr->m_general_stats[i].nr_workers.resize(entries);
				plr->m_general_stats[i].nr_buildings.resize(entries);
				plr->m_general_stats[i].nr_wares.resize(entries);
				plr->m_general_stats[i].productivity.resize(entries);
				plr->m_general_stats[i].nr_kills.resize(entries);
				plr->m_general_stats[i].miltary_strength.resize(entries);
			}

		for( uint i =0; i < game->get_map()->get_nrplayers(); i++) {
			if( !game->get_player(i+1)) continue;

			for( uint j = 0; j < plr->m_general_stats[i].land_size.size(); j++) {
				game->get_ipl()->m_general_stats[i].land_size[j] = fr.Unsigned32();
				game->get_ipl()->m_general_stats[i].nr_workers[j] = fr.Unsigned32();
				game->get_ipl()->m_general_stats[i].nr_buildings[j] = fr.Unsigned32();
				game->get_ipl()->m_general_stats[i].nr_wares[j] = fr.Unsigned32();
				game->get_ipl()->m_general_stats[i].productivity[j] = fr.Unsigned32();
				game->get_ipl()->m_general_stats[i].nr_kills[j] = fr.Unsigned32();
				game->get_ipl()->m_general_stats[i].miltary_strength[j] = fr.Unsigned32();
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
	fw.Unsigned32(plr->m_display_flags);

	// Statistic stuff
	fw.Unsigned32(plr->m_last_stats_update);
	fw.Unsigned16(plr->m_current_statistics.size());
	fw.Unsigned16( plr->m_ware_productions[0].size() );
	for( uint i = 0; i < plr->m_current_statistics.size(); i++) {
		fw.Unsigned32(plr->m_current_statistics[i]);
		for( uint j = 0; j < plr->m_ware_productions[i].size(); j++)
			fw.Unsigned32(plr->m_ware_productions[i][j]);
	}

	// General statistics
	for( uint i =0; i < game->get_map()->get_nrplayers(); i++)
		if( game->get_player(i+1)) {
			fw.Unsigned16(plr->m_general_stats[i].land_size.size());
			break;
		}

	for( uint i =0; i < game->get_map()->get_nrplayers(); i++) {
		if( !game->get_player(i+1)) continue;

		for( uint j = 0; j < plr->m_general_stats[i].land_size.size(); j++) {
			fw.Unsigned32(game->get_ipl()->m_general_stats[i].land_size[j]);
			fw.Unsigned32(game->get_ipl()->m_general_stats[i].nr_workers[j]);
			fw.Unsigned32(game->get_ipl()->m_general_stats[i].nr_buildings[j]);
			fw.Unsigned32(game->get_ipl()->m_general_stats[i].nr_wares[j]);
			fw.Unsigned32(game->get_ipl()->m_general_stats[i].productivity[j]);
			fw.Unsigned32(game->get_ipl()->m_general_stats[i].nr_kills[j]);
			fw.Unsigned32(game->get_ipl()->m_general_stats[i].miltary_strength[j]);
		}
	}

	fw.Write( fs, "binary/interactive_player" );
}
