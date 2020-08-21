/*
 * Copyright (C) 2007-2020 by the Widelands Development Team
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

#include "map_io/map_players_view_packet.h"

#include "base/log.h"
#include "base/wexception.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/field.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/world/world.h"
#include "logic/player.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 2;

void MapPlayersViewPacket::read(FileSystem& fs, EditorGameBase& egbase) {

	FileRead fr;
	if (!fr.try_open(fs, "binary/view")) {
		// TODO(Nordfriese): Savegame compatibility – require this packet after v1.0
		log("WARNING: New-style view packet not found. There may be strange effects regarding unseen "
		    "areas.\n");
		return;
	}

	try {
		uint16_t const packet_version = fr.unsigned_16();
		const Widelands::Map& map = egbase.map();
		// TODO(Nordfriese): Savegame compatibility, remove after v1.0
		if (packet_version >= 1 && packet_version <= kCurrentPacketVersion) {
			for (uint8_t i = fr.unsigned_8(); i; --i) {
				Player& player = *egbase.get_player(fr.unsigned_8());

				player.revealed_fields_.clear();
				for (uint32_t j = fr.unsigned_32(); j; --j) {
					player.revealed_fields_.insert(fr.unsigned_32());
				}

				for (MapIndex m = map.max_index(); m; --m) {
					Player::Field& f = player.fields_[m - 1];

					f.owner = fr.unsigned_8();

					f.seeing = static_cast<SeeUnseeNode>(fr.unsigned_8());

					if (f.seeing != SeeUnseeNode::kPreviouslySeen && packet_version > 1) {
						if (f.seeing == SeeUnseeNode::kVisible) {
							player.rediscover_node(map, map.get_fcoords(map[m - 1]));
						}
						continue;
					}

					f.time_node_last_unseen = fr.unsigned_32();
					f.time_triangle_last_surveyed[0] = fr.unsigned_32();
					f.time_triangle_last_surveyed[1] = fr.unsigned_32();

					f.resource_amounts.d = fr.unsigned_8();
					f.resource_amounts.r = fr.unsigned_8();

					f.terrains.d = fr.unsigned_32();
					f.terrains.r = fr.unsigned_32();

					f.r_e = static_cast<RoadSegment>(fr.unsigned_8());
					f.r_se = static_cast<RoadSegment>(fr.unsigned_8());
					f.r_sw = static_cast<RoadSegment>(fr.unsigned_8());

					f.border = fr.unsigned_8();
					f.border_r = fr.unsigned_8();
					f.border_br = fr.unsigned_8();
					f.border_bl = fr.unsigned_8();

					std::string descr = fr.string();
					if (descr.empty()) {
						f.map_object_descr = nullptr;
					} else {
						// I here assume that no two immovables will have the same internal name
						if (descr == "flag") {
							f.map_object_descr = &g_flag_descr;
						} else if (descr == "portdock") {
							f.map_object_descr = &g_portdock_descr;
						} else {
							DescriptionIndex di = egbase.tribes().building_index(descr);
							if (di != INVALID_INDEX) {
								f.map_object_descr = egbase.tribes().get_building_descr(di);
							} else {
								di = egbase.world().get_immovable_index(descr);
								if (di != INVALID_INDEX) {
									f.map_object_descr = egbase.world().get_immovable_descr(di);
								} else {
									di = egbase.tribes().immovable_index(descr);
									if (di != INVALID_INDEX) {
										f.map_object_descr = egbase.tribes().get_immovable_descr(di);
									} else {
										throw GameDataError("invalid map_object_descr: %s", descr.c_str());
									}
								}
							}
						}

						if (packet_version > 1) {
							if (f.map_object_descr->type() == MapObjectType::DISMANTLESITE) {
								f.partially_finished_building.dismantlesite.building =
								   egbase.tribes().get_building_descr(
								      egbase.tribes().safe_building_index(fr.string()));
								f.partially_finished_building.dismantlesite.progress = fr.unsigned_32();
							} else if (f.map_object_descr->type() == MapObjectType::CONSTRUCTIONSITE) {
								f.partially_finished_building.constructionsite.becomes =
								   egbase.tribes().get_building_descr(
								      egbase.tribes().safe_building_index(fr.string()));
								descr = fr.string();
								f.partially_finished_building.constructionsite.was =
								   descr.empty() ? nullptr :
								                   egbase.tribes().get_building_descr(
								                      egbase.tribes().safe_building_index(descr));

								for (uint32_t j = fr.unsigned_32(); j; --j) {
									f.partially_finished_building.constructionsite.intermediates.push_back(
									   egbase.tribes().get_building_descr(
									      egbase.tribes().safe_building_index(fr.string())));
								}

								f.partially_finished_building.constructionsite.totaltime = fr.unsigned_32();
								f.partially_finished_building.constructionsite.completedtime =
								   fr.unsigned_32();
							}
						} else {
							descr = fr.string();
							if (descr.empty()) {
								f.partially_finished_building.dismantlesite.building = nullptr;
								f.partially_finished_building.dismantlesite.progress = 0;
							} else {
								f.partially_finished_building.constructionsite.becomes =
								   egbase.tribes().get_building_descr(
								      egbase.tribes().safe_building_index(descr));

								descr = fr.string();
								f.partially_finished_building.constructionsite.was =
								   descr.empty() ? nullptr :
								                   egbase.tribes().get_building_descr(
								                      egbase.tribes().safe_building_index(descr));

								for (uint32_t j = fr.unsigned_32(); j; --j) {
									f.partially_finished_building.constructionsite.intermediates.push_back(
									   egbase.tribes().get_building_descr(
									      egbase.tribes().safe_building_index(fr.string())));
								}

								f.partially_finished_building.constructionsite.totaltime = fr.unsigned_32();
								f.partially_finished_building.constructionsite.completedtime =
								   fr.unsigned_32();
							}
						}
					}
				}
			}
		} else {
			throw UnhandledVersionError("MapPlayersViewPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("view: %s", e.what());
	}
}

void MapPlayersViewPacket::write(FileSystem& fs, EditorGameBase& egbase) {
	FileWrite fw;

	fw.unsigned_16(kCurrentPacketVersion);

	std::list<const Player*> all_players;
	for (PlayerNumber p = 1; p <= kMaxPlayers; ++p) {
		if (const Player* player = egbase.get_player(p)) {
			all_players.push_back(player);
		}
	}

	fw.unsigned_8(all_players.size());
	for (const Player* p : all_players) {
		fw.unsigned_8(p->player_number());

		// Explicitly revealed fields
		fw.unsigned_32(p->revealed_fields_.size());
		for (const MapIndex& m : p->revealed_fields_) {
			fw.unsigned_32(m);
		}

		// Fields data
		for (MapIndex m = egbase.map().max_index(); m; --m) {
			const Player::Field& f = p->fields_[m - 1];

			fw.unsigned_8(f.owner);

			fw.unsigned_8(static_cast<uint8_t>(f.seeing));

			if (f.seeing != SeeUnseeNode::kPreviouslySeen) {
				continue;
			}

			fw.unsigned_32(f.time_node_last_unseen);
			fw.unsigned_32(f.time_triangle_last_surveyed[0]);
			fw.unsigned_32(f.time_triangle_last_surveyed[1]);

			fw.unsigned_8(f.resource_amounts.d);
			fw.unsigned_8(f.resource_amounts.r);

			fw.unsigned_32(f.terrains.d);
			fw.unsigned_32(f.terrains.r);

			fw.unsigned_8(static_cast<uint8_t>(f.r_e));
			fw.unsigned_8(static_cast<uint8_t>(f.r_se));
			fw.unsigned_8(static_cast<uint8_t>(f.r_sw));

			fw.unsigned_8(f.border ? 1 : 0);
			fw.unsigned_8(f.border_r ? 1 : 0);
			fw.unsigned_8(f.border_br ? 1 : 0);
			fw.unsigned_8(f.border_bl ? 1 : 0);

			if (f.map_object_descr) {
				fw.string(f.map_object_descr->name());

				if (f.map_object_descr->type() == MapObjectType::DISMANTLESITE) {
					// `building` can only be nullptr in compatibility cases.
					// Remove the non-null check after v1.0
					fw.string(f.partially_finished_building.dismantlesite.building ?
					             f.partially_finished_building.dismantlesite.building->name() :
					             "dismantlesite");
					fw.unsigned_32(f.partially_finished_building.dismantlesite.progress);
				} else if (f.map_object_descr->type() == MapObjectType::CONSTRUCTIONSITE) {
					fw.string(f.partially_finished_building.constructionsite.becomes->name());
					fw.string(f.partially_finished_building.constructionsite.was ?
					             f.partially_finished_building.constructionsite.was->name() :
					             "");

					fw.unsigned_32(f.partially_finished_building.constructionsite.intermediates.size());
					for (const BuildingDescr* d :
					     f.partially_finished_building.constructionsite.intermediates) {
						fw.string(d->name());
					}

					fw.unsigned_32(f.partially_finished_building.constructionsite.totaltime);
					fw.unsigned_32(f.partially_finished_building.constructionsite.completedtime);
				}
			} else {
				fw.string("");
			}
		}
	}

	fw.write(fs, "binary/view");
}
}  // namespace Widelands
