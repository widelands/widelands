/*
 * Copyright (C) 2002, 2003, 2006-2010, 2011-2013 by the Widelands Development Team
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

#include "s2map.h"

#include <iomanip>
#include <iostream>

#include <boost/foreach.hpp>
#include <libintl.h>

#include "constants.h"
#include "i18n.h"
#include "io/filesystem/layered_filesystem.h"
#include "log.h"
#include "logic/editor_game_base.h"
#include "logic/field.h"
#include "logic/game.h"
#include "logic/map.h"
#include "logic/mapregion.h"
#include "logic/message.h"
#include "logic/player.h"
#include "logic/world.h"
#include "map_io/map_loader.h"
#include "upcast.h"
#include "wexception.h"

using std::cerr;
using std::endl;
using std::ios;
using std::setiosflags;

// this is a detail of S2 maps
#define CRITTER_PER_DEFINITION   1

S2_Map_Loader::S2_Map_Loader(const char * filename, Widelands::Map & M)
: Widelands::Map_Loader(filename, M)
{
	strncpy(m_filename, filename, sizeof(m_filename));
	if (m_filename[sizeof(m_filename) - 1])
		throw;
}


/// Load the header. The map will then return valid information when
/// get_width(), get_nrplayers(), get_author() and so on are called.
int32_t S2_Map_Loader::preload_map(bool const scenario) {
	assert(get_state() != STATE_LOADED);

	m_map.cleanup();

	load_s2mf_header();

	if (not Widelands::World::exists_world(m_map.get_world_name()))
		throw wexception("world %s does not exist", m_map.get_world_name());

	if (scenario) {
		//  Load this as scenario. There is no such a thing as S2 scenario,
		//  therefore set the tribes and some default names.

		const char * const names[] = { //  Just for fun: some roman names
			"Marius",
			"Avitus",
			"Silvanus",
			"Caius",
			"Augustus",
			"Maximus",
			"Titus",
			"Rufus",
		};

		Widelands::Player_Number const nr_players = m_map.get_nrplayers();
		iterate_player_numbers(i, nr_players) {
			m_map.set_scenario_player_tribe(i, "empire");
			m_map.set_scenario_player_name(i, names[i - 1]);
			m_map.set_scenario_player_ai(i, "");
			m_map.set_scenario_player_closeable(i, false);
		}
	}


	set_state(STATE_PRELOADED);

	return 0;
}

/**
 * load predefined world of the S2Map
 */
void S2_Map_Loader::load_world() {
	assert(get_state() == STATE_PRELOADED);
	m_map.load_world();
	set_state(STATE_WORLD_LOADED);
}


/**
 * Completely loads the map, loads the
 * corresponding world, loads the graphics
 * and places all the objects. From now on
 * the Map* can't be set to another one.
 */
int32_t S2_Map_Loader::load_map_complete
	(Widelands::Editor_Game_Base & egbase, bool)
{
	if (get_state() == STATE_PRELOADED)
		load_world();
	assert(get_state() == STATE_WORLD_LOADED);

	m_map.set_size(m_map.m_width, m_map.m_height);
	load_s2mf(egbase);

	m_map.recalc_whole_map();

	postload_fix_conversion(egbase);

	set_state(STATE_LOADED);

	return 0;
}


/// Some of the original S2 maps have rather odd sizes. In that case, however,
/// width (and height?) are rounded up to some alignment. The in-file size of a
/// section is stored in the section header (I think ;)).
/// This is the work-around.
///
/// Returns a pointer to the (packed) contents of the section. 0 if the read
/// failed.
/// If successful, you must free the returned pointer.
uint8_t * S2_Map_Loader::load_s2mf_section
	(FileRead & fr, int32_t const width, int32_t const height)
{
	char buffer[256];

	memcpy(buffer, fr.Data(6), 6);
	if
		(buffer[0] != 0x10 or
		 buffer[1] != 0x27 or
		 buffer[2] != 0x00 or
		 buffer[3] != 0x00 or
		 buffer[4] != 0x00 or
		 buffer[5] != 0x00)
	{
		cerr << "Section marker not found" << endl;
		return nullptr;
	}

	uint16_t const dw = fr.Unsigned16();
	uint16_t const dh = fr.Unsigned16();

	{
		uint16_t const one = fr.Unsigned16();
		if (one != 1)
			throw wexception("expected 1 but found %u", one);
	}
	int32_t const size = fr.Signed32();
	if (size != dw * dh)
		throw wexception("expected %u but found %u", dw * dh, size);

	if (dw < width || dh < height) {
		cerr << "Section not big enough" << endl;
		return nullptr;
	}

	uint8_t * const section = static_cast<uint8_t *>(malloc(size));

	try {
		int32_t y = 0;
		for (; y < height; ++y) {
			uint8_t const * const ptr =
				reinterpret_cast<uint8_t *>(fr.Data(width));
			memcpy(section + y * width, ptr, width);
			fr.Data(dw - width); //  skip the alignment junk
		}
		while (y < dh) {
			fr.Data(dw); //  more alignment junk
			++y;
		}
	} catch (...) {
		free(section);
		throw;
	}

	return section;
}


