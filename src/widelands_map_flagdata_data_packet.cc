/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#include "widelands_map_flagdata_data_packet.h"

#include "building.h"
#include "editor_game_base.h"
#include "map.h"
#include "player.h"
#include "transport.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"

#include "upcast.h"

#include <map>

namespace Widelands {

#define CURRENT_PACKET_VERSION 1


Map_Flagdata_Data_Packet::~Map_Flagdata_Data_Packet() {}


void Map_Flagdata_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Map_Map_Object_Loader * const ol)
throw (_wexception)
{
	if (skip)
      return;

	FileRead fr;
	try {fr.Open(fs, "binary/flag_data");} catch (...) {return;}

	const uint16_t packet_version = fr.Unsigned16();
	if (packet_version == CURRENT_PACKET_VERSION) {
		const Extent extent = egbase->map().extent();
		for (;;) {
			const uint32_t ser = fr.Unsigned32();

			if (ser == 0xffffffff)
				break; // end of flags
         assert(ol->is_object_known(ser)); //  FIXME NEVER USE assert TO VALIDATE INPUT!!!
         assert(ol->get_object_by_file_index(ser)->get_type()==Map_Object::FLAG); //  FIXME NEVER USE assert TO VALIDATE INPUT!!!

			upcast(Flag, flag, ol->get_object_by_file_index(ser));

         // The owner is already set, nothing to do from
         // PlayerImmovable

			try {flag->m_position = fr.Coords32(extent);}
			catch (const FileRead::Data_Error & e) {
				throw wexception
					("Map_Flagdata_Data_Packet::Read: in binary/flag_data:%u: "
					 "Coordinates of flag %u: %s",
					 fr.GetPos() - 4, ser, e.message().c_str());
			}
         flag->m_animstart=fr.Unsigned16();
         int32_t building=fr.Unsigned32();
			if (building) {
            assert(ol->is_object_known(building));
            flag->m_building=static_cast<Building*>(ol->get_object_by_file_index(building));
			} else
            flag->m_building=0;


         // Roads are set somewhere else

			for (uint32_t i = 0; i < 6; ++i)
				flag->m_items_pending[i] = fr.Unsigned32();
         flag->m_item_capacity=fr.Unsigned32();
         flag->m_item_filled=fr.Unsigned32();

         // items
			for (int32_t i = 0; i < flag->m_item_filled; ++i) {
            flag->m_items[i].pending=fr.Unsigned8();
            uint32_t item=fr.Unsigned32();
            assert(ol->is_object_known(item));
            flag->m_items[i].item=static_cast<WareInstance*>(ol->get_object_by_file_index(item));

            uint32_t nextstep=fr.Unsigned32();
				if (nextstep) {
               assert(ol->is_object_known(nextstep));
               flag->m_items[i].nextstep=static_cast<PlayerImmovable*>(ol->get_object_by_file_index(nextstep));
				} else {
               flag->m_items[i].nextstep=0;
				}
			}

         // always call
         uint32_t always_call=fr.Unsigned32();
			if (always_call) {
            assert(ol->is_object_known(always_call));
            flag->m_always_call_for_flag=static_cast<Flag*>(ol->get_object_by_file_index(always_call));
			} else
            flag->m_always_call_for_flag=0;

         // Workers waiting
			const uint16_t nr_workers = fr.Unsigned16();
         flag->m_capacity_wait.resize(nr_workers);
			for (uint32_t i = 0; i < nr_workers; ++i) {
            uint32_t id=fr.Unsigned32();
            assert(ol->is_object_known(id));
            flag->m_capacity_wait[i]=ol->get_object_by_file_index(id);
			}

         // Flag jobs
			const uint16_t nr_jobs = fr.Unsigned16();
         assert(!flag->m_flag_jobs.size());
			for (uint16_t i = 0; i < nr_jobs; ++i) {
            Flag::FlagJob f;
            bool request=fr.Unsigned8();
				if (!request)
               f.request=0;
				else {
               f.request = new Request(flag, 1,
	                        &Flag::flag_job_request_callback, flag, Request::WORKER);
               f.request->Read(&fr, egbase, ol);
				}
            f.program=fr.CString();
            flag->m_flag_jobs.push_back(f);
			}

         ol->mark_object_as_loaded(flag);
		}
	} else
		throw wexception
			("Unknown version %u in Map_Flagdata_Data_Packet!", packet_version);
}


