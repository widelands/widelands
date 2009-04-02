/*
 * Copyright (C) 2002, 2003, 2006-2008 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "s2map.h"

#include "editor_game_base.h"
#include "field.h"
#include "i18n.h"
#include "layered_filesystem.h"
#include "map.h"
#include "map_loader.h"
#include "wexception.h"
#include "world.h"

#include <iostream>

using std::cerr;
using std::ios;
using std::endl;

// TEMP
#define hex ios::hex
#define dec ios::dec

// this is a detail of S2 maps
#define CRITTER_PER_DEFINITION   1

S2_Map_Loader::S2_Map_Loader(const char * filename, Widelands::Map & M)
: Widelands::Map_Loader(filename, M)
{
	strncpy(m_filename, filename, sizeof(m_filename));
	if (m_filename[sizeof(m_filename) - 1]) throw;
}


S2_Map_Loader::~S2_Map_Loader() {}


/**
 * load the header
 *
 * preloads the map. The map will then return valid
 * infos when get_width() or get_nrplayers(),
 * get_author() and so on are called
 */
int32_t S2_Map_Loader::preload_map(bool scenario) {
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
	(Widelands::Editor_Game_Base * const game, bool)
{
	if (get_state() == STATE_PRELOADED)
		load_world();
	assert(get_state() == STATE_WORLD_LOADED);

	m_map.set_size(m_map.m_width, m_map.m_height);
	load_s2mf(game);

	m_map.recalc_whole_map();

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
		return 0;
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
		return 0;
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

	//  Header must be swapped for big-endian Systems, works at the moment only //  FIXME generalize
	//  for PowerPC architecture                                               //  FIXME generalize
#if defined(__ppc__)                                                          //  FIXME generalize
	header.w = Swap16(header.w);                                               //  FIXME generalize
	header.h = Swap16(header.h);                                               //  FIXME generalize
#endif                                                                        //  FIXME generalize

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
	}
}


/**
 * This loads a given file as a settlers 2 map file
 */
