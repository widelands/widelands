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
#include "filesystem.h"
#include "editor_game_base.h"
#include "map.h"
#include "player.h"
#include "tribe.h"
#include "world.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_bob_data_packet.h"
#include "error.h"

// VERSION 1: inital release
// VERSION 2: workers are also handled here, registering through Map_Object_Loader/Saver
#define CURRENT_PACKET_VERSION 2

/*
 * Destructor
 */
Widelands_Map_Bob_Data_Packet::~Widelands_Map_Bob_Data_Packet(void) {
}

/*
 * Read Function
 */
void Widelands_Map_Bob_Data_Packet::Read(FileRead* fr, Editor_Game_Base* egbase, bool skip, Widelands_Map_Map_Object_Loader* mol) throw(wexception) {
   Map* map=egbase->get_map();

   // First packet version
   int packet_version=fr->Unsigned16();

   if(packet_version==1) {
      read_packet_version_1(fr,egbase,skip,mol);
      return;
   } else if (packet_version==CURRENT_PACKET_VERSION) {
      // Now get all the the bobs
      for(ushort y=0; y<map->get_height(); y++) {
         for(ushort x=0; x<map->get_width(); x++) {
            uint nr_bobs=fr->Unsigned32();
      
            uint i=0;

            assert(!egbase->get_map()->get_field(Coords(x,y))->get_first_bob());
            
            for(i=0;i<nr_bobs;i++) {
               std::string owner=fr->CString();
               std::string name=fr->CString();
               uchar subtype=fr->Unsigned8();
               
               uint reg=fr->Unsigned32();
               assert(!mol->is_object_known(reg));

               Bob* bob=0;
               if(owner=="world") {
                  assert(subtype==Bob::CRITTER);
                  int idx=egbase->get_map()->get_world()->get_bob(name.c_str());
                  if(idx==-1) 
                     throw wexception("Map defines Bob %s, but world doesn't deliver!\n", name.c_str());
                  bob=egbase->create_bob(Coords(x,y),idx);
               } else {
                  if(skip) continue; // We do no load player bobs when no scenario 
                  assert(subtype==Bob::WORKER);
                  egbase->manually_load_tribe(owner.c_str()); // Make sure that the correct tribe is known and loaded 
                  Tribe_Descr* tribe=egbase->get_tribe(owner.c_str());
                  if(!tribe) 
                     throw wexception("Map asks for Tribe %s, but world doesn't deliver!\n", owner.c_str());
                  int idx=tribe->get_worker_index(name.c_str());
                  if(idx==-1) 
                     throw wexception("Map defines Bob %s, but tribe %s doesn't deliver!\n", name.c_str(), owner.c_str());
                  Worker_Descr* descr=tribe->get_worker_descr(idx);
                  bob=descr->create_object();
                  bob->set_position(egbase, Coords(x,y));
                  bob->init(egbase);
               }
            
               assert(bob);

               // Register the bob for further loading
               if(!skip)
                  mol->register_object(egbase, reg, bob);
            }
         }
      }
      // DONE
      return;
   }
   
      assert(0); // never here
}


/*
 * Write Function
 */
void Widelands_Map_Bob_Data_Packet::Write(FileWrite* fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver* mos) throw(wexception) {
   assert(mos);

   // first of all the magic bytes
   fw->Unsigned16(PACKET_BOB);

   // now packet version
   fw->Unsigned16(CURRENT_PACKET_VERSION);

   // Now, all bob id and registerd it 
   // A Field can have more
   // than one bob, we have to take this into account
   //  uchar   numbers of bob for field
   //      bob1
   //      bob2
   //      ...
   //      bobn
   Map* map=egbase->get_map();
   for(ushort y=0; y<map->get_height(); y++) {
      for(ushort x=0; x<map->get_width(); x++) {
            std::vector<Bob*> bobarr;
         
            egbase->get_map()->find_bobs(Coords(x,y), 0, &bobarr);
            fw->Unsigned32(bobarr.size());

            for(uint i=0; i<bobarr.size(); i++) {
               Bob* ibob=bobarr[i];
               for(uint j=i; j<bobarr.size(); j++) {
                  Bob* jbob=bobarr[j];
                  if(ibob->get_file_serial() < jbob->get_file_serial()) {
                     bobarr[i] = jbob;
                     bobarr[j] = ibob;
                     ibob=jbob;
                  }
               }
            }
 
            for(uint i=0;i<bobarr.size(); i++) {
               // write serial number
               assert(!mos->is_object_known(bobarr[i])); // a bob can't be owned by two fields
               uint reg=mos->register_object(bobarr[i]);
               // Write its owner
               std::string owner_tribe = bobarr[i]->get_descr()->get_owner_tribe() ? bobarr[i]->get_descr()->get_owner_tribe()->get_name() : "world";
               fw->CString(owner_tribe.c_str());
               // Write it's name
               fw->CString(bobarr[i]->get_name().c_str());
               // Write it's subtype
               fw->Unsigned8(bobarr[i]->get_bob_type());
               // And it's file register index
               fw->Unsigned32(reg);
            }
      }
   }

   // DONE
}


/* 
 * Below here are the read functions for old map formats
 */
void Widelands_Map_Bob_Data_Packet::read_packet_version_1(FileRead* fr, Editor_Game_Base* egbase, bool skip, Widelands_Map_Map_Object_Loader* mol) {
   Map* map=egbase->get_map();
   World* world=map->get_world(); 
   int nr_bobs=fr->Unsigned16();
   if(nr_bobs>world->get_nr_bobs()) throw wexception("Number of bobs in map (%i) is bigger than in world (%i)",
         nr_bobs, world->get_nr_bobs());

   // construct ids and map
   std::map<uchar,Bob_Descr*> smap;
   char* buffer;
   for(int i=0; i<nr_bobs; i++) {
      int id=fr->Unsigned16();
      buffer=fr->CString();
      if(!world->get_bob_descr(world->get_bob(buffer))) throw wexception("Bob '%s' exists in map, not in world!", buffer);
      smap[id]=world->get_bob_descr(world->get_bob(buffer));
   }

   // Now get all the the bobs
   for(ushort y=0; y<map->get_height(); y++) {
      for(ushort x=0; x<map->get_width(); x++) {
         int nr_bobs=fr->Unsigned8();

         int i=0;
         for(i=0;i<nr_bobs;i++) {
            int id=fr->Unsigned16();
            egbase->create_bob(Coords(x, y), id);
         }
      }
   }
}

