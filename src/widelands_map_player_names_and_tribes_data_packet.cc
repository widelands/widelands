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

#include "widelands_map_player_names_and_tribes_data_packet.h"

#include "editor_game_base.h"
#include "error.h"
#include "filesystem.h"
#include "map.h"
#include "profile.h"
#include "widelands_map_data_packet_ids.h"
#include "world.h"

#define CURRENT_PACKET_VERSION 1

/*
 * Destructor
 */
Widelands_Map_Player_Names_And_Tribes_Data_Packet::~Widelands_Map_Player_Names_And_Tribes_Data_Packet() {
}

/*
 * Read Function
 *
 * this is a scenario packet, it might be that we have to skip it
 */
void Widelands_Map_Player_Names_And_Tribes_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Widelands_Map_Map_Object_Loader * const)
throw (_wexception)
{
   Pre_Read(fs, egbase->get_map(), skip);
}

/*
 * Pre Read function
 */
void Widelands_Map_Player_Names_And_Tribes_Data_Packet::Pre_Read
(FileSystem & fs, Map* map, const bool skip)
{

   Profile prof;
   prof.read("player_names", 0, fs);
   Section* s = prof.get_section("global");

   const int packet_version = s->get_int("packet_version");
	if (packet_version == CURRENT_PACKET_VERSION) {
     std::string name, tribe;
     char buf[256];
		const Player_Number nr_players = map->get_nrplayers();
		for (Player_Number i = 1; i <= nr_players; ++i) {
			sprintf(buf, "player_%u", i);
         s = prof.get_section(buf);
         name = s->get_string("name");
         tribe = s->get_string("tribe");
			if (not skip) {
            map->set_scenario_player_name(i, name);
            map->set_scenario_player_tribe(i, tribe);
			}
		}
      return;
	}
   throw wexception("Wrong packet version for Player_Names_And_Tribes_Data_Packet: %i\n", packet_version);

   assert(0);
}

/*
 * Write Function
 */
void Widelands_Map_Player_Names_And_Tribes_Data_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const)
throw (_wexception)
{
   char buf[256];

   Profile prof;
   Section* s = prof.create_section("global");

   // packet version
   s->set_int("packet_version", CURRENT_PACKET_VERSION);

   int i=0;
   Map* map=egbase->get_map();
   std::string name, tribe;
   for (i=1; i<=map->get_nrplayers(); i++) {
      tribe=map->get_scenario_player_tribe(i);
      name=map->get_scenario_player_name(i);

      sprintf(buf, "player_%i", i);
      s = prof.create_section(buf);
      s->set_string("name", name.c_str());
      s->set_string("tribe", tribe.c_str());
	}

   prof.write("player_names", false, fs);
   // DONE
}
