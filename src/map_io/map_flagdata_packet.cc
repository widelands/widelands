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

#include "map_io/map_flagdata_packet.h"

#include <map>

#include "base/macros.h"
#include "economy/flag.h"
#include "economy/request.h"
#include "economy/ware_instance.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/building.h"
#include "logic/game.h"
#include "logic/map.h"
#include "logic/player.h"
#include "logic/widelands_geometry_io.h"
#include "logic/worker.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 4;

void MapFlagdataPacket::read
	(FileSystem            &       fs,
	 EditorGameBase      &       egbase,
	 bool                    const skip,
	 MapObjectLoader &       mol)
{
	if (skip)
		return;

	FileRead fr;
	try {fr.open(fs, "binary/flag_data");} catch (...) {return;}

	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			const Map  & map    = egbase.map();
			while (! fr.end_of_file()) {
				Serial const serial = fr.unsigned_32();
				try {
					Flag & flag = mol.get<Flag>(serial);

					//  Owner is already set, nothing to do from PlayerImmovable.

					flag.m_animstart = fr.unsigned_16();

					{
						FCoords building_position = map.get_fcoords(flag.m_position);
						map.get_tln(building_position, &building_position);
						flag.m_building =
							dynamic_cast<Building *>
								(building_position.field->get_immovable());
					}

					//  Roads are set somewhere else.

					// Compatibility stuff: Read 6 bytes of attic stuff
					// that is no longer used (was m_wares_pending)
					for (uint32_t i = 0; i < 6; ++i)
						fr.unsigned_32();

					flag.m_ware_capacity = fr.unsigned_32();

					{
						uint32_t const wares_filled = fr.unsigned_32();
						flag.m_ware_filled = wares_filled;
						for (uint32_t i = 0; i < wares_filled; ++i) {
							flag.m_wares[i].pending = fr.unsigned_8();
							flag.m_wares[i].priority = fr.signed_32();
							uint32_t const ware_serial = fr.unsigned_32();
							try {
								flag.m_wares[i].ware =
									&mol.get<WareInstance>(ware_serial);

								if (uint32_t const nextstep_serial = fr.unsigned_32()) {
									try {
										flag.m_wares[i].nextstep =
											&mol.get<PlayerImmovable>(nextstep_serial);
									} catch (const WException & e) {
										throw GameDataError
											("next step (%u): %s",
											 nextstep_serial, e.what());
									}
								} else
									flag.m_wares[i].nextstep = nullptr;
							} catch (const WException & e) {
								throw GameDataError
									("ware #%u (%u): %s", i, ware_serial, e.what());
							}
						}

						if (uint32_t const always_call_serial = fr.unsigned_32()) {
							try {
								flag.m_always_call_for_flag =
									&mol.get<Flag>(always_call_serial);
							} catch (const WException & e) {
								throw GameDataError
									("always_call (%u): %s",
									 always_call_serial, e.what());
							}
						} else
							flag.m_always_call_for_flag = nullptr;

						//  workers waiting
						uint16_t const nr_workers = fr.unsigned_16();
						for (uint32_t i = 0; i < nr_workers; ++i) {
							uint32_t const worker_serial = fr.unsigned_32();
							try {
								//  The check that this worker actually has a
								//  waitforcapacity task for this flag is in
								//  Flag::load_finish, which is called after the worker
								//  (with his stack of tasks) has been fully loaded.
								flag.m_capacity_wait.push_back
									(&mol.get<Worker>(worker_serial));
							} catch (const WException & e) {
								throw GameDataError
									("worker #%u (%u): %s", i, worker_serial, e.what());
							}
						}

						//  flag jobs
						uint16_t const nr_jobs = fr.unsigned_16();
						assert(flag.m_flag_jobs.empty());
						for (uint16_t i = 0; i < nr_jobs; ++i) {
							Flag::FlagJob f;
							if (fr.unsigned_8()) {
								f.request =
									new Request
										(flag,
										 0,
										 Flag::flag_job_request_callback,
										 wwWORKER);
								f.request->read
									(fr, dynamic_cast<Game&>(egbase), mol);
							} else {
								f.request = nullptr;
							}
							f.program = fr.c_string();
							flag.m_flag_jobs.push_back(f);
						}

						mol.mark_object_as_loaded(flag);
					}
				} catch (const WException & e) {
					throw GameDataError("%u: %s", serial, e.what());
				}
			}
		} else {
			throw UnhandledVersionError(packet_version, kCurrentPacketVersion);
		}
	} catch (const WException & e) {
		throw GameDataError("flagdata: %s", e.what());
	}
}