void S2_Map_Loader::load_s2mf(Widelands::Editor_Game_Base * const game)
{
	uint8_t * section = 0;
	uint8_t *bobs = 0;
	uint8_t *buildings = 0;

	uint8_t *pc;

	try {
		FileRead fr;
		fr.Open(*g_fs, m_filename);

		S2MapDescrHeader header;
		memcpy(&header, fr.Data(sizeof(header)), sizeof(header));

		//  Header must be swapped for big-endian Systems, works at the moment //  FIXME generalize
		//  only for PowerPC architecture.                                    //  FIXME generalize
#if defined(__ppc__)                                                        //  FIXME generalize
		header.w = Swap16(header.w);                                          //  FIXME generalize
		header.h = Swap16(header.h);                                          //  FIXME generalize
#endif                                                                      //  FIXME generalize

		//  The header must already have been processed.
		assert(m_map.m_world);
		assert(m_map.m_fields);
		Widelands::X_Coordinate const mapwidth  = m_map.get_width ();
		Widelands::Y_Coordinate const mapheight = m_map.get_height();
		assert(mapwidth  == header.w);
		assert(mapheight == header.h);
		game->allocate_player_maps(); // initalises player_fields.vision


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
		section = 0;


		//  SWD-SECTION 2: Terrain 1
		section = load_s2mf_section(fr, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 2 (Terrain 1) not found");

		f = m_map.m_fields;
		pc = section;
		for (Widelands::Y_Coordinate y = 0; y < mapheight; ++y)
			for (Widelands::X_Coordinate x = 0; x < mapwidth; ++x, ++f, ++pc) {
				char c = *pc;
				c &= 0x1f;
				switch (static_cast<int32_t>(c)) {
				case 0x00: c = 0; break;
				case 0x01: c = 1; break;
				case 0x02: c = 2; break;
				case 0x03: c = 3; break;
				case 0x04: c = 4; break;
				case 0x05: c = 5; break;

				case 0x08: c = 6; break;
				case 0x09: c = 7; break;
				case 0x0a: c = 8; break;
				case 0x0b: c = 9; break;
				case 0x0c: c = 10; break;
				case 0x0d: c = 11; break;
				case 0x0e: c = 12; break;
				case 0x0f: c = 13; break;

				case 0x10: c = 14; break;
				case 0x12: c = 15; break;

				case 0x07: c = 4; break; // Unknown texture
				case 0x13: c = 4; break; // unknown texture!
				default:
					c = 7;
					cerr
						<< "ERROR: Unknown texture1: " << hex << c << dec << " ("
						<< x << "," << y << ") (defaults to water!)" << endl;
				}
				f->set_terrain_d(c);
			}
		free(section);
		section = 0;


		//  SWD-SECTION 3: Terrain 2
		section = load_s2mf_section(fr, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 3 (Terrain 2) not found");

		f = m_map.m_fields;
		pc = section;
		for (Widelands::Y_Coordinate y = 0; y < mapheight; ++y)
			for (Widelands::X_Coordinate x = 0; x < mapwidth; ++x, ++f, ++pc) {
				char c = *pc;
				c &= 0x1f;
				switch (static_cast<int32_t>(c)) {
				case 0x00: c =  0; break;
				case 0x01: c =  1; break;
				case 0x02: c =  2; break;
				case 0x03: c =  3; break;
				case 0x04: c =  4; break;
				case 0x05: c =  5; break;

				case 0x08: c =  6; break;
				case 0x09: c =  7; break;
				case 0x0a: c =  8; break;
				case 0x0b: c =  9; break;
				case 0x0c: c = 10; break;
				case 0x0d: c = 11; break;
				case 0x0e: c = 12; break;
				case 0x0f: c = 13; break;

				case 0x10: c = 14; break;
				case 0x12: c = 15; break;

				case 0x07: c = 4; break; // Unknown texture
				case 0x13: c = 4; break; // unknown texture!
				default:
					c = 7;
					cerr
						<< "ERROR: Unknown texture1: " << hex << c << dec << " ("
						<< x << "," << y << ") (defaults to water!)" << endl;
				}
				f->set_terrain_r(c);
			}
		free(section);
		section = 0;


		//  SWD-SECTION 4: Existing Roads
		//  As loading of Roads at game-start is not supported, yet - we simply
		//  skip it.
		section = load_s2mf_section(fr, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 4 (Existing Roads) not found");
		free(section);
		section = 0;


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
		section = 0;


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
				const char *bobname = 0;

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
				//  case 0x09: bobname = "donkey"; break; -> Not implemented yet.
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
						game->create_bob(Widelands::Coords(x, y), idx);
				}
			}
		}
		free(section);
		section = 0;


		//  SWD-SECTION 8: Unknown
		//  Skipped
		section = load_s2mf_section(fr, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 8 (Unknown) not found");
		free(section);
		section = 0;


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
		section = 0;


		//  SWD-SECTION 11: Settlers2 Mapeditor tool position
		//  In this section the positions of the Mapeditor tools seem to be
		//  saved. But as this is unusable for playing or the WL-Editor, we just
		//  skip it!
		section = load_s2mf_section(fr, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 11 (Tool Position) not found");
		free(section);
		section = 0;


		//  SWD-SECTION 12: Resources
		//  0x00 == Water
		//  0x87 == ?? (but nothing)
		//  0x21 == things laying around (nothing)
		//  0x40 == nothing
		//  0x51-57 == gold 1-7
		//  0x49-4f == iron 1-7
		//  0x41-47 == cowl 1-7
		//  0x59-5f == stones 1-7
		section = load_s2mf_section(fr, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 12 (Resources) not found");

		f = m_map.m_fields;
		pc = section;
		char const * res;
		int32_t amount = 0;
		for (Widelands::Y_Coordinate y = 0; y < mapheight; ++y)
			for (Widelands::X_Coordinate x = 0; x < mapwidth; ++x, ++f, ++pc) {
				char c = *pc;

				switch (c & 0xF8) {
				case 0x40: res = "coal";   amount = c & 7; break;
				case 0x48: res = "iron";   amount = c & 7; break;
				case 0x50: res = "gold";   amount = c & 7; break;
				case 0x59: res = "granit"; amount = c & 7; break;
				default:   res = "";       amount = 0;
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
		section = 0;


		//  SWD-SECTION 13: Higlights and Shadows
		//  It seems as if the Settlers2 Mapeditor saves the highlights and
		//  shadows from slopes to this section.
		//  But as this is unusable for the WL engine, we just skip it.
		section = load_s2mf_section(fr, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 13 (Highlights and Shadows) not found");
		free(section);
		section = 0;


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
		section = 0;

		fr.Close();


		//  Map is completely read into memory.
		//  Now try to convert the last stuff to Widelands-format
		uint8_t c;
		for (Widelands::Y_Coordinate y = 0; y < mapheight; ++y)
			for (Widelands::X_Coordinate x = 0; x < mapwidth; ++x) {
				char const * bobname = 0;

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
						game->create_immovable(Widelands::Coords(x, y), idx, 0);
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
					game->create_immovable(Widelands::Coords(x, y), idx, 0);
				}
			}

		// WORKAROUND: unfortunally the widelands-engine is not complete compatible
		// to the one of settlers 2 : space for buildings is differently defined.
		// To allow a loading of settlers 2 maps in the majority of cases, check
		// all startingpositions and try to make it widelands-compatible, if
		// it's size is too small.
		m_map.recalc_whole_map(); //  to initialize buildcaps

		char msg[128];
		const Widelands::Player_Number nr_players = m_map.get_nrplayers();
		snprintf
			(msg, sizeof(msg),
			 "Checking starting position for all %u players:\n", nr_players);
		log(msg);
		for (Widelands::Player_Number p = 1; p <= nr_players; ++p) {
			snprintf(msg, sizeof(msg), "-> Player %u: ", p);
			log(msg);

			Widelands::Coords starting_pos = m_map.get_starting_pos(p);
			if (!starting_pos) {
				//  Do not throw exception, else map will not be loadable in the
				//  editor. Player initialization will keep track of wrong starting
				//  positions.
				log("Has no starting position.\n");
				continue;
			}
			Widelands::FCoords fpos = m_map.get_fcoords(starting_pos);

			// WTF? can anyone tell me why get_caps() returns 39 for
			// BUILDCAPS_BIG and 36 for BUILDCAPS_SMALL ?
			// Below is a hack to make it work - but I didn't found the reason yet.
			int32_t BIG = 39;
			if (fpos.field->get_caps() != BIG) { //Widelands::BUILDCAPS_BIG) {
				log("wrong size - trying to fix it:\n");
				// Try to find a BUILDCAPS_BIG place near original start point
				Widelands::FCoords tl = m_map.tl_n(fpos);
				Widelands::FCoords  l = m_map .l_n(fpos);
				Widelands::FCoords bl = m_map.bl_n(fpos);
				Widelands::FCoords br = m_map.br_n(fpos);
				Widelands::FCoords  r = m_map .r_n(fpos);
				Widelands::FCoords tr = m_map.tr_n(fpos);
				bool fixed = false;

				// Begin with a circle of radius = 1 :
				if (!fixed & (tl.field->get_caps() == BIG)) {
					m_map.set_starting_pos(p, tl);
					fixed = true;
				}
				if (!fixed & (l.field->get_caps() == BIG)) {
					m_map.set_starting_pos(p,  l);
					fixed = true;
				}
				if (!fixed & (bl.field->get_caps() == BIG)) {
					m_map.set_starting_pos(p, bl);
					fixed = true;
				}
				if (!fixed & (br.field->get_caps() == BIG)) {
					m_map.set_starting_pos(p, br);
					fixed = true;
				}
				if (!fixed &  (r.field->get_caps() == BIG)) {
					m_map.set_starting_pos(p,  r);
					fixed = true;
				}
				if (!fixed & (tr.field->get_caps() == BIG)) {
					m_map.set_starting_pos(p, tr);
					fixed = true;
				}
				// check whether starting position was fixed.
				if (fixed) {
					log
						("   Starting position was successfully fixed during 1st "
						 "try!\n");
				} else {
					// Second try - with a circle of radius = 2 :
						// the three points at the top of the circle
					if (!fixed & (m_map.tl_n(tl).field->get_caps() == BIG)) {
						m_map.set_starting_pos(p, m_map.tl_n(tl));
						fixed = true;
					}
					if (!fixed & (m_map.tr_n(tl).field->get_caps() == BIG)) {
						m_map.set_starting_pos(p, m_map.tr_n(tl));
						fixed = true;
					}
					if (!fixed & (m_map.tr_n(tr).field->get_caps() == BIG)) {
						m_map.set_starting_pos(p, m_map.tr_n(tr));
						fixed = true;
					}
					//  the three points at the bottom of the circle
					if (!fixed & (m_map.bl_n(bl).field->get_caps() == BIG)) {
						m_map.set_starting_pos(p, m_map.bl_n(bl));
						fixed = true;
					}
					if (!fixed & (m_map.br_n(bl).field->get_caps() == BIG)) {
						m_map.set_starting_pos(p, m_map.br_n(bl));
						fixed = true;
					}
					if (!fixed & (m_map.br_n(br).field->get_caps() == BIG)) {
						m_map.set_starting_pos(p, m_map.br_n(br));
						fixed = true;
					}
					//  the three points at the left side of the circle
					if (!fixed & (m_map. l_n(tl).field->get_caps() == BIG)) {
						m_map.set_starting_pos(p, m_map. l_n(tl));
						fixed = true;
					}
					if (!fixed & (m_map. l_n (l).field->get_caps() == BIG)) {
						m_map.set_starting_pos(p, m_map. l_n (l));
						fixed = true;
					}
					if (!fixed & (m_map. l_n(bl).field->get_caps() == BIG)) {
						m_map.set_starting_pos(p, m_map. l_n(bl));
						fixed = true;
					}
					//  the three points at the right side of the circle
					if (!fixed & (m_map. r_n(tr).field->get_caps() == BIG)) {
						m_map.set_starting_pos(p, m_map. r_n(tr));
						fixed = true;
					}
					if (!fixed & (m_map. r_n (r).field->get_caps() == BIG)) {
						m_map.set_starting_pos(p, m_map. r_n (r));
						fixed = true;
					}
					if (!fixed & (m_map. r_n(br).field->get_caps() == BIG)) {
						m_map.set_starting_pos(p, m_map. r_n(br));
						fixed = true;
					}
					// check whether starting position was fixed.
					if (fixed) {
						log
							("   Starting position was successfully fixed during 2nd "
							 "try!\n");
					} else {
						//  Do not throw exception, else map will not be loadable in
						//  the editor. Player initialization will keep track of
						//  wrong starting positions.
						log("invalid starting position, that could not be fixed.\n");
						log("   Please try to fix it manually in the editor.\n");
					}
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
