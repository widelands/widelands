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

#include "bob.h"
#include "filesystem.h"
#include "editor.h"
#include "editorinteractive.h"
#include "editor_game_base.h"
#include "game.h"
#include "map.h"
#include "player.h"
#include "transport.h"
#include "tribe.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_waredata_data_packet.h"
#include "worker.h"
#include "error.h"

#define CURRENT_PACKET_VERSION 1

/*
 * Destructor
 */
Widelands_Map_Waredata_Data_Packet::~Widelands_Map_Waredata_Data_Packet(void) {
}

/*
 * Read Function
 */
void Widelands_Map_Waredata_Data_Packet::Read(FileRead* fr, Editor_Game_Base* egbase, bool skip, Widelands_Map_Map_Object_Loader* ol) throw(wexception) {
   // First packet version
   int packet_version=fr->Unsigned16();

   if(packet_version==CURRENT_PACKET_VERSION) {
      // Now the rest data len
      uint len = fr->Unsigned32();
      if(skip) {
         // Skip the rest, flags are not our problem here
         fr->Data(len);
         return;
      }

      while(1) {
         uint reg=fr->Unsigned32();
         if(reg==0xffffffff) break; // end of wares
         assert(ol->is_object_known(reg));
         assert(ol->get_object_by_file_index(reg)->get_type()==Map_Object::WARE);

         WareInstance* ware=static_cast<WareInstance*>(ol->get_object_by_file_index(reg));

         reg=fr->Unsigned32();
         Map_Object* location;
         assert(reg);

         assert(ol->is_object_known(reg));
         location=ol->get_object_by_file_index(reg);

         ware->m_ware=fr->Signed32();
         switch(location->get_type()) {
            case Map_Object::BUILDING: // Fallthrough 
            case Map_Object::FLAG: 
               log("Adding ware with id %i from %s\n", ware->m_ware, location->get_type() == Map_Object::FLAG ?  "Flag" : "Building");
               ware->m_economy=0; // We didn't know what kind of ware we were till now, so no economy might have a clue of us
               ware->m_ware_descr=static_cast<PlayerImmovable*>(location)->get_owner()->get_tribe()->get_ware_descr(ware->m_ware); 
               ware->set_economy(static_cast<PlayerImmovable*>(location)->get_economy());
               break;

            case Map_Object::BOB:
               log("Adding ware with id %i from Worker\n", ware->m_ware);
               assert(static_cast<Bob*>(location)->get_bob_type()==Bob::WORKER);
               ware->m_ware_descr=static_cast<Worker*>(location)->get_tribe()->get_ware_descr(ware->m_ware);
               ware->m_economy=0;
               // The worker sets our economy
               break;

            default: throw wexception("Ware is on unknown object type: %i\n", location->get_type());
         }

         // Do not touch supply or transfer

         // m_transfer_nextstep 
         reg=fr->Unsigned32();
         if(reg) {
            assert(ol->is_object_known(reg));
            ware->m_transfer_nextstep=ol->get_object_by_file_index(reg);
         } else
            ware->m_transfer_nextstep=(Map_Object*)(0);

         // Do some kind of init
         if(egbase->is_game()) {
            Game* g=static_cast<Game*>(egbase);
            ware->set_location(g, location);
         }
         ol->mark_object_as_loaded(ware);
         log("Loadede ware %p (descr is: %p)\n", ware, ware->m_ware_descr);
      }
      // DONE
      return;
   }
   throw wexception("Unknown version %i in Widelands_Map_Waredata_Data_Packet!\n", packet_version);
}


/*
 * Write Function
 */
void Widelands_Map_Waredata_Data_Packet::Write(FileWrite* fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver* os) throw(wexception) {
   // first of all the magic bytes
   fw->Unsigned16(PACKET_WAREDATA);

   // now packet version
   fw->Unsigned16(CURRENT_PACKET_VERSION);
  
   // Here we will insert skip data (packet lenght) 
   // later, write a dummy for know
   int filepos = fw->GetFilePos();
   fw->Unsigned32(0x00000000);
   fw->ResetByteCounter();
   
   // We transverse the map and whenever we find a suitable object, we check if it has wares of some kind
   Map* map=egbase->get_map();
   std::vector<uint> ids;
   for(ushort y=0; y<map->get_height(); y++) {
      for(ushort x=0; x<map->get_width(); x++) {
         Field* f=map->get_field(Coords(x,y));
         
         // First, check for Flags
         BaseImmovable* imm=f->get_immovable();
         if(imm && imm->get_type()==Map_Object::FLAG) {
            Flag* f=static_cast<Flag*>(imm);
            for(int i=0; i<f->m_item_filled; i++) {
               assert(os->is_object_known(f->m_items[i].item));
               write_ware(fw,egbase,os,f->m_items[i].item);
            }
         }
      
         // Now, check for workers
         Bob* b=f->get_first_bob();
         while(b) {
            if(b->get_bob_type()==Bob::WORKER) {
               Worker* w=static_cast<Worker*>(b);
               WareInstance* ware=w->get_carried_item(egbase);
               if(ware) {
                  assert(os->is_object_known(ware));
                  write_ware(fw,egbase,os,ware);
               }
            }
            b=b->get_next_bob();
         }
      }
   }
   fw->Unsigned32(0xffffffff); // End of wares
   
   // Now, write the packet length
   fw->Unsigned32(fw->GetByteCounter(), filepos);

   // DONE
}

/*
 * Write this ware instances data to disk
 */
void Widelands_Map_Waredata_Data_Packet::write_ware(FileWrite* fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver* os, WareInstance* ware) {
   // First, id
   fw->Unsigned32(os->get_object_file_index(ware));

   // Location
   Map_Object* obj=ware->m_location.get(egbase);
   if(obj) {
      assert(os->is_object_known(obj));
      fw->Unsigned32(os->get_object_file_index(obj));
   } else 
      fw->Unsigned32(0);
	
   // Economy is set by set_location()

   fw->Signed32(ware->m_ware);
   // Description is set manually

   // Skip Supply
   
   // Transfer is handled automatically
/*   if(ware->m_transfer)
      fw->Unsigned8(1);
   else
      fw->Unsigned8(0);
*/
   // m_transfer_nextstep
	obj=ware->m_transfer_nextstep.get(egbase);
   if(obj) {
      assert(os->is_object_known(obj));
      fw->Unsigned32(os->get_object_file_index(obj));
   } else 
      fw->Unsigned32(0);

   os->mark_object_as_saved(ware);
}


