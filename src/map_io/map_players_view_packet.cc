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
#include "base/macros.h"
#include "base/wexception.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "economy/waterway.h"
#include "io/fileread.h"
#include "io/filesystem/filesystem_exceptions.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/field.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/world/world.h"
#include "logic/player.h"

namespace Widelands {

#define PLAYERDIRNAME_TEMPLATE "player/%u"
#define DIRNAME_TEMPLATE PLAYERDIRNAME_TEMPLATE "/view"

constexpr uint8_t kCurrentPacketVersionUnseenTimes = 1;
#define UNSEEN_TIMES_FILENAME_TEMPLATE DIRNAME_TEMPLATE "/unseen_times_%u"

constexpr uint8_t kCurrentPacketVersionImmovableKinds = 2;
#define NODE_IMMOVABLE_KINDS_FILENAME_TEMPLATE DIRNAME_TEMPLATE "/node_immovable_kinds_%u"
#define TRIANGLE_IMMOVABLE_KINDS_FILENAME_TEMPLATE DIRNAME_TEMPLATE "/triangle_immovable_kinds_%u"

constexpr uint8_t kCurrentPacketVersionImmovables = 2;
#define NODE_IMMOVABLES_FILENAME_TEMPLATE DIRNAME_TEMPLATE "/node_immovables_%u"
#define TRIANGLE_IMMOVABLES_FILENAME_TEMPLATE DIRNAME_TEMPLATE "/triangle_immovables_%u"

constexpr uint8_t kCurrentPacketVersionRoads = 2;
#define ROADS_FILENAME_TEMPLATE DIRNAME_TEMPLATE "/roads_%u"

constexpr uint8_t kCurrentPacketVersionTerrains = 2;
#define TERRAINS_FILENAME_TEMPLATE DIRNAME_TEMPLATE "/terrains_%u"

constexpr uint8_t kCurrentPacketVersionOwners = 0;
#define OWNERS_FILENAME_TEMPLATE DIRNAME_TEMPLATE "/owners_%u"

constexpr uint8_t kCurrentPacketVersionSurveys = 2;
#define SURVEYS_FILENAME_TEMPLATE DIRNAME_TEMPLATE "/surveys_%u"

constexpr uint8_t kCurrentPacketVersionSurveyAmounts = 2;
#define SURVEY_AMOUNTS_FILENAME_TEMPLATE DIRNAME_TEMPLATE "/survey_amounts_%u"

constexpr uint8_t kCurrentPacketVersionSurveyTimes = 1;
#define SURVEY_TIMES_FILENAME_TEMPLATE DIRNAME_TEMPLATE "/survey_times_%u"

constexpr uint8_t kCurrentPacketVersionHidden = 1;
#define HIDDEN_FILENAME_TEMPLATE DIRNAME_TEMPLATE "/hidden_%u"

constexpr uint8_t kCurrentPacketVersionVision = 1;
#define VISION_FILENAME_TEMPLATE DIRNAME_TEMPLATE "/vision_%u"

constexpr uint8_t kCurrentPacketVersionBorder = 1;
#define BORDER_FILENAME_TEMPLATE DIRNAME_TEMPLATE "/border_%u"

#define FILENAME_SIZE 64

enum {
	UNSEEN_NONE = 0,
	UNSEEN_TRIBEORWORLD = 1,
	UNSEEN_FLAG = 2,
	UNSEEN_BUILDING = 3,
	UNSEEN_PORTDOCK = 4
};

//  The map is traversed by row and column. In each step we process of one map
//  field (which is 1 node, 2 triangles and 3 edges that are stored together).
//  For this processing we need to keep track of 4 nodes:
//  *  f: the node of the processed field
//  * bl: the bottom left neighbour of f
//  * br: the bottom right neighbour of f
//  *  r: the right neighbour of f
//
//  The layout of the processing region is as follows:
//
//                     f------ r
//                    / \     /
//                   /  \    /
//                  / D \ R /
//                 /    \  /
//                /     \ /
//              bl------br

struct MapObjectData {
	MapObjectData() : map_object_descr(nullptr) {
	}
	const MapObjectDescr* map_object_descr;
	ConstructionsiteInformation csi;
};

namespace {
#define OPEN_INPUT_FILE(filetype, file, filename, filename_template, version)                      \
	char(filename)[FILENAME_SIZE];                                                                  \
	snprintf(filename, sizeof(filename), filename_template, plnum, version);                        \
	filetype file;                                                                                  \
	try {                                                                                           \
		(file).open(fs, filename);                                                                   \
	} catch (const FileError&) {                                                                    \
		throw GameDataError("MapPlayersViewPacket::read: player %u:Could not open "                  \
		                    "\"%s\" for reading. This file should exist when \"%s\" exists",         \
		                    static_cast<unsigned int>(plnum), filename, unseen_times_filename);      \
	}

// Try to find the file with newest fitting version number
#define OPEN_INPUT_FILE_NEW_VERSION(filetype, file, filename, fileversion, filename_template,      \
                                    version)                                                       \
	uint8_t fileversion = version;                                                                  \
	filetype file;                                                                                  \
	char(filename)[FILENAME_SIZE];                                                                  \
	for (;; --fileversion) {                                                                        \
		snprintf(filename, sizeof(filename), filename_template, plnum, fileversion);                 \
		try {                                                                                        \
			(file).open(fs, filename);                                                                \
			break;                                                                                    \
		} catch (...) {                                                                              \
			if (fileversion == 0)                                                                     \
				throw GameDataError("MapPlayersViewPacket::read: player %u:Could not open "            \
				                    "\"%s\" for reading. This file should exist when \"%s\" exists",   \
				                    static_cast<unsigned int>(plnum), filename,                        \
				                    unseen_times_filename);                                            \
		}                                                                                            \
	}

// Using this macro, if no file exists, fileversion will be set to -1
#define OPEN_INPUT_FILE_NEW_VERSION_SILENT(filetype, file, filename, fileversion, file_templ, v)   \
	int8_t fileversion = v;                                                                         \
	filetype file;                                                                                  \
	char(filename)[FILENAME_SIZE];                                                                  \
	for (; fileversion >= -1; --fileversion) {                                                      \
		snprintf(filename, sizeof(filename), file_templ, plnum, fileversion);                        \
		try {                                                                                        \
			(file).open(fs, filename);                                                                \
			break;                                                                                    \
		} catch (...) {                                                                              \
		}                                                                                            \
	}

#define CHECK_TRAILING_BYTES(file, filename)                                                       \
	if (!(file).end_of_file())                                                                      \
		throw GameDataError("MapPlayersViewPacket::read: player %u:"                                 \
		                    "Found %" PRIuS " trailing bytes in \"%s\"",                             \
		                    static_cast<unsigned int>(plnum),                                        \
		                    static_cast<size_t>((file).get_size() - (file).get_pos()), filename);

// Errors for the Read* functions.
struct TribeImmovableNonexistent : public FileRead::DataError {
	explicit TribeImmovableNonexistent(const std::string& Name)
	   : DataError("immovable type \"%s\" does not seem to be a tribe immovable", Name.c_str()),
	     name(Name) {
	}

