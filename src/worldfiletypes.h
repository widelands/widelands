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

#ifndef __WORLDFILETYPES_H
#define __WORLDFILETYPES_H

/* WordFile format 0.1
WordFile
{
	WordFileHeader	h;
	ResourceDesc	r[h.nResources];
	TerrainType		t[h.nTerrains];
	BobDesc			b[h.nBobs];
	PictureInfo		p[h.nPictures];
}
*/

#include "mytypes.h"

#define WLWF_MAGIC		"WLwf"
#define WLWF_VERSION	0x0001
#define WLWF_VERSIONMAJOR(a)	(a >> 8)
#define WLWF_VERSIONMINOR(a)	(a & 0xFF)

struct WorldFileHeader
{
	char magic[6];			// "WLwf\0\0"
	ushort version;			// file format version
	char author[64];
	char name[32];
	uint checksum;			// not yet used
	uint resources;
	uint terrains;
	uint bobs;
	uint pictures;
};

struct TerrainType
{
	char name[16];
	uint attributes;
	uint texture;			// index in WorldFileHeader::picture
	uint heightMax;
	uint heightMin;
	uint resources;			// das begrenzt die resources auf 32; hm. nicht so schoen.
							// aber ich will diese struct (vorerst) fixed-size halten.
};

enum AnimKey
{
	TIME,
	STOCK
	// ...
};

struct BobDesc
{
	char name[16];
	uint attributes;
	//uint texture;
	uint anim;
	char ware[16];
	int stock;
	AnimKey animKey;
	uint animFactor;
	int heir;
};

struct ResourceDesc
{
	char name[16];
//	uint attributes;
	uint occurence;			// how much of this is will be in this world
							// (we'll have to define some unit for this)
	//?
};

enum PictureType
{
	PIC_OTHER,
	PIC_TERRAIN,
	PIC_BOB,
	FORCE_DWORD = 0x7FFFFFF		// trying to force this enum to compile
								// to same size on different compilers;
								// we'll see if it works
};

struct PictureInfo
{
	// name & type: not really needed, but safe, editor- & debugger-friendly
	char name[16];
	PictureType type;		// terrain-texture/bob-picture/resource-picture
	
	uint width;
	uint height;
	uint depth;				// bits per pixel
	uint flags;
	// image data
};

struct Anim
{
	uint pics;
	uint* pic;
};

#endif //__WORLDFILETYPES_H