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

#include "widelands_map_roaddata_data_packet.h"

#include "economy/flag.h"
#include "economy/request.h"
#include "economy/road.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "map.h"
#include "logic/player.h"
#include "tribe.h"
#include "upcast.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"

#include <map>

namespace Widelands {

#define CURRENT_PACKET_VERSION 1


void Map_Roaddata_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader * const ol)
throw (_wexception)
{
	if (skip)
		return;

	FileRead fr;
	try {fr.Open(fs, "binary/road_data");} catch (...) {return;}

	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CURRENT_PACKET_VERSION) {
			Map   const &       map        = egbase.map();
			Extent        const extent     = map.extent       ();
			Player_Number const nr_players = map.get_nrplayers();
			for (;;) {
				Serial const serial = fr.Unsigned32();
				//  FIXME Just test EndOfFile instead in the next packet version.
				if (serial == 0xffffffff) {
					if (not fr.EndOfFile())
						throw wexception
							("expected end of file after serial 0xffffffff");
					break;
				}
				try {
					Road & road = ol->get<Road>(serial);
					if (ol->is_object_loaded(&road))
						throw wexception("already loaded");
					Player & plr = egbase.player(fr.Player_Number8(nr_players));

					road.set_owner(&plr);
					road.m_type = fr.Unsigned32();
					{
						uint32_t const flag_0_serial = fr.Unsigned32();
						try {
							road.m_flags[0] = &ol->get<Flag>(flag_0_serial);
						} catch (_wexception const & e) {
							throw wexception
								("flag 0 (%u): %s", flag_0_serial, e.what());
						}
					}
					{
						uint32_t const flag_1_serial = fr.Unsigned32();
						try {
							road.m_flags[1] = &ol->get<Flag>(flag_1_serial);
						} catch (_wexception const & e) {
							throw wexception
								("flag 1 (%u): %s", flag_1_serial, e.what());
						}
					}
					road.m_flagidx[0] = fr.Unsigned32();
					road.m_flagidx[1] = fr.Unsigned32();

					road.m_cost[0] = fr.Unsigned32();
					road.m_cost[1] = fr.Unsigned32();
					Path::Step_Vector::size_type const nr_steps = fr.Unsigned16();
					if (not nr_steps)
						throw wexception("nr_steps = 0");
					Path p(road.m_flags[0]->get_position());
					for (Path::Step_Vector::size_type i = nr_steps; i; --i)
						try {
							p.append(egbase.map(), fr.Direction8());
						} catch (_wexception const & e) {
							throw wexception
								("step #%lu: %s",
								 static_cast<long unsigned int>(nr_steps - i),
								 e.what());
						}
					road._set_path(egbase, p);

					//  Now that all rudimentary data is set, init this road. Then
					//  overwrite the initialization values.
					road._link_into_flags(egbase);

					road.m_idle_index      = fr.Unsigned32();
					road.m_desire_carriers = fr.Unsigned32();
					assert(!road.m_carrier.get(egbase));
					if (uint32_t const carrier_serial = fr.Unsigned32())
						try {
							road.m_carrier = &ol->get<Map_Object>(carrier_serial);
						} catch (_wexception const & e) {
							throw wexception
								("carrier (%u): %s", carrier_serial, e.what());
						}
					else
						road.m_carrier = 0;

					delete road.m_carrier_request; road.m_carrier_request = 0;

					if (fr.Unsigned8()) {
						if (dynamic_cast<Game const *>(&egbase))
							(road.m_carrier_request =
							 	new Request
							 		(road,
							 		 Ware_Index::First(),
							 		 Road::_request_carrier_callback,
							 		 Request::WORKER))
							->Read(fr, egbase, ol);
					} else
						road.m_carrier_request = 0;

					ol->mark_object_as_loaded(&road);
				} catch (_wexception const & e) {
					throw wexception("road %u: %s", serial, e.what());
				}
			}
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("roaddata: %s", e.what());
	}
}


void Map_Roaddata_Data_Packet::Write
	(FileSystem           &       fs,
	 Editor_Game_Base     &       egbase,
	 Map_Map_Object_Saver * const os)
throw (_wexception)
{
	FileWrite fw;

	fw.Unsigned16(CURRENT_PACKET_VERSION);

	Map   const & map        = egbase.map();
	Field const & fields_end = map[map.max_index()];
	for (Field const * field = &map[0]; field < &fields_end; ++field)
		if (upcast(Road const, r, field->get_immovable()))
			if (not os->is_object_saved(*r)) {
				assert(os->is_object_known(*r));

				fw.Unsigned32(os->get_object_file_index(*r));

				//  First, write PlayerImmovable Stuff
				//  Theres only the owner
				fw.Unsigned8(r->owner().get_player_number());

				fw.Unsigned32(r->m_type);

				//  serial of flags
				assert(os->is_object_known(*r->m_flags[0]));
				assert(os->is_object_known(*r->m_flags[1]));
				fw.Unsigned32(os->get_object_file_index(*r->m_flags[0]));
				fw.Unsigned32(os->get_object_file_index(*r->m_flags[1]));

				fw.Unsigned32(r->m_flagidx[0]);
				fw.Unsigned32(r->m_flagidx[1]);

				fw.Unsigned32(r->m_cost[0]);
				fw.Unsigned32(r->m_cost[1]);

				const Path & path = r->m_path;
				const Path::Step_Vector::size_type nr_steps = path.get_nsteps();
				fw.Unsigned16(nr_steps);
				for (Path::Step_Vector::size_type i = 0; i < nr_steps; ++i)
					fw.Unsigned8(path[i]);

				fw.Unsigned32(r->m_idle_index); //  FIXME do not save this

				fw.Unsigned32(r->m_desire_carriers);

				if (r->m_carrier.get(egbase)) {
					assert(os->is_object_known(*r->m_carrier.get(egbase)));
					fw.Unsigned32
						(os->get_object_file_index(*r->m_carrier.get(egbase)));
				} else
					fw.Unsigned32(0);

				if (r->m_carrier_request) {
					fw.Unsigned8(1);
					r->m_carrier_request->Write(fw, egbase, os);
				} else
					fw.Unsigned8(0);

				os->mark_object_as_saved(*r);
			}

	fw.Unsigned32(0xFFFFFFFF); // End of roads

	fw.Write(fs, "binary/road_data");
}

};
