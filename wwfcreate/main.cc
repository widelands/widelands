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

#include "../src/widelands.h"
#include <iostream>

#include "../src/md5file.h"
#include "../wtfcreate/file_descr.h"
#include "../src/worlddata.h"
#include "wwfcreate.h"
#include "parse.h"
#include "write_worldfile.h"
#include "../wtfcreate/need_list.h"

char g_dirname[1024];
void usage(void)
{
   cout << "Creates Widelands World Files." << endl
      << "usage: wwfcreate <directory name>" << endl
      << endl 
      << "See World FAQ for details of world creation" << endl;
}

int main(int argc, char** argv) {
   cout << "Widelands World File Creator VERSION " <<  VERSION_MAJOR(WLWF_VERSION) << "." << VERSION_MINOR(WLWF_VERSION) << endl;

   if (argc != 2)
   {
      usage();
      return -1;
   }

   // get input dir name
   strcpy(g_dirname, argv[1]);
   if (g_dirname[strlen(g_dirname) - 1] != SEPERATOR)
   {
      g_dirname[strlen(g_dirname)] = SEPERATOR;
      g_dirname[strlen(g_dirname) + 1] = 0;
	}
 
   // parse all the config files
   char name[30];
   char author[30];
   char descr[1024];
   if(parse(name, author, descr)) 
      return -1;

   cout << "All data collected. Writing file!" << endl;
  
   // write all the stuff
   char* fname= new char[strlen(name)+strlen(WWF_EXT)+1];
   strcpy(fname, name);
   strcat(fname, WWF_EXT);
   MD5_Binary_file f;

   f.open(fname, File::WRITE);
   if(f.get_state() != File::OPEN) {
      cerr << "Couldn't create file: <" << fname << ">. Check rights and space and try again!" << endl;
      delete[] fname;
      return -1;
   }
   delete[] fname;
  
   write_worldfile(&f, name, author, descr);
   
   return RET_OK;
}
