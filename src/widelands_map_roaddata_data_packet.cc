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

#include "widelands_map_roaddata_data_packet.h"

#include "editor_game_base.h"
#include "fileread.h"
#include "filewrite.h"
#include "game.h"
#include "map.h"
#include "player.h"
#include <stdint.h>
#include "transport.h"
#include "tribe.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"

#include <map>

#define CURRENT_PACKET_VERSION 1

/*
 * Destructor
 */
Widelands_Map_Roaddata_Data_Packet::~Widelands_Map_Roaddata_Data_Packet() {
}

/*
 * Read Function
 */
void Widelands_Map_Roaddata_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Widelands_Map_Map_Object_Loader * const ol)
throw (_wexception)
{
	if (skip) return;

   FileRead fr;
	try {fr.Open(fs, "binary/road_data");} catch (...) {return;}

	const int packet_version = fr.Unsigned16();
	if (packet_version == CURRENT_PACKET_VERSION)
		for (;;) {
         uint32_t ser=fr.Unsigned32();
         if (ser==0xffffffff) // end of roaddata
            break;
         assert(ol->is_object_known(ser));
         assert(ol->get_object_by_file_index(ser)->get_type()==Map_Object::ROAD);

         Road* r=static_cast<Road*>(ol->get_object_by_file_index(ser));

         assert(!ol->is_object_loaded(r));

         Player* plr = egbase->get_safe_player(fr.Unsigned8());
         assert(plr);

         r->set_owner(plr);
         r->m_type=fr.Unsigned32();
         ser=fr.Unsigned32();
         uint32_t ser1=fr.Unsigned32();
         assert(ol->is_object_known(ser));
         assert(ol->is_object_known(ser1));
         r->m_flags[0]=static_cast<Flag*>(ol->get_object_by_file_index(ser));
         r->m_flags[1]=static_cast<Flag*>(ol->get_object_by_file_index(ser1));
         r->m_flagidx[0]=fr.Unsigned32();
         r->m_flagidx[1]=fr.Unsigned32();

         r->m_cost[0]=fr.Unsigned32();
         r->m_cost[1]=fr.Unsigned32();
			const Path::Step_Vector::size_type nsteps = fr.Unsigned16();
			assert(nsteps); //  FIXME NEVER USE assert TO VALIDATE INPUT!!!
			Path p(r->m_flags[0]->get_position());
			for (Path::Step_Vector::size_type i = 0; i < nsteps; ++i)
				p.append(egbase->map(), fr.Unsigned8()); //  FIXME validate that the value is a direction
         r->set_path(egbase, p);

         // Now that all rudimentary data is set, init this road,
         // than overwrite the initialization values
         r->link_into_flags(egbase);

         r->m_idle_index=fr.Unsigned32();
         r->m_desire_carriers=fr.Unsigned32();
         assert(!r->m_carrier.get(egbase));
         uint32_t carrierid=fr.Unsigned32();
         if (carrierid) {
            assert(ol->is_object_known(carrierid));
            r->m_carrier=ol->get_object_by_file_index(carrierid);
			} else
            r->m_carrier=0;

            delete r->m_carrier_request;
            r->m_carrier_request=0;

         bool request_exists=fr.Unsigned8();
         if (request_exists) {
            if (dynamic_cast<const Game *>(egbase)) {
               r->m_carrier_request = new Request(r, 0,
                     &Road::request_carrier_callback, r, Request::WORKER);
               r->m_carrier_request->Read(&fr, egbase, ol);
				}
			} else {
            r->m_carrier_request=0;
			}

         ol->mark_object_as_loaded(r);
		}
	else
		throw wexception
			("Unknown version %i in Widelands_Map_Roaddata_Data_Packet!",
			 packet_version);
}


/*
 * Write Function
 */
void Widelands_Map_Roaddata_Data_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const os)
throw (_wexception)
{
   FileWrite fw;

   // now packet version
   fw.Unsigned16(CURRENT_PACKET_VERSION);

   // We walk the map again for roads
   Map* map=egbase->get_map();
   for (uint16_t y=0; y<map->get_height(); y++) {
      for (uint16_t x=0; x<map->get_width(); x++) {
         Field* f=map->get_field(Coords(x, y));
         BaseImmovable* imm=f->get_immovable();
         if (!imm) continue;

         if (imm->get_type()==Map_Object::ROAD) {
            Road* r=static_cast<Road*>(imm);
            assert(os->is_object_known(r));
            if (os->is_object_saved(r))
               continue;
            uint32_t ser=os->get_object_file_index(r);

            // First, write serial
            fw.Unsigned32(ser);

            // First, write PlayerImmovable Stuff
            // Theres only the owner
            fw.Unsigned8(r->get_owner()->get_player_number());

            // type
            fw.Unsigned32(r->m_type);

            // Serial of flags
            assert(os->is_object_known(r->m_flags[0]));
            assert(os->is_object_known(r->m_flags[1]));
            fw.Unsigned32(os->get_object_file_index(r->m_flags[0]));
            fw.Unsigned32(os->get_object_file_index(r->m_flags[1]));

            // Flags index
            fw.Unsigned32(r->m_flagidx[0]);
            fw.Unsigned32(r->m_flagidx[1]);

            // Cost
            fw.Unsigned32(r->m_cost[0]);
            fw.Unsigned32(r->m_cost[1]);

            // Path
				const Path & path = r->m_path;
				const Path::Step_Vector::size_type nr_steps = path.get_nsteps();
				fw.Unsigned16(nr_steps);
				for (Path::Step_Vector::size_type i = 0; i < nr_steps; ++i)
					fw.Unsigned8(path[i]);

            // Idle index
            fw.Unsigned32(r->m_idle_index);

            // Desired carrier
            fw.Unsigned32(r->m_desire_carriers);

            // Carrier
            if (r->m_carrier.get(egbase)) {
               assert(os->is_object_known(r->m_carrier.get(egbase)));
               fw.Unsigned32(os->get_object_file_index(r->m_carrier.get(egbase)));
				} else {
               fw.Unsigned32(0);
				}

            // Request
            if (r->m_carrier_request) {
               fw.Unsigned8(1);
               r->m_carrier_request->Write(&fw, egbase, os);
				} else
               fw.Unsigned8(0);

            os->mark_object_as_saved(r);
			}
		}
	}

   fw.Unsigned32(0xFFFFFFFF); // End of roads
   // DONE

   fw.Write(fs, "binary/road_data");
}
