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

#include "map_io/s2map.h"

#include <iomanip>
#include <iostream>
#include <memory>

#include "base/i18n.h"
#include "base/log.h"
#include "base/macros.h"
#include "base/scoped_timer.h"
#include "base/wexception.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/editor_game_base.h"
#include "logic/field.h"
#include "logic/game.h"
#include "logic/map.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/map_object.h"
#include "logic/mapregion.h"
#include "map_io/map_loader.h"

using std::cerr;
using std::endl;
using std::ios;

namespace {

// Do not change the contents of this struct, segfaults will ensue.
struct S2MapDescrHeader {
	char magic[10];  // "WORLD_V1.0"
	char name[20];   // We need fixed char arrays rather than strings here. Otherwise, this will
	                 // segfault.
	int16_t w;
	int16_t h;
	int8_t uses_world;  // 0 = green, 1 =black, 2 = winter
	int8_t nplayers;
	char author[26];
	char bulk[2290];  // unknown
} /* size 2352 */;

// Some basic checks to identify obviously invalid headers
bool is_valid_header(const S2MapDescrHeader& header) {
	if (strncmp(header.magic, "WORLD_V1.0", 10) != 0) {
		return false;
	}
	if (header.name[19]) {
		return false;
	}
	if (header.w <= 0 || header.h <= 0) {
		return false;
	}
	if (header.uses_world < 0 || header.uses_world > 2) {
		return false;
	}
	if (header.nplayers < 0 || header.nplayers > 7) {
		return false;
	}
	if (header.author[19]) {
		return false;
	}
	return true;
}

// TODO(unknown): the following bob types appear in S2 maps but are unknown
//  Somebody who can run Settlers II please check them out
//  11 (0x0B)
//  40 (0x28)
//  41 (0x29)

const int BOB_NONE = 0x00;

const int BOB_STONE1 = 0x01;
const int BOB_STONE2 = 0x02;
const int BOB_STONE3 = 0x03;
const int BOB_STONE4 = 0x04;
const int BOB_STONE5 = 0x05;
const int BOB_STONE6 = 0x06;

const int BOB_SKELETON1 = 0x07;
const int BOB_SKELETON2 = 0x08;
const int BOB_SKELETON3 = 0x21;

const int BOB_STANDING_STONES1 = 0x18;
const int BOB_STANDING_STONES2 = 0x19;
const int BOB_STANDING_STONES3 = 0x1a;
const int BOB_STANDING_STONES4 = 0x1b;
const int BOB_STANDING_STONES5 = 0x1c;
const int BOB_STANDING_STONES6 = 0x1d;
const int BOB_STANDING_STONES7 = 0x1e;

const int BOB_MUSHROOM1 = 0x01;
const int BOB_MUSHROOM2 = 0x22;

const int BOB_PEBBLE1 = 0x02;
const int BOB_PEBBLE2 = 0x03;
const int BOB_PEBBLE3 = 0x04;
const int BOB_PEBBLE4 = 0x25;
const int BOB_PEBBLE5 = 0x26;
const int BOB_PEBBLE6 = 0x27;

const int BOB_DEADTREE1 = 0x05;
const int BOB_DEADTREE2 = 0x06;
const int BOB_DEADTREE3 = 0x20;
const int BOB_DEADTREE4 = 0x1f;

const int BOB_CACTUS1 = 0x0c;
const int BOB_CACTUS2 = 0x0d;

const int BOB_BUSH1 = 0x11;
const int BOB_BUSH2 = 0x13;
const int BOB_BUSH3 = 0x10;
const int BOB_BUSH4 = 0x12;
const int BOB_BUSH5 = 0x0a;

// Settlers 2 has 8 types of trees.
// I assume that different animation states are stored in the map file
// to create the following 32 values. I assume that 4 trees are grouped
// together.
// Unfortunately, I can't verify that (can't run the S2 editor).
// In the end, it doesn't matter much anyway.
const int BOB_TREE1_T = 0x70;
const int BOB_TREE1_S = 0x71;
const int BOB_TREE1_M = 0x72;
const int BOB_TREE1 = 0x73;

const int BOB_TREE2_T = 0x74;
const int BOB_TREE2_S = 0x75;
const int BOB_TREE2_M = 0x76;
const int BOB_TREE2 = 0x77;

const int BOB_TREE3_T = 0xb0;
const int BOB_TREE3_S = 0xb1;
const int BOB_TREE3_M = 0xb2;
const int BOB_TREE3 = 0xb3;

const int BOB_TREE4_T = 0xb4;
const int BOB_TREE4_S = 0xb5;
const int BOB_TREE4_M = 0xb6;
const int BOB_TREE4 = 0xb7;

const int BOB_TREE5_T = 0xf0;
const int BOB_TREE5_S = 0xf1;
const int BOB_TREE5_M = 0xf2;
const int BOB_TREE5 = 0xf3;

const int BOB_TREE6_T = 0xf4;
const int BOB_TREE6_S = 0xf5;
const int BOB_TREE6_M = 0xf6;
const int BOB_TREE6 = 0xf7;

const int BOB_TREE7_T = 0x30;
const int BOB_TREE7_S = 0x31;
const int BOB_TREE7_M = 0x32;
const int BOB_TREE7 = 0x33;

const int BOB_TREE8_T = 0x34;
const int BOB_TREE8_S = 0x35;
const int BOB_TREE8_M = 0x36;
const int BOB_TREE8 = 0x37;

const int BOB_GRASS1 = 0x0e;
const int BOB_GRASS2 = 0x14;
const int BOB_GRASS3 = 0x0f;

/// Some of the original S2 maps have rather odd sizes. In that case, however,
/// width (and height?) are rounded up to some alignment. The in-file size of a
/// section is stored in the section header (I think ;)).
/// This is the work-around.
///
/// Returns a pointer to the (packed) contents of the section. nullptr if the
/// read failed.
std::unique_ptr<uint8_t[]>
load_s2mf_section(FileRead& fr, int32_t const width, int32_t const height) {
	char buffer[256];

	std::unique_ptr<uint8_t[]> section;
	memcpy(buffer, fr.data(6), 6);
	if (buffer[0] != 0x10 || buffer[1] != 0x27 || buffer[2] != 0x00 || buffer[3] != 0x00 ||
	    buffer[4] != 0x00 || buffer[5] != 0x00) {
		cerr << "Section marker not found" << endl;
		return section;
	}

	uint16_t const dw = fr.unsigned_16();
	uint16_t const dh = fr.unsigned_16();

	{
		uint16_t const one = fr.unsigned_16();
		if (one != 1) {
			throw wexception("expected 1 but found %u", one);
		}
	}
	int32_t const size = fr.signed_32();
	if (size != dw * dh) {
		throw wexception("expected %u but found %u", dw * dh, size);
	}

	if (dw < width || dh < height) {
		cerr << "Section not big enough" << endl;
		return nullptr;
	}

	section.reset(new uint8_t[size]);
	int32_t y = 0;
	for (; y < height; ++y) {
		uint8_t const* const ptr = reinterpret_cast<uint8_t*>(fr.data(width));
		memcpy(section.get() + y * width, ptr, width);
		fr.data(dw - width);  // skip the alignment junk
	}
	while (y < dh) {
		fr.data(dw);  // more alignment junk
		++y;
	}

	return section;
}

std::string get_world_name(S2MapLoader::WorldType world) {
	switch (world) {
	case S2MapLoader::WorldType::kGreenland:
		return "greenland";
	case S2MapLoader::WorldType::kBlackland:
		return "blackland";
	case S2MapLoader::WorldType::kWinterland:
		return "winterland";
	}
	NEVER_HERE();
}

/// Returns S2 terrain index into (pre one-world) terrain names. Those are then
/// looked up in the legacy conversion code and this gives the Widelands
/// terrain.
class TerrainConverter {
public:
	explicit TerrainConverter(Widelands::Descriptions* descriptions);
	Widelands::DescriptionIndex lookup(S2MapLoader::WorldType world, int8_t c) const;

protected:
	Widelands::Descriptions* descriptions_;
	const std::map<S2MapLoader::WorldType, std::vector<std::string>> table_;

private:
	DISALLOW_COPY_AND_ASSIGN(TerrainConverter);
};

TerrainConverter::TerrainConverter(Widelands::Descriptions* descriptions)
   : descriptions_(descriptions),
     table_{std::make_pair(
               S2MapLoader::WorldType::kGreenland,
               std::vector<std::string>{
                  "summer_steppe", "summer_mountain1", "summer_snow", "summer_swamp",
                  "summer_beach", "summer_water", "summer_meadow1", "summer_meadow2",
                  "summer_meadow3", "summer_mountain2", "summer_mountain3", "summer_mountain4",
                  "summer_steppe_barren", "summer_meadow4", "lava", "summer_mountain_meadow"}),
            std::make_pair(S2MapLoader::WorldType::kBlackland,
                           std::vector<std::string>{
                              "ashes", "mountain1", "lava-stone1", "lava-stone2", "strand", "water",
                              "hardground1", "hardground2", "hardground3", "mountain2", "mountain3",
                              "mountain4", "ashes2", "hardground4", "lava", "hardlava"}),
            std::make_pair(S2MapLoader::WorldType::kWinterland,
                           std::vector<std::string>{
                              "tundra", "mountain1", "ice_flows", "ice_flows2", "ice", "water",
                              "tundra_taiga", "tundra2", "tundra3", "mountain2", "mountain3",
                              "mountain4", "strand", "taiga", "lava", "snow"})} {
}

Widelands::DescriptionIndex TerrainConverter::lookup(S2MapLoader::WorldType world, int8_t c) const {
	switch (c) {
	// the following comments are valid for greenland - blackland and winterland have equivalents
	// source: https://settlers2.net/documentation/world-map-file-format-wldswd/
	case 0x00:
		c = 0;
		break;  // steppe meadow1
	case 0x01:
		c = 1;
		break;  // mountain 1
	case 0x02:
		c = 2;
		break;  // snow
	case 0x03:
		c = 3;
		break;  // swamp
	case 0x04:
		c = 4;
		break;  // steppe = strand
	case 0x05:
		c = 5;
		break;  // water
	case 0x06:
		c = 4;
		break;  // strand
	case 0x07:
		c = 12;
		break;  // steppe 2 = dry land
	case 0x08:
		c = 6;
		break;  // meadow 1
	case 0x09:
		c = 7;
		break;  // meadow 2
	case 0x0a:
		c = 8;
		break;  // meadow 3
	case 0x0b:
		c = 9;
		break;  // mountain 2
	case 0x0c:
		c = 10;
		break;  // mountain 3
	case 0x0d:
		c = 11;
		break;  // mountain 4
	case 0x0e:
		c = 12;
		break;  // steppe meadow 2
	case 0x0f:
		c = 13;
		break;  // flower meadow
	case 0x10:
		c = 14;
		break;  // lava
	// case 0x11: // color
	case 0x12:
		c = 15;
		break;  // mountain meadow
	case 0x13:
		c = 4;
		break;  // unknown texture

	default:
		log_warn("Unknown texture %x. Defaulting to water.\n", c);
		c = 7;
		break;
	}

	const std::string& terrain_name = table_.at(world)[c];
	try {
		return descriptions_->load_terrain(terrain_name);
	} catch (const WException&) {
		throw wexception("world does not define terrain type %s, you cannot "
		                 "play this settlers map",
		                 terrain_name.c_str());
	}
}

}  // namespace

