/*
 * Copyright (C) 2002 by Holger Rapp 
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
inline int g_main(int argn, char** argc);
int main(int argn, char** argc)
{
	return g_main(argn, argc);
}

#include <iostream.h>
#include <stdio.h>
#include <string.h>
#include "../src/font.h"
#include "../src/myfile.h"
#include "../src/graphic.h"

inline int g_main(int argn, char** argc) {
		  if(argn != 5) {
					 cout << "wffcreate <fontname> <clrkey r> <clrkey g> <clrkey b>" << endl;
					 cout << "This will create the file <fontname>. you can savly rename it." << WLFF_SUFFIX << endl;
					 cout << "This program depens from the files 32.bmp - 127.bmp (ascii nummers" << endl;
					 cout << "of the characters)" << endl;
					 return -1;
		  }

		  Binary_file f;
		  char buf[300];
		  char name[20];
		  
		  strncpy(name, argc[1], 19);
		  name[19]='\0';

		  strcpy(buf, name);
		  strcat(buf, WLFF_SUFFIX);
		  f.open(buf, File::WRITE);
		  if(f.get_state() != File::OPEN) {
					 cerr << "Couldn't create file!" << endl;
					 return -1;
		  }
		  
		  ushort tmp;
		  f.write(WLFF_MAGIC, 6);
		  tmp=WLFF_VERSION;
		  f.write((char*) &tmp, sizeof(ushort));
		  f.write(name, 20);
		  tmp=Graph::pack_rgb(atoi(argc[2]), atoi(argc[3]), atoi(argc[4]));
		  f.write((char*) &tmp, sizeof(ushort));
		 
		  SDL_Surface* sur;
		  uint h=0;
		  for(uchar c=32; c<=127; c++) {
					 sprintf(buf, "%i.bmp", c);
					 sur=SDL_LoadBMP(buf);
					 if(!sur) {
								cerr << buf << ": file not found or other error!" << endl;
								return -1;
					 }
					 
					 if(!h) {
								h=sur->h;
								f.write((char*) &h, sizeof(ushort));
								// HEADER FINISHED
					 }

					 f.write((char*) &c, 1);
					 tmp=sur->w;
					 f.write((char*) &tmp, sizeof(ushort));
					 ushort* pixel=((ushort*) sur->pixels);
					 for(unsigned int y=0; y<h; y++) {
								for(int x=0; x<sur->w; x++) {
										  tmp=*pixel;
										  f.write((char*) &tmp, sizeof(ushort));
										  ++pixel;
								}
					 }
		  }

		  return 0;
}
