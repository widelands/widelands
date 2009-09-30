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

#include "logic/building.h"
#include "economy/flag.h"
#include "economy/request.h"
#include "economy/ware_instance.h"
#include "logic/game.h"
#include "map.h"
#include "logic/player.h"
#include "upcast.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"
#include "logic/worker.h"

#include <map>

namespace Widelands {

#define CURRENT_PACKET_VERSION 2

void Map_Flagdata_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader * const ol)
throw (_wexception)
{
	if (skip)
		return;

	FileRead fr;
	try {fr.Open(fs, "binary/flag_data");} catch (...) {return;}

	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (1 <= packet_version and packet_version <= CURRENT_PACKET_VERSION) {
			Extent const extent = egbase.map().extent();
			for (;;) {
				if (2 <= packet_version and fr.EndOfFile())
					break;
				Serial const serial = fr.Unsigned32();
				if (packet_version < 2 and serial == 0xffffffff) {
					if (not fr.EndOfFile())
						throw wexception
							("expected end of file after serial 0xffffffff");
					break;
				}
				try {
					Flag & flag = ol->get<Flag>(serial);

					//  Owner is already set, nothing to do from PlayerImmovable.

					flag.m_position = fr.Coords32(extent);
					flag.m_animstart = fr.Unsigned16();

					//  FIXME This should not be explicitly read from file. A flag's
					//  FIXME building is always the one to the northwest (if any).
					//  FIXME There is no way that a flag could have a building
					//  FIXME somewhere else or not have the building to the
					//  FIXME northwest.
					if (int32_t const building_serial = fr.Unsigned32())
						try {
							flag.m_building = &ol->get<Building>(building_serial);
						} catch (_wexception const & e) {
							throw wexception
								("building (%u): %s", building_serial, e.what());
						}
					else
						flag.m_building = 0;


					//  Roads are set somewhere else.

					for (uint32_t i = 0; i < 6; ++i)
						flag.m_items_pending[i] = fr.Unsigned32();
					flag.m_item_capacity = fr.Unsigned32();

					{
						uint32_t const items_filled = fr.Unsigned32();
						flag.m_item_filled = items_filled;
						for (uint32_t i = 0; i < items_filled; ++i) {
							flag.m_items[i].pending = fr.Unsigned8();
							uint32_t const item_serial = fr.Unsigned32();
							try {
								flag.m_items[i].item =
									&ol->get<WareInstance>(item_serial);

								if (uint32_t const nextstep_serial = fr.Unsigned32()) {
									try {
										flag.m_items[i].nextstep =
											&ol->get<PlayerImmovable>(nextstep_serial);
									} catch (_wexception const & e) {
										throw wexception
											("next step (%u): %s",
											 nextstep_serial, e.what());
									}
								} else
									flag.m_items[i].nextstep = 0;
							} catch (_wexception const & e) {
								throw wexception
									("item #%u (%u): %s", i, item_serial, e.what());
							}
						}

						if (uint32_t const always_call_serial = fr.Unsigned32())
							try {
								flag.m_always_call_for_flag =
									&ol->get<Flag>(always_call_serial);
							} catch (_wexception const & e) {
								throw wexception
									("always_call (%u): %s",
									 always_call_serial, e.what());
							}
						else
							flag.m_always_call_for_flag = 0;

						//  workers waiting
						uint16_t const nr_workers = fr.Unsigned16();
						for (uint32_t i = 0; i < nr_workers; ++i) {
							uint32_t const worker_serial = fr.Unsigned32();
							try {
								// Hack to support old savegames.
								// See the corresponding section in Write().
								if (worker_serial)
									flag.m_capacity_wait.push_back
										(&ol->get<Worker>(worker_serial));
							} catch (_wexception const & e) {
								throw wexception
									("worker #%u (%u): %s", i, worker_serial, e.what());
							}
						}

						//  flag jobs
						uint16_t const nr_jobs = fr.Unsigned16();
						assert(flag.m_flag_jobs.empty());
						for (uint16_t i = 0; i < nr_jobs; ++i) {
							Flag::FlagJob f;
							if (fr.Unsigned8()) {
								f.request =
									new Request
										(flag,
										 Ware_Index::First(),
										 Flag::flag_job_request_callback,
										 Request::WORKER);
								f.request->Read
									(fr, ref_cast<Game, Editor_Game_Base>(egbase), ol);
							} else {
								f.request = 0;
							}
							f.program = fr.CString();
							flag.m_flag_jobs.push_back(f);
						}

						ol->mark_object_as_loaded(&flag);
					}
				} catch (_wexception const & e) {
					throw wexception("%u: %s", serial, e.what());
				}
			}
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("flagdata: %s", e.what());
	}
}


