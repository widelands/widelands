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


#include <iostream>
#include <stdio.h>
#include <string.h>
#include "font.h"
#include "myfile.h"
#include "graphic.h"

int main(int argn, char** argc) {

		  if(argn != 5) {
					 cout << "wffcreate <fontname> <clrkey r> <clrkey g> <clrkey b>" << endl;
					 cout << "This will create the file <fontname>. you can savly rename it." << WLFF_SUFFIX << endl;
					 cout << "This program depens from the files 32.bmp - 127.bmp (ascii nummers" << endl;
					 cout << "of the characters)" << endl;
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
		 
		  Pic p;
		  uint h=0;
		  for(uchar c=32; c<=127; c++) {
					 sprintf(buf, "%i.bmp", c);
					 if(p.load(buf)) {
								cerr << buf << ": file not found or other error!" << endl;
								return -1;
					 }
					 
					 if(!h) {
								h=p.get_h();
								f.write((char*) &h, sizeof(ushort));
								// HEADER FINISHED
					 }

					 f.write((char*) &c, 1);
					 tmp=p.get_w();
					 f.write((char*) &tmp, sizeof(ushort));

					 for(unsigned int y=0; y<h; y++) {
								tmp=p.get_pixel(0, y);
								f.write((char*) &tmp, sizeof(ushort));
								for(unsigned int x=1; x<p.get_w(); x++) {
										  tmp=p.get_npixel();
										  f.write((char*) &tmp, sizeof(ushort));
								}
					 }
		  }

		  return 0;
}
