/*
 * Copyright (C) 2002 by Florian Bluemel, Holger Rapp
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
#include "file_descr.h"
#include "building_descr.h"
#include "tribe_descr.h"
#include "tribedata.h"
#include "parse.h"
#include "write_tribefile.h"
#include "need_list.h"

#define OK 0
#define ERROR 1

char g_dirname[1024];
void usage(void)
{
   cout << "Creates Widelands Tribe Files." << endl
      << "usage: wtfcreate <directory name>" << endl
      << endl 
      << "See Tribe FAQ for details of tribe creation" << endl;
}

int main(int argc, char** argv) {
   cout << "Widelands Tribe File Creator VERSION " <<  VERSION_MAJOR(WLTF_VERSION) << "." << VERSION_MINOR(WLTF_VERSION) << endl;

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

   Buildings_Descr buildings;
   Tribe_Header header;
   Regent_Descr regent;
  
   // parse all the config files
   if(parse(&buildings, &header, &regent)) 
      return -1;
   
   if(needl.validate(cerr, cout)) {
      return -1;
   }
   cout << "All data collected. Writing file!" << endl;
  
   // write all the stuff
   char* name= new char[strlen(header.get_name())+strlen(WTF_EXT)+1];
   strcpy(name, header.get_name());
   strcat(name, WTF_EXT);
   MD5_Binary_file f;

   f.open(name, File::WRITE);
   if(f.get_state() != File::OPEN) {
      cerr << "Couldn't create file: <" << name << ">. Check rights and space and try again!" << endl;
      delete[] name;
      return -1;
   }
   delete[] name;
  
   write_tribefile(&f, &buildings, &header, &regent);
  
   
   // deleting buildings, rest is cleaned up automatically
   if(buildings.nbuilds) {   
      uint i;
      for(i=0; i<buildings.nbuilds; i++) {
         delete buildings.builds[i];
      }
   } 
   free(buildings.builds);

   return OK;
}
