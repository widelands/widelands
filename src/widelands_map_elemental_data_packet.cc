/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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
#include "filesystem.h"
#include "editor_game_base.h"
#include "map.h"
#include "world.h"

#define CURRENT_PACKET_VERSION 1

/*
 * Destructor
 */
Widelands_Map_Elemental_Data_Packet::~Widelands_Map_Elemental_Data_Packet(void) {
}

/*
 * Preread function
 */
void Widelands_Map_Elemental_Data_Packet::Pre_Read(FileRead* fr, Map* map) throw(wexception) {
   char buf[2000];

   memcpy(buf, fr->Data(sizeof(WLMF_MAGIC)), sizeof(WLMF_MAGIC));
   buf[4]='\0';
   if(strcmp(buf, WLMF_MAGIC)) throw wexception("Invalid File! Magic is '%s' instead of '%s'", buf, WLMF_MAGIC);

   m_version=fr->Unsigned16();
   if(m_version > WLMF_VERSION) throw wexception("Map newer than binary!");

   // check packet version
   int packet_version=fr->Unsigned16();

   if(packet_version == CURRENT_PACKET_VERSION) {
      map->m_width=fr->Unsigned16();
      map->m_height=fr->Unsigned16();
      map->set_nrplayers(fr->Unsigned8());

      // World name
      memcpy(buf, fr->Data(WORLD_NAME_LEN), WORLD_NAME_LEN);
      map->set_world_name(buf);

      // Name
      memcpy(buf, fr->Data(MAP_NAME_LEN), MAP_NAME_LEN);
      map->set_name(buf);

      // Author
      memcpy(buf, fr->Data(MAP_AUTHOR_LEN), MAP_AUTHOR_LEN);
      map->set_author(buf);

      // Descr
      memcpy(buf, fr->Data(MAP_DESCR_LEN), MAP_DESCR_LEN);
      map->set_description(buf);
      return;
   }
   assert(0); // should never be here
}



/*
 * Read Function
 */
void Widelands_Map_Elemental_Data_Packet::Read(FileRead* fr, Editor_Game_Base* egbase) throw(wexception) {
   Pre_Read(fr, egbase->get_map());
}


/*
 * Write Function
 */
void Widelands_Map_Elemental_Data_Packet::Write(FileWrite* fw, Editor_Game_Base* egbase) throw(wexception) {
   // first of all the magic bytes
   fw->Data(WLMF_MAGIC, sizeof(WLMF_MAGIC));

   // Now, map version
   fw->Unsigned16(WLMF_VERSION);

   // Packet version
   fw->Unsigned16(CURRENT_PACKET_VERSION);

   // Map dimensions
   Map* map=egbase->get_map();
   fw->Unsigned16(map->get_width());
   fw->Unsigned16(map->get_height());

   // Nr of players
   fw->Unsigned8(map->get_nrplayers());

   // World name
   fw->Data(map->get_world_name(), WORLD_NAME_LEN);

   // Name
   fw->Data(map->get_name(), MAP_NAME_LEN);

   // Author
   fw->Data(map->get_author(), MAP_AUTHOR_LEN);

   // Descr
   fw->Data(map->get_description(), WORLD_DESCR_LEN);
}
