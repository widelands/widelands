/*
 * Copyright (C) 2002-2004, 2006-2008, 2010 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "map_io/widelands_map_roaddata_data_packet.h"

#include <map>

#include "economy/flag.h"
#include "economy/request.h"
#include "economy/road.h"
#include "logic/carrier.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/map.h"
#include "logic/player.h"
#include "logic/tribe.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "map_io/widelands_map_map_object_saver.h"
#include "upcast.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 4

void Map_Roaddata_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader &       mol)
{
	if (skip)
		return;

	FileRead fr;
	try {fr.Open(fs, "binary/road_data");} catch (...) {return;}

	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (1 <= packet_version and packet_version <= CURRENT_PACKET_VERSION) {
			const Map   &       map        = egbase.map();
			Player_Number const nr_players = map.get_nrplayers();
			for (;;) {
				if (2 <= packet_version and fr.EndOfFile())
					break;
				Serial const serial = fr.Unsigned32();
				if (packet_version < 2 and serial == 0xffffffff) {
					if (not fr.EndOfFile())
						throw game_data_error
							("expected end of file after serial 0xffffffff");
					break;
				}
				try {
					Road & road = mol.get<Road>(serial);
					if (mol.is_object_loaded(road))
						throw game_data_error("already loaded");
					Player & plr = egbase.player(fr.Player_Number8(nr_players));

					road.set_owner(&plr);
					if (4 <= packet_version) {
						road.m_busyness             = fr.Unsigned32();
						road.m_busyness_last_update = fr.Unsigned32();
					}
					road.m_type = fr.Unsigned32();
					{
						uint32_t const flag_0_serial = fr.Unsigned32();
						try {
							road.m_flags[0] = &mol.get<Flag>(flag_0_serial);
						} catch (const _wexception & e) {
							throw game_data_error
								("flag 0 (%u): %s", flag_0_serial, e.what());
						}
					}
					{
						uint32_t const flag_1_serial = fr.Unsigned32();
						try {
							road.m_flags[1] = &mol.get<Flag>(flag_1_serial);
						} catch (const _wexception & e) {
							throw game_data_error
								("flag 1 (%u): %s", flag_1_serial, e.what());
						}
					}
					road.m_flagidx[0] = fr.Unsigned32();
					road.m_flagidx[1] = fr.Unsigned32();

					road.m_cost[0] = fr.Unsigned32();
					road.m_cost[1] = fr.Unsigned32();
					Path::Step_Vector::size_type const nr_steps = fr.Unsigned16();
					if (not nr_steps)
						throw game_data_error("nr_steps = 0");
					Path p(road.m_flags[0]->get_position());
					for (Path::Step_Vector::size_type i = nr_steps; i; --i)
						try {
							p.append(egbase.map(), fr.Direction8());
						} catch (const _wexception & e) {
							throw game_data_error
								("step #%lu: %s",
								 static_cast<long unsigned int>(nr_steps - i),
								 e.what());
						}
					road._set_path(egbase, p);

					//  Now that all rudimentary data is set, init this road. Then
					//  overwrite the initialization values.
					road._link_into_flags(ref_cast<Game, Editor_Game_Base>(egbase));

					road.m_idle_index      = fr.Unsigned32();

					uint32_t const count = fr.Unsigned32();
					if (not count)
						throw game_data_error("no carrier slot");
					if (packet_version <= 2 and 1 < count)
						throw game_data_error
							(
						 	 "expected 1 but found %u carrier slots in road saved "
						 	 "with packet version 2 (old)",
							 count);

					for (uint32_t i = 0; i < count; ++i) {
						Carrier * carrier = nullptr;
						Request * carrier_request = nullptr;


						if (uint32_t const carrier_serial = fr.Unsigned32())
							try {
								//log("Read carrier serial %u", carrier_serial);
								carrier = &mol.get<Carrier>(carrier_serial);
							} catch (const _wexception & e) {
								throw game_data_error
									("carrier (%u): %s", carrier_serial, e.what());
							}
						else {
							carrier = nullptr;
							//log("No carrier in this slot");
						}

						//delete road.m_carrier_slots[i].carrier_request;
						//carrier_request = 0;

						if (fr.Unsigned8()) {
							//log("Reading request");
							(carrier_request =
							 	new Request
							 		(road,
							 		 Ware_Index::First(),
							 		 Road::_request_carrier_callback,
							 		 wwWORKER))
							->Read(fr, ref_cast<Game, Editor_Game_Base>(egbase), mol);
						} else {
							carrier_request = nullptr;
							//log("No request in this slot");
						}
						uint8_t const carrier_type =
							packet_version < 3 ? 1 : fr.Unsigned32();

						if
							(i < road.m_carrier_slots.size() and
							 road.m_carrier_slots[i].carrier_type == carrier_type)
						{
							assert(!road.m_carrier_slots[i].carrier.get(egbase));

							road.m_carrier_slots[i].carrier = carrier;
							if (carrier or carrier_request) {
								delete road.m_carrier_slots[i].carrier_request;
								road.m_carrier_slots[i].carrier_request =
									carrier_request;
							}
						} else {
							delete carrier_request;
							if (carrier) {
								//carrier->set_location (0);
								carrier->reset_tasks
									(ref_cast<Game,
									 Editor_Game_Base>(egbase));
								//carrier->send_signal
								//(ref_cast<Game,
								//Editor_Game_Base>(egbase), "location");
							}
						}
					}

					mol.mark_object_as_loaded(road);
				} catch (const _wexception & e) {
					throw game_data_error("road %u: %s", serial, e.what());
				}
			}
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("roaddata: %s", e.what());
	}
}


void Map_Roaddata_Data_Packet::Write
	(FileSystem & fs, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	FileWrite fw;

	fw.Unsigned16(CURRENT_PACKET_VERSION);

	const Map   & map        = egbase.map();
	const Field & fields_end = map[map.max_index()];
	for (Field const * field = &map[0]; field < &fields_end; ++field)
		if (upcast(Road const, r, field->get_immovable()))
			if (not mos.is_object_saved(*r)) {
				assert(mos.is_object_known(*r));

				fw.Unsigned32(mos.get_object_file_index(*r));

				//  First, write PlayerImmovable Stuff
				//  Theres only the owner
				fw.Unsigned8(r->owner().player_number());

				fw.Unsigned32(r->m_busyness);
				fw.Unsigned32(r->m_busyness_last_update);

				fw.Unsigned32(r->m_type);

				//  serial of flags
				assert(mos.is_object_known(*r->m_flags[0]));
				assert(mos.is_object_known(*r->m_flags[1]));
				fw.Unsigned32(mos.get_object_file_index(*r->m_flags[0]));
				fw.Unsigned32(mos.get_object_file_index(*r->m_flags[1]));

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


				fw.Unsigned32(r->m_carrier_slots.size());

				container_iterate_const
					(std::vector<Road::CarrierSlot>, r->m_carrier_slots, iter)
				{

					if
						(Carrier const * const carrier =
						 iter.current->carrier.get(egbase))
					{
						assert(mos.is_object_known(*carrier));
						fw.Unsigned32(mos.get_object_file_index(*carrier));
					} else {
						fw.Unsigned32(0);
					}

					if (iter.current->carrier_request) {
						fw.Unsigned8(1);
						iter.current->carrier_request->Write
							(fw, ref_cast<Game, Editor_Game_Base>(egbase), mos);
					} else {
						fw.Unsigned8(0);
					}
					fw.Unsigned32(iter.current->carrier_type);
				}
				mos.mark_object_as_saved(*r);
			}

	fw.Write(fs, "binary/road_data");
}
}
