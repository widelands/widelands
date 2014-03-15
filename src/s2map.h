/*
 * Copyright (C) 2002, 2008, 2013 by the Widelands Development Team
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

#ifndef S2MAP_H
#define S2MAP_H

#include "io/fileread.h"
#include "map_io/map_loader.h"

struct S2MapDescrHeader {
	char    magic[10]; // "WORLD_V1.0"
	char    name [20];
	int16_t w;
	int16_t h;
	int8_t  uses_world; // 0 = green, 1 =black, 2 = winter
	int8_t  nplayers;
	char    author[26];
	char    bulk[2290]; // unknown
} /* size 2352 */;

// MILLIONS of Definitions
// Bobs

// TODO: the following bob types appear in S2 maps but are unknown
//  Somebody who can run Settlers II please check them out
//  11 (0x0B)
//  40 (0x28)
//  41 (0x29)

#define BOB_NONE             0x00

#define BOB_STONE1           0x01
#define BOB_STONE2           0x02
#define BOB_STONE3           0x03
#define BOB_STONE4           0x04
#define BOB_STONE5           0x05
#define BOB_STONE6           0x06

#define BOB_SKELETON1        0x07
#define BOB_SKELETON2        0x08
#define BOB_SKELETON3        0x21

#define BOB_STANDING_STONES1 0x18
#define BOB_STANDING_STONES2 0x19
#define BOB_STANDING_STONES3 0x1a
#define BOB_STANDING_STONES4 0x1b
#define BOB_STANDING_STONES5 0x1c
#define BOB_STANDING_STONES6 0x1d
#define BOB_STANDING_STONES7 0x1e

#define BOB_MUSHROOM1        0x01
#define BOB_MUSHROOM2        0x22

#define BOB_PEBBLE1          0x02
#define BOB_PEBBLE2          0x03
#define BOB_PEBBLE3          0x04
#define BOB_PEBBLE4          0x25
#define BOB_PEBBLE5          0x26
#define BOB_PEBBLE6          0x27

#define BOB_DEADTREE1        0x05
#define BOB_DEADTREE2        0x06
#define BOB_DEADTREE3        0x20
#define BOB_DEADTREE4        0x1f

#define BOB_CACTUS1          0x0c
#define BOB_CACTUS2          0x0d

#define BOB_BUSH1            0x11
#define BOB_BUSH2            0x13
#define BOB_BUSH3            0x10
#define BOB_BUSH4            0x12
#define BOB_BUSH5            0x0a

// Settlers 2 has 8 types of trees.
// I assume that different animation states are stored in the map file
// to create the following 32 values. I assume that 4 trees are grouped
// together.
// Unfortunately, I can't verify that (can't run the S2 editor).
// In the end, it doesn't matter much anyway.
#define BOB_TREE1_T          0x70
#define BOB_TREE1_S          0x71
#define BOB_TREE1_M          0x72
#define BOB_TREE1            0x73

#define BOB_TREE2_T          0x74
#define BOB_TREE2_S          0x75
#define BOB_TREE2_M          0x76
#define BOB_TREE2            0x77

#define BOB_TREE3_T          0xb0
#define BOB_TREE3_S          0xb1
#define BOB_TREE3_M          0xb2
#define BOB_TREE3            0xb3

#define BOB_TREE4_T          0xb4
#define BOB_TREE4_S          0xb5
#define BOB_TREE4_M          0xb6
#define BOB_TREE4            0xb7

#define BOB_TREE5_T          0xf0
#define BOB_TREE5_S          0xf1
#define BOB_TREE5_M          0xf2
#define BOB_TREE5            0xf3

#define BOB_TREE6_T          0xf4
#define BOB_TREE6_S          0xf5
#define BOB_TREE6_M          0xf6
#define BOB_TREE6            0xf7

#define BOB_TREE7_T          0x30
#define BOB_TREE7_S          0x31
#define BOB_TREE7_M          0x32
#define BOB_TREE7            0x33

#define BOB_TREE8_T          0x34
#define BOB_TREE8_S          0x35
#define BOB_TREE8_M          0x36
#define BOB_TREE8            0x37

#define BOB_GRASS1           0x0e
#define BOB_GRASS2           0x14
#define BOB_GRASS3           0x0f


struct S2_Map_Loader : public Widelands::Map_Loader {
	S2_Map_Loader(const char *, Widelands::Map &);

	virtual int32_t preload_map(bool) override;
	void load_world() override;
	virtual int32_t load_map_complete
		(Widelands::Editor_Game_Base &, bool scenario) override;

private:
	char  m_filename[256];

	uint8_t * load_s2mf_section(FileRead &, int32_t width, int32_t height);
	void load_s2mf_header();
	void load_s2mf(Widelands::Editor_Game_Base &);
	void postload_fix_conversion(Widelands::Editor_Game_Base &);
};


#endif