	std::string name;
};
struct WorldImmovableNonexistent : public FileRead::DataError {
	explicit WorldImmovableNonexistent(char const* const Name)
	   : DataError("world does not define immovable type \"%s\"", Name), name(Name) {
	}
	char const* const name;
};
struct BuildingNonexistent : public FileRead::DataError {
	explicit BuildingNonexistent(char const* const Name)
	   : DataError("tribes do not define building type \"%s\"", Name), name(Name) {
	}
	char const* const name;
};

// reads an immovable depending on whether it is a tribe or world immovable
const ImmovableDescr& read_immovable_type(StreamRead* fr,
                                          const EditorGameBase& egbase,
                                          const TribesLegacyLookupTable& tribes_lookup_table,
                                          const WorldLegacyLookupTable& world_lookup_table) {
	uint8_t owner = fr->unsigned_8();
	char const* const name = fr->c_string();
	if (owner == static_cast<uint8_t>(MapObjectDescr::OwnerType::kWorld)) {
		DescriptionIndex const index =
		   egbase.world().get_immovable_index(world_lookup_table.lookup_immovable(name));
		if (index == Widelands::INVALID_INDEX) {
			throw WorldImmovableNonexistent(name);
		}
		return *egbase.world().get_immovable_descr(index);
	} else {
		assert(owner == static_cast<uint8_t>(MapObjectDescr::OwnerType::kTribe));
		DescriptionIndex const index =
		   egbase.tribes().immovable_index(tribes_lookup_table.lookup_immovable(name));
		if (index == Widelands::INVALID_INDEX) {
			throw TribeImmovableNonexistent(name);
		}
		return *egbase.tribes().get_immovable_descr(index);
	}
}

// Reads a c_string and interprets it as the name of an immovable type.
//
// \returns a reference to the building type description.
//
// \throws Building_Nonexistent if there is no building type with that name
const BuildingDescr& read_building_type(StreamRead* fr, const EditorGameBase& egbase) {
	char const* const name = fr->c_string();
	DescriptionIndex const index = egbase.tribes().building_index(name);
	if (!egbase.tribes().building_exists(index)) {
		throw BuildingNonexistent(name);
	}
	return *egbase.tribes().get_building_descr(index);
}

// Encode a Immovable_Type into 'wr'.
void write_immovable_type(StreamWrite* wr, const ImmovableDescr& immovable) {
	wr->unsigned_8(static_cast<uint8_t>(immovable.owner_type()));
	wr->string(immovable.name());
}

// Encode a Building_Type into 'wr'.
void write_building_type(StreamWrite* wr, const BuildingDescr& building) {
	wr->string(building.name());
}

}  // namespace

inline static MapObjectData
read_unseen_immovable(const EditorGameBase& egbase,
                      uint8_t& immovable_kind,
                      FileRead& immovables_file,
                      const TribesLegacyLookupTable& tribes_lookup_table,
                      const WorldLegacyLookupTable& world_lookup_table,
                      uint8_t& version) {
	MapObjectData m;
	try {
		switch (immovable_kind) {
		case UNSEEN_NONE:  //  The player sees no immovable.
			m.map_object_descr = nullptr;
			break;
		case UNSEEN_TRIBEORWORLD:  //  The player sees a tribe or world immovable.
			m.map_object_descr =
			   &read_immovable_type(&immovables_file, egbase, tribes_lookup_table, world_lookup_table);
			break;
		case UNSEEN_FLAG:  //  The player sees a flag.
			m.map_object_descr = &g_flag_descr;
			break;
		case UNSEEN_BUILDING:  //  The player sees a building.
			m.map_object_descr = &read_building_type(&immovables_file, egbase);
			if (version == kCurrentPacketVersionImmovables) {
				// Read data from immovables file
				if (immovables_file.unsigned_8() == 1) {  // the building is a constructionsite
					m.csi.becomes = &read_building_type(&immovables_file, egbase);
					if (immovables_file.unsigned_8() == 1) {
						m.csi.was = &read_building_type(&immovables_file, egbase);
					}
					m.csi.totaltime = immovables_file.unsigned_32();
					m.csi.completedtime = immovables_file.unsigned_32();
				}
			} else {
				throw UnhandledVersionError(
				   "MapPlayersViewPacket", version, kCurrentPacketVersionImmovables);
			}
			break;
		case UNSEEN_PORTDOCK:  // The player sees a port dock
			m.map_object_descr = &g_portdock_descr;
			break;
		default:
			throw GameDataError("Unknown immovable-kind type %d", immovable_kind);
		}
	} catch (const WException& e) {
		throw GameDataError("unseen immovable: %s", e.what());
	}
	return m;
}