void MapFlagdataPacket::write
	(FileSystem & fs, EditorGameBase & egbase, MapObjectSaver & mos)

{
	FileWrite fw;

	fw.unsigned_16(kCurrentPacketVersion);

	const Map & map = egbase.map();
	const Field & fields_end = map[map.max_index()];
	for (Field * field = &map[0]; field < &fields_end; ++field)
		if (upcast(Flag const, flag, field->get_immovable())) {
			assert(mos.is_object_known(*flag));
			assert(!mos.is_object_saved(*flag));

			fw.unsigned_32(mos.get_object_file_index(*flag));

			//  Owner is already written in the existanz packet.

			//  Animation is set by creator.
			fw.unsigned_16(flag->m_animstart);

			//  Roads are not saved, they are set on load.

			// Compatibility stuff: Write 6 bytes of attic stuff that is
			// no longer used (was m_wares_pending)
			for (uint32_t i = 0; i < 6; ++i)
				fw.unsigned_32(0);

			fw.unsigned_32(flag->m_ware_capacity);

			fw.unsigned_32(flag->m_ware_filled);

			for (int32_t i = 0; i < flag->m_ware_filled; ++i) {
				fw.unsigned_8(flag->m_wares[i].pending);
				fw.signed_32(flag->m_wares[i].priority);
				assert(mos.is_object_known(*flag->m_wares[i].ware));
				fw.unsigned_32(mos.get_object_file_index(*flag->m_wares[i].ware));
				if
					(PlayerImmovable const * const nextstep =
					 	flag->m_wares[i].nextstep.get(egbase))
					fw.unsigned_32(mos.get_object_file_index(*nextstep));
				else
					fw.unsigned_32(0);
			}

			if (Flag const * const always_call_for = flag->m_always_call_for_flag)
			{
				assert(mos.is_object_known(*always_call_for));
				fw.unsigned_32(mos.get_object_file_index(*always_call_for));
			} else
				fw.unsigned_32(0);

			//  worker waiting for capacity
			const Flag::CapacityWaitQueue & capacity_wait =
				flag->m_capacity_wait;
			fw.unsigned_16(capacity_wait.size());
			for (const OPtr<Worker >&  temp_worker : capacity_wait) {
				Worker const * const obj = temp_worker.get(egbase);
				assert
					(obj);
				assert
					(obj->get_state(Worker::taskWaitforcapacity));
				assert
					(obj->get_state(Worker::taskWaitforcapacity)->objvar1.serial()
					 ==
					 flag                                               ->serial());
				assert(mos.is_object_known(*obj));
				fw.unsigned_32(mos.get_object_file_index(*obj));
			}
			const Flag::FlagJobs & flag_jobs = flag->m_flag_jobs;
			fw.unsigned_16(flag_jobs.size());

			for (const Flag::FlagJob& temp_job : flag_jobs) {
				if (temp_job.request) {
					fw.unsigned_8(1);
					temp_job.request->write
						(fw, dynamic_cast<Game&>(egbase), mos);
				} else
					fw.unsigned_8(0);

				fw.string(temp_job.program);
			}

			mos.mark_object_as_saved(*flag);

		}

	fw.write(fs, "binary/flag_data");
}

}
