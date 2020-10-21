/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "base/macros.h"
#include "economy/flag.h"
#include "economy/request.h"
#include "economy/ware_instance.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/map.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/player.h"
#include "logic/widelands_geometry_io.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 5;

void MapFlagdataPacket::read(FileSystem& fs,
                             EditorGameBase& egbase,
                             bool const skip,
                             MapObjectLoader& mol,
                             const TribesLegacyLookupTable& tribes_lookup_table) {
	if (skip) {
		return;
	}

	FileRead fr;
	try {
		fr.open(fs, "binary/flag_data");
	} catch (...) {
		return;
	}

	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			const Map& map = egbase.map();
			while (!fr.end_of_file()) {
				Serial const serial = fr.unsigned_32();
				try {
					Flag& flag = mol.get<Flag>(serial);

					//  Owner is already set, nothing to do from PlayerImmovable.

					// TODO(Nordfriese): Saveloading compatibility:
					// Change this line when we next increase the packet version:
					flag.animstart_ = Time(fr.unsigned_16());
					// SHOULD BE: flag.animstart_ = Time(fr);
					// (same below)

					{
						FCoords building_position = map.get_fcoords(flag.position_);
						map.get_tln(building_position, &building_position);
						flag.building_ =
						   dynamic_cast<Building*>(building_position.field->get_immovable());
					}

					//  Roads are set somewhere else.
					flag.ware_capacity_ = fr.unsigned_32();

					{
						uint32_t const wares_filled = fr.unsigned_32();
						flag.ware_filled_ = wares_filled;
						for (uint32_t i = 0; i < wares_filled; ++i) {
							flag.wares_[i].pending = fr.unsigned_8();
							flag.wares_[i].priority = fr.signed_32();
							uint32_t const ware_serial = fr.unsigned_32();
							try {
								flag.wares_[i].ware = &mol.get<WareInstance>(ware_serial);

								if (uint32_t const nextstep_serial = fr.unsigned_32()) {
									try {
										flag.wares_[i].nextstep = &mol.get<PlayerImmovable>(nextstep_serial);
									} catch (const WException& e) {
										throw GameDataError("next step (%u): %s", nextstep_serial, e.what());
									}
								} else {
									flag.wares_[i].nextstep = nullptr;
								}
							} catch (const WException& e) {
								throw GameDataError("ware #%u (%u): %s", i, ware_serial, e.what());
							}
						}

						if (uint32_t const always_call_serial = fr.unsigned_32()) {
							try {
								flag.always_call_for_flag_ = &mol.get<Flag>(always_call_serial);
							} catch (const WException& e) {
								throw GameDataError("always_call (%u): %s", always_call_serial, e.what());
							}
						} else {
							flag.always_call_for_flag_ = nullptr;
						}

						//  workers waiting
						uint16_t const nr_workers = fr.unsigned_16();
						for (uint32_t i = 0; i < nr_workers; ++i) {
							uint32_t const worker_serial = fr.unsigned_32();
							try {
								//  The check that this worker actually has a
								//  waitforcapacity task for this flag is in
								//  Flag::load_finish, which is called after the worker
								//  (with his stack of tasks) has been fully loaded.
								flag.capacity_wait_.push_back(&mol.get<Worker>(worker_serial));
							} catch (const WException& e) {
								throw GameDataError("worker #%u (%u): %s", i, worker_serial, e.what());
							}
						}

						//  flag jobs
						uint16_t const nr_jobs = fr.unsigned_16();
						assert(flag.flag_jobs_.empty());
						for (uint16_t i = 0; i < nr_jobs; ++i) {
							Flag::FlagJob f;
							if (fr.unsigned_8()) {
								f.request = new Request(flag, 0, Flag::flag_job_request_callback, wwWORKER);
								f.request->read(fr, dynamic_cast<Game&>(egbase), mol, tribes_lookup_table);
							} else {
								f.request = nullptr;
							}
							f.program = fr.c_string();
							flag.flag_jobs_.push_back(f);
						}

						mol.mark_object_as_loaded(flag);
					}
				} catch (const WException& e) {
					throw GameDataError("%u: %s", serial, e.what());
				}
			}
		} else {
			throw UnhandledVersionError("MapFlagdataPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("flagdata: %s", e.what());
	}
}

void MapFlagdataPacket::write(FileSystem& fs, EditorGameBase& egbase, MapObjectSaver& mos)

{
	FileWrite fw;

	fw.unsigned_16(kCurrentPacketVersion);

	const Map& map = egbase.map();
	const Field& fields_end = map[map.max_index()];
	for (Field* field = &map[0]; field < &fields_end; ++field) {
		if (upcast(Flag const, flag, field->get_immovable())) {
			assert(mos.is_object_known(*flag));
			assert(!mos.is_object_saved(*flag));

			fw.unsigned_32(mos.get_object_file_index(*flag));

			//  Owner is already written in the existanz packet.

			//  Animation is set by creator.
			// TODO(Nordfriese): Saveloading compatibility:
			// Change this line when we next increase the packet version:
			fw.unsigned_16(flag->animstart_.get());
			// SHOULD BE: flag->animstart_.save(fw);
			// (same above)

			//  Roads are not saved, they are set on load.

			fw.unsigned_32(flag->ware_capacity_);

			fw.unsigned_32(flag->ware_filled_);

			for (int32_t i = 0; i < flag->ware_filled_; ++i) {
				fw.unsigned_8(flag->wares_[i].pending);
				fw.signed_32(flag->wares_[i].priority);
				assert(mos.is_object_known(*flag->wares_[i].ware));
				fw.unsigned_32(mos.get_object_file_index(*flag->wares_[i].ware));
				if (PlayerImmovable const* const nextstep = flag->wares_[i].nextstep.get(egbase)) {
					fw.unsigned_32(mos.get_object_file_index(*nextstep));
				} else {
					fw.unsigned_32(0);
				}
			}

			if (Flag const* const always_call_for = flag->always_call_for_flag_) {
				assert(mos.is_object_known(*always_call_for));
				fw.unsigned_32(mos.get_object_file_index(*always_call_for));
			} else {
				fw.unsigned_32(0);
			}

			//  worker waiting for capacity
			const Flag::CapacityWaitQueue& capacity_wait = flag->capacity_wait_;
			fw.unsigned_16(capacity_wait.size());
			for (const OPtr<Worker>& temp_worker : capacity_wait) {
				Worker const* const obj = temp_worker.get(egbase);
				assert(obj);
				assert(obj->get_state(Worker::taskWaitforcapacity));
				assert(obj->get_state(Worker::taskWaitforcapacity)->objvar1.serial() == flag->serial());
				assert(mos.is_object_known(*obj));
				fw.unsigned_32(mos.get_object_file_index(*obj));
			}
			const Flag::FlagJobs& flag_jobs = flag->flag_jobs_;
			fw.unsigned_16(flag_jobs.size());

			for (const Flag::FlagJob& temp_job : flag_jobs) {
				if (temp_job.request) {
					fw.unsigned_8(1);
					temp_job.request->write(fw, dynamic_cast<Game&>(egbase), mos);
				} else {
					fw.unsigned_8(0);
				}

				fw.string(temp_job.program);
			}

			mos.mark_object_as_saved(*flag);
		}
	}
	fw.write(fs, "binary/flag_data");
}
}  // namespace Widelands
