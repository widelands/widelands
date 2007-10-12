/*
 * Copyright (C) 2002, 2003, 2006-2007 by the Widelands Development Team
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
#include "fileread.h"
#include "filewrite.h"
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

/**
 * class S2_Map_Loader
 *
 * the implementation of the S2 Map Loader
 */


/**
 * S2_Map_Loader::S2_Map_Loader() - inits the map loader
 */
S2_Map_Loader::S2_Map_Loader(const char* filename, Map* map) : Map_Loader(filename, map)
{
	snprintf(m_filename, sizeof(m_filename), "%s", filename);
	m_map=map;
}


/**
 * S2_Map_Loader::~S2_Map_Loader() - cleanups
 */
S2_Map_Loader::~S2_Map_Loader() {
}


/**
 * S2_Map_Loader::preload_map() - load the header
 *
 * preloads the map. The map will then return valid
 * infos when get_width() or get_nrplayers(),
 * get_author() and so on are called
 */
int32_t S2_Map_Loader::preload_map(bool scenario) {
   assert(get_state()!=STATE_LOADED);

   m_map->cleanup();

   load_s2mf_header();

	if (not World::exists_world(m_map->get_world_name()))
		throw wexception("%s: World doesn't exist!", m_map->get_world_name());

	if (scenario) {
      // Load this as scenario.
      // there is no such a think as S2 scenarios, therefore
      // set the tribes and some default names

      // Just for fun: some roman names
		const char * const names[] = {
         "Marius",
         "Avitus",
         "Silvanus",
         "Caius",
         "Augustus",
         "Maximus",
         "Titus",
         "Rufus",
		};

		const Player_Number nr_players = m_map->get_nrplayers();
		iterate_player_numbers(i, nr_players) {
	      m_map->set_scenario_player_tribe(i, "empire"); // Even if AI doesn't work for the empire, yet - this will only be used, if you select scenario-mode
         m_map->set_scenario_player_name(i, names[i-1]);
		}
	}


   set_state(STATE_PRELOADED);

   return 0;
}

/**
 * S2_Map_Loader::load_world()
 *
 * load predefined world of the S2Map
 */
void S2_Map_Loader::load_world() {
	assert(get_state() == STATE_PRELOADED);
	m_map->load_world();
	set_state(STATE_WORLD_LOADED);
}


/**
 * S2_Map_Loader::load_map_complete()
 *
 * Completly loads the map, loads the
 * corresponding world, loads the graphics
 * and places all the objects. From now on
 * the Map* can't be set to another one.
 */
int32_t S2_Map_Loader::load_map_complete(Editor_Game_Base * game, bool) {
	assert(get_state() == STATE_WORLD_LOADED);

   // Postload the world which provides all the immovables found on a map
   m_map->m_world->postload(game);
   m_map->set_size(m_map->m_width, m_map->m_height);
   load_s2mf(game);

   m_map->recalc_whole_map();

   set_state(STATE_LOADED);

   return 0;
}


/**
 * S2_Map_Loader::load_s2mf_section
 *
 * Some of the original S2 maps have rather odd sizes. In that case, however,
 * width (and height?) are rounded up to some alignment. The in-file size of
 * a section is stored in the section header (I think ;)).
 * This is the work-around.
 *
 * Returns a pointer to the (packed) contents of the section. 0 if the read
 * failed.
 * If successful, you must free the returned pointer.
 */
uint8_t *S2_Map_Loader::load_s2mf_section(FileRead *file, int32_t width, int32_t height)
{
   uint16_t dw, dh;
   char buffer[256];
   uint16_t one;
   int32_t size;

   memcpy(buffer, file->Data(6), 6);
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

   dw = file->Unsigned16();
   dh = file->Unsigned16();

   one = file->Unsigned16();
   assert(one == 1);
	size = file->Signed32();
   assert(size == dw*dh);

   if (dw < width || dh < height) {
      cerr << "Section not big enough" << endl;
      return 0;
	}

   uint8_t *section = (uint8_t *)malloc(dw * dh);

	try {
		int32_t y = 0;
		for (; y < height; ++y) {
			uint8_t *ptr = (uint8_t*)file->Data(width);
			memcpy(section + y*width, ptr, width);
			file->Data(dw - width); //  skip the alignment junk
		}
		while (y < dh) {
			file->Data(dw); // more alignment junk
	      y++;
		}
	} catch (...) {
		free(section);
		throw;
	}

   return section;
}


