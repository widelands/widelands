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

#include <boost/algorithm/string.hpp>

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

bool from_unsigned(uint8_t value) {
	return value == 1;
}

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
		if (packet_version == kCurrentPacketVersion) {
			std::list<const Player*> all_players;
			for (uint8_t i = fr.unsigned_8(); i; --i) {
				Player& player = *egbase.get_player(fr.unsigned_8());
				all_players.push_back(&player);

				player.revealed_fields_.clear();
				for (uint32_t j = fr.unsigned_32(); j; --j) {
					player.revealed_fields_.insert(fr.unsigned_32());
				}
			}

			MapIndex upper_bound = egbase.map().max_index() - 1;
			// Read numerical field infos as combined strings to reduce number of hard disk write operations
			for (const Player* p : all_players) {
				std::vector<std::string> field_vector;
				// Fields data
				{
					// Owner
					const std::string& parseme = fr.c_string();
					boost::split(field_vector, parseme, boost::is_any_of("|"));
					if (field_vector.size() != upper_bound + 1) {
						throw GameDataError("wrong number of fields owner data for player %d - expected %u"
											" but got %" PRIuS,
											static_cast<unsigned>(p->player_number()), upper_bound + 1, field_vector.size());
					}
					for (MapIndex m = 0; m < upper_bound; ++m) {
						p->fields_[m].owner = stoi(field_vector[m]);
					}
				}
				{
					// Seeing
					const std::string& parseme = fr.c_string();
					boost::split(field_vector, parseme, boost::is_any_of("|"));
					if (field_vector.size() != upper_bound + 1) {
						throw GameDataError("wrong number of fields seeing data for player %d - expected %u"
											" but got %" PRIuS,
											static_cast<unsigned>(p->player_number()), upper_bound + 1, field_vector.size());
					}
					for (MapIndex m = 0; m < upper_bound; ++m) {
						p->fields_[m].seeing = static_cast<Widelands::SeeUnseeNode>(stoi(field_vector[m]));
					}
				}
				{
					// Last Unseen
					const std::string& parseme = fr.c_string();
					boost::split(field_vector, parseme, boost::is_any_of("|"));
					if (field_vector.size() != upper_bound + 1) {
						throw GameDataError("wrong number of fields last seen data for player %d - expected %u"
											" but got %" PRIuS,
											static_cast<unsigned>(p->player_number()), upper_bound + 1, field_vector.size());
					}
					for (MapIndex m = 0; m < upper_bound; ++m) {
						p->fields_[m].time_node_last_unseen = stoi(field_vector[m]);
					}
				}
				{
					// Last Surveyed
					const std::string& parseme = fr.c_string();
					boost::split(field_vector, parseme, boost::is_any_of("|"));
					if (field_vector.size() != upper_bound + 1) {
						throw GameDataError("wrong number of fields last surveyed data for player %d - expected %u"
											" but got %" PRIuS,
											static_cast<unsigned>(p->player_number()), upper_bound + 1, field_vector.size());
					}
					for (MapIndex m = 0; m < upper_bound; ++m) {
						std::vector<std::string> data_vector;
						boost::split(data_vector, field_vector[m], boost::is_any_of("-"));
						if (data_vector.size() != 2) {
							throw GameDataError("wrong number of records in last surveyed data for player %d in field %d - expected 2"
												" but got %" PRIuS,
												static_cast<unsigned>(p->player_number()), m, field_vector.size());
						}
						Player::Field& f = p->fields_[m];
						f.time_triangle_last_surveyed[0] = stoi(data_vector[0]);
						f.time_triangle_last_surveyed[1] = stoi(data_vector[1]);
					}
				}
				{
					// Resource Amounts
					const std::string& parseme = fr.c_string();
					boost::split(field_vector, parseme, boost::is_any_of("|"));
					if (field_vector.size() != upper_bound + 1) {
						throw GameDataError("wrong number of fields resource amount data for player %d - expected %u"
											" but got %" PRIuS,
											static_cast<unsigned>(p->player_number()), upper_bound + 1, field_vector.size());
					}
					for (MapIndex m = 0; m < upper_bound; ++m) {
						std::vector<std::string> data_vector;
						boost::split(data_vector, field_vector[m], boost::is_any_of("-"));
						if (data_vector.size() != 2) {
							throw GameDataError("wrong number of records in resource amount data for player %d in field %d - expected 2"
												" but got %" PRIuS,
												static_cast<unsigned>(p->player_number()), m, field_vector.size());
						}
						Player::Field& f = p->fields_[m];
						f.resource_amounts.d = stoi(data_vector[0]);
						f.resource_amounts.r = stoi(data_vector[1]);
					}
				}
				{
					// Terrains
					const std::string& parseme = fr.c_string();
					boost::split(field_vector, parseme, boost::is_any_of("|"));
					if (field_vector.size() != upper_bound + 1) {
						throw GameDataError("wrong number of fields terrain data for player %d - expected %u"
											" but got %" PRIuS,
											static_cast<unsigned>(p->player_number()), upper_bound + 1, field_vector.size());
					}
					for (MapIndex m = 0; m < upper_bound; ++m) {
						std::vector<std::string> data_vector;
						boost::split(data_vector, field_vector[m], boost::is_any_of("-"));
						if (data_vector.size() != 2) {
							throw GameDataError("wrong number of records in terrain data for player %d in field %d - expected 2"
												" but got %" PRIuS,
												static_cast<unsigned>(p->player_number()), m, field_vector.size());
						}
						Player::Field& f = p->fields_[m];
						f.terrains.d = stoi(data_vector[0]);
						f.terrains.r = stoi(data_vector[1]);
					}
				}
				{
					// Roads
					const std::string& parseme = fr.c_string();
					boost::split(field_vector, parseme, boost::is_any_of("|"));
					if (field_vector.size() != upper_bound + 1) {
						throw GameDataError("wrong number of fields roads data for player %d - expected %u"
											" but got %" PRIuS,
											static_cast<unsigned>(p->player_number()), upper_bound + 1, field_vector.size());
					}
					for (MapIndex m = 0; m < upper_bound; ++m) {
						std::vector<std::string> data_vector;
						boost::split(data_vector, field_vector[m], boost::is_any_of("-"));
						if (data_vector.size() != 3) {
							throw GameDataError("wrong number of records in roads data for player %d in field %d - expected 3"
												" but got %" PRIuS,
												static_cast<unsigned>(p->player_number()), m, field_vector.size());
						}
						Player::Field& f = p->fields_[m];
						f.r_e = static_cast<Widelands::RoadSegment>(stoi(data_vector[0]));
						f.r_se = static_cast<Widelands::RoadSegment>(stoi(data_vector[1]));
						f.r_sw = static_cast<Widelands::RoadSegment>(stoi(data_vector[2]));
					}
				}
				{
					// Borders
					const std::string& parseme = fr.c_string();
					boost::split(field_vector, parseme, boost::is_any_of("|"));
					if (field_vector.size() != upper_bound + 1) {
						throw GameDataError("wrong number of fields borders data for player %d - expected %u"
											" but got %" PRIuS,
											static_cast<unsigned>(p->player_number()), upper_bound + 1, field_vector.size());
					}
					for (MapIndex m = 0; m < upper_bound; ++m) {
						std::vector<std::string> data_vector;
						boost::split(data_vector, field_vector[m], boost::is_any_of("-"));
						if (data_vector.size() != 4) {
							throw GameDataError("wrong number of records in borders data for player %d in field %d - expected 4"
												" but got %" PRIuS,
												static_cast<unsigned>(p->player_number()), m, field_vector.size());
						}
						Player::Field& f = p->fields_[m];
						f.border = stoi(data_vector[0]);
						f.border_r = stoi(data_vector[1]);
						f.border_br = stoi(data_vector[2]);
						f.border_bl = stoi(data_vector[3]);
					}
				}

				// Map objects
				for (MapIndex m = 0; m < upper_bound; ++m) {
					Player::Field& f = p->fields_[m];
					std::string descr = fr.string();
					if (descr.empty()) {
						f.map_object_descr = nullptr;
						f.constructionsite.becomes = nullptr;
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

						descr = fr.string();
						if (descr.empty()) {
							f.constructionsite.becomes = nullptr;
						} else {
							f.constructionsite.becomes = egbase.tribes().get_building_descr(
							   egbase.tribes().safe_building_index(descr));

							descr = fr.string();
							f.constructionsite.was = descr.empty() ?
							                            nullptr :
							                            egbase.tribes().get_building_descr(
							                               egbase.tribes().safe_building_index(descr));

							for (uint8_t j = fr.unsigned_32(); j; --j) {
								f.constructionsite.intermediates.push_back(
								   egbase.tribes().get_building_descr(
								      egbase.tribes().safe_building_index(fr.string())));
							}

							f.constructionsite.totaltime = fr.unsigned_32();
							f.constructionsite.completedtime = fr.unsigned_32();
						}
					}
				}
			}
		} else if (packet_version == 1) {
			for (uint8_t i = fr.unsigned_8(); i; --i) {
				Player& player = *egbase.get_player(fr.unsigned_8());

				player.revealed_fields_.clear();
				for (uint32_t j = fr.unsigned_32(); j; --j) {
					player.revealed_fields_.insert(fr.unsigned_32());
				}

				for (MapIndex m = egbase.map().max_index(); m; --m) {
					Player::Field& f = player.fields_[m - 1];

					f.owner = fr.unsigned_8();

					f.seeing = static_cast<SeeUnseeNode>(fr.unsigned_8());
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
						f.constructionsite.becomes = nullptr;
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

						descr = fr.string();
						if (descr.empty()) {
							f.constructionsite.becomes = nullptr;
						} else {
							f.constructionsite.becomes = egbase.tribes().get_building_descr(
							   egbase.tribes().safe_building_index(descr));

							descr = fr.string();
							f.constructionsite.was = descr.empty() ?
							                            nullptr :
							                            egbase.tribes().get_building_descr(
							                               egbase.tribes().safe_building_index(descr));

							for (uint8_t j = fr.unsigned_32(); j; --j) {
								f.constructionsite.intermediates.push_back(
								   egbase.tribes().get_building_descr(
								      egbase.tribes().safe_building_index(fr.string())));
							}

							f.constructionsite.totaltime = fr.unsigned_32();
							f.constructionsite.completedtime = fr.unsigned_32();
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

uint8_t to_unsigned(bool value) {
	return value ? 1 : 0;
}
/* NOCOM this speeds up writing Europa map:
took 266672ms
took  13037ms

Values on Build 21:
Writing Exploration Data ... took 18ms
Writing Players Unseen Data ... took 70ms

 * */
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
	}

	MapIndex upper_bound = egbase.map().max_index() - 1;
	// Write numerical field infos as combined strings to reduce number of hard disk write operations
	for (const Player* p : all_players) {
		// Fields data
		{
			// Owner
			std::ostringstream oss("");
			for (MapIndex m = 0; m < upper_bound; ++m) {
				oss << p->fields_[m].owner << "|";
			}
			oss << p->fields_[upper_bound].owner;
			fw.c_string(oss.str());
		}
		{
			// Seeing
			std::ostringstream oss("");
			for (MapIndex m = 0; m < upper_bound; ++m) {
				oss << static_cast<uint8_t>(p->fields_[m].seeing) << "|";
			}
			oss << static_cast<uint8_t>(p->fields_[upper_bound].seeing);
			fw.c_string(oss.str());
		}
		{
			// Last Unseen
			std::ostringstream oss("");
			for (MapIndex m = 0; m < upper_bound; ++m) {
				oss << p->fields_[m].time_node_last_unseen << "|";
			}
			oss << p->fields_[upper_bound].time_node_last_unseen;
			fw.c_string(oss.str());
		}
		{
			// Last Surveyed
			std::ostringstream oss("");
			for (MapIndex m = 0; m < upper_bound; ++m) {
				const Player::Field& f = p->fields_[m];
				oss << f.time_triangle_last_surveyed[0] << "-" << f.time_triangle_last_surveyed[1] << "|";
			}
			const Player::Field& f = p->fields_[upper_bound];
			oss << f.time_triangle_last_surveyed[0] << "-" << f.time_triangle_last_surveyed[1];
			fw.c_string(oss.str());
		}
		{
			// Resource Amounts
			std::ostringstream oss("");
			for (MapIndex m = 0; m < upper_bound; ++m) {
				const Player::Field& f = p->fields_[m];
				oss << f.resource_amounts.d << "-" << f.resource_amounts.r << "|";
			}
			const Player::Field& f = p->fields_[upper_bound];
			oss << f.resource_amounts.d << "-" << f.resource_amounts.r;
			fw.c_string(oss.str());
		}
		{
			// Terrains
			std::ostringstream oss("");
			for (MapIndex m = 0; m < upper_bound; ++m) {
				const Player::Field& f = p->fields_[m];
				oss << f.terrains.d << "-" << f.terrains.r << "|";
			}
			const Player::Field& f = p->fields_[upper_bound];
			oss << f.terrains.d << "-" << f.terrains.r;
			fw.c_string(oss.str());
		}
		{
			// Roads
			std::ostringstream oss("");
			for (MapIndex m = 0; m < upper_bound; ++m) {
				const Player::Field& f = p->fields_[m];
				oss << static_cast<uint8_t>(f.r_e) << "-" << static_cast<uint8_t>(f.r_se) << "-" << static_cast<uint8_t>(f.r_sw) << "|";
			}
			const Player::Field& f = p->fields_[upper_bound];
			oss << static_cast<uint8_t>(f.r_e) << "-" << static_cast<uint8_t>(f.r_se) << "-" << static_cast<uint8_t>(f.r_sw);
			fw.c_string(oss.str());
		}
		{
			// Borders
			std::ostringstream oss("");
			for (MapIndex m = 0; m < upper_bound; ++m) {
				const Player::Field& f = p->fields_[m];
				oss << to_unsigned(f.border) << "-" << to_unsigned(f.border_r) << "-" << to_unsigned(f.border_br) << "-" << to_unsigned(f.border_bl) << "|";
			}
			const Player::Field& f = p->fields_[upper_bound];
			oss << to_unsigned(f.border) << "-" << to_unsigned(f.border_r) << "-" << to_unsigned(f.border_br) << "-" << to_unsigned(f.border_bl);
			fw.c_string(oss.str());
		}

		// Map objects
		for (MapIndex m = 0; m < upper_bound; ++m) {
			const Player::Field& f = p->fields_[m];
			if (f.map_object_descr) {
				fw.string(f.map_object_descr->name());

				if (f.constructionsite.becomes) {
					fw.string(f.constructionsite.becomes->name());
					fw.string(f.constructionsite.was ? f.constructionsite.was->name() : "");

					fw.unsigned_32(f.constructionsite.intermediates.size());
					for (const BuildingDescr* d : f.constructionsite.intermediates) {
						fw.string(d->name());
					}

					fw.unsigned_32(f.constructionsite.totaltime);
					fw.unsigned_32(f.constructionsite.completedtime);
				} else {
					fw.string("");
				}
			} else {
				fw.string("");
			}
		}
	}

	fw.write(fs, "binary/view");
}
}  // namespace Widelands