void MapPlayersViewPacket::read(FileSystem& fs,
                                EditorGameBase& egbase,
                                bool const skip,
                                MapObjectLoader&,
                                const TribesLegacyLookupTable& tribes_lookup_table,
                                const WorldLegacyLookupTable& world_lookup_table)

{
	if (skip) {
		return;
	}

	const Map& map = egbase.map();
	const uint16_t mapwidth = map.get_width();
	const uint16_t mapheight = map.get_height();
	Field& first_field = map[0];
	const PlayerNumber nr_players = map.get_nrplayers();
	iterate_players_existing(plnum, nr_players, egbase, player) {
		Player::Field* const player_fields = player->fields_.get();
		uint32_t const gametime = egbase.get_gametime();

		char unseen_times_filename[FILENAME_SIZE];
		snprintf(unseen_times_filename, sizeof(unseen_times_filename), UNSEEN_TIMES_FILENAME_TEMPLATE,
		         static_cast<unsigned int>(plnum), kCurrentPacketVersionUnseenTimes);
		FileRead unseen_times_file;
		struct NotFound {};

		if (!unseen_times_file.try_open(fs, unseen_times_filename)) {
			log("MapPlayersViewPacket::read: WARNING: Could not open "
			    "\"%s\" for reading. Assuming that the game is from an old "
			    "version without player point of view. Will give player %u "
			    "knowledge of unseen nodes, edges and triangles (but not "
			    "resources).",
			    unseen_times_filename, static_cast<unsigned int>(plnum));

			for (FCoords first_in_row(Coords(0, 0), &first_field); first_in_row.y < mapheight;
			     ++first_in_row.y, first_in_row.field += mapwidth) {
				FCoords r = first_in_row, br = map.bl_n(r);
				MapIndex r_index = r.field - &first_field;
				MapIndex br_index = br.field - &first_field;
				Player::Field* r_player_field = player_fields + r_index;
				Player::Field* br_player_field = player_fields + br_index;
				Vision r_vision = r_player_field->vision;
				Vision br_vision = br_player_field->vision;
				do {
					const FCoords f = r;
					Player::Field& f_player_field = *r_player_field;
					const Vision f_vision = r_vision, bl_vision = br_vision;
					move_r(mapwidth, r, r_index);
					move_r(mapwidth, br, br_index);
					r_player_field = player_fields + r_index;
					br_player_field = player_fields + br_index;
					r_vision = r_player_field->vision;
					br_vision = br_player_field->vision;

					f_player_field.time_node_last_unseen = gametime;

					if (f_vision) {  //  node
						//  owner
						f_player_field.owner = f.field->get_owned_by();
						assert(f_player_field.owner < 0x20);

						//  map_object_descr
						const MapObjectDescr* map_object_descr;
						if (const BaseImmovable* base_immovable = f.field->get_immovable()) {
							map_object_descr = &base_immovable->descr();
							if (Road::is_road_descr(map_object_descr) ||
							    Waterway::is_waterway_descr(map_object_descr)) {
								map_object_descr = nullptr;
							} else if (upcast(Building const, building, base_immovable)) {
								if (building->get_position() != f) {
									//  TODO(unknown): This is not the building's main position
									//  so we can not see it. But it should be
									//  possible to see it from a distance somehow.
									map_object_descr = nullptr;
								}
							}
						} else {
							map_object_descr = nullptr;
						}
						f_player_field.map_object_descr = map_object_descr;
					}

					{  //  triangles
						//  Must be initialized because the rendering code is
						//  accessing it even for triangles that the player does not
						//  see (it is the darkening that actually hides the ground
						//  from the player).
						Field::Terrains terrains;
						terrains.d = terrains.r = 0;

						if (f_vision | bl_vision | br_vision) {
							terrains.d = f.field->terrain_d();
						}
						if (f_vision | br_vision | r_vision) {
							terrains.r = f.field->terrain_r();
						}
						f_player_field.terrains = terrains;
					}

					{  //  edges
						if (f_vision | bl_vision) {
							f_player_field.r_sw = f.field->get_road(WALK_SW);
						}
						if (f_vision | br_vision) {
							f_player_field.r_se = f.field->get_road(WALK_SE);
						}
						if (f_vision | r_vision) {
							f_player_field.r_e = f.field->get_road(WALK_E);
						}
					}

					//  The player is not given information about resources that he
					//  has discovered, because there is no such information in old
					//  savegames, except for the resource indicators that are
					//  merely immovables with limited lifetime.
				} while (r.x);
			}
			return;
		}

		// Verify the vision values
		FileRead vision_file;
		bool have_vision = false;

		try {
			char fname[FILENAME_SIZE];
			snprintf(fname, sizeof(fname), VISION_FILENAME_TEMPLATE, static_cast<unsigned int>(plnum),
			         kCurrentPacketVersionVision);
			vision_file.open(fs, fname);
			have_vision = true;
		} catch (...) {
		}

		if (have_vision) {
			for (FCoords first_in_row(Coords(0, 0), &first_field); first_in_row.y < mapheight;
			     ++first_in_row.y, first_in_row.field += mapwidth) {
				FCoords r = first_in_row;
				MapIndex r_index = r.field - &first_field;
				Player::Field* r_player_field = player_fields + r_index;
				do {
					Player::Field& f_player_field = *r_player_field;
					move_r(mapwidth, r, r_index);
					r_player_field = player_fields + r_index;

					uint32_t file_vision = vision_file.unsigned_32();

					// There used to be a check here that the calculated, and the
					// loaded vision were the same. I removed this check, because
					// scripting could have given the player a permanent view of
					// this field. That's why we save this stuff in the first place!
					if (file_vision != f_player_field.vision) {
						f_player_field.vision = file_vision;
					}
				} while (r.x);
			}

			log("Vision check successful for player %u\n", static_cast<unsigned int>(plnum));
		}

		// Read the player's knowledge about all fields
		OPEN_INPUT_FILE_NEW_VERSION(FileRead, node_immovable_kinds_file,
		                            node_immovable_kinds_filename, node_immovable_kinds_file_version,
		                            NODE_IMMOVABLE_KINDS_FILENAME_TEMPLATE,
		                            kCurrentPacketVersionImmovableKinds)

		OPEN_INPUT_FILE_NEW_VERSION(FileRead, node_immovables_file, node_immovables_filename,
		                            node_immovables_file_version, NODE_IMMOVABLES_FILENAME_TEMPLATE,
		                            kCurrentPacketVersionImmovables)

		OPEN_INPUT_FILE_NEW_VERSION(FileRead, roads_file, roads_filename, road_file_version,
		                            ROADS_FILENAME_TEMPLATE, kCurrentPacketVersionRoads)

		OPEN_INPUT_FILE_NEW_VERSION(FileRead, terrains_file, terrains_filename, terrains_file_version,
		                            TERRAINS_FILENAME_TEMPLATE, kCurrentPacketVersionTerrains)

		OPEN_INPUT_FILE_NEW_VERSION(
		   FileRead, triangle_immovable_kinds_file, triangle_immovable_kinds_filename,
		   triangle_immovable_kinds_file_version, TRIANGLE_IMMOVABLE_KINDS_FILENAME_TEMPLATE,
		   kCurrentPacketVersionImmovableKinds)

		OPEN_INPUT_FILE_NEW_VERSION(FileRead, triangle_immovables_file, triangle_immovables_filename,
		                            triangle_immovables_file_version,
		                            TRIANGLE_IMMOVABLES_FILENAME_TEMPLATE,
		                            kCurrentPacketVersionImmovables)

		OPEN_INPUT_FILE(FileRead, owners_file, owners_filename, OWNERS_FILENAME_TEMPLATE,
		                kCurrentPacketVersionOwners)

		OPEN_INPUT_FILE_NEW_VERSION(FileRead, surveys_file, surveys_filename, surveys_file_version,
		                            SURVEYS_FILENAME_TEMPLATE, kCurrentPacketVersionSurveys)

		OPEN_INPUT_FILE_NEW_VERSION(FileRead, survey_amounts_file, survey_amounts_filename,
		                            survey_amounts_file_version, SURVEY_AMOUNTS_FILENAME_TEMPLATE,
		                            kCurrentPacketVersionSurveyAmounts)

		OPEN_INPUT_FILE(FileRead, survey_times_file, survey_times_filename,
		                SURVEY_TIMES_FILENAME_TEMPLATE, kCurrentPacketVersionSurveyTimes)

		OPEN_INPUT_FILE_NEW_VERSION(FileRead, border_file, border_filename, border_file_version,
		                            BORDER_FILENAME_TEMPLATE, kCurrentPacketVersionBorder)

		OPEN_INPUT_FILE_NEW_VERSION_SILENT(FileRead, hidden_file, hidden_filename,
		                                   hidden_file_version, HIDDEN_FILENAME_TEMPLATE,
		                                   kCurrentPacketVersionHidden)

		for (FCoords first_in_row(Coords(0, 0), &first_field); first_in_row.y < mapheight;
		     ++first_in_row.y, first_in_row.field += mapwidth) {
			FCoords r = first_in_row, br = map.bl_n(r);
			MapIndex r_index = r.field - &first_field;
			MapIndex br_index = br.field - &first_field;
			Player::Field* r_player_field = player_fields + r_index;
			Player::Field* br_player_field = player_fields + br_index;
			Vision r_vision = r_player_field->vision;
			Vision br_vision = br_player_field->vision;
			bool r_everseen = r_vision, r_seen = 1 < r_vision;
			bool br_everseen = br_vision, br_seen = 1 < br_vision;
			do {
				const FCoords f = r;
				Player::Field& f_player_field = *r_player_field;
				const Vision f_vision = r_vision;
				const bool f_everseen = r_everseen;
				const bool bl_everseen = br_everseen;
				const bool f_seen = r_seen;
				const bool bl_seen = br_seen;
				move_r(mapwidth, r, r_index);
				move_r(mapwidth, br, br_index);
				r_player_field = player_fields + r_index;
				br_player_field = player_fields + br_index;
				r_vision = r_player_field->vision;
				br_vision = br_player_field->vision;
				r_everseen = r_vision;
				r_seen = 1 < r_vision;
				br_everseen = br_vision;
				br_seen = 1 < br_vision;

				//  Store the player's view of ownership in these
				//  temporary variables and save it in the player when set.
				PlayerNumber owner = 0;

				switch (f_vision) {  //  owner and map_object_descr
				case 0:
					//  The player has never seen this node, so he has no
					//  information about it. Neither should he be informed about
					//  it now.
					break;
				case 1: {
					//  The player has seen the node but does not see it now. Load
					//  his information about the node from file.
					try {
						f_player_field.time_node_last_unseen = unseen_times_file.unsigned_32();
					} catch (const FileRead::FileBoundaryExceeded&) {
						throw GameDataError("MapPlayersViewPacket::read: player %u: in "
						                    "\"%s\":%" PRIuS ": node (%i, %i): unexpected end of file "
						                    "while reading time_node_last_unseen",
						                    static_cast<unsigned int>(plnum), unseen_times_filename,
						                    static_cast<size_t>(unseen_times_file.get_pos() - 4), f.x,
						                    f.y);
					}

					try {
						owner = owners_file.unsigned_8();
					} catch (const FileRead::FileBoundaryExceeded&) {
						throw GameDataError("MapPlayersViewPacket::read: player %u: in "
						                    "\"%s\":%" PRIuS ": node (%i, %i): unexpected end of file "
						                    "while reading owner",
						                    static_cast<unsigned int>(plnum), unseen_times_filename,
						                    static_cast<size_t>(unseen_times_file.get_pos() - 1), f.x,
						                    f.y);
					}
					if (nr_players < owner) {
						throw GameDataError("MapPlayersViewPacket::read: player %u: in "
						                    "\"%s\":%" PRIuS " & 0xf: node (%i, %i): Player thinks that "
						                    "this node is owned by player %u, but there are only %u "
						                    "players",
						                    static_cast<unsigned int>(plnum), owners_filename,
						                    static_cast<size_t>(owners_file.get_pos() - 1), f.x, f.y,
						                    owner, nr_players);
					}
					uint8_t imm_kind = 0;
					if (node_immovable_kinds_file_version == kCurrentPacketVersionImmovableKinds) {
						imm_kind = node_immovable_kinds_file.unsigned_8();
					} else {
						throw UnhandledVersionError("MapPlayersViewPacket - Node Immovable kinds",
						                            node_immovable_kinds_file_version,
						                            kCurrentPacketVersionImmovableKinds);
					}
					MapObjectData mod =
					   read_unseen_immovable(egbase, imm_kind, node_immovables_file, tribes_lookup_table,
					                         world_lookup_table, node_immovables_file_version);
					f_player_field.map_object_descr = mod.map_object_descr;
					f_player_field.constructionsite = mod.csi;

					// Read in whether this field had a border the last time it was seen
					if (border_file_version == kCurrentPacketVersionBorder) {
						uint8_t borders = border_file.unsigned_8();
						f_player_field.border = borders & 1;
						f_player_field.border_r = borders & 2;
						f_player_field.border_br = borders & 4;
						f_player_field.border_bl = borders & 8;
					} else {
						throw UnhandledVersionError("MapPlayersViewPacket - Border file",
						                            border_file_version, kCurrentPacketVersionBorder);
					}
					break;
				}
				default:
					//  The player currently sees the node. Therefore his
					//  information about the node has not been saved. Fill in the
					//  information from the game state.

					//  owner
					owner = f.field->get_owned_by();
					assert(owner <= nr_players);

					//  map_object_descr
					const MapObjectDescr* map_object_descr;
					if (const BaseImmovable* base_immovable = f.field->get_immovable()) {
						map_object_descr = &base_immovable->descr();
						if (Road::is_road_descr(map_object_descr) ||
						    Waterway::is_waterway_descr(map_object_descr)) {
							map_object_descr = nullptr;
						} else if (upcast(Building const, building, base_immovable)) {
							if (building->get_position() != f) {
								//  TODO(unknown): This is not the building's main position so
								//  we can not see it. But it should be possible
								//  to see it from a distance somehow.
								map_object_descr = nullptr;
							}
						}
					} else {
						map_object_descr = nullptr;
					}
					f_player_field.map_object_descr = map_object_descr;
					break;
				}

				//  triangles
				if (f_seen | bl_seen | br_seen) {
					//  The player currently sees the D triangle. Therefore his
					//  information about the triangle has not been saved. Fill in
					//  the information from the game state.
					f_player_field.terrains.d = f.field->terrain_d();
				} else if (f_everseen | bl_everseen | br_everseen) {
					//  The player has seen the D triangle but does not see it now.
					//  Load his information about the triangle from file.
					if (terrains_file_version == kCurrentPacketVersionTerrains) {
						f_player_field.terrains.d = terrains_file.unsigned_8();
					} else {
						throw UnhandledVersionError("MapPlayersViewPacket - Terrains",
						                            terrains_file_version, kCurrentPacketVersionTerrains);
					}
					uint8_t im_kind = 0;
					if (triangle_immovable_kinds_file_version == kCurrentPacketVersionImmovableKinds) {
						im_kind = triangle_immovable_kinds_file.unsigned_8();
					} else {
						throw UnhandledVersionError("MapPlayersViewPacket - Triangle Immovable kinds",
						                            triangle_immovable_kinds_file_version,
						                            kCurrentPacketVersionImmovableKinds);
					}
					// We read and ignore the immovable information on the D
					// triangle. This was done because there were vague plans of
					// suporting immovables on the triangles instead as on the
					// nodes.
					// TODO(sirver): Remove this logic the next time we break
					// savegame compatibility.
					read_unseen_immovable(egbase, im_kind, triangle_immovables_file, tribes_lookup_table,
					                      world_lookup_table, triangle_immovables_file_version);
				}
				if (f_seen | br_seen | r_seen) {
					//  The player currently sees the R triangle. Therefore his
					//  information about the triangle has not been saved. Fill in
					//  the information from the game state.
					f_player_field.terrains.r = f.field->terrain_r();
				} else if (f_everseen | br_everseen | r_everseen) {
					//  The player has seen the R triangle but does not see it now.
					//  Load his information about the triangle from file.
					if (terrains_file_version == kCurrentPacketVersionTerrains) {
						f_player_field.terrains.r = terrains_file.unsigned_8();
					} else {
						throw UnhandledVersionError("MapPlayersViewPacket - Terrains",
						                            terrains_file_version, kCurrentPacketVersionTerrains);
					}
					uint8_t im_kind = 0;
					if (triangle_immovable_kinds_file_version == kCurrentPacketVersionImmovableKinds) {
						im_kind = triangle_immovable_kinds_file.unsigned_8();
					} else {
						throw UnhandledVersionError("MapPlayersViewPacket - Triangle Immovable kinds",
						                            triangle_immovable_kinds_file_version,
						                            kCurrentPacketVersionImmovableKinds);
					}
					// We read and ignore the immovable information on the D
					// triangle. This was done because there were vague plans of
					// suporting immovables on the triangles instead as on the
					// nodes.
					read_unseen_immovable(egbase, im_kind, triangle_immovables_file, tribes_lookup_table,
					                      world_lookup_table, triangle_immovables_file_version);
				}

				{  //  edges
					if (f_seen | bl_seen) {
						f_player_field.r_sw = f.field->get_road(WALK_SW);
					} else if (f_everseen | bl_everseen) {
						//  The player has seen the SouthWest edge but does not see
						//  it now. Load his information about this edge from file.
						if (road_file_version == kCurrentPacketVersionRoads) {
							f_player_field.r_sw = static_cast<RoadSegment>(roads_file.unsigned_8());
						} else {
							throw UnhandledVersionError("MapPlayersViewPacket - Road file",
							                            road_file_version, kCurrentPacketVersionRoads);
						}
					}
					if (f_seen | br_seen) {
						f_player_field.r_se = f.field->get_road(WALK_SE);
					} else if (f_everseen | br_everseen) {
						//  The player has seen the SouthEast edge but does not see
						//  it now. Load his information about this edge from file.
						if (road_file_version == kCurrentPacketVersionRoads) {
							f_player_field.r_se = static_cast<RoadSegment>(roads_file.unsigned_8());
						} else {
							throw UnhandledVersionError("MapPlayersViewPacket - Road file",
							                            road_file_version, kCurrentPacketVersionRoads);
						}
					}
					if (f_seen | r_seen) {
						f_player_field.r_e = f.field->get_road(WALK_E);
					} else if (f_everseen | r_everseen) {
						//  The player has seen the      East edge but does not see
						//  it now. Load his information about this edge from file.
						if (road_file_version == kCurrentPacketVersionRoads) {
							f_player_field.r_e = static_cast<RoadSegment>(roads_file.unsigned_8());
						} else {
							throw UnhandledVersionError("MapPlayersViewPacket - Road file",
							                            road_file_version, kCurrentPacketVersionRoads);
						}
					}
				}

				//  Now save this information in the player field.
				f_player_field.owner = owner;

				//  geologic survey
				try {
					bool survey = false;
					if (surveys_file_version == kCurrentPacketVersionSurveys) {
						survey = (f_everseen & bl_everseen & br_everseen) && surveys_file.unsigned_8();
					} else {
						throw UnhandledVersionError("MapPlayersViewPacket - Surveys file",
						                            surveys_file_version, kCurrentPacketVersionSurveys);
					}
					if (survey) {
						if (survey_amounts_file_version == kCurrentPacketVersionSurveyAmounts) {
							f_player_field.resource_amounts.d = survey_amounts_file.unsigned_8();
						} else {
							throw UnhandledVersionError("MapPlayersViewPacket - Survey amounts",
							                            survey_amounts_file_version,
							                            kCurrentPacketVersionSurveyAmounts);
						}
						try {
							f_player_field
							   .time_triangle_last_surveyed[static_cast<int>(TriangleIndex::D)] =
							   survey_times_file.unsigned_32();
						} catch (const FileRead::FileBoundaryExceeded&) {
							throw GameDataError(
							   "MapPlayersViewPacket::read: player %u: in "
							   "\"%s\":%" PRIuS ": node (%i, %i) t = D: unexpected end of "
							   "file while reading time_triangle_last_surveyed",
							   static_cast<unsigned int>(plnum), survey_times_filename,
							   static_cast<size_t>(survey_times_file.get_pos() - 4), f.x, f.y);
						}
					}
				} catch (const FileRead::FileBoundaryExceeded&) {
					throw GameDataError("MapPlayersViewPacket::read: player %u: in \"%s\": "
					                    "node (%i, %i) t = D: unexpected end of file while reading "
					                    "survey bit",
					                    static_cast<unsigned int>(plnum), surveys_filename, f.x, f.y);
				}
				try {
					bool survey = false;
					if (surveys_file_version == kCurrentPacketVersionSurveys) {
						survey = (f_everseen & br_everseen & r_everseen) && surveys_file.unsigned_8();
					} else {
						throw UnhandledVersionError("MapPlayersViewPacket - Surveys file",
						                            surveys_file_version, kCurrentPacketVersionSurveys);
					}
					if (survey) {
						if (survey_amounts_file_version == kCurrentPacketVersionSurveyAmounts) {
							f_player_field.resource_amounts.r = survey_amounts_file.unsigned_8();
						} else {
							throw UnhandledVersionError("MapPlayersViewPacket - Survey amounts",
							                            survey_amounts_file_version,
							                            kCurrentPacketVersionSurveyAmounts);
						}
						try {
							f_player_field
							   .time_triangle_last_surveyed[static_cast<int>(TriangleIndex::R)] =
							   survey_times_file.unsigned_32();
						} catch (const FileRead::FileBoundaryExceeded&) {
							throw GameDataError(
							   "MapPlayersViewPacket::read: player %u: in "
							   "\"%s\":%" PRIuS ": node (%i, %i) t = R: unexpected end of "
							   "file while reading time_triangle_last_surveyed",
							   static_cast<unsigned int>(plnum), survey_times_filename,
							   static_cast<size_t>(survey_times_file.get_pos() - 4), f.x, f.y);
						}
					}
				} catch (const FileRead::FileBoundaryExceeded&) {
					throw GameDataError("MapPlayersViewPacket::read: player %u: in \"%s\": "
					                    "node (%i, %i) t = R: unexpected end of file while reading "
					                    "survey bit",
					                    static_cast<unsigned int>(plnum), surveys_filename, f.x, f.y);
				}
			} while (r.x);
		}

		// Read the number of explicitly hidden fields and then loop through them
		if (hidden_file_version == kCurrentPacketVersionHidden) {
			const uint32_t no_of_hidden_fields = hidden_file.unsigned_32();
			for (uint32_t i = 0; i < no_of_hidden_fields; ++i) {
				player->hidden_fields_.insert(
				   std::make_pair(hidden_file.unsigned_32(), hidden_file.unsigned_16()));
			}
		} else if (hidden_file_version < 0) {
			// TODO(GunChleoc): Savegame compatibility - remove after Build 20
			log("MapPlayersViewPacket - No hidden fields to read for Player %d - probably an old save "
			    "file\n",
			    static_cast<unsigned int>(plnum));
		} else {
			throw UnhandledVersionError("MapPlayersViewPacket - Hidden fields file",
			                            hidden_file_version, kCurrentPacketVersionHidden);
		}

		CHECK_TRAILING_BYTES(unseen_times_file, unseen_times_filename)
		CHECK_TRAILING_BYTES(node_immovable_kinds_file, node_immovable_kinds_filename)
		CHECK_TRAILING_BYTES(node_immovables_file, node_immovables_filename)
		CHECK_TRAILING_BYTES(roads_file, roads_filename)
		CHECK_TRAILING_BYTES(terrains_file, terrains_filename)
		CHECK_TRAILING_BYTES(triangle_immovable_kinds_file, triangle_immovable_kinds_filename)
		CHECK_TRAILING_BYTES(triangle_immovables_file, triangle_immovables_filename)
		CHECK_TRAILING_BYTES(owners_file, owners_filename)
		CHECK_TRAILING_BYTES(surveys_file, surveys_filename)
		CHECK_TRAILING_BYTES(survey_amounts_file, survey_amounts_filename)
		CHECK_TRAILING_BYTES(survey_times_file, survey_times_filename)
		CHECK_TRAILING_BYTES(border_file, border_filename)
		CHECK_TRAILING_BYTES(hidden_file, hidden_filename)
	}
}

