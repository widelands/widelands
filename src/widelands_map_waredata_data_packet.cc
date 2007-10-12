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

#include "widelands_map_waredata_data_packet.h"

#include "bob.h"
#include "fileread.h"
#include "filewrite.h"
#include "editor_game_base.h"
#include "game.h"
#include "map.h"
#include "player.h"
#include <stdint.h>
#include "transport.h"
#include "tribe.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"
#include "worker.h"

#define CURRENT_PACKET_VERSION 1


Widelands_Map_Waredata_Data_Packet::~Widelands_Map_Waredata_Data_Packet() {}


void Widelands_Map_Waredata_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Widelands_Map_Map_Object_Loader * const ol)
throw (_wexception)
{
   if (skip)
      return;

   FileRead fr;
   try {
      fr.Open(fs, "binary/ware_data");
	} catch (...) {
      // not there, so skip
      return ;
	}

	const uint16_t packet_version = fr.Unsigned16();
	if (packet_version == CURRENT_PACKET_VERSION) for (;;) {
         uint32_t reg=fr.Unsigned32();
         if (reg==0xffffffff) break; // end of wares
		if
			(WareInstance * const ware = dynamic_cast<WareInstance *>
			 (ol->get_object_by_file_index(reg)))
		{
			reg = fr.Unsigned32();
			if (Map_Object * const location = ol->get_object_by_file_index(reg))
			{
				const uint32_t ware_index_from_file = fr.Unsigned32();
				ware->m_descr_index = ware_index_from_file;
				if
					(PlayerImmovable * const player_immovable =
					 dynamic_cast<PlayerImmovable *>(location))
				{
					if
						(dynamic_cast<const Building *>(player_immovable)
						 or
						 dynamic_cast<const Flag *>(player_immovable))
					{
						//  We didn't know what kind of ware we were till now, so no
						//  economy might have a clue of us.
						ware->m_economy = 0;

						const Tribe_Descr & tribe = player_immovable->owner().tribe();
						if (tribe.get_nrwares() <= static_cast<int32_t>(ware_index_from_file))
							throw wexception
								("Widelands_Map_Waredata_Data_Packet: ware index out "
								 "of range: %i\n",
								 ware_index_from_file);
						ware->m_ware_descr =
							tribe.get_ware_descr(ware->descr_index());
						ware->set_economy(player_immovable->get_economy());
					} else
						throw wexception
							("Widelands_Map_Waredata_Data_Packet: location is "
							 "PlayerImmovable but not Building or Flag\n");
				} else if
					(Worker * const worker = dynamic_cast<Worker *>(location))
				{
					const Tribe_Descr & tribe = *worker->get_tribe();
					if (tribe.get_nrwares() <= static_cast<int32_t>(ware_index_from_file))
						throw wexception
							("Widelands_Map_Waredata_Data_Packet: ware index out of "
							 "range: %i\n",
							 ware_index_from_file);
					ware->m_ware_descr = tribe.get_ware_descr(ware->descr_index());
					ware->m_economy = 0; //  The worker sets our economy.
				} else
					throw wexception
						("Widelands_Map_Waredata_Data_Packet: location is not "
						 "PlayerImmovable or Worker\n");
         // Do not touch supply or transfer

         // m_transfer_nextstep
         reg=fr.Unsigned32();
         if (reg) {
            assert(ol->is_object_known(reg));
            ware->m_transfer_nextstep=ol->get_object_by_file_index(reg);
			} else
            ware->m_transfer_nextstep=(Map_Object*)(0);

         // Do some kind of init
				if (Game * const game = dynamic_cast<Game *>(egbase))
					ware->set_location(game, location);
         ol->mark_object_as_loaded(ware);
			} else
				throw wexception
					("Widelands_Map_Waredata_Data_Packet: location with serial "
					 "number %u is not known",
					 reg);
		} else
			throw wexception
				("Widelands_Map_Waredata_Data_Packet: ware with serial number %u "
				 "is not known",
				 reg);
	} else
		throw wexception
			("Unknown version %u in Widelands_Map_Waredata_Data_Packet!",
			 packet_version);
}


/*
 * Write Function
 */
void Widelands_Map_Waredata_Data_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const os)
throw (_wexception)
{
   FileWrite fw;

   // now packet version
   fw.Unsigned16(CURRENT_PACKET_VERSION);

   // We transverse the map and whenever we find a suitable object, we check if it has wares of some kind
   Map* map=egbase->get_map();
   std::vector<uint32_t> ids;
	for (uint16_t y = 0; y < map->get_height(); ++y) {
		for (uint16_t x = 0; x < map->get_width(); ++x) {
         Field* f=map->get_field(Coords(x, y));

         // First, check for Flags
         BaseImmovable* imm=f->get_immovable();
         if (imm && imm->get_type()==Map_Object::FLAG) {
            Flag* fl=static_cast<Flag*>(imm);
            for (int32_t i = 0; i < fl->m_item_filled; ++i) {
               assert(os->is_object_known(fl->m_items[i].item));
               write_ware(&fw, egbase, os, fl->m_items[i].item);
				}
			}

         // Now, check for workers
         Bob* b=f->get_first_bob();
         while (b) {
            if (b->get_bob_type()==Bob::WORKER) {
               Worker* w=static_cast<Worker*>(b);
               WareInstance* ware=w->get_carried_item(egbase);
               if (ware) {
                  assert(os->is_object_known(ware));
                  write_ware(&fw, egbase, os, ware);
					}
				}
            b=b->get_next_bob();
			}
		}
	}
   fw.Unsigned32(0xffffffff); // End of wares

   fw.Write(fs, "binary/ware_data");
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
   if (obj) {
      assert(os->is_object_known(obj));
      fw->Unsigned32(os->get_object_file_index(obj));
	} else
      fw->Unsigned32(0);

   // Economy is set by set_location()

	fw->Signed32(ware->descr_index());
   // Description is set manually

   // Skip Supply

   // Transfer is handled automatically
//   if (ware->m_transfer)
//      fw->Unsigned8(1);
//   else
//      fw->Unsigned8(0);
//
   // m_transfer_nextstep
	obj=ware->m_transfer_nextstep.get(egbase);
   if (obj) {
      assert(os->is_object_known(obj));
      fw->Unsigned32(os->get_object_file_index(obj));
	} else
      fw->Unsigned32(0);

   os->mark_object_as_saved(ware);
}