S2MapLoader::S2MapLoader(const std::string& filename, Widelands::Map& M)
   : Widelands::MapLoader(filename, M), filename_(filename), worldtype_(WorldType::kGreenland) {
}

/// Load the header. The map will then return valid information when
/// get_width(), get_nrplayers(), get_author() and so on are called.
int32_t S2MapLoader::preload_map(bool const scenario, AddOns::AddOnsList* addons) {
	assert(get_state() != State::kLoaded);

	// s2 maps don't have world add-ons
	// (UNTESTED because I don't have an s2 map to test this with)
	if (addons) {
		for (auto it = addons->begin(); it != addons->end();) {
			if ((*it)->category == AddOns::AddOnCategory::kWorld) {
				it = addons->erase(it);
			} else {
				++it;
			}
		}
	}

	map_.cleanup();

	FileRead fr;
	fr.open(*g_fs, filename_);

	load_s2mf_header(fr);

	if (scenario) {
		//  Load this as scenario. There is no such a thing as S2 scenario,
		//  therefore set the tribes and some default names.

		const char* const names[] = {
		   //  Just for fun: some roman names
		   "Marius", "Avitus", "Silvanus", "Caius", "Augustus", "Maximus", "Titus", "Rufus",
		};

		Widelands::PlayerNumber const nr_players = map_.get_nrplayers();
		iterate_player_numbers(i, nr_players) {
			map_.set_scenario_player_tribe(i, "empire");
			map_.set_scenario_player_name(i, names[i - 1]);
			map_.set_scenario_player_ai(i, "");
			map_.set_scenario_player_closeable(i, false);
		}
	}

	set_state(State::kPreLoaded);

	return 0;
}

