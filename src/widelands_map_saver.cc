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

#include "editor_game_base.h"
#include "filesystem.h"
#include "map.h"
#include "player.h"
#include "tribe.h"
#include "wexception.h"
#include "widelands_map_allowed_buildings_data_packet.h"
#include "widelands_map_battle_data_packet.h"
#include "widelands_map_bob_data_packet.h"
#include "widelands_map_bobdata_data_packet.h"
#include "widelands_map_building_data_packet.h"
#include "widelands_map_buildingdata_data_packet.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_elemental_data_packet.h"
#include "widelands_map_flag_data_packet.h"
#include "widelands_map_flagdata_data_packet.h"
#include "widelands_map_event_data_packet.h"
#include "widelands_map_heights_data_packet.h"
#include "widelands_map_immovable_data_packet.h"
#include "widelands_map_immovabledata_data_packet.h"
#include "widelands_map_map_object_saver.h"
#include "widelands_map_owned_fields_data_packet.h"
#include "widelands_map_player_names_and_tribes_data_packet.h"
#include "widelands_map_player_position_data_packet.h"
#include "widelands_map_resources_data_packet.h"
#include "widelands_map_road_data_packet.h"
#include "widelands_map_roaddata_data_packet.h"
#include "widelands_map_saver.h"
#include "widelands_map_seen_fields_data_packet.h"
#include "widelands_map_terrain_data_packet.h"
#include "widelands_map_trigger_data_packet.h"
#include "widelands_map_ware_data_packet.h"
#include "widelands_map_waredata_data_packet.h"

/*
 * Constructor
 */
Widelands_Map_Saver::Widelands_Map_Saver(std::string filename, Editor_Game_Base* egbase) {
   m_filename=filename;
   m_egbase=egbase;
   m_fw=0;
   m_mos=0;
}
Widelands_Map_Saver::Widelands_Map_Saver(FileWrite* fw, Editor_Game_Base* egbase) {
   m_fw=fw;
   m_egbase=egbase;
   m_mos=0;
}

/*
 * Destructor
 */
Widelands_Map_Saver::~Widelands_Map_Saver(void) {
   if(m_mos)
      delete m_mos;
}


/*
 * save function
 */
