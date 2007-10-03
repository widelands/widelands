/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#include "game_preload_data_packet.h"

#include "game.h"
#include "map.h"
#include "profile.h"

#define CURRENT_PACKET_VERSION 1


Game_Preload_Data_Packet::~Game_Preload_Data_Packet() {}


void Game_Preload_Data_Packet::Read
(FileSystem & fs, Game *, Widelands_Map_Map_Object_Loader * const)
throw(_wexception)
{

   Profile prof;
   prof.read("preload", 0, fs);
   Section* s = prof.get_section("global");

	const int32_t packet_version = s->get_int("packet_version");
	if (packet_version == CURRENT_PACKET_VERSION) {
      m_gametime = s->get_safe_int("gametime");
      m_mapname = s->get_safe_string("mapname");
	} else
		throw wexception
			("Unknown version in Game_Preload_Data_Packet: %i", packet_version);
}

/**
 * Write Function
 */
void Game_Preload_Data_Packet::Write
(FileSystem & fs, Game* game, Widelands_Map_Map_Object_Saver * const)
throw (_wexception)
{

   Profile prof;
   Section* s = prof.create_section("global");

   // packet version
   s->set_int("packet_version", CURRENT_PACKET_VERSION);

   // save some kind of header.
   s->set_int("gametime", game->get_gametime()); // Time in milliseconds of elapsed game time (without pauses)
   s->set_string("mapname", game->get_map()->get_name());  // Name of map

   prof.write("preload", false, fs);
}