/**
 * S2_Map_Loader::load_s2mf_header [private]
 *
 * Load informational data of an S2 map
 */
void S2_Map_Loader::load_s2mf_header()
{
	FileRead file;

   file.Open(*g_fs, m_filename);

	S2MapDescrHeader header;
	memcpy(&header, file.Data(sizeof(header)), sizeof(header));

        //  Header must be swaped for big-endian Systems, works at the moment only for PowerPC architecture
#if defined(__ppc__)
        header.w=Swap16(header.w);
        header.h=Swap16(header.h);
#endif

   // don't really set size, but make the structures
   // valid
   m_map->m_width=header.w;
   m_map->m_height=header.h;

	m_map->set_author(header.author);
	m_map->set_name(header.name);
	m_map->set_nrplayers(header.nplayers);
	m_map->set_description(_("Bluebyte Settlers II Map. No comment defined!").c_str());

    // This is not the name, this is the directory of the world, so please do not translate it.
	switch (header.uses_world) {
	case 0: m_map->set_world_name("greenland");  break;
	case 1: m_map->set_world_name("blackland");  break;
	case 2: m_map->set_world_name("winterland"); break;
	}
}


/**
 * S2_Map_Loader::load_s2mf [private]
 *
 * This loads a given file as a settlers 2 map file
 */
