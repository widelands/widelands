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

#include <map>
#include "constructionsite.h"
#include "filesystem.h"
#include "editor.h"
#include "editorinteractive.h"
#include "editor_game_base.h"
#include "map.h"
#include "player.h"
#include "tribe.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_building_data_packet.h"
#include "error.h"

#define CURRENT_PACKET_VERSION 1

/*
 * Destructor
 */
Widelands_Map_Building_Data_Packet::~Widelands_Map_Building_Data_Packet(void) {
}

/*
 * Read Function
 */
void Widelands_Map_Building_Data_Packet::Read(FileSystem* fs, Editor_Game_Base* egbase, bool skip, Widelands_Map_Map_Object_Loader* ol) throw(wexception) {
   if( skip ) 
      return;

   FileRead fr;
   try {
      fr.Open( fs, "binary/building" );
   } catch ( ... ) {
      // not there, so skip
      return ;
   }
   
   Map* map=egbase->get_map();

   // First packet version
   int packet_version=fr.Unsigned16();
   if(packet_version==CURRENT_PACKET_VERSION) {
      for(ushort y=0; y<map->get_height(); y++) {
         for(ushort x=0; x<map->get_width(); x++) {
            uchar exists=fr.Unsigned8();
            if(exists) {
               // Ok, now read all the additional data
               uchar owner=fr.Unsigned8();
               uint serial=fr.Unsigned32();
               std::string name=fr.CString();
               bool is_constructionsite=fr.Unsigned8();

               // No building lives on more than one main place
               assert(!ol->is_object_known(serial));

               // Get the tribe and the building index
               Player* plr=egbase->get_safe_player(owner);
               assert(plr); // He must be there
               int index=plr->get_tribe()->get_building_index(name.c_str());
               if(index==-1) 
                  throw wexception("Widelands_Map_Building_Data_Packet::Read(): Should create building %s in tribe %s, but building is unknown!\n", 
                        name.c_str(), plr->get_tribe()->get_name());

               // Now, create this Building, take extra special care for constructionsites
               Building* building=0;
               if(is_constructionsite)
                  building=egbase->warp_constructionsite(Coords(x,y),plr->get_player_number(),index,0); // all data is read later
               else
                  building=egbase->warp_building(Coords(x,y), plr->get_player_number(), index);

               log("Set building %p at (%i,%i)\n", building, x, y);
               assert(building);

               // Reference the players tribe if in editor
               if(!egbase->is_game()) {
                  assert(!skip);
                  static_cast<Editor_Interactive*>(egbase->get_iabase())->reference_player_tribe(owner, plr->get_tribe());
               }
               // and register it with the object loader for further loading
               ol->register_object(egbase, serial, building);
            }
         }

      }
      // DONE
      return;
   }
   throw wexception("Unknown version %i in Widelands_Map_Building_Data_Packet!\n", packet_version);
   assert(0); // Never here
}


/*
 * Write Function
 */
void Widelands_Map_Building_Data_Packet::Write(FileSystem* fs, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver* os) throw(wexception) {
   FileWrite fw;

   // now packet version
   fw.Unsigned16(CURRENT_PACKET_VERSION);
  
   // Write buildings and owner, register this with the map_object_saver so that
   // it's data can be saved later.
   Map* map=egbase->get_map();
   for(ushort y=0; y<map->get_height(); y++) {
      for(ushort x=0; x<map->get_width(); x++) {
         BaseImmovable* immovable=map->get_field(Coords(x,y))->get_immovable();
         // We only write Buildings 
         if(immovable && immovable->get_type()==Map_Object::BUILDING) {
            Building* building=static_cast<Building*>(immovable);
          
            if(building->get_position()!=Coords(x,y)) {
               // This is not this buildings main position 
               fw.Unsigned8('\0');
               continue; 
            }

            // Buildings can life on only one main position
            assert(!os->is_object_known(building));
            uint serial=os->register_object(building);
 
            fw.Unsigned8(1);
            fw.Unsigned8(building->get_owner()->get_player_number());
            // write id
            fw.Unsigned32(serial);
            
            bool constructionsite=building->get_building_type()==Building::CONSTRUCTIONSITE;

            if(constructionsite) {
               ConstructionSite* cs=static_cast<ConstructionSite*>(building);
               fw.CString(cs->get_building()->get_name());
               fw.Unsigned8(1);
            } else {
            // write name
            std::string name=building->get_name();
            fw.CString(name.c_str());
            fw.Unsigned8(0);
            }
            
         } else {
            // No existance, no owner
            fw.Unsigned8(0);
         }
      }
   }
   
   fw.Write( fs, "binary/building" );
   // DONE
}
