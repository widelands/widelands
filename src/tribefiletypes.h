/*
 * Copyright (C) 2002 by Florian Bluemel
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

#ifndef __TRIBEFILETYPES_H
#define __TRIBEFILETYPES_H

#include "mytypes.h"

#if defined(WIN32) && defined(_MSC_VER)
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif


#define WLTF_MAGIC				"WLtf"
#define WLTF_VERSION			0x0001

struct TribeFileHeader
{
	char magic[6];			// "WLtf\0\0"
	ushort version;			// file format version
	char author[64];
	char name[32];
	uint checksum;			// not yet used
	uint buildings;
	uint wares;
	uint creatures;
	uint pictures;
	uint anims;
};

enum ProfessionType
{
	PROFESSION_SEARCH,
	PROFESSION_SIT,
	PROFESSION_FORCE_DWORD = 0x7FFFFFF
};


enum ConsumeType
{
	CONSUME_BOB,
	CONSUME_RESOURCE,
	CONSUME_FORCE_DWORD = 0x7FFFFFF
};


struct BuildingDesc
{
	char			name[16];
	uint			profession;
	ProfessionType	professionType;
	uint			anim;
};

struct CreatureDesc
{
	char			name[16];
	uint			consume;
	ConsumeType		consumeType;
	uint			produce;
	uint			prodTime;
	uint			anim;
};

struct Ware
{
	char name[16];
	uint pic;
	uint menupic;
};


#if defined(WIN32) && defined(_MSC_VER)
#pragma pack(pop, 1)
#else 
#pragma pack()
#endif

#endif //__TRIBEFILETYPES_H