void Widelands_Map_Saver::save(void) throw(wexception) {

   FileWrite* fw;
   if(m_fw)
      fw=m_fw;
   else
      fw=new FileWrite();


   std::string filename=m_filename;
   Widelands_Map_Data_Packet* dp;
   if(m_mos)
      delete m_mos;
   m_mos=new Widelands_Map_Map_Object_Saver();

   log("Writing Elemental Data ... ");
   // MANDATORY PACKETS
   // Start with writing the map out, first Elemental data
   // PRELOAD DATA BEGIN
   dp= new Widelands_Map_Elemental_Data_Packet();
   dp->Write(fw, m_egbase, m_mos);
   delete dp;
   log("done!\n ");


   // now player names and tribes
   log("Writing Player Names And Tribe Data ... ");
   dp=new Widelands_Map_Player_Names_And_Tribes_Data_Packet();
   dp->Write(fw, m_egbase, m_mos);
   delete dp;
   log("done!\n ");
   // PRELOAD DATA END

   // now heights
   log("Writing Heights Data ... ");
   dp=new Widelands_Map_Heights_Data_Packet();
   dp->Write(fw, m_egbase, m_mos);
   delete dp;
   log("done!\n ");

   // and terrains
   log("Writing Terrain Data ... ");
   dp=new Widelands_Map_Terrain_Data_Packet();
   dp->Write(fw, m_egbase, m_mos);
   delete dp;
   log("done!\n ");

   // now immovables
   log("Writing Immovable Data ... ");
   dp=new Widelands_Map_Immovable_Data_Packet();
   dp->Write(fw, m_egbase, m_mos);
   delete dp;
   log("done!\n ");

   // now player pos
   log("Writing Player Start Position Data ... ");
   dp=new Widelands_Map_Player_Position_Data_Packet();
   dp->Write(fw, m_egbase, m_mos);
   delete dp;
   log("done!\n ");

   // now bobs
   log("Writing Bob Data ... ");
   dp=new Widelands_Map_Bob_Data_Packet();
   dp->Write(fw, m_egbase, m_mos);
   delete dp;
   log("done!\n ");

   // now resources
   log("Writing Resources Data ... ");
   dp=new Widelands_Map_Resources_Data_Packet();
   dp->Write(fw, m_egbase, m_mos);
   delete dp;
   log("done!\n ");

   // NON MANDATORY PACKETS BELOW THIS POINT

   // Triggers
   if(m_egbase->get_map()->get_number_of_triggers()) {
      log("Writing Trigger Data ... ");
      dp=new Widelands_Map_Trigger_Data_Packet();
      dp->Write(fw, m_egbase, m_mos);
      delete dp;
      log("done!\n ");
   }
   // Events
   if(m_egbase->get_map()->get_number_of_events()) {
      log("Writing Event Data ... ");
      dp=new Widelands_Map_Event_Data_Packet();
      dp->Write(fw, m_egbase, m_mos);
      delete dp;
      log("done!\n ");
   }

   // Allowed buildings
   bool write_allowed_buildings=false;
   int i;
   for(i=1; i<=m_egbase->get_map()->get_nrplayers(); i++) {
      int b=0;
      Player* player=m_egbase->get_player(i);
      if(!player) continue;
      for(b=0; b<player->get_tribe()->get_nrbuildings(); b++) 
         if(player->is_building_allowed(b)) {
            write_allowed_buildings=true;
            break;
         }
      if(write_allowed_buildings) break;
   }
   if(write_allowed_buildings) {
      log("Writing Allowed Data ... ");
      dp=new Widelands_Map_Allowed_Buildings_Data_Packet();
      dp->Write(fw, m_egbase, m_mos);
      delete dp;
      log("done!\n ");
   }

   // We always write the next few packets since it 
   // takes too much time looking if it really is needed
   // !!!!!!!!!! NOTE  
   // This packet must be before any building or road packet. So do not 
   // change this order without knowing what you do
   // EXISTENT PACKETS
   log("Writing Flag Data ... ");
   dp=new Widelands_Map_Flag_Data_Packet();
   dp->Write(fw, m_egbase, m_mos);
   delete dp;
   log("done!\n ");

   log("Writing Road Data ... ");
   dp=new Widelands_Map_Road_Data_Packet();
   dp->Write(fw, m_egbase, m_mos);
   delete dp;
   log("done!\n ");

   log("Writing Building Data ... ");
   dp=new Widelands_Map_Building_Data_Packet();
   dp->Write(fw, m_egbase, m_mos);
   delete dp;
   log("done!\n ");


   log("Writing Map Ware Data ... ");
   dp=new Widelands_Map_Ware_Data_Packet();
   dp->Write(fw, m_egbase, m_mos);
   delete dp;
   log("done!\n ");

   // DATA PACKETS 
   if(m_mos->get_nr_flags()) {  
      log("Writing Flagdata Data ... ");
      dp=new Widelands_Map_Flagdata_Data_Packet();
      dp->Write(fw, m_egbase, m_mos);
      delete dp;
      log("done!\n ");
   }

   if(m_mos->get_nr_roads()) { 
      log("Writing Roaddata Data ... ");
      dp=new Widelands_Map_Roaddata_Data_Packet();
      dp->Write(fw, m_egbase, m_mos);
      delete dp;
      log("done!\n ");
   }


   if(m_mos->get_nr_buildings()) {
      log("Writing Buildingdata Data ... ");
      dp=new Widelands_Map_Buildingdata_Data_Packet();
      dp->Write(fw, m_egbase, m_mos);
      delete dp;
      log("done!\n ");
   }


   if(m_mos->get_nr_wares()) {
      log("Writing Waredata Data ... ");
      dp=new Widelands_Map_Waredata_Data_Packet();
      dp->Write(fw, m_egbase, m_mos);
      delete dp;
      log("done!\n ");
   }

   if(m_mos->get_nr_bobs()) {
      log("Writing Bobdata Data ... ");
      dp=new Widelands_Map_Bobdata_Data_Packet();
      dp->Write(fw, m_egbase, m_mos);
      delete dp;
      log("done!\n ");
   } 

   if(m_mos->get_nr_immovables()) {
      log("Writing Immovabledata Data ... ");
      dp=new Widelands_Map_Immovabledata_Data_Packet();
      dp->Write(fw, m_egbase, m_mos);
      delete dp;
      log("done!\n ");
   }

   log("Writing Owned-Fields Data ... ");
   dp=new Widelands_Map_Owned_Fields_Data_Packet();
   dp->Write(fw, m_egbase, m_mos);
   delete dp;
   log("done!\n ");

   log("Writing Seen-Fields Data ... ");
   dp=new Widelands_Map_Seen_Fields_Data_Packet();
   dp->Write(fw, m_egbase, m_mos);
   delete dp;
   log("done!\n ");


   // This should be at least after loading Soldiers (Bobs)
   // NOTE DO NOT CHANGE THE PLACE UNLESS YOU KNOW WHAT ARE YOU DOING
   log("Writing Battle Data ... ");
   dp=new Widelands_Map_Battle_Data_Packet();
   dp->Write(fw, m_egbase, m_mos);
   delete dp;
   log("done!\n ");

   
   // Write end of map data
   fw->Unsigned16(PACKET_END_OF_MAP_DATA);

   if(!m_fw) {
      fw->Write(g_fs,m_filename);
      delete fw;
   }

   if(m_mos->get_nr_unsaved_objects())
      throw wexception("There are %i unsaved objects. This is a bug, please consider committing!\n", m_mos->get_nr_unsaved_objects());
}