void S2_Map_Loader::load_s2mf(Editor_Game_Base *game)
{
   uint8_t *section = 0;
	uint8_t *bobs = 0;
	uint8_t *buildings = 0;

	uint8_t *pc;

	try
	{
		FileRead file;
		file.Open(*g_fs, m_filename);

		S2MapDescrHeader header;
		memcpy(&header, file.Data(sizeof(header)), sizeof(header));

                //  Header must be swaped for big-endian Systems, works at the moment only for PowerPC architecture
#if defined(__ppc__)
                header.w=Swap16(header.w);
                header.h=Swap16(header.h);
#endif

		// The header must already have been processed
		assert(m_map->m_world);
		assert(m_map->m_fields);
		const X_Coordinate mapwidth  = m_map->get_width ();
		const Y_Coordinate mapheight = m_map->get_height();
		assert(mapwidth  == header.w);
		assert(mapheight == header.h);


		/*
          SWD-SECTION 1: Heights
        */
		section = load_s2mf_section(&file, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 1 (Heights) not found");

		Field *f = m_map->m_fields;
		pc = section;
		for (Y_Coordinate y = 0; y < mapheight; ++y)
			for (X_Coordinate x = 0; x < mapwidth; ++x, ++f, ++pc)
				f->set_height(*pc);
		free(section);
		section = 0;


        /*
          SWD-SECTION 2: Terrain 1
        */
		section = load_s2mf_section(&file, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 2 (Terrain 1) not found");

		f = m_map->m_fields;
		pc = section;
		for (Y_Coordinate y = 0; y < mapheight; ++y)
			for (X_Coordinate x = 0; x < mapwidth; ++x, ++f, ++pc) {
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
				default:   c = 7;
					cerr
						<< "ERROR: Unknown texture1: " << hex << c << dec << " ("
						<< x << "," << y << ") (defaults to water!)" << endl;
				}
				f->set_terrain_d(c);
			}
		free(section);
		section = 0;


        /*
		  SWD-SECTION 3: Terrain 2
        */
		section = load_s2mf_section(&file, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 3 (Terrain 2) not found");

		f = m_map->m_fields;
		pc = section;
		for (Y_Coordinate y = 0; y < mapheight; ++y)
			for (X_Coordinate x = 0; x < mapwidth; ++x, ++f, ++pc) {
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
				default:   c = 7;
					cerr
						<< "ERROR: Unknown texture1: " << hex << c << dec << " ("
						<< x << "," << y << ") (defaults to water!)" << endl;
				}
				f->set_terrain_r(c);
			}
		free(section);
		section = 0;


		/*
          SWD-SECTION 4: Existing Roads
           As loading of Roads at game-start is not supported, yet - we simply skip it.
        */
		section = load_s2mf_section(&file, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 4 (Existing Roads) not found");
		free(section);
		section = 0;


        /*
          SWD-SECTION 5: Bobs
        */
		bobs = load_s2mf_section(&file, mapwidth, mapheight);
		if (!bobs)
			throw wexception("Section 5 (Bobs) not found");


        /*
          SWD-SECTION 6: Ways
           This describes where you can put ways
		   0xc* == it's not possible to build ways here now
		   0x80 == Heres a HQ, owner is Player number
		     owner == 0 -> blue
		     owner == 1 -> yellow
		     owner == 2 -> red
		     owner == 3 -> pink
		     owner == 4 -> grey
		     owner == 6 -> green
		     owner == 6 -> orange
        */
		section = load_s2mf_section(&file, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 6 (Ways) not found");

		for (Y_Coordinate y = 0; y < mapheight; ++y) {
			uint32_t i = y * mapwidth;
			for (X_Coordinate x = 0; x < mapwidth; ++x, ++i) {
				// ignore everything but HQs
				if (section[i]==0x80) {
					if (bobs[i] < m_map->get_nrplayers())
						m_map->set_starting_pos(bobs[i]+1, Coords(x, y));
				}
			}
		}
		free(section);
		section = 0;


        /*
          SWD-SECTION 7: Animals
		   0x01        == Bunny
		   0x02        == fox
		   0x03        == reindeer
		   0x04 + 0x07 == deer
		   0x05 + 0x08 == duck
		   0x06        == sheep
           0x09        == donkey
		*/
		section = load_s2mf_section(&file, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 7 (Animals) not found");

		for (Y_Coordinate y = 0; y < mapheight; ++y) {
			uint32_t i = y * mapwidth;
			for (X_Coordinate x = 0; x < mapwidth; ++x, ++i) {
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
					// case 0x09: bobname = "donkey"; break; -> Not implemented, yet.
				default:
					cerr << "Unsupported animal: " << static_cast<int32_t>(section[i]) << endl;
						break;
				}

				if (bobname) {
					int32_t idx = m_map->m_world->get_bob(bobname);
					if (idx < 0)
						throw wexception("Missing bob type %s", bobname);
					for (uint32_t z = 0; z < CRITTER_PER_DEFINITION; ++z)
						game->create_bob(Coords(x, y), idx);
				}
			}
		}
		free(section);
		section = 0;


        /*
          SWD-SECTION 8: Unknown
           Skipped
        */
		section = load_s2mf_section(&file, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 8 (Unknown) not found");
		free(section);
		section = 0;


        /*
          SWD-SECTION 9: Buildings
		   What kind of buildings can be build?
		     0x01 == flags (??)
		     0x02 == buildings (small) (??)
		     0x04 == buildings
		     0x09 == flags
		     0x0a == buildings (small) (??)
		     0x0c == buildings (big) (??)
		     0x0d == mining
		     0x68 == trees
		     0x78 == no buildings
		*/
		buildings = load_s2mf_section(&file, mapwidth, mapheight);
		if (!buildings)
			throw wexception("Section 9 (Buildings) not found");


        /*
          SWD-SECTION 10: Unknown
           Skipped
        */
		section = load_s2mf_section(&file, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 10 (Unknown) not found");
		free(section);
		section = 0;


        /*
          SWD-SECTION 11: Settlers2 Mapeditor tool position
		   In this section the positions of the Mapeditor tools seem to be saved.
           But as this is unusable for playing or the WL-Editor, we just skip it!
		*/
		section = load_s2mf_section(&file, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 11 (Tool Position) not found");
		free(section);
		section = 0;


        /*
          SWD-SECTION 12: Resources
		   0x00 == Water
		   0x87 == ?? (but nothing)
		   0x21 == things laying around (nothing)
		   0x40 == nothing
		   0x51-57 == gold 1-7
		   0x49-4f == iron 1-7
		   0x41-47 == cowl 1-7
		   0x59-5f == stones 1-7
		*/
		section = load_s2mf_section(&file, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 12 (Resources) not found");

		f = m_map->m_fields;
		pc = section;
      const char * res;
      int32_t amount=0;
		for (Y_Coordinate y = 0; y < mapheight; ++y)
			for (X_Coordinate x = 0; x < mapwidth; ++x, ++f, ++pc) {
				char c = *pc;

				switch (c & 0xF8) {
				case 0x40: res = "coal";   amount = c & 7; break;
				case 0x48: res = "iron";   amount = c & 7; break;
				case 0x50: res = "gold";   amount = c & 7; break;
				case 0x59: res = "granit"; amount = c & 7; break;
				default:   res = "";       amount = 0;
				}

            int32_t nres=0;
				if (*res) {
					nres = m_map->world().get_resource(res);
					if (nres == -1)
						throw wexception
							("World doesn't define Resource %s, you can not play "
							 "settler maps here!",
							 res);
				}
				const int32_t real_amount = static_cast<int32_t>
					(2.86 * static_cast<float>(amount));
            f->set_resources(nres, real_amount);
            f->set_starting_res_amount(real_amount);
			}

		free(section);
		section = 0;


        /*
          SWD-SECTION 13: Higlights and Shadows
           It seems as if the Settlers2 Mapeditor saves the highlights and shadows from slopes to this section.
           But as this is unusable for the WL engine, we just skip it.
		*/
		section = load_s2mf_section(&file, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 13 (Highlights and Shadows) not found");
		free(section);
		section = 0;


        /*
          SWD-SECTION 14: Fieldcount
		   Describes to which island the field sticks
		    0 == water
		    1 == island 1
		    2 == island 2
		    ...
		    fe == killing field (lava)

          Unusable, yet - so we simply skip it.
        */
		section = load_s2mf_section(&file, mapwidth, mapheight);
		if (!section)
			throw wexception("Section 14 (Island id) not found");
		free(section);
		section = 0;

		file.Close();


        /*
          Map is completely read into memory.
          Now try to convert the last stuff to Widelands-format
        */
		uint8_t c;
		for (Y_Coordinate y = 0; y < mapheight; ++y)
			for (X_Coordinate x = 0; x < mapwidth; ++x) {
				const char *bobname = 0;

				const Coords location(x, y);
				const Map::Index index = Map::get_index(location, mapwidth);
				c = bobs[index];
				if (buildings[index] == 0x78) {
					switch (c) {
					case BOB_STONE1:        bobname = "stones1"; break;
					case BOB_STONE2:        bobname = "stones2"; break;
					case BOB_STONE3:        bobname = "stones3"; break;
					case BOB_STONE4:        bobname = "stones4"; break;
					case BOB_STONE5:        bobname = "stones5"; break;
					case BOB_STONE6:        bobname = "stones6"; break;
						default: break;
					}
					if (bobname) {
						int32_t idx = m_map->m_world->get_immovable_index(bobname);
						if (idx < 0)
							throw wexception("Missing immovable type %s", bobname);
						game->create_immovable(Coords(x, y), idx, 0);
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

                    // May it be, that here are the different growing stats?
                    // BOB_TREE1 = tiny tree1
                    // BOB_TREE2 = small tree1
                    // BOB_TREE3 = medium tree1
                    // BOB_TREE4 = tree1
                    // ?????????????????
				case BOB_TREE1:
				case BOB_TREE2:
				case BOB_TREE3:
				case BOB_TREE4:            bobname = "tree1";     break;

				case BOB_TREE5:
				case BOB_TREE6:
				case BOB_TREE7:
				case BOB_TREE8:            bobname = "tree2";     break;

				case BOB_TREE9:
				case BOB_TREE10:
				case BOB_TREE11:
				case BOB_TREE12:           bobname = "tree3";     break;

				case BOB_TREE13:
				case BOB_TREE14:
				case BOB_TREE15:
				case BOB_TREE16:           bobname = "tree4";     break;

				case BOB_TREE17:
				case BOB_TREE18:
				case BOB_TREE19:
				case BOB_TREE20:           bobname = "tree6";     break;

				case BOB_TREE21:
				case BOB_TREE22:
				case BOB_TREE23:
				case BOB_TREE24:           bobname = "tree6";     break;

				case BOB_TREE25:
				case BOB_TREE26:
				case BOB_TREE27:
				case BOB_TREE28:           bobname = "tree7";     break;

				case BOB_TREE29:
				case BOB_TREE30:
				case BOB_TREE31:
				case BOB_TREE32:           bobname = "tree8";     break;

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

				case BOB_CACTUS1:          bobname = "cactus1";   break;
				case BOB_CACTUS2:          bobname = "cactus2";   break;

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
					int32_t idx = m_map->m_world->get_immovable_index(bobname);
					if (idx < 0)
						throw wexception("Missing immovable type %s", bobname);
					game->create_immovable(Coords(x, y), idx, 0);
				}
			}
	}
	catch (...)
	{
		if (section)
			free(section);
		if (bobs)
			free(bobs);
		if (buildings)
			free(buildings);
		throw;
	}

	free(bobs);
	free(buildings);
}
