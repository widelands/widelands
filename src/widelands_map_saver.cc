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

#include "widelands_map_saver.h"
#include "wexception.h"
#include "filesystem.h"
#include "map.h"
#include "widelands_map_elemental_data_packet.h"
#include "widelands_map_heights_data_packet.h"
#include "widelands_map_terrain_data_packet.h"
#include "widelands_map_immovable_data_packet.h"
#include "widelands_map_player_position_data_packet.h"
#include "widelands_map_bob_data_packet.h"
#include "widelands_map_resources_data_packet.h"

/*
 * Constructor
 */
Widelands_Map_Saver::Widelands_Map_Saver(std::string filename, Editor_Game_Base* egbase) {
   m_filename=filename;
   m_egbase=egbase;
}

/*
 * Destructor
 */
Widelands_Map_Saver::~Widelands_Map_Saver(void) {
}


/*
 * save function
 */
void Widelands_Map_Saver::save(void) throw(wexception) {
   std::string filename=m_filename;
   m_filename+=WLMF_SUFFIX;

   FileWrite fw;
   Widelands_Map_Data_Packet* dp;

   // MANDATORY PACKETS
   // Start with writing the map out, first Elemental data
   dp= new Widelands_Map_Elemental_Data_Packet();
   dp->Write(&fw, m_egbase);
   delete dp;

   // now heights
   dp=new Widelands_Map_Heights_Data_Packet();
   dp->Write(&fw, m_egbase);
   delete dp;

   // and terrains
   dp=new Widelands_Map_Terrain_Data_Packet();
   dp->Write(&fw, m_egbase);
   delete dp;

   // now immovables 
   dp=new Widelands_Map_Immovable_Data_Packet();
   dp->Write(&fw, m_egbase);
   delete dp;

   // now player pos
   dp=new Widelands_Map_Player_Position_Data_Packet();
   dp->Write(&fw, m_egbase);
   delete dp;
  
   // now bobs
   dp=new Widelands_Map_Bob_Data_Packet();
   dp->Write(&fw, m_egbase);
   delete dp;
   
   // now resources
   dp=new Widelands_Map_Resources_Data_Packet();
   dp->Write(&fw, m_egbase);
   delete dp;

   // NON MANDATORY PACKETS
   fw.Write(g_fs,m_filename); 
}