void Map_Flagdata_Data_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Map_Map_Object_Saver * const os)
throw (_wexception)
{
	FileWrite fw;

   // now packet version
   fw.Unsigned16(CURRENT_PACKET_VERSION);

	const Map & map = egbase->map();
	const Field & fields_end = map[map.max_index()];
	for (Field * field = &map[0]; field < &fields_end; ++field) if //  FIXME field should be "const Field *"
		(upcast(Flag, flag, field->get_immovable()))
	{
            assert(os->is_object_known(flag));
            assert(!os->is_object_saved(flag));

            // Write serial
            fw.Unsigned32(os->get_object_file_index(flag));

            // Owner is already written in the existanz packet

		fw.Coords32  (flag->m_position);

            // Animation is set by creator
            fw.Unsigned16(flag->m_animstart);

            // Building is not used, it is set by Building_Data packet through
            // attach building.
		if (flag->m_building) {
               assert(os->is_object_known(flag->m_building));
               fw.Unsigned32(os->get_object_file_index(flag->m_building));
		} else fw.Unsigned32(0);

            // Roads are not saved, they are set on load

            // Pending items
		for (uint32_t i = 0; i < 6; ++i)
                  fw.Unsigned32(flag->m_items_pending[i]);

            // Capacity
            fw.Unsigned32(flag->m_item_capacity);

            // Items filled
            fw.Unsigned32(flag->m_item_filled);

            // items
		for (int32_t i = 0; i < flag->m_item_filled; ++i) {
               fw.Unsigned8(flag->m_items[i].pending);
               assert(os->is_object_known(flag->m_items[i].item));
               fw.Unsigned32(os->get_object_file_index(flag->m_items[i].item));
			fw.Unsigned32
				(os->is_object_known      (flag->m_items[i].nextstep) ?
				 os->get_object_file_index(flag->m_items[i].nextstep) : 0);
		}

            // always call
		if (flag->m_always_call_for_flag) {
               assert(os->is_object_known(flag->m_always_call_for_flag));
               fw.Unsigned32(os->get_object_file_index(flag->m_always_call_for_flag));
		} else fw.Unsigned32(0);

            // Worker waiting for capacity
		const std::vector<Object_Ptr> & capacity_wait = flag->m_capacity_wait;
		const std::vector<Object_Ptr>::const_iterator capacity_wait_end =
			capacity_wait.end();
		fw.Unsigned16(capacity_wait.size());
		for
			(std::vector<Object_Ptr>::const_iterator it = capacity_wait.begin();
			 it != capacity_wait_end;
			 ++it)
		{
			const Map_Object * const obj = it->get(egbase);
               assert(os->is_object_known(obj));
               fw.Unsigned32(os->get_object_file_index(obj));
		}

            // Flag jobs
		const std::list<Flag::FlagJob> & flag_jobs = flag->m_flag_jobs;
		const std::list<Flag::FlagJob>::const_iterator flag_jobs_end =
			flag_jobs.end();
		fw.Unsigned16(flag_jobs.size());
		for
			(std::list<Flag::FlagJob>::const_iterator it = flag_jobs.begin();
			 it != flag_jobs_end;
			 ++it)
		{
			if (it->request) {
                  fw.Unsigned8(1);
				it->request->Write(&fw, egbase, os);
			} else fw.Unsigned8(0);


			fw.String(it->program);
		}

            os->mark_object_as_saved(flag);

	}

   fw.Unsigned32(0xffffffff); // End of flags

   fw.Write(fs, "binary/flag_data");
}

};