/**
 * Completely loads the map, loads the graphics and places all the objects.
 * From now on the Map* can't be set to another one.
 */
int32_t S2MapLoader::load_map_complete(Widelands::EditorGameBase& egbase,
                                       MapLoader::LoadType /* type */) {
	std::string timer_message = "S2MapLoader::load_map_complete() for '";
	timer_message += map_.get_name();
	timer_message += "' took %ums";
	ScopedTimer timer(timer_message, true);
	Notifications::publish(UI::NoteLoadingMessage(_("Loading map…")));

	load_s2mf(egbase);

	map_.recalc_whole_map(egbase);

	postload_set_port_spaces(egbase);

	set_state(State::kLoaded);

	return 0;
}

/**
 * Loads informational data of an S2 map.
 * Throws exception if data is invalid.
 */
void S2MapLoader::load_s2mf_header(FileRead& fr) {
	// no need to check file size: fr.data(..) already throws if the file is too small
	S2MapDescrHeader header;
	memcpy(&header, fr.data(sizeof(header)), sizeof(header));

//  Header must be swapped for big-endian Systems, works at the moment only
//  for PowerPC architecture
//  TODO(unknown): Generalize this
#if defined(__ppc__)
	header.w = swap_16(header.w);
	header.h = swap_16(header.h);
#endif

	// Check header validity to prevent unexpected crashes later
	if (!is_valid_header(header)) {
		throw wexception("invalid S2 file");
	}

	//  don't really set size, but make the structures valid
	map_.width_ = header.w;
	map_.height_ = header.h;

	map_.set_author(header.author);
	map_.set_name(header.name);
	map_.set_nrplayers(header.nplayers);
	map_.set_description(_("Bluebyte Settlers II Map. No comment defined!"));

	worldtype_ = static_cast<WorldType>(header.uses_world);
}