inline static void write_unseen_immovable(MapObjectData const* map_object_data,
                                          FileWrite& immovable_kinds_file,
                                          FileWrite& immovables_file) {
	MapObjectDescr const* const map_object_descr = map_object_data->map_object_descr;
	const ConstructionsiteInformation& csi = map_object_data->csi;
	assert(!Road::is_road_descr(map_object_descr) && !Waterway::is_waterway_descr(map_object_descr));
	uint8_t immovable_kind = 255;

	if (!map_object_descr) {
		immovable_kind = UNSEEN_NONE;
	} else if (upcast(ImmovableDescr const, immovable_descr, map_object_descr)) {
		immovable_kind = UNSEEN_TRIBEORWORLD;
		write_immovable_type(&immovables_file, *immovable_descr);
	} else if (map_object_descr->type() == MapObjectType::FLAG) {
		immovable_kind = UNSEEN_FLAG;
	} else if (upcast(BuildingDescr const, building_descr, map_object_descr)) {
		immovable_kind = UNSEEN_BUILDING;
		write_building_type(&immovables_file, *building_descr);
		if (!csi.becomes) {
			immovables_file.unsigned_8(0);
		} else {
			// the building is a constructionsite
			immovables_file.unsigned_8(1);
			write_building_type(&immovables_file, *csi.becomes);
			if (!csi.was) {
				immovables_file.unsigned_8(0);
			} else {
				// constructionsite is an enhancement, therefor we write down the enhancement
				immovables_file.unsigned_8(1);
				write_building_type(&immovables_file, *csi.was);
			}
			immovables_file.unsigned_32(csi.totaltime);
			immovables_file.unsigned_32(csi.completedtime);
		}
	} else if (map_object_descr->type() == MapObjectType::PORTDOCK) {
		immovable_kind = UNSEEN_PORTDOCK;
	} else {
		// We should never get here.. output some information about the situation.
		log("\nwidelands_map_players_view_data_packet.cc::write_unseen_immovable(): ");
		log("%s %s was not expected.\n", typeid(*map_object_descr).name(),
		    map_object_descr->name().c_str());
		NEVER_HERE();
	}
	immovable_kinds_file.unsigned_8(immovable_kind);
}

