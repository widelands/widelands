/*
 * Copyright (C) 2002 by the Widelands Development Team
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


// ok. just a little hack to avoid using msvcrt.dll
// the point is: if main is declared after including sdl, sdl will use its
// own main function. if it does not, we will not have to link msvcrt.lib
// (that is, compile singlethreaded and not multithreaded dll)
// sadly, graphic.h has to include sdl.h, so at the moment this is the only
// way to avoid using that ugly dll
inline int g_main(int argc, char** argv);
int main(int argc, char** argv)
{
	return g_main(argc, argv);
}

#include "../src/widelands.h"
#include <iostream.h>
#include "../src/font.h"
#include "../src/graphic.h"

FileSystem *tool_fs;

inline int g_main(int argc, char** argv)
{
	tool_fs = FileSystem::CreateFromDirectory(".");

	if(argc != 5) {
		cout << "wffcreate <fontname> <clrkey r> <clrkey g> <clrkey b>" << endl;
		cout << "This will create the file <fontname>. you can savly rename it." << WLFF_SUFFIX << endl;
		cout << "This program depens from the files 32.bmp - 127.bmp (ascii nummers" << endl;
		cout << "of the characters)" << endl;
		return -1;
	}

	FileWrite f;
	char filename[300];
	char hdrname[20];
		  
	snprintf(hdrname, sizeof(hdrname), "%s", argv[1]);
	snprintf(filename, sizeof(filename), "%s%s", hdrname, WLFF_SUFFIX);
	
	ushort tmp;
	f.Data(WLFF_MAGIC, 6);
	f.Unsigned16(WLFF_VERSION);
	f.Data(hdrname, 20);
	tmp=pack_rgb(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
	f.Unsigned16(tmp);
		 
	SDL_Surface* sur;
	uint h=0;
	for(uchar c=32; c<=127; c++) {
		char buf[16];
		sprintf(buf, "%i.bmp", c);
		sur=SDL_LoadBMP(buf);
		if(!sur) {
			cerr << buf << ": file not found or other error!" << endl;
			return -1;
		}
					 
		if(!h) {
			h=sur->h;
			f.Unsigned16(h);
			// HEADER FINISHED
		}

		f.Unsigned8(c);
		f.Unsigned16(sur->w);
		ushort* pixel=((ushort*) sur->pixels);
		for(unsigned int y=0; y<h; y++) {
			for(int x=0; x<sur->w; x++) {
				tmp=*pixel;
				f.Unsigned16(tmp);
				++pixel;
			}
		}
	}
	
	f.Write(tool_fs, filename);
	
	delete tool_fs;
	return 0;
}
