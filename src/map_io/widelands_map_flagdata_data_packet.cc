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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "widelands_map_flagdata_data_packet.h"

#include "logic/building.h"
#include "economy/flag.h"
#include "economy/request.h"
#include "economy/ware_instance.h"
#include "logic/game.h"
#include "logic/map.h"
#include "logic/player.h"
#include "upcast.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"
#include "logic/worker.h"

#include <map>

namespace Widelands {

#define CURRENT_PACKET_VERSION 3

void Map_Flagdata_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader &       mol)
throw (_wexception)
{
	if (skip)
		return;

	FileRead fr;
	try {fr.Open(fs, "binary/flag_data");} catch (...) {return;}

	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (1 <= packet_version and packet_version <= CURRENT_PACKET_VERSION) {
			Map const  & map    = egbase.map();
			Extent const extent = map.extent();
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
					Flag & flag = mol.get<Flag>(serial);

					//  Owner is already set, nothing to do from PlayerImmovable.

					if (packet_version < 3) {
						if
							(upcast
							 	(Flag const,
							 	 mf,
							 	 map[flag.m_position = fr.Coords32(extent)]
							 	 .get_immovable()))
						{
							if (mf != &flag)
								throw game_data_error
									(_("wrong flag (%u) at given position (%i, %i)"),
									 mf->serial(),
									 flag.m_position.x, flag.m_position.y);
						} else
							throw game_data_error
								(_("no flag at given position (%i, %i)"),
								 flag.m_position.x, flag.m_position.y);
					}
					flag.m_animstart = fr.Unsigned16();

					{
						FCoords building_position = map.get_fcoords(flag.m_position);
						map.get_tln(building_position, &building_position);
						flag.m_building =
							dynamic_cast<Building *>
								(building_position.field->get_immovable());
					}
					if (packet_version < 3) {
						if (uint32_t const building_serial = fr.Unsigned32())
							try {
								Building const & building =
									mol.get<Building>(building_serial);
								if (flag.m_building != &building)
									throw game_data_error
										(_
										 	("has building %u at (%i, %i), which is not "
										 	 "at the top left node"),
										 building_serial,
										 building.get_position().x,
										 building.get_position().y);
							} catch (_wexception const & e) {
								throw game_data_error
									(_("building (%u): %s"), building_serial, e.what());
							}
						else
							flag.m_building = 0;
					}

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
									&mol.get<WareInstance>(item_serial);

								if (uint32_t const nextstep_serial = fr.Unsigned32()) {
									try {
										flag.m_items[i].nextstep =
											&mol.get<PlayerImmovable>(nextstep_serial);
									} catch (_wexception const & e) {
										throw game_data_error
											("next step (%u): %s",
											 nextstep_serial, e.what());
									}
								} else
									flag.m_items[i].nextstep = 0;
							} catch (_wexception const & e) {
								throw game_data_error
									("item #%u (%u): %s", i, item_serial, e.what());
							}
						}

						if (uint32_t const always_call_serial = fr.Unsigned32())
							try {
								flag.m_always_call_for_flag =
									&mol.get<Flag>(always_call_serial);
							} catch (_wexception const & e) {
								throw game_data_error
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
								//  The check that this worker actually has a
								//  waitforcapacity task for this flag is in
								//  Flag::load_finish, which is called after the worker
								//  (with his stack of tasks) has been fully loaded.
								flag.m_capacity_wait.push_back
									(&mol.get<Worker>(worker_serial));
							} catch (_wexception const & e) {
								throw game_data_error
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
									(fr, ref_cast<Game, Editor_Game_Base>(egbase), mol);
							} else {
								f.request = 0;
							}
							f.program = fr.CString();
							flag.m_flag_jobs.push_back(f);
						}

						mol.mark_object_as_loaded(flag);
					}
				} catch (_wexception const & e) {
					throw game_data_error(_("%u: %s"), serial, e.what());
				}
			}
		} else
			throw game_data_error
				(_("unknown/unhandled version %u"), packet_version);
	} catch (_wexception const & e) {
		throw game_data_error(_("flagdata: %s"), e.what());
	}
}


void Map_Flagdata_Data_Packet::Write
	(FileSystem & fs, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
	throw (_wexception)
{
	FileWrite fw;

	fw.Unsigned16(CURRENT_PACKET_VERSION);

	Map const & map = egbase.map();
	const Field & fields_end = map[map.max_index()];
	for (Field * field = &map[0]; field < &fields_end; ++field)
		if (upcast(Flag const, flag, field->get_immovable())) {
			assert(mos.is_object_known(*flag));
			assert(!mos.is_object_saved(*flag));

			fw.Unsigned32(mos.get_object_file_index(*flag));

			//  Owner is already written in the existanz packet.

			//  Animation is set by creator.
			fw.Unsigned16(flag->m_animstart);

			//  Roads are not saved, they are set on load.

			for (uint32_t i = 0; i < 6; ++i)
				fw.Unsigned32(flag->m_items_pending[i]);

			fw.Unsigned32(flag->m_item_capacity);

			fw.Unsigned32(flag->m_item_filled);

			for (int32_t i = 0; i < flag->m_item_filled; ++i) {
				fw.Unsigned8(flag->m_items[i].pending);
				assert(mos.is_object_known(*flag->m_items[i].item));
				fw.Unsigned32(mos.get_object_file_index(*flag->m_items[i].item));
				if
					(PlayerImmovable const * const nextstep =
					 	flag->m_items[i].nextstep.get(egbase))
					fw.Unsigned32(mos.get_object_file_index(*nextstep));
				else
					fw.Unsigned32(0);
			}

			if (Flag const * const always_call_for = flag->m_always_call_for_flag)
			{
				assert(mos.is_object_known(*always_call_for));
				fw.Unsigned32(mos.get_object_file_index(*always_call_for));
			} else
				fw.Unsigned32(0);

			//  worker waiting for capacity
			Flag::CapacityWaitQueue const & capacity_wait =
				flag->m_capacity_wait;
			fw.Unsigned16(capacity_wait.size());
			container_iterate_const(Flag::CapacityWaitQueue, capacity_wait, i) {
				Worker const * const obj = i.current->get(egbase);
				assert
					(obj);
				assert
					(obj->get_state(Worker::taskWaitforcapacity));
				assert
					(obj->get_state(Worker::taskWaitforcapacity)->objvar1.serial()
					 ==
					 flag                                               ->serial());
				assert(mos.is_object_known(*obj));
				fw.Unsigned32(mos.get_object_file_index(*obj));
			}
			Flag::FlagJobs const & flag_jobs = flag->m_flag_jobs;
			fw.Unsigned16(flag_jobs.size());
			container_iterate_const(Flag::FlagJobs, flag_jobs, i) {
				if (i.current->request) {
					fw.Unsigned8(1);
					i.current->request->Write
						(fw, ref_cast<Game, Editor_Game_Base>(egbase), mos);
				} else
					fw.Unsigned8(0);


				fw.String(i.current->program);
			}

			mos.mark_object_as_saved(*flag);

		}

	fw.Write(fs, "binary/flag_data");
}

}