#define WRITE(file, filename_template, version)                                                    \
	snprintf(filename, sizeof(filename), filename_template, plnum, version);                        \
	(file).write(fs, filename);

void MapPlayersViewPacket::write(FileSystem& fs, EditorGameBase& egbase, MapObjectSaver&) {
	fs.ensure_directory_exists("player");
	const Map& map = egbase.map();
	const uint16_t mapwidth = map.get_width();
	const uint16_t mapheight = map.get_height();
	// TODO(unknown): make first_field const when FCoords has been templatized so it can
	// have "const Field * field;"
	Field& first_field = map[0];
	const PlayerNumber nr_players = map.get_nrplayers();
	iterate_players_existing_const(
	   plnum, nr_players, egbase,
	   player) if (const Player::Field* const player_fields = player->fields_.get()) {
		FileWrite unseen_times_file;
		FileWrite node_immovable_kinds_file;
		FileWrite node_immovables_file;
		FileWrite roads_file;
		FileWrite terrains_file;
		FileWrite triangle_immovable_kinds_file;
		FileWrite triangle_immovables_file;
		FileWrite owners_file;
		FileWrite surveys_file;
		FileWrite survey_amounts_file;
		FileWrite survey_times_file;
		FileWrite hidden_file;
		FileWrite vision_file;
		FileWrite border_file;
		for (FCoords first_in_row(Coords(0, 0), &first_field); first_in_row.y < mapheight;
		     ++first_in_row.y, first_in_row.field += mapwidth) {
			FCoords r = first_in_row, br = map.bl_n(r);
			MapIndex r_index = r.field - &first_field;
			MapIndex br_index = br.field - &first_field;
			const Player::Field* r_player_field = player_fields + r_index;
			const Player::Field* br_player_field = player_fields + br_index;
			Vision r_vision = r_player_field->vision;
			Vision br_vision = br_player_field->vision;
			bool r_everseen = r_vision, r_seen = 1 < r_vision;
			bool br_everseen = br_vision, br_seen = 1 < br_vision;
			do {
				const Player::Field& f_player_field = *r_player_field;
				const bool f_everseen = r_everseen;
				const bool bl_everseen = br_everseen;
				const bool f_seen = r_seen;
				const bool bl_seen = br_seen;
				move_r(mapwidth, r, r_index);
				move_r(mapwidth, br, br_index);
				r_player_field = player_fields + r_index;
				br_player_field = player_fields + br_index;
				r_vision = r_player_field->vision;
				br_vision = br_player_field->vision;
				r_everseen = r_vision;
				r_seen = 1 < r_vision;
				br_everseen = br_vision;
				br_seen = 1 < br_vision;

				vision_file.unsigned_32(f_player_field.vision);

				if (!f_seen) {

					if (f_everseen) {  //  node
						unseen_times_file.unsigned_32(f_player_field.time_node_last_unseen);
						assert(f_player_field.owner < 0x20);
						owners_file.unsigned_8(f_player_field.owner);
						MapObjectData mod;
						mod.map_object_descr = f_player_field.map_object_descr;
						mod.csi = f_player_field.constructionsite;
						write_unseen_immovable(&mod, node_immovable_kinds_file, node_immovables_file);

						// write whether this field had a border the last time it was seen
						uint8_t borders = 0;
						borders |= f_player_field.border;
						borders |= f_player_field.border_r << 1;
						borders |= f_player_field.border_br << 2;
						borders |= f_player_field.border_bl << 3;
						border_file.unsigned_8(borders);
					}

					//  triangles
					if
					   //  the player does not see the D triangle now but has
					   //  seen it
					   ((!bl_seen) & (!br_seen) & (f_everseen | bl_everseen | br_everseen)) {
						terrains_file.unsigned_8(f_player_field.terrains.d);
						MapObjectData mod;
						mod.map_object_descr = nullptr;
						write_unseen_immovable(
						   &mod, triangle_immovable_kinds_file, triangle_immovables_file);
					}
					if
					   //  the player does not see the R triangle now but has
					   //  seen it
					   ((!br_seen) & (!r_seen) & (f_everseen | br_everseen | r_everseen)) {
						terrains_file.unsigned_8(f_player_field.terrains.r);
						MapObjectData mod;
						mod.map_object_descr = nullptr;
						write_unseen_immovable(
						   &mod, triangle_immovable_kinds_file, triangle_immovables_file);
					}

					//  edges
					if ((!bl_seen) && (f_everseen || bl_everseen)) {
						roads_file.unsigned_8(f_player_field.road_sw());
					}
					if ((!br_seen) && (f_everseen || br_everseen)) {
						roads_file.unsigned_8(f_player_field.road_se());
					}
					if ((!r_seen) && (f_everseen || r_everseen)) {
						roads_file.unsigned_8(f_player_field.road_e());
					}
				}

				//  geologic survey
				if (f_everseen & bl_everseen & br_everseen) {
					const uint32_t time_last_surveyed =
					   f_player_field.time_triangle_last_surveyed[static_cast<int>(TriangleIndex::D)];
					const uint8_t has_info = time_last_surveyed != 0xffffffff;
					surveys_file.unsigned_8(has_info);
					if (has_info) {
						survey_amounts_file.unsigned_8(f_player_field.resource_amounts.d);
						survey_times_file.unsigned_32(time_last_surveyed);
					}
				}
				if (f_everseen & br_everseen & r_everseen) {
					const uint32_t time_last_surveyed =
					   f_player_field.time_triangle_last_surveyed[static_cast<int>(TriangleIndex::R)];
					const uint8_t has_info = time_last_surveyed != 0xffffffff;
					surveys_file.unsigned_8(has_info);
					if (has_info) {
						survey_amounts_file.unsigned_8(f_player_field.resource_amounts.r);
						survey_times_file.unsigned_32(time_last_surveyed);
					}
				}
			} while (r.x);
		}
		// Write the number of explicitly hidden fields and then loop through them
		hidden_file.unsigned_32(player->hidden_fields_.size());
		for (const auto& hidden : player->hidden_fields_) {
			hidden_file.unsigned_32(hidden.first);
			hidden_file.unsigned_16(hidden.second);
		}

		char filename[FILENAME_SIZE];

		fs.ensure_directory_exists(
		   (boost::format(PLAYERDIRNAME_TEMPLATE) % static_cast<unsigned int>(plnum)).str());
		fs.ensure_directory_exists(
		   (boost::format(DIRNAME_TEMPLATE) % static_cast<unsigned int>(plnum)).str());

		WRITE(unseen_times_file, UNSEEN_TIMES_FILENAME_TEMPLATE, kCurrentPacketVersionUnseenTimes)

		WRITE(node_immovable_kinds_file, NODE_IMMOVABLE_KINDS_FILENAME_TEMPLATE,
		      kCurrentPacketVersionImmovableKinds)

		WRITE(
		   node_immovables_file, NODE_IMMOVABLES_FILENAME_TEMPLATE, kCurrentPacketVersionImmovables)

		WRITE(roads_file, ROADS_FILENAME_TEMPLATE, kCurrentPacketVersionRoads)

		WRITE(terrains_file, TERRAINS_FILENAME_TEMPLATE, kCurrentPacketVersionTerrains)

		WRITE(triangle_immovable_kinds_file, TRIANGLE_IMMOVABLE_KINDS_FILENAME_TEMPLATE,
		      kCurrentPacketVersionImmovableKinds)

		WRITE(triangle_immovables_file, TRIANGLE_IMMOVABLES_FILENAME_TEMPLATE,
		      kCurrentPacketVersionImmovables)

		WRITE(owners_file, OWNERS_FILENAME_TEMPLATE, kCurrentPacketVersionOwners)

		WRITE(surveys_file, SURVEYS_FILENAME_TEMPLATE, kCurrentPacketVersionSurveys)

		WRITE(
		   survey_amounts_file, SURVEY_AMOUNTS_FILENAME_TEMPLATE, kCurrentPacketVersionSurveyAmounts)

		WRITE(survey_times_file, SURVEY_TIMES_FILENAME_TEMPLATE, kCurrentPacketVersionSurveyTimes)

		WRITE(hidden_file, HIDDEN_FILENAME_TEMPLATE, kCurrentPacketVersionHidden)

		WRITE(vision_file, VISION_FILENAME_TEMPLATE, kCurrentPacketVersionVision)

		WRITE(border_file, BORDER_FILENAME_TEMPLATE, kCurrentPacketVersionBorder)
	}
}
}  // namespace Widelands
