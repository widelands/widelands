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
#include <iostream.h>
#include "../wtfcreate/logic_bob_descr.h"
#include "write_worldfile.h"
#include "../src/worlddata.h"
#include "../wtfcreate/need_list.h"
#include "resource_descr.h"
#include "terrain_descr.h"

void write_bobs(Binary_file* f) {
   ushort temp;
   
   // write magic and number of bobs
   f->write("Bobs\0", 5);
   temp=bobf.get_nitems();
   f->write(&temp, sizeof(ushort));

   // write each bob
   Logic_Bob_Descr* b;
   b=bobf.start_enum();
   while(b) {
      b->write(f);
      b=bobf.get_nitem();
   }
}

void write_resources(Binary_file* f) {
   ushort temp;
   
   // write magic and number
   f->write("Resources\0", 10);
   temp=resf.get_nitems();
   f->write(&temp, sizeof(ushort));

   Resource_Descr* b;
   b=resf.start_enum();
   while(b) {
      b->write(f);
      b=resf.get_nitem();
   }
}

void write_terrains(Binary_file* f) {
   ushort temp;
   
   // write magic and number
   f->write("Terrains\0", 9);
   temp=terf.get_nitems();
   f->write(&temp, sizeof(ushort));

   Terrain_Descr* b;
   b=terf.start_enum();
   while(b) {
      b->write(f);
      b=terf.get_nitem();
   }
}



void write_checksum(MD5_Binary_file* f) {
   ulong* sum=f->get_chksum();
   
   f->write(sum, 16);
}

int write_worldfile(MD5_Binary_file* f,  char* name, char* author, char* descr) {
   
   // header out
   // write magic, version
   f->write("WLwf\0", 5);
   ushort version=WLWF_VERSION;
   f->write(&version, sizeof(version));
   f->write(name, 30);
   f->write(author, 30);
   f->write(descr, 1024);


   // provide list out
   needl.write_provides(f);

   // write resources
   write_resources(f);
   
   // write terrains
   write_terrains(f);
   
   // write bobs
   write_bobs(f);
  
   // write checksum
   write_checksum(f);

   // print out checksum for debugging
//   uchar* buf= (uchar*) f->get_chksum();
//   printf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
  //       buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7],
    //     buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
    
    // DONE!!!
   return RET_OK;
}
