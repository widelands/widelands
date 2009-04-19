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

#include "widelands_map_ware_data_packet.h"

#include "economy/flag.h"
#include "economy/ware_instance.h"
#include "editor_game_base.h"
#include "map.h"
#include "player.h"
#include "tribe.h"
#include "upcast.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"
#include "worker.h"

#include <map>

namespace Widelands {

#define CURRENT_PACKET_VERSION 1


void Map_Ware_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader * const ol)
throw (_wexception)
{
	if (skip)
		return;

	FileRead fr;
	try {fr.Open(fs, "binary/ware");} catch (...) {return;}

	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CURRENT_PACKET_VERSION) {
			//  now the rest data len
			uint32_t const nr_files = fr.Unsigned32();
			for (uint32_t i = 0; i < nr_files; ++i) {
				//  data is read elsewhere
				Serial const serial = fr.Unsigned32();
				try {
					ol->register_object<WareInstance>
						(serial,
						 *new WareInstance(Ware_Index::Null(), 0))
						.init(egbase);
				} catch (_wexception const & e) {
					throw wexception("%u: %s", serial, e.what());
				}
			}
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("ware data: %s", e.what());
	}
}


void Map_Ware_Data_Packet::Write
	(FileSystem           &       fs,
	 Editor_Game_Base     &       egbase,
	 Map_Map_Object_Saver * const os)
throw (_wexception)
{

	FileWrite fw;

	fw.Unsigned16(CURRENT_PACKET_VERSION);

	//  Traverse the map and whenever we find a suitable object, check if it has
	//  wares.
	Map const & map = egbase.map();
	std::vector<uint32_t> ids;
	Field * field = &map[0];
	const Field * const fields_end = field + map.max_index();
	for (; field < fields_end; ++field) {

		//  check for flags
		if (upcast(Flag const, flag, field->get_immovable())) {
			const Flag::PendingItem * item = flag->m_items;
			const Flag::PendingItem & items_end =  *(item + flag->m_item_filled);
			for (; item < &items_end; ++item) {
				assert(not os->is_object_known(*item->item));
				ids.push_back(os->register_object(*item->item));
			}
		}

		//  check for workers
		for (Bob * b = field->get_first_bob(); b; b = b->get_next_bob())
			if (upcast(Worker, w, b))
				if (const WareInstance * const ware = w->get_carried_item(egbase))
				{
					assert(!os->is_object_known(*ware));
					ids.push_back(os->register_object(*ware));
				}
	}

	//  All checked, we only need to save those stuff to disk.
	fw.Unsigned32(ids.size());
	for (uint32_t i = 0; i < ids.size(); ++i)
		fw.Unsigned32(ids[i]);

	fw.Write(fs, "binary/ware");
}

};
