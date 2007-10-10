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

#include "widelands_map_elemental_data_packet.h"

#include "editor_game_base.h"
#include "filesystem.h"
#include "i18n.h"
#include "map.h"
#include "profile.h"
#include "world.h"

#define CURRENT_PACKET_VERSION 1

/*
 * Destructor
 */
Widelands_Map_Elemental_Data_Packet::~Widelands_Map_Elemental_Data_Packet() {
}

/*
 * Preread function
 */
void Widelands_Map_Elemental_Data_Packet::Pre_Read(FileSystem & fs, Map* map)
throw (_wexception)
{
    //Load maps textdomain
    i18n::grab_textdomain("maps");

   Profile prof;
   prof.read("elemental", 0, fs);
   Section* s = prof.get_section("global");

	const int32_t packet_version = s->get_int("packet_version");
	if (packet_version == CURRENT_PACKET_VERSION) {
      map->m_width= s->get_int("map_w");
      map->m_height= s->get_int("map_h");
      map->set_nrplayers(s->get_int("nr_players"));

      // World name
      map->set_world_name(s->get_string("world"));

      // Name
      map->set_name(s->get_string("name"));

      // Author
      map->set_author(s->get_string("author"));

      // Descr
      map->set_description(s->get_string("descr"));

	  // Loader background
      map->set_background(s->get_string("background"));

    // Release maps textdomain
    i18n::release_textdomain();

	} else
		throw wexception
			("Map Elemental Data with unknown/unhandled version %i in map!",
			 packet_version);
}


/*
 * Read Function
 */
void Widelands_Map_Elemental_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base * egbase,
 const bool,
 Widelands_Map_Map_Object_Loader * const)
throw (_wexception)
{Pre_Read(fs, egbase->get_map());}


/*
 * Write Function
 */
void Widelands_Map_Elemental_Data_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const)
throw (_wexception)
{

   Profile prof;
   Section* s = prof.create_section("global");

   // Packet version
   s->set_int("packet_version", CURRENT_PACKET_VERSION);
   // Map Dimension
   Map* map=egbase->get_map();
   s->set_int("map_w", map->get_width());
   s->set_int("map_h", map->get_height());
   // NR players
   s->set_int("nr_players", map->get_nrplayers());
   // Worldname
   s->set_string("world", map->get_world_name());
   // Map Name
   s->set_string("name", map->get_name());
   // Author
   s->set_string("author", map->get_author());
   // Descr
   s->set_string("descr", map->get_description());

   prof.write("elemental", false, fs);
}
