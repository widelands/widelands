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

#include "game_game_class_data_packet.h"

#include "fileread.h"
#include "filewrite.h"
#include "game.h"

#define CURRENT_PACKET_VERSION 2


Game_Game_Class_Data_Packet::~Game_Game_Class_Data_Packet() {}


void Game_Game_Class_Data_Packet::Read
(FileSystem & fs, Game* game, Widelands_Map_Map_Object_Loader * const)
throw (_wexception)
{
	FileRead fr;

	fr.Open(fs, "binary/game_class");

	// read packet version
	const Uint16 packet_version = fr.Unsigned16();

	if (packet_version <= CURRENT_PACKET_VERSION) {
		// Can't load netgames
		game->m_netgame=0;

		game->m_speed=fr.Signed16();

		game->m_gametime=fr.Unsigned32();
	} else
		throw wexception
			("Unknown version in Game_Game_Class_Data_Packet: %u\n", packet_version);
}

/*
 * Write Function
 */
void Game_Game_Class_Data_Packet::Write
(FileSystem & fs, Game* game, Widelands_Map_Map_Object_Saver * const)
throw (_wexception)
{
	FileWrite fw;

	// Packet version
	fw.Unsigned16(CURRENT_PACKET_VERSION);

	// State is running, we do not need to save this
	// Save speed
	fw.Signed16(game->m_speed);

	// From the interactive player, is saved somewhere else
	// Computer players are saved somewhere else

	// CMD Queue is saved later
	// We do not care for real time.

	// EDITOR GAME CLASS
	// Write gametime
	fw.Unsigned32(game->m_gametime);

	// We do not care for players, since they were set
	// on game initialization to match Map::scenario_player_[names|tribes]
	// or vice versa, so this is handled by map loader

	// Objects are loaded and saved by map

	// Tribes and wares are handled by map
	// Interactive_Base doesn't need saving

	// Map is handled by map saving

	// Track pointers are not saved in save games

	fw.Write(fs, "binary/game_class");
}
