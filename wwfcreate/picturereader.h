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

#ifndef __PICTUREREADER_H
#define __PICTUREREADER_H

#include "../src/mytypes.h"

struct PictureInfo;
class Binary_file;

class Picture_Reader
{
	char dir[1024];
	void read_bmp(Binary_file* file, PictureInfo* pic, ushort** data);
public:
	Picture_Reader(const char* picdir);
	~Picture_Reader();
	void read_picture(const char* name, PictureInfo* pic, ushort** data);
};

#endif //__PICTUREREADER_H

