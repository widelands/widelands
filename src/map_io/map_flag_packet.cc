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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "map_io/map_flag_packet.h"

#include "base/macros.h"
#include "economy/flag.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/map.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 3;

void MapFlagPacket::read(FileSystem& fs,
                         EditorGameBase& egbase,
                         bool const skip,
                         MapObjectLoader& mol) {
	if (skip) {
		return;
	}

	FileRead fr;
	try {
		fr.open(fs, "binary/flag");
	} catch (...) {
		return;
	}

	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			const Map& map = egbase.map();
			PlayerNumber const nr_players = map.get_nrplayers();
			Widelands::Extent const extent = map.extent();
			iterate_Map_FCoords(map, extent, fc) if (fr.unsigned_8() != 0u) {
				PlayerNumber const owner = fr.unsigned_8();
				if (!(0 < owner && owner <= nr_players)) {
					throw GameDataError("Invalid player number: %i.", owner);
				}
				const Serial ware_economy_serial = fr.unsigned_32();
				const Serial worker_economy_serial = fr.unsigned_32();

				Serial const serial = fr.unsigned_32();

				try {
					if (fc.field->get_owned_by() != owner) {
						throw GameDataError("the node is owned by player %u", fc.field->get_owned_by());
					}

					for (Direction dir = 6; dir != 0u; --dir) {
						FCoords n;
						map.get_neighbour(fc, dir, &n);
						try {

							//  Check that the flag owner owns all neighbour nodes (so
							//  that the flag is not on his border). We can not check
							//  the border bit of the node because it has not been set
							//  yet.
							if (n.field->get_owned_by() != owner) {
								throw GameDataError("is owned by player %u", n.field->get_owned_by());
							}

							//  Check that there is not already a flag on a neighbour
							//  node. We read the map in order and only need to check
							//  the nodes that we have already read.
							if (n.field < fc.field) {
								if (upcast(Flag const, nf, n.field->get_immovable())) {
									throw GameDataError("has a flag (%u)", nf->serial());
								}
							}
						} catch (const WException& e) {
							throw GameDataError("neighbour node (%i, %i): %s", n.x, n.y, e.what());
						}
					}

					//  No flag lives on more than one place.

					// Get economy from serial
					Player* player = egbase.get_player(owner);
					Economy* ware_economy = player->get_economy(ware_economy_serial);
					if (ware_economy == nullptr) {
						ware_economy = player->create_economy(ware_economy_serial, wwWARE);
					}
					Economy* worker_economy = player->get_economy(worker_economy_serial);
					if (worker_economy == nullptr) {
						worker_economy = player->create_economy(worker_economy_serial, wwWORKER);
					}

					//  Now, create this Flag. Directly create it, do not call
					//  the player class since we recreate the data in another
					//  packet. We always create this, no matter what skip is
					//  since we have to read the data packets. We delete this
					//  object later again, if it is not wanted.
					Flag* flag =
					   new Flag(dynamic_cast<Game&>(egbase), player, fc, ware_economy, worker_economy);
					mol.register_object<Flag>(serial, *flag);

				} catch (const WException& e) {
					throw GameDataError(
					   "%u (at (%i, %i), owned by player %u): %s", serial, fc.x, fc.y, owner, e.what());
				}
			}
		} else {
			throw UnhandledVersionError("MapFlagPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("flags: %s", e.what());
	}
}

void MapFlagPacket::write(FileSystem& fs, EditorGameBase& egbase, MapObjectSaver& mos) {
	FileWrite fw;

	fw.unsigned_16(kCurrentPacketVersion);

	//  Write flags and owner, register this with the map_object_saver so that
	//  it's data can be saved later.
	const Map& map = egbase.map();
	const Field& fields_end = map[map.max_index()];
	for (Field const* field = &map[0]; field < &fields_end; ++field) {
		//  we only write flags, so the upcast is safe
		if (upcast(Flag const, flag, field->get_immovable())) {
			//  Flags can't life on multiply positions, therefore this flag
			//  shouldn't be registered.
			assert(!mos.is_object_known(*flag));
			assert(field->get_owned_by() == flag->owner().player_number());

			fw.unsigned_8(1);
			fw.unsigned_8(flag->owner().player_number());
			fw.unsigned_32(flag->economy(wwWARE).serial());
			fw.unsigned_32(flag->economy(wwWORKER).serial());
			fw.unsigned_32(mos.register_object(*flag));
		} else {  //  no existence, no owner
			fw.unsigned_8(0);
		}
	}
	fw.write(fs, "binary/flag");
}
}  // namespace Widelands