/**
 * Load informational data of an S2 map
 */
void S2_Map_Loader::load_s2mf_header()
{
	FileRead fr;

	fr.Open(*g_fs, m_filename);

	S2MapDescrHeader header;
	memcpy(&header, fr.Data(sizeof(header)), sizeof(header));

	//  Header must be swapped for big-endian Systems, works at the moment only
	//  for PowerPC architecture
	//  \todo: Generalize this
#if defined(__ppc__)
	header.w = Swap16(header.w);
	header.h = Swap16(header.h);
#endif

	//  don't really set size, but make the structures valid
	m_map.m_width  = header.w;
	m_map.m_height = header.h;

	m_map.set_author(header.author);
	m_map.set_name(header.name);
	m_map.set_nrplayers(header.nplayers);
	m_map.set_description(_("Bluebyte Settlers II Map. No comment defined!"));

	//  This is not the name, this is the directory of the world, so please
	//  do not translate it.
	switch (header.uses_world) {
	case 0: m_map.set_world_name("greenland");  break;
	case 1: m_map.set_world_name("blackland");  break;
	case 2: m_map.set_world_name("winterland"); break;
	default:
		assert(false);
		break;
	}
}


/**
 * This loads a given file as a settlers 2 map file
 */
void S2_Map_Loader::load_s2mf(Widelands::Editor_Game_Base & egbase)
{
	uint8_t * section   = nullptr;
	uint8_t * bobs      = nullptr;
	uint8_t * buildings = nullptr;

	uint8_t * pc;

	try {
		FileRead fr;
		fr.Open(*g_fs, m_filename);

		S2MapDescrHeader header;
		memcpy(&header, fr.Data(sizeof(header)), sizeof(header));

		//  Header must be swapped for big-endian Systems, works at the moment
		//  only for PowerPC architecture.
		//
		//  \todo Generalize
#if defined(__ppc__)
		header.w = Swap16(header.w);
		header.h = Swap16(header.h);
#endif

		//  The header must already have been processed.
		assert(m_map.m_world);
		assert(m_map.m_fields);
		Widelands::X_Coordinate const mapwidth  = m_map.get_width ();
		Widelands::Y_Coordinate const mapheight = m_map.get_height();
		assert(mapwidth  == header.w);
		assert(mapheight == header.h);
		egbase.allocate_player_maps(); //  initializes player_fields.vision


		//  SWD-SECTION 1: Heights
		section = load_s2mf_section(fr, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 1 (Heights) not found");

		Widelands::Field * f = m_map.m_fields;
		pc = section;
		for (Widelands::Y_Coordinate y = 0; y < mapheight; ++y)
			for (Widelands::X_Coordinate x = 0; x < mapwidth; ++x, ++f, ++pc)
				f->set_height(*pc);
		free(section);
		section = nullptr;


		//  SWD-SECTION 2: Terrain 1
		section = load_s2mf_section(fr, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 2 (Terrain 1) not found");

		f = m_map.m_fields;
		pc = section;
		for (Widelands::Y_Coordinate y = 0; y < mapheight; ++y)
			for (Widelands::X_Coordinate x = 0; x < mapwidth; ++x, ++f, ++pc) {
				uint8_t c = *pc;
				// Harbour buildspace & textures - Information taken from:
				if (c & 0x40)
					m_map.set_port_space(Widelands::Coords(x, y), true);
				c &= 0x1f;
				switch (c) {
				// the following comments are valid for greenland - blackland and winterland have equivalents
				// source: http://bazaar.launchpad.net/~xaser/s25rttr/s25edit/view/head:/WLD_reference.txt
				case 0x00: c =  0; break; // steppe meadow1
				case 0x01: c =  1; break; // mountain 1
				case 0x02: c =  2; break; // snow
				case 0x03: c =  3; break; // swamp
				case 0x04: c =  4; break; // steppe = strand
				case 0x05: c =  5; break; // water
				case 0x06: c =  4; break; // strand
				case 0x07: c = 12; break; // steppe 2 = dry land
				case 0x08: c =  6; break; // meadow 1
				case 0x09: c =  7; break; // meadow 2
				case 0x0a: c =  8; break; // meadow 3
				case 0x0b: c =  9; break; // mountain 2
				case 0x0c: c = 10; break; // mountain 3
				case 0x0d: c = 11; break; // mountain 4
				case 0x0e: c = 12; break; // steppe meadow 2
				case 0x0f: c = 13; break; // flower meadow
				case 0x10: c = 14; break; // lava
				// case 0x11: // color
				case 0x12: c = 15; break; // mountain meadow
				case 0x13: c =  4; break; // unknown texture

				default:
					c = 7;
					cerr
						<< "ERROR: Unknown texture1: "
						<< setiosflags(ios::hex) << c
						<< setiosflags(ios::dec) << " ("
						<< x << "," << y << ") (defaults to water!)" << endl;
						break;
				}
				f->set_terrain_d(c);
			}
		free(section);
		section = nullptr;


		//  SWD-SECTION 3: Terrain 2
		section = load_s2mf_section(fr, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 3 (Terrain 2) not found");

		f = m_map.m_fields;
		pc = section;
		for (Widelands::Y_Coordinate y = 0; y < mapheight; ++y)
			for (Widelands::X_Coordinate x = 0; x < mapwidth; ++x, ++f, ++pc) {
				uint8_t c = *pc;
				// Harbour buildspace & textures - Information taken from:
				// http://bazaar.launchpad.net/~xaser/s25rttr/s25edit/view/head:/WLD_reference.txt
				if (c & 0x40)
					m_map.set_port_space(Widelands::Coords(x, y), true);
				c &= 0x1f;
				switch (c) {
				// the following comments are valid for greenland - blackland and winterland have equivalents
				// source: http://bazaar.launchpad.net/~xaser/s25rttr/s25edit/view/head:/WLD_reference.txt
				case 0x00: c =  0; break; // steppe meadow1
				case 0x01: c =  1; break; // mountain 1
				case 0x02: c =  2; break; // snow
				case 0x03: c =  3; break; // swamp
				case 0x04: c =  4; break; // steppe = strand
				case 0x05: c =  5; break; // water
				case 0x06: c =  4; break; // strand
				case 0x07: c = 12; break; // steppe 2 = dry land
				case 0x08: c =  6; break; // meadow 1
				case 0x09: c =  7; break; // meadow 2
				case 0x0a: c =  8; break; // meadow 3
				case 0x0b: c =  9; break; // mountain 2
				case 0x0c: c = 10; break; // mountain 3
				case 0x0d: c = 11; break; // mountain 4
				case 0x0e: c = 12; break; // steppe meadow 2
				case 0x0f: c = 13; break; // flower meadow
				case 0x10: c = 14; break; // lava
				// case 0x11: // color
				case 0x12: c = 15; break; // mountain meadow
				case 0x13: c =  4; break; // unknown texture

				default:
					c = 7;
					cerr
						<< "ERROR: Unknown texture1: "
						<< setiosflags(ios::hex) << c
						<< setiosflags(ios::dec) << " ("
						<< x << "," << y << ") (defaults to water!)" << endl;
					break;
				}
				f->set_terrain_r(c);
			}
		free(section);
		section = nullptr;


		//  SWD-SECTION 4: Existing Roads
		//  As loading of Roads at game-start is not supported, yet - we simply
		//  skip it.
		section = load_s2mf_section(fr, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 4 (Existing Roads) not found");
		free(section);
		section = nullptr;


		//  SWD-SECTION 5: Bobs
		bobs = load_s2mf_section(fr, mapwidth, mapheight);
		if (!bobs)
			throw wexception("Section 5 (Bobs) not found");


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
		section = load_s2mf_section(fr, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 6 (Ways) not found");

		for (Widelands::Y_Coordinate y = 0; y < mapheight; ++y) {
			uint32_t i = y * mapwidth;
			for (Widelands::X_Coordinate x = 0; x < mapwidth; ++x, ++i) {
				// ignore everything but HQs
				if (section[i] == 0x80) {
					if (bobs[i] < m_map.get_nrplayers())
						m_map.set_starting_pos
							(bobs[i] + 1, Widelands::Coords(x, y));
				}
			}
		}
		free(section);
		section = nullptr;


		//  SWD-SECTION 7: Animals
		//  0x01        == Bunny
		//  0x02        == fox
		//  0x03        == reindeer
		//  0x04 + 0x07 == deer
		//  0x05 + 0x08 == duck
		//  0x06        == sheep
		//  0x09        == donkey
		section = load_s2mf_section(fr, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 7 (Animals) not found");

		for (Widelands::Y_Coordinate y = 0; y < mapheight; ++y) {
			uint32_t i = y * mapwidth;
			for (Widelands::X_Coordinate x = 0; x < mapwidth; ++x, ++i) {
				char const * bobname = nullptr;

				switch (section[i]) {
				case 0: break;
				case 0x01: bobname = "bunny";    break;
				case 0x02: bobname = "fox";      break;
				case 0x03: bobname = "reindeer"; break;
				case 0x04: bobname = "deer";     break;
				case 0x05: bobname = "duck";     break;
				case 0x06: bobname = "sheep";    break;
				case 0x07: bobname = "deer";     break;
				case 0x08: bobname = "duck";     break;
				case 0x09: bobname = "elk";      break; // original "donkey"
				default:
					cerr
						<< "Unsupported animal: " << static_cast<int32_t>(section[i])
						<< endl;
					break;
				}

				if (bobname) {
					int32_t const idx = m_map.world().get_bob(bobname);
					if (idx < 0)
						throw wexception("Missing bob type %s", bobname);
					for (uint32_t z = 0; z < CRITTER_PER_DEFINITION; ++z)
						egbase.create_bob(Widelands::Coords(x, y), idx);
				}
			}
		}
		free(section);
		section = nullptr;


		//  SWD-SECTION 8: Unknown
		//  Skipped
		section = load_s2mf_section(fr, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 8 (Unknown) not found");
		free(section);
		section = nullptr;


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
		buildings = load_s2mf_section(fr, mapwidth, mapheight);
		if (!buildings)
			throw wexception("Section 9 (Buildings) not found");


		//  SWD-SECTION 10: Unknown
		//  Skipped
		section = load_s2mf_section(fr, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 10 (Unknown) not found");
		free(section);
		section = nullptr;


		//  SWD-SECTION 11: Settlers2 Mapeditor tool position
		//  In this section the positions of the Mapeditor tools seem to be
		//  saved. But as this is unusable for playing or the WL-Editor, we just
		//  skip it!
		section = load_s2mf_section(fr, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 11 (Tool Position) not found");
		free(section);
		section = nullptr;


		//  SWD-SECTION 12: Resources
		//  0x00 == Water
		//  0x87 == fish
		//  0x21 == ground water
		//  0x40 == nothing
		//  0x51-57 == gold 1-7
		//  0x49-4f == iron 1-7
		//  0x41-47 == coal 1-7
		//  0x59-5f == granite 1-7
		section = load_s2mf_section(fr, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 12 (Resources) not found");

		f = m_map.m_fields;
		pc = section;
		char const * res;
		int32_t amount = 0;
		for (Widelands::Y_Coordinate y = 0; y < mapheight; ++y)
			for (Widelands::X_Coordinate x = 0; x < mapwidth; ++x, ++f, ++pc) {
				uint8_t c = *pc;

				switch (c & 0xF8) {
				case 0x40: res = "coal";   amount = c & 7; break;
				case 0x48: res = "iron";   amount = c & 7; break;
				case 0x50: res = "gold";   amount = c & 7; break;
				case 0x59: res = "granit"; amount = c & 7; break;
				default:   res = "";       amount = 0; break;
				};

				int32_t nres = 0;
				if (*res) {
					nres = m_map.world().get_resource(res);
					if (nres == -1)
						throw wexception
							("world does not define resource type %s, you can not "
							 "play settler maps here",
							 res);
				}
				const int32_t real_amount = static_cast<int32_t>
					(2.86 * static_cast<float>(amount));
				f->set_resources(nres, real_amount);
				f->set_starting_res_amount(real_amount);
			}

		free(section);
		section = nullptr;


		//  SWD-SECTION 13: Higlights and Shadows
		//  It seems as if the Settlers2 Mapeditor saves the highlights and
		//  shadows from slopes to this section.
		//  But as this is unusable for the WL engine, we just skip it.
		section = load_s2mf_section(fr, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 13 (Highlights and Shadows) not found");
		free(section);
		section = nullptr;


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
		if (!section)
			throw wexception("Section 14 (Island id) not found");
		free(section);
		section = nullptr;

		fr.Close();


		//  Map is completely read into memory.
		//  Now try to convert the last stuff to Widelands-format
		uint8_t c;
		for (Widelands::Y_Coordinate y = 0; y < mapheight; ++y)
			for (Widelands::X_Coordinate x = 0; x < mapwidth; ++x) {
				char const * bobname = nullptr;

				Widelands::Coords const location(x, y);
				Widelands::Map_Index const index =
					Widelands::Map::get_index(location, mapwidth);
				c = bobs[index];
				if (buildings[index] == 0x78) {
					switch (c) {
					case BOB_STONE1:        bobname = "stones1"; break;
					case BOB_STONE2:        bobname = "stones2"; break;
					case BOB_STONE3:        bobname = "stones3"; break;
					case BOB_STONE4:        bobname = "stones4"; break;
					case BOB_STONE5:        bobname = "stones5"; break;
					case BOB_STONE6:        bobname = "stones6"; break;
					default:
						break;
					}
					if (bobname) {
						int32_t const idx =
							m_map.world().get_immovable_index(bobname);
						if (idx < 0)
							throw wexception("Missing immovable type %s", bobname);
						egbase.create_immovable(Widelands::Coords(x, y), idx, nullptr);
						continue;
					}
				}

				switch (c) {
				case BOB_NONE : break; // DO nothing

				case BOB_PEBBLE1:          bobname = "pebble1";   break;
				case BOB_PEBBLE2:          bobname = "pebble2";   break;
				case BOB_PEBBLE3:          bobname = "pebble3";   break;
				case BOB_PEBBLE4:          bobname = "pebble4";   break;
				case BOB_PEBBLE5:          bobname = "pebble5";   break;
				case BOB_PEBBLE6:          bobname = "pebble6";   break;

				case BOB_MUSHROOM1:        bobname = "mushroom1"; break;
				case BOB_MUSHROOM2:        bobname = "mushroom2"; break;

				case BOB_DEADTREE1:        bobname = "deadtree1"; break;
				case BOB_DEADTREE2:        bobname = "deadtree2"; break;
				case BOB_DEADTREE3:        bobname = "deadtree3"; break;
				case BOB_DEADTREE4:        bobname = "deadtree4"; break;

				case BOB_TREE1_T:          bobname = "tree1_t";   break;
				case BOB_TREE1_S:          bobname = "tree1_s";   break;
				case BOB_TREE1_M:          bobname = "tree1_m";   break;
				case BOB_TREE1:            bobname = "tree1";     break;

				case BOB_TREE2_T:          bobname = "tree2_t";   break;
				case BOB_TREE2_S:          bobname = "tree2_s";   break;
				case BOB_TREE2_M:          bobname = "tree2_m";   break;
				case BOB_TREE2:            bobname = "tree2";     break;

				case BOB_TREE3_T:          bobname = "tree3_t";   break;
				case BOB_TREE3_S:          bobname = "tree3_s";   break;
				case BOB_TREE3_M:          bobname = "tree3_m";   break;
				case BOB_TREE3:            bobname = "tree3";     break;

				case BOB_TREE4_T:          bobname = "tree4_t";   break;
				case BOB_TREE4_S:          bobname = "tree4_s";   break;
				case BOB_TREE4_M:          bobname = "tree4_m";   break;
				case BOB_TREE4:            bobname = "tree4";     break;

				case BOB_TREE5_T:          bobname = "tree5_t";   break;
				case BOB_TREE5_S:          bobname = "tree5_s";   break;
				case BOB_TREE5_M:          bobname = "tree5_m";   break;
				case BOB_TREE5:            bobname = "tree5";     break;

				case BOB_TREE6_T:          bobname = "tree6_t";   break;
				case BOB_TREE6_S:          bobname = "tree6_s";   break;
				case BOB_TREE6_M:          bobname = "tree6_m";   break;
				case BOB_TREE6:            bobname = "tree6";     break;

				case BOB_TREE7_T:          bobname = "tree7_t";   break;
				case BOB_TREE7_S:          bobname = "tree7_s";   break;
				case BOB_TREE7_M:          bobname = "tree7_m";   break;
				case BOB_TREE7:            bobname = "tree7";     break;

				case BOB_TREE8_T:          bobname = "tree8_t";   break;
				case BOB_TREE8_S:          bobname = "tree8_s";   break;
				case BOB_TREE8_M:          bobname = "tree8_m";   break;
				case BOB_TREE8:            bobname = "tree8";     break;


				case BOB_GRASS1:           bobname = "grass1";    break;
				case BOB_GRASS2:           bobname = "grass2";    break;
				case BOB_GRASS3:           bobname = "grass3";    break;

				case BOB_STANDING_STONES1: bobname = "sstones1";  break;
				case BOB_STANDING_STONES2: bobname = "sstones2";  break;
				case BOB_STANDING_STONES3: bobname = "sstones3";  break;
				case BOB_STANDING_STONES4: bobname = "sstones4";  break;
				case BOB_STANDING_STONES5: bobname = "sstones5";  break;
				case BOB_STANDING_STONES6: bobname = "sstones6";  break;
				case BOB_STANDING_STONES7: bobname = "sstones7";  break;

				case BOB_SKELETON1:        bobname = "skeleton1"; break;
				case BOB_SKELETON2:        bobname = "skeleton2"; break;
				case BOB_SKELETON3:        bobname = "skeleton3"; break;

				case BOB_CACTUS1:
					bobname =
						strcmp(m_map.get_world_name(), "winterland") ?
						"cactus1" : "snowman";
					break;
				case BOB_CACTUS2:
					bobname =
						strcmp(m_map.get_world_name(), "winterland") ?
						"cactus2" : "track";
					break;

				case BOB_BUSH1:            bobname = "bush1";     break;
				case BOB_BUSH2:            bobname = "bush2";     break;
				case BOB_BUSH3:            bobname = "bush3";     break;
				case BOB_BUSH4:            bobname = "bush4";     break;
				case BOB_BUSH5:            bobname = "bush5";     break;

				default:
					cerr << "Unknown bob " << static_cast<uint32_t>(c) << endl;
					break;
				}

				if (bobname) {
					int32_t idx = m_map.world().get_immovable_index(bobname);
					if (idx < 0)
						throw wexception("Missing immovable type %s", bobname);
					egbase.create_immovable(Widelands::Coords(x, y), idx, nullptr);
				}
			}

		//  WORKAROUND:
		//  Unfortunately the Widelands engine is not completely compatible with
		//  the Settlers 2; space for buildings is defined differently. To allow
		//  loading of Settlers 2 maps in the majority of cases, check all
		//  starting positions and try to make it Widelands compatible, if its
		//  size is too small.
		m_map.recalc_whole_map(); //  to initialize buildcaps

		const Widelands::Player_Number nr_players = m_map.get_nrplayers();
		log("Checking starting position for all %u players:\n", nr_players);
		iterate_player_numbers(p, nr_players) {
			log("-> Player %u: ", p);

			Widelands::Coords starting_pos = m_map.get_starting_pos(p);
			if (!starting_pos) {
				//  Do not throw exception, else map will not be loadable in the
				//  editor. Player initialization will keep track of wrong starting
				//  positions.
				log("Has no starting position.\n");
				continue;
			}
			Widelands::FCoords fpos = m_map.get_fcoords(starting_pos);

			if (!(m_map.get_max_nodecaps(fpos) & Widelands::BUILDCAPS_BIG)) {
				log("wrong size - trying to fix it: ");
				bool fixed = false;

				Widelands::MapRegion<Widelands::Area<Widelands::FCoords> >
					mr(m_map, Widelands::Area<Widelands::FCoords>(fpos, 3));
				do {
					if
						(m_map.get_max_nodecaps(const_cast<Widelands::FCoords &>(mr.location()))
						 &
						 Widelands::BUILDCAPS_BIG)
					{
						m_map.set_starting_pos(p, mr.location());
						fixed = true;
						break;
					}
				} while (mr.advance(m_map));


				// check whether starting position was fixed.
				if (fixed)
					log("Fixed!\n");
				else {
					//  Do not throw exception, else map will not be loadable in
					//  the editor. Player initialization will keep track of
					//  wrong starting positions.
					log("FAILED!\n");
					log("   Invalid starting position, that could not be fixed.\n");
					log("   Please try to fix it manually in the editor.\n");
				}
			} else
				log("OK\n");
		}
	} catch (...) {
		free(section);
		free(bobs);
		free(buildings);
		throw;
	}

	free(bobs);
	free(buildings);
}


/// Try to fix data, which is incompatible between S2 and Widelands
void S2_Map_Loader::postload_fix_conversion(Widelands::Editor_Game_Base & egbase) {

/*
 * 1: Try to fix port spaces
 */
	const Widelands::Map::PortSpacesSet ports(m_map.get_port_spaces());
	uint16_t num_failed = 0;
	char buf[256];

	// Check if port spaces are valid
	BOOST_FOREACH(const Widelands::Coords & c, ports) {
		Widelands::FCoords fc = m_map.get_fcoords(c);
		Widelands::NodeCaps nc = m_map.get_max_nodecaps(fc);
		if
			((nc & Widelands::BUILDCAPS_SIZEMASK) != Widelands::BUILDCAPS_BIG
			 ||
			 m_map.find_portdock(fc).empty())
		{
			log("Invalid port build space: ");
			m_map.set_port_space(c, false);

			bool fixed = false;
			Widelands::MapRegion<Widelands::Area<Widelands::FCoords> >
				mr(m_map, Widelands::Area<Widelands::FCoords>(fc, 3));
			do {
				// Check whether the maximum theoretical possible NodeCap of the field is big + port
				Widelands::NodeCaps nc2 = m_map.get_max_nodecaps(const_cast<Widelands::FCoords &>(mr.location()));
				if
					((nc2 & Widelands::BUILDCAPS_SIZEMASK) == Widelands::BUILDCAPS_BIG
					 &&
					 (!m_map.find_portdock(mr.location()).empty()))
				{
					m_map.set_port_space(Widelands::Coords(mr.location().x, mr.location().y), true);
					fixed = true;
				}
			} while (mr.advance(m_map) && !fixed);
			if (!fixed) {
				++num_failed;
				log("FAILED! No alternative port buildspace for (%i, %i) found!\n", fc.x, fc.y);
			}
			else
				log("Fixed!\n");
		}
	}

	sprintf(buf, "%i %s %s", num_failed,
		ngettext("WARNING: %i invalid port building space could not be fixed and has been removed! "
				"Some islands might be unreachable now. Please consider fixing the map in the map editor.",
				"WARNING: %i invalid port building spaces could not be fixed and have been removed! "
				"Some islands might be unreachable now. Please consider fixing the map in the map editor.",
				num_failed),
		"\n\n");
	fputs(buf, stdout);

	// If fixing failed and this is a game, inform the players about the problem
	if (num_failed > 0)
		if (upcast(Widelands::Game, game, &egbase)) {
			std::string rt_description = "<rt image=pics/port.png><p font-size=14 font-face=DejaVuSerif>";
			rt_description += buf;
			rt_description += "</p></rt>";

			Widelands::Message m = Widelands::Message("S2_Map_Loader", 0, 3600000, _("WARNING"), rt_description);

			for (uint8_t i = 0; i < MAX_PLAYERS; ++i) {
				Widelands::Player * p = game->get_player(i + 1);
				if (p)
					p->add_message(*game, *(new Widelands::Message(m)));
			}
		}
}