void Map_Flagdata_Data_Packet::Write
	(FileSystem           &       fs,
	 Editor_Game_Base     &       egbase,
	 Map_Map_Object_Saver * const os)
	throw (_wexception)
{
	FileWrite fw;

	fw.Unsigned16(CURRENT_PACKET_VERSION);

	Map const & map = egbase.map();
	const Field & fields_end = map[map.max_index()];
	for (Field * field = &map[0]; field < &fields_end; ++field)
		if (upcast(Flag const, flag, field->get_immovable())) {
			assert(os->is_object_known(*flag));
			assert(!os->is_object_saved(*flag));

			fw.Unsigned32(os->get_object_file_index(*flag));

			//  Owner is already written in the existanz packet.

			fw.Coords32  (flag->m_position);

			//  Animation is set by creator.
			fw.Unsigned16(flag->m_animstart);

			//  FIXME This should not be explicitly written to file. A flag's
			//  FIXME building is always the one to the northwest (if any). There
			//  FIXME is no way that a flag could have a building somewhere else
			//  FIXME or not have the building to the northwest.
			//  Building is not used, it is set by Building_Data packet through
			//  attach building.
			if (Building const * const building = flag->m_building) {
				assert(os->is_object_known(*building));
				fw.Unsigned32(os->get_object_file_index(*building));
			} else
				fw.Unsigned32(0);

			//  Roads are not saved, they are set on load.

			for (uint32_t i = 0; i < 6; ++i)
				fw.Unsigned32(flag->m_items_pending[i]);

			fw.Unsigned32(flag->m_item_capacity);

			fw.Unsigned32(flag->m_item_filled);

			for (int32_t i = 0; i < flag->m_item_filled; ++i) {
				fw.Unsigned8(flag->m_items[i].pending);
				assert(os->is_object_known(*flag->m_items[i].item));
				fw.Unsigned32(os->get_object_file_index(*flag->m_items[i].item));
				if
					(PlayerImmovable const * const nextstep =
					 	flag->m_items[i].nextstep.get(egbase))
					fw.Unsigned32(os->get_object_file_index(*nextstep));
				else
					fw.Unsigned32(0);
			}

			if (Flag const * const always_call_for = flag->m_always_call_for_flag)
			{
				assert(os->is_object_known(*always_call_for));
				fw.Unsigned32(os->get_object_file_index(*always_call_for));
			} else
				fw.Unsigned32(0);

			//  worker waiting for capacity
			std::vector<OPtr<Worker> > const & capacity_wait =
				flag->m_capacity_wait;
			fw.Unsigned16(capacity_wait.size());
			container_iterate_const(std::vector<OPtr<Worker> >, capacity_wait, i)
			{
				Worker const * const obj = i.current->get(egbase);
				//  This is a very crude hack to support old and broken savegames,
				//  where workers were not correctly removed from the capacity wait
				//  queue. See bug #1919495.
				if (obj && obj->get_state(Worker::taskWaitforcapacity)) {
					assert(os->is_object_known(*obj));
					fw.Unsigned32(os->get_object_file_index(*obj));
				} else {
					fw.Unsigned32(0);
				}
			}
			Flag::FlagJobs const & flag_jobs = flag->m_flag_jobs;
			fw.Unsigned16(flag_jobs.size());
			container_iterate_const(Flag::FlagJobs, flag_jobs, i) {
				if (i.current->request) {
					fw.Unsigned8(1);
					i.current->request->Write
						(fw, ref_cast<Game, Editor_Game_Base>(egbase), os);
				} else
					fw.Unsigned8(0);


				fw.String(i.current->program);
			}

			os->mark_object_as_saved(*flag);

		}

	fw.Write(fs, "binary/flag_data");
}

}
