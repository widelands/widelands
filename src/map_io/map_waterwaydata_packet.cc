/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "map_io/map_waterwaydata_packet.h"

#include "base/macros.h"
#include "economy/ferry_fleet.h"
#include "economy/flag.h"
#include "economy/waterway.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/map.h"
#include "logic/map_objects/tribes/ferry.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"
#include "logic/widelands_geometry_io.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 1;

void MapWaterwaydataPacket::read(FileSystem& fs,
                                 EditorGameBase& egbase,
                                 bool const skip,
                                 MapObjectLoader& mol) {
	if (skip) {
		return;
	}

	FileRead fr;
	try {
		fr.open(fs, "binary/waterway_data");
	} catch (...) {
		return;
	}

	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			const Map& map = egbase.map();
			PlayerNumber const nr_players = map.get_nrplayers();
			Game& game = dynamic_cast<Game&>(egbase);
			while (!fr.end_of_file()) {
				Serial const serial = fr.unsigned_32();
				try {
					Waterway& ww = mol.get<Waterway>(serial);
					if (mol.is_object_loaded(ww)) {
						throw GameDataError("already loaded");
					}
					PlayerNumber player_index = fr.unsigned_8();
					if (!(0 < player_index && player_index <= nr_players)) {
						throw GameDataError(
						   "Invalid player number: %i.", static_cast<unsigned int>(player_index));
					}

					ww.set_owner(egbase.get_player(player_index));
					{
						uint32_t const flag_0_serial = fr.unsigned_32();
						try {
							ww.flags_[0] = &mol.get<Flag>(flag_0_serial);
						} catch (const WException& e) {
							throw GameDataError("flag 0 (%u): %s", flag_0_serial, e.what());
						}
					}
					{
						uint32_t const flag_1_serial = fr.unsigned_32();
						try {
							ww.flags_[1] = &mol.get<Flag>(flag_1_serial);
						} catch (const WException& e) {
							throw GameDataError("flag 1 (%u): %s", flag_1_serial, e.what());
						}
					}
					ww.flagidx_[0] = fr.unsigned_32();
					ww.flagidx_[1] = fr.unsigned_32();

					ww.cost_[0] = fr.unsigned_32();
					ww.cost_[1] = fr.unsigned_32();
					Path::StepVector::size_type const nr_steps = fr.unsigned_16();
					if (!nr_steps) {
						throw GameDataError("nr_steps = 0");
					}
					Path p(ww.flags_[0]->get_position());
					for (Path::StepVector::size_type i = nr_steps; i; --i) {
						try {
							p.append(map, read_direction_8(&fr));
						} catch (const WException& e) {
							throw GameDataError("step #%" PRIuS ": %s", nr_steps - i, e.what());
						}
					}
					ww.set_path(egbase, p);
					ww.idle_index_ = p.get_nsteps() / 2;

					//  Now that all rudimentary data is set, init this waterway. Then
					//  overwrite the initialization values.
					ww.link_into_flags(game, true);

					uint32_t fleet_serial = fr.unsigned_32();
					uint32_t ferry_serial = fr.unsigned_32();
					if (fleet_serial > 0) {
						try {
							ww.fleet_ = &mol.get<FerryFleet>(fleet_serial);
						} catch (const WException& e) {
							throw GameDataError("fleet (%u): %s", fleet_serial, e.what());
						}
					} else {
						ww.fleet_ = nullptr;
					}
					if (ferry_serial > 0) {
						try {
							ww.ferry_ = &mol.get<Ferry>(ferry_serial);
						} catch (const WException& e) {
							throw GameDataError("ferry (%u): %s", ferry_serial, e.what());
						}
					} else {
						ww.ferry_ = nullptr;
					}

					mol.mark_object_as_loaded(ww);
				} catch (const WException& e) {
					throw GameDataError("waterway %u: %s", serial, e.what());
				}
			}
		} else {
			throw UnhandledVersionError(
			   "MapWaterwaydataPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("waterwaydata: %s", e.what());
	}
}

void MapWaterwaydataPacket::write(FileSystem& fs, EditorGameBase& egbase, MapObjectSaver& mos) {
	FileWrite fw;

	fw.unsigned_16(kCurrentPacketVersion);

	const Map& map = egbase.map();
	const Field& fields_end = map[map.max_index()];
	for (Field const* field = &map[0]; field < &fields_end; ++field) {
		if (upcast(Waterway const, r, field->get_immovable())) {
			if (!mos.is_object_saved(*r)) {
				assert(mos.is_object_known(*r));

				fw.unsigned_32(mos.get_object_file_index(*r));

				//  First, write PlayerImmovable Stuff
				//  There's only the owner
				fw.unsigned_8(r->owner().player_number());

				//  serial of flags
				assert(mos.is_object_known(*r->flags_[0]));
				assert(mos.is_object_known(*r->flags_[1]));
				fw.unsigned_32(mos.get_object_file_index(*r->flags_[0]));
				fw.unsigned_32(mos.get_object_file_index(*r->flags_[1]));

				fw.unsigned_32(r->flagidx_[0]);
				fw.unsigned_32(r->flagidx_[1]);

				fw.unsigned_32(r->cost_[0]);
				fw.unsigned_32(r->cost_[1]);

				const Path& path = r->path_;
				const Path::StepVector::size_type nr_steps = path.get_nsteps();
				fw.unsigned_16(nr_steps);
				for (Path::StepVector::size_type i = 0; i < nr_steps; ++i) {
					fw.unsigned_8(path[i]);
				}

				FerryFleet* fleet = r->fleet_.get(egbase);
				fw.unsigned_32(fleet ? mos.get_object_file_index(*fleet) : 0);
				Ferry* ferry = r->ferry_.get(egbase);
				fw.unsigned_32(ferry ? mos.get_object_file_index(*ferry) : 0);

				mos.mark_object_as_saved(*r);
			}
		}
	}
	fw.write(fs, "binary/waterway_data");
}
}  // namespace Widelands
