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

#include "picturereader.h"
#include "../src/myfile.h"
#include "../src/worldfiletypes.h"
#include "../src/mytypes.h"
#include <string.h>

#ifdef WIN32
#define SEPERATOR	'\\'
#else
#define SEPERATOR	'/'
#define strcmpi		strcasecmp
#endif

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
struct BitmapFileHeader
{
	ushort	type;
	uint	size;
	ushort	bfReserved1;
	ushort	bfReserved2;
	uint	offBits;
};

struct BitmapInfoHeader
{
   uint		size;
   int		width;
   int		height;
   ushort	planes;
   ushort	bitCount;
   uint		compression;
   uint		sizeImage;
   int		xPelsPerMeter;
   int		yPelsPerMeter;
   uint		clrUsed;
   uint		clrImportant;
};
#ifdef _MSC_VER
#pragma pack(pop)
#endif

inline ushort pack_rgb(uchar r, uchar g, uchar b)
{
	return (b>>3) | ((g>>2)<<5) | ((r>>3)<<11);
}

inline ushort pack_rgb(uint rgb)
{
	return pack_rgb(rgb & 0xFF, (rgb >> 8) & 0xFF, (rgb >> 16) & 0xFF);
}

Picture_Reader::Picture_Reader(const char* picdir)
{
	strcpy(this->dir, picdir);
	int len = strlen(this->dir);
	if (this->dir[len-1] != SEPERATOR)
	{
		this->dir[len] = SEPERATOR;
		this->dir[len+1] = 0;
	}
}

Picture_Reader::~Picture_Reader()
{
}

void Picture_Reader::read_picture(const char* name, PictureInfo* pic, ushort** data)
{
	memset(pic, 0, sizeof(PictureInfo));
	*data = NULL;

	char filename[1024];
	strcpy(filename, this->dir);
	strcat(filename, name);
	char* ext = strrchr(name, '.');
	if (!ext)		// this can't find out the picture type but by extension
		return;

	Binary_file* file = new Binary_file();
	file->open(filename, File::READ);
	//printf("%s\n", filename);
	if (file->get_state() != File::CLOSE)
	{
		if (strcmpi(ext, ".bmp") == 0)
			this->read_bmp(file, pic, data);
		// else if (strcmpi(ext, ".blabla") == 0)
		// ...
		else
		{
			delete file;
			return;
		}
		strcpy(pic->name, name);
	}
	delete file;
}

void Picture_Reader::read_bmp(Binary_file* file, PictureInfo* pic, ushort** p)
{
	BitmapFileHeader fileHeader;
	BitmapInfoHeader infoHeader;
	file->read(&fileHeader, sizeof(BitmapFileHeader));
	file->read(&infoHeader, sizeof(BitmapInfoHeader));

	if (fileHeader.type != 'MB')
		return;
	if (infoHeader.bitCount != 24 && infoHeader.bitCount != 32)
		return;

	pic->width = infoHeader.width;
	pic->height = infoHeader.height;
	pic->depth = 16;

	if (infoHeader.bitCount == 24)
	{
		uint size = infoHeader.sizeImage / 3;
		if (!size)
			size = infoHeader.width * infoHeader.height;
		uchar* data = new uchar[size*3];
		*p = new ushort[size];
		ushort* pixels = *p;
		file->read(data, size*3);
		int i=0;
		int j=0;
		int pitch = ((infoHeader.width * 3 + 3) >> 2) << 2;
		for (int y=0; y<infoHeader.height; y++)
		{
			for (int x=0; x<infoHeader.width; x++)
				pixels[j + x] = pack_rgb(	data[i + x*3 + 0],
											data[i + x*3 + 1],
											data[i + x*3 + 2]);
			i += pitch;
			j += infoHeader.width;
		}
	}
	else if (infoHeader.bitCount == 32)
	{
		uint size = infoHeader.sizeImage / 4;
		if (!size)
			size = infoHeader.width * infoHeader.height;
		uint* data = new uint[size];
		*p = new ushort[size];
		ushort* pixels = *p;
		file->read(data, size*4);
		for (uint i=0; i<size; i++)
			pixels[i] = pack_rgb(data[i]);
	}
}