/**
 * This loads a given file as a settlers 2 map file
 */
void S2MapLoader::load_s2mf(Widelands::EditorGameBase& egbase) {
	uint8_t* pc;

	FileRead fr;
	fr.open(*g_fs, filename_);

	load_s2mf_header(fr);
	map_.set_size(map_.width_, map_.height_);

	//  The header must already have been processed.
	assert(map_.fields_.get());
	int16_t const mapwidth = map_.get_width();
	int16_t const mapheight = map_.get_height();
	assert(mapwidth > 0 && mapheight > 0);
	egbase.allocate_player_maps();  //  initializes player_fields.vision

	//  SWD-SECTION 1: Heights
	std::unique_ptr<uint8_t[]> section(load_s2mf_section(fr, mapwidth, mapheight));
	if (!section) {
		throw wexception("Section 1 (Heights) not found");
	}

	Widelands::Field* f = map_.fields_.get();
	pc = section.get();
	for (int16_t y = 0; y < mapheight; ++y) {
		for (int16_t x = 0; x < mapwidth; ++x, ++f, ++pc) {
			f->set_height(*pc);
		}
	}

	//  SWD-SECTION 2: Terrain 1
	section = load_s2mf_section(fr, mapwidth, mapheight);
	if (!section) {
		throw wexception("Section 2 (Terrain 1) not found");
	}

	egbase.mutable_descriptions()->set_old_world_name(get_world_name(worldtype_));

	Widelands::Descriptions* descriptions = egbase.mutable_descriptions();
	TerrainConverter terrain_converter(descriptions);

	f = map_.fields_.get();
	pc = section.get();
	for (int16_t y = 0; y < mapheight; ++y) {
		for (int16_t x = 0; x < mapwidth; ++x, ++f, ++pc) {
			uint8_t c = *pc;
			// Harbour buildspace & textures - Information taken from:
			if (c & 0x40) {
				port_spaces_to_set_.insert(Widelands::Coords(x, y));
			}
			f->set_terrain_d(terrain_converter.lookup(worldtype_, c & 0x1f));
		}
	}

	//  SWD-SECTION 3: Terrain 2
	section = load_s2mf_section(fr, mapwidth, mapheight);
	if (!section) {
		throw wexception("Section 3 (Terrain 2) not found");
	}

	f = map_.fields_.get();
	pc = section.get();
	for (int16_t y = 0; y < mapheight; ++y) {
		for (int16_t x = 0; x < mapwidth; ++x, ++f, ++pc) {
			uint8_t c = *pc;
			// Harbour buildspace & textures - Information taken from:
			// https://settlers2.net/documentation/world-map-file-format-wldswd/
			if (c & 0x40) {
				port_spaces_to_set_.insert(Widelands::Coords(x, y));
			}
			f->set_terrain_r(terrain_converter.lookup(worldtype_, c & 0x1f));
		}
	}

	//  SWD-SECTION 4: Existing Roads
	//  As loading of Roads at game-start is not supported, yet - we simply
	//  skip it.
	section = load_s2mf_section(fr, mapwidth, mapheight);
	if (!section) {
		throw wexception("Section 4 (Existing Roads) not found");
	}

	//  SWD-SECTION 5: Bobs
	std::unique_ptr<uint8_t[]> bobs(load_s2mf_section(fr, mapwidth, mapheight));
	if (!bobs) {
		throw wexception("Section 5 (Bobs) not found");
	}

	//  SWD-SECTION 6: Ways
	//  This describes where you can put ways
	//  0xc* == it's not possible to build ways here now
	//  0x80 == Heres a HQ, owner is Player number
	//   owner == 0 -> blue
	//   owner == 1 -> yellow
	//   owner == 2 -> red
	//   owner == 3 -> pink
	//   owner == 4 -> grey
	//   owner == 6 -> green
	//   owner == 6 -> orange
	// TODO(hessenfarmer): this comment is not correct section 6 determines how the values of section
	// 5 are to be interpreted. Solution is working more the less though
	section = load_s2mf_section(fr, mapwidth, mapheight);
	if (!section) {
		throw wexception("Section 6 (Ways) not found");
	}

	for (int16_t y = 0; y < mapheight; ++y) {
		uint32_t i = y * mapwidth;
		for (int16_t x = 0; x < mapwidth; ++x, ++i) {
			// ignore everything but HQs
			if (section[i] == 0x80) {
				if (bobs[i] < map_.get_nrplayers()) {
					map_.set_starting_pos(bobs[i] + 1, Widelands::Coords(x, y));
				}
			}
		}
	}

	//  SWD-SECTION 7: Animals
	//  0x01        == Bunny
	//  0x02        == fox
	//  0x03        == reindeer
	//  0x04 + 0x07 == deer
	//  0x05 + 0x08 == duck
	//  0x06        == sheep
	//  0x09        == donkey
	section = load_s2mf_section(fr, mapwidth, mapheight);
	if (!section) {
		throw wexception("Section 7 (Animals) not found");
	}

	for (uint16_t y = 0; y < mapheight; ++y) {
		uint32_t i = y * mapwidth;
		for (uint16_t x = 0; x < mapwidth; ++x, ++i) {
			std::string bobname;

			switch (section[i]) {
			case 0:
				break;
			case 0x01:
				bobname = "bunny";
				break;
			case 0x02:
				bobname = "fox";
				break;
			case 0x03:
				bobname = "reindeer";
				break;
			case 0x04:
				bobname = "deer";
				break;
			case 0x05:
				bobname = "duck";
				break;
			case 0x06:
				bobname = "sheep";
				break;
			case 0x07:
				bobname = "deer";
				break;
			case 0x08:
				bobname = "duck";
				break;
			case 0x09:
				bobname = "moose";
				break;  // original "donkey"
			default:
				cerr << "Unsupported animal: " << static_cast<int32_t>(section[i]) << endl;
				break;
			}

			if (!bobname.empty()) {
				const Widelands::DescriptionIndex idx = descriptions->load_critter(bobname);
				egbase.create_critter(Widelands::Coords(x, y), idx);
			}
		}
	}

	//  SWD-SECTION 8: Unknown
	//  Skipped
	section = load_s2mf_section(fr, mapwidth, mapheight);
	if (!section) {
		throw wexception("Section 8 (Unknown) not found");
	}

	//  SWD-SECTION 9: Buildings
	//  What kind of buildings can be build?
	//  0x01 == flags (??)
	//  0x02 == buildings (small) (??)
	//  0x04 == buildings
	//  0x09 == flags
	//  0x0a == buildings (small) (??)
	//  0x0c == buildings (big) (??)
	//  0x0d == mining
	//  0x68 == trees
	//  0x78 == no buildings
	std::unique_ptr<uint8_t[]> buildings(load_s2mf_section(fr, mapwidth, mapheight));
	if (!buildings) {
		throw wexception("Section 9 (Buildings) not found");
	}

	//  SWD-SECTION 10: Unknown
	//  Skipped
	section = load_s2mf_section(fr, mapwidth, mapheight);
	if (!section) {
		throw wexception("Section 10 (Unknown) not found");
	}

	//  SWD-SECTION 11: Settlers2 Mapeditor tool position
	//  In this section the positions of the Mapeditor tools seem to be
	//  saved. But as this is unusable for playing or the WL-Editor, we just
	//  skip it!
	section = load_s2mf_section(fr, mapwidth, mapheight);
	if (!section) {
		throw wexception("Section 11 (Tool Position) not found");
	}

	//  SWD-SECTION 12: Resources
	//  0x00 == Water
	//  0x87 == fish
	//  0x21 == ground water
	//  0x40 == nothing
	//  0x51-57 == gold 1-7
	//  0x49-4f == iron 1-7
	//  0x41-47 == coal 1-7
	//  0x59-5f == granite 1-7
	// fish and ground water are ignored here as they are infinite in S2 and not in widelands
	// fish an water are defaulted per terrain in widelands game, so it is safe to ignore them
	section = load_s2mf_section(fr, mapwidth, mapheight);
	if (!section) {
		throw wexception("Section 12 (Resources) not found");
	}

	pc = section.get();
	char const* res;
	int32_t amount = 0;
	for (uint16_t y = 0; y < mapheight; ++y) {
		for (uint16_t x = 0; x < mapwidth; ++x, ++pc) {
			auto c = map_.get_fcoords(Widelands::Coords(x, y));
			uint8_t value = *pc;

			switch (value & 0xF8) {
			case 0x40:
				res = "resource_coal";
				amount = value & 7;
				break;
			case 0x48:
				res = "resource_iron";
				amount = value & 7;
				break;
			case 0x50:
				res = "resource_gold";
				amount = value & 7;
				break;
			case 0x58:
				res = "resource_stones";
				amount = value & 7;
				break;
			default:
				res = "";
				amount = 0;
				break;
			}

			Widelands::DescriptionIndex nres = 0;
			if (*res) {
				try {
					nres = descriptions->load_resource(res);
				} catch (const WException&) {
					throw wexception("world does not define resource type %s, you cannot "
					                 "play this settlers map",
					                 res);
				}
			}
			const Widelands::ResourceAmount real_amount =
			   static_cast<Widelands::ResourceAmount>(2.86f * amount);
			map_.initialize_resources(c, nres, real_amount);
		}
	}

	//  SWD-SECTION 13: Higlights and Shadows
	//  It seems as if the Settlers2 Mapeditor saves the highlights and
	//  shadows from slopes to this section.
	//  But as this is unusable for the WL engine, we just skip it.
	section = load_s2mf_section(fr, mapwidth, mapheight);
	if (!section) {
		throw wexception("Section 13 (Highlights and Shadows) not found");
	}

	//  SWD-SECTION 14: Fieldcount
	//  Describes to which island the field sticks
	//  0 == water
	//  1 == island 1
	//  2 == island 2
	//  ...
	//  fe == killing field (lava)
	//
	//  Unusable (and if it was needed, it would have to be recomputed anyway
	//  to verify it) so we simply skip it.
	section = load_s2mf_section(fr, mapwidth, mapheight);
	if (!section) {
		throw wexception("Section 14 (Island id) not found");
	}

	fr.close();

	//  Map is completely read into memory.
	//  Now try to convert the remaining stuff to Widelands-format. This will
	//  read and construct the name of the old immovables before the one world
	//  conversion. We will then convert them using the
	//  Descriptions compatibility information for the old world name.
	// Puts an immovable with the 'immovable_name' onto the field 'locations'.
	auto place_immovable = [&egbase, descriptions](
	                          const Widelands::Coords& location, const std::string& immovable_name) {
		try {
			const Widelands::DescriptionIndex idx = descriptions->load_immovable(immovable_name);
			egbase.create_immovable(location, idx, nullptr /* owner */);
		} catch (const WException&) {
			throw wexception("world does not define immovable type %s, you cannot "
			                 "play this settlers map",
			                 immovable_name.c_str());
		}
	};

	uint8_t c;
	for (uint16_t y = 0; y < mapheight; ++y) {
		for (uint16_t x = 0; x < mapwidth; ++x) {
			const Widelands::Coords location(x, y);
			Widelands::MapIndex const index = Widelands::Map::get_index(location, mapwidth);
			c = bobs[index];
			std::string bobname;
			if (buildings[index] == 0x78) {
				switch (c) {
				case BOB_STONE1:
					bobname = "stones1";
					break;
				case BOB_STONE2:
					bobname = "stones2";
					break;
				case BOB_STONE3:
					bobname = "stones3";
					break;
				case BOB_STONE4:
					bobname = "stones4";
					break;
				case BOB_STONE5:
					bobname = "stones5";
					break;
				case BOB_STONE6:
					bobname = "stones6";
					break;
				default:
					break;
				}
				if (!bobname.empty()) {
					place_immovable(location, bobname);
					continue;
				}
			}

			switch (c) {
			case BOB_NONE:
				break;  // DO nothing

			case BOB_PEBBLE1:
				bobname = "pebble1";
				break;
			case BOB_PEBBLE2:
				bobname = "pebble2";
				break;
			case BOB_PEBBLE3:
				bobname = "pebble3";
				break;
			case BOB_PEBBLE4:
				bobname = "pebble4";
				break;
			case BOB_PEBBLE5:
				bobname = "pebble5";
				break;
			case BOB_PEBBLE6:
				bobname = "pebble6";
				break;

			case BOB_MUSHROOM1:
				bobname = "mushroom1";
				break;
			case BOB_MUSHROOM2:
				bobname = "mushroom2";
				break;

			case BOB_DEADTREE1:
				bobname = "deadtree1";
				break;
			case BOB_DEADTREE2:
				bobname = "deadtree2";
				break;
			case BOB_DEADTREE3:
				bobname = "deadtree3";
				break;
			case BOB_DEADTREE4:
				bobname = "deadtree4";
				break;

			case BOB_TREE1_T:
				bobname = "tree1_t";
				break;
			case BOB_TREE1_S:
				bobname = "tree1_s";
				break;
			case BOB_TREE1_M:
				bobname = "tree1_m";
				break;
			case BOB_TREE1:
				bobname = "tree1";
				break;

			case BOB_TREE2_T:
				bobname = "tree2_t";
				break;
			case BOB_TREE2_S:
				bobname = "tree2_s";
				break;
			case BOB_TREE2_M:
				bobname = "tree2_m";
				break;
			case BOB_TREE2:
				bobname = "tree2";
				break;

			case BOB_TREE3_T:
				bobname = "tree3_t";
				break;
			case BOB_TREE3_S:
				bobname = "tree3_s";
				break;
			case BOB_TREE3_M:
				bobname = "tree3_m";
				break;
			case BOB_TREE3:
				bobname = "tree3";
				break;

			case BOB_TREE4_T:
				bobname = "tree4_t";
				break;
			case BOB_TREE4_S:
				bobname = "tree4_s";
				break;
			case BOB_TREE4_M:
				bobname = "tree4_m";
				break;
			case BOB_TREE4:
				bobname = "tree4";
				break;

			case BOB_TREE5_T:
				bobname = "tree5_t";
				break;
			case BOB_TREE5_S:
				bobname = "tree5_s";
				break;
			case BOB_TREE5_M:
				bobname = "tree5_m";
				break;
			case BOB_TREE5:
				bobname = "tree5";
				break;

			case BOB_TREE6_T:
				bobname = "tree6_t";
				break;
			case BOB_TREE6_S:
				bobname = "tree6_s";
				break;
			case BOB_TREE6_M:
				bobname = "tree6_m";
				break;
			case BOB_TREE6:
				bobname = "tree6";
				break;

			case BOB_TREE7_T:
				bobname = "tree7_t";
				break;
			case BOB_TREE7_S:
				bobname = "tree7_s";
				break;
			case BOB_TREE7_M:
				bobname = "tree7_m";
				break;
			case BOB_TREE7:
				bobname = "tree7";
				break;

			case BOB_TREE8_T:
				bobname = "tree8_t";
				break;
			case BOB_TREE8_S:
				bobname = "tree8_s";
				break;
			case BOB_TREE8_M:
				bobname = "tree8_m";
				break;
			case BOB_TREE8:
				bobname = "tree8";
				break;

			case BOB_GRASS1:
				bobname = "grass1";
				break;
			case BOB_GRASS2:
				bobname = "grass2";
				break;
			case BOB_GRASS3:
				bobname = "grass3";
				break;

			case BOB_STANDING_STONES1:
				bobname = "sstones1";
				break;
			case BOB_STANDING_STONES2:
				bobname = "sstones2";
				break;
			case BOB_STANDING_STONES3:
				bobname = "sstones3";
				break;
			case BOB_STANDING_STONES4:
				bobname = "sstones4";
				break;
			case BOB_STANDING_STONES5:
				bobname = "sstones5";
				break;
			case BOB_STANDING_STONES6:
				bobname = "sstones6";
				break;
			case BOB_STANDING_STONES7:
				bobname = "sstones7";
				break;

			case BOB_SKELETON1:
				bobname = "skeleton1";
				break;
			case BOB_SKELETON2:
				bobname = "skeleton2";
				break;
			case BOB_SKELETON3:
				bobname = "skeleton3";
				break;

			case BOB_CACTUS1:
				bobname = worldtype_ != S2MapLoader::WorldType::kWinterland ? "cactus1" : "snowman";
				break;
			case BOB_CACTUS2:
				bobname = worldtype_ != S2MapLoader::WorldType::kWinterland ? "cactus2" : "track";
				break;

			case BOB_BUSH1:
				bobname = "bush1";
				break;
			case BOB_BUSH2:
				bobname = "bush2";
				break;
			case BOB_BUSH3:
				bobname = "bush3";
				break;
			case BOB_BUSH4:
				bobname = "bush4";
				break;
			case BOB_BUSH5:
				bobname = "bush5";
				break;

			default:
				cerr << "Unknown bob " << static_cast<uint32_t>(c) << endl;
				break;
			}

			if (!bobname.empty()) {
				place_immovable(location, bobname);
			}
		}
	}
	//  WORKAROUND:
	//  Unfortunately the Widelands engine is not completely compatible with
	//  the Settlers 2; space for buildings is defined differently. To allow
	//  loading of Settlers 2 maps in the majority of cases, check all
	//  starting positions and try to make it Widelands compatible, if its
	//  size is too small.
	map_.recalc_whole_map(egbase);  //  to initialize buildcaps

	const Widelands::PlayerNumber nr_players = map_.get_nrplayers();
	log_info("Checking starting position for all %u players:\n", nr_players);
	iterate_player_numbers(p, nr_players) {
		log_info("-> Player %u: ", p);

		Widelands::Coords starting_pos = map_.get_starting_pos(p);
		if (!starting_pos) {
			//  Do not throw exception, else map will not be loadable in the
			//  editor. Player initialization will keep track of wrong starting
			//  positions.
			log_warn("Has no starting position.\n");
			continue;
		}
		Widelands::FCoords fpos = map_.get_fcoords(starting_pos);

		if (!(map_.get_max_nodecaps(egbase, fpos) & Widelands::BUILDCAPS_BIG)) {
			log_warn("wrong size - trying to fix it: ");
			bool fixed = false;

			Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
			   map_, Widelands::Area<Widelands::FCoords>(fpos, 3));
			do {
				if (map_.get_max_nodecaps(egbase, const_cast<Widelands::FCoords&>(mr.location())) &
				    Widelands::BUILDCAPS_BIG) {
					map_.set_starting_pos(p, mr.location());
					fixed = true;
					break;
				}
			} while (mr.advance(map_));

			// check whether starting position was fixed.
			if (fixed) {
				log_info("Fixed!\n");
			} else {
				//  Do not throw exception, else map will not be loadable in
				//  the editor. Player initialization will keep track of
				//  wrong starting positions.
				log_err("FAILED!\n");
				log_err("Invalid starting position, that could not be fixed.\n");
				log_err("Please try to fix it manually in the editor.\n");
			}
		} else {
			log_info("OK\n");
		}
	}
}

/// Try to fix data which is incompatible between S2 and Widelands.
/// This is only the port space locations.
void S2MapLoader::postload_set_port_spaces(const Widelands::EditorGameBase& egbase) {
	// Set port spaces near desired locations if possible
	for (const Widelands::Coords& coords : port_spaces_to_set_) {
		bool was_set = map_.set_port_space(egbase, coords, true);
		const Widelands::FCoords fc = map_.get_fcoords(coords);
		if (!was_set) {
			// Try to set a port space at alternative location
			Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
			   map_, Widelands::Area<Widelands::FCoords>(fc, 3));
			do {
				was_set = map_.set_port_space(
				   egbase, Widelands::Coords(mr.location().x, mr.location().y), true);
			} while (!was_set && mr.advance(map_));
		}
		if (!was_set) {
			log_err("FAILED! No port buildspace for (%i, %i) found!\n", fc.x, fc.y);
		} else {
			log_info("SUCCESS! Port buildspace set for (%i, %i) \n", fc.x, fc.y);
		}
	}
	map_.recalculate_allows_seafaring();
}
