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

#include "map_io/map_roaddata_packet.h"

#include "base/macros.h"
#include "economy/flag.h"
#include "economy/request.h"
#include "economy/road.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/map.h"
#include "logic/map_objects/tribes/carrier.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"
#include "logic/widelands_geometry_io.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 5;

void MapRoaddataPacket::read(FileSystem& fs,
                             EditorGameBase& egbase,
                             bool const skip,
                             MapObjectLoader& mol) {
	if (skip) {
		return;
	}

	FileRead fr;
	try {
		fr.open(fs, "binary/road_data");
	} catch (...) {
		return;
	}

	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			const Map& map = egbase.map();
			PlayerNumber const nr_players = map.get_nrplayers();
			while (!fr.end_of_file()) {
				Serial const serial = fr.unsigned_32();
				try {
					Game& game = dynamic_cast<Game&>(egbase);
					Road& road = mol.get<Road>(serial);
					if (mol.is_object_loaded(road)) {
						throw GameDataError("already loaded");
					}
					PlayerNumber player_index = fr.unsigned_8();
					if (!(0 < player_index && player_index <= nr_players)) {
						throw GameDataError("Invalid player number: %i.", player_index);
					}

					road.set_owner(egbase.get_player(player_index));
					road.wallet_ = fr.unsigned_32();
					road.last_wallet_charge_ = Time(fr);
					road.busy_ = fr.unsigned_8() > 1;
					{
						uint32_t const flag_0_serial = fr.unsigned_32();
						try {
							road.flags_[0] = &mol.get<Flag>(flag_0_serial);
						} catch (const WException& e) {
							throw GameDataError("flag 0 (%u): %s", flag_0_serial, e.what());
						}
					}
					{
						uint32_t const flag_1_serial = fr.unsigned_32();
						try {
							road.flags_[1] = &mol.get<Flag>(flag_1_serial);
						} catch (const WException& e) {
							throw GameDataError("flag 1 (%u): %s", flag_1_serial, e.what());
						}
					}
					road.flagidx_[0] = fr.unsigned_32();
					road.flagidx_[1] = fr.unsigned_32();

					road.cost_[0] = fr.unsigned_32();
					road.cost_[1] = fr.unsigned_32();
					Path::StepVector::size_type const nr_steps = fr.unsigned_16();
					if (!nr_steps) {
						throw GameDataError("nr_steps = 0");
					}
					Path p(road.flags_[0]->get_position());
					for (Path::StepVector::size_type i = nr_steps; i; --i) {
						try {
							p.append(map, read_direction_8(&fr));
						} catch (const WException& e) {
							throw GameDataError("step #%" PRIuS ": %s", nr_steps - i, e.what());
						}
					}
					road.set_path(egbase, p);
					road.idle_index_ = p.get_nsteps() / 2;

					//  Now that all rudimentary data is set, init this road. Then
					//  overwrite the initialization values.
					road.link_into_flags(game, true);

					uint32_t const count = fr.unsigned_32();
					if (!count) {
						throw GameDataError("no carrier slot");
					}

					for (uint32_t i = 0; i < count; ++i) {
						road.carrier_slots_.emplace_back();
						Carrier* carrier = nullptr;
						Request* carrier_request = nullptr;

						if (uint32_t const carrier_serial = fr.unsigned_32()) {
							try {
								carrier = &mol.get<Carrier>(carrier_serial);
							} catch (const WException& e) {
								throw GameDataError("carrier (%u): %s", carrier_serial, e.what());
							}
						} else {
							carrier = nullptr;
						}

						if (fr.unsigned_8()) {
							(carrier_request =
							    new Request(road, 0, Road::request_carrier_callback, wwWORKER))
							   ->read(fr, game, mol);
						} else {
							carrier_request = nullptr;
						}

						const uint32_t carrier_type_id = fr.unsigned_32();
						assert(carrier_type_id > 0);
						road.carrier_slots_[i].carrier_type_id = carrier_type_id - 1;

						assert(!road.carrier_slots_[i].carrier.get(egbase));
						road.carrier_slots_[i].carrier = carrier;
						if (carrier || carrier_request) {
							delete road.carrier_slots_[i].carrier_request;
							road.carrier_slots_[i].carrier_request = carrier_request;
						}
					}

					mol.mark_object_as_loaded(road);
				} catch (const WException& e) {
					throw GameDataError("road %u: %s", serial, e.what());
				}
			}
		} else {
			throw UnhandledVersionError("MapRoaddataPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("roaddata: %s", e.what());
	}
}

void MapRoaddataPacket::write(FileSystem& fs, EditorGameBase& egbase, MapObjectSaver& mos) {
	FileWrite fw;

	fw.unsigned_16(kCurrentPacketVersion);

	const Map& map = egbase.map();
	const Field& fields_end = map[map.max_index()];
	for (Field const* field = &map[0]; field < &fields_end; ++field) {
		if (upcast(Road const, r, field->get_immovable())) {
			if (!mos.is_object_saved(*r)) {
				assert(mos.is_object_known(*r));

				fw.unsigned_32(mos.get_object_file_index(*r));

				//  First, write PlayerImmovable Stuff
				//  Theres only the owner
				fw.unsigned_8(r->owner().player_number());

				fw.unsigned_32(r->wallet_);
				r->last_wallet_charge_.save(fw);

				fw.unsigned_8(r->busy_ ? 2 : 1);

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

				fw.unsigned_32(r->carrier_slots_.size());

				for (const Road::CarrierSlot& temp_slot : r->carrier_slots_) {
					if (Carrier const* const carrier = temp_slot.carrier.get(egbase)) {
						assert(mos.is_object_known(*carrier));
						fw.unsigned_32(mos.get_object_file_index(*carrier));
					} else {
						fw.unsigned_32(0);
					}

					if (temp_slot.carrier_request) {
						fw.unsigned_8(1);
						temp_slot.carrier_request->write(fw, dynamic_cast<Game&>(egbase), mos);
					} else {
						fw.unsigned_8(0);
					}
					fw.unsigned_32(1u + temp_slot.carrier_type_id);
				}
				mos.mark_object_as_saved(*r);
			}
		}
	}
	fw.write(fs, "binary/road_data");
}
}  // namespace Widelands
