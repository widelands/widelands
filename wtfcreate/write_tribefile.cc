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
#include "logic_bob_descr.h"
#include "write_tribefile.h"
#include "tribedata.h"
#include "need_list.h"
#include "soldier_descr.h"

#define OK 0
#define ERROR 1

void write_header(Binary_file* f, Tribe_Header* h) {
   
   // write magic, version
   f->write("WLtf\0", 5);
   ushort version=WLTF_VERSION;
   f->write(&version, sizeof(version));
   h->write(f);

}

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

void write_wares(Binary_file* f) {
   ushort temp;

   // write magics and nwares
   f->write("Wares\0", 6);
   temp=waref.get_nitems();
   f->write(&temp, sizeof(ushort));

   // write each ware
   Ware_Descr* w;
   w=waref.start_enum();
   while(w) {
      w->write(f);
      w=waref.get_nitem();
   }
}

void write_soldiers(Binary_file* f) {
   ushort temp;

   // write magic and nitems
   f->write("Soldiers\0", 9);
   temp=soldierf.get_nitems();
   f->write(&temp, sizeof(ushort));

   // write each soldier
   Soldier_Descr* s;
   s=soldierf.start_enum();
   while(s) {
      s->write(f);
      s=soldierf.get_nitem();
   }
}

void write_workers(Binary_file* f) {
   ushort temp;

   // write magic and nitems
   f->write("Workers\0", 8);
   temp=workerf.get_nitems();
   f->write(&temp, sizeof(ushort));

   // write each worker
   Worker_Descr* w;
   w=workerf.start_enum();
   while(w) {
      w->write(f);
      w=workerf.get_nitem();
   }
}
  
void write_buildings(Binary_file* f, Buildings_Descr* b) {
   // write magic
   f->write("Buildings\0", 10);
   f->write(&b->nbuilds, sizeof(ushort));

   // write each building
   uint i;
#if 1
   for(i=0; i<b->nbuilds; i++) {
      b->builds[i]->write(f);
   }
#endif
   
#if 0
   cerr << "################## HQ WRITE START" << endl;
      b->builds[0]->write(f);
   cerr << "################## HQ_WRITE_ENDS" << endl;
#endif
}

void write_checksum(MD5_Binary_file* f) {
   ulong* sum=f->get_chksum();
   
   f->write(sum, 16);
}

int write_tribefile(MD5_Binary_file* f,  Buildings_Descr* buildings, Tribe_Header* header, Regent_Descr* regent) {
  
   // header out
   write_header(f, header);

   // write needlist
   needl.write_needs(f);
  
   // write regent data
   regent->write(f);

   // write bobs
   write_bobs(f);
  
   // write wares
   write_wares(f);
  
   // write soldiers
   write_soldiers(f);

   // write workers
   write_workers(f);
   
   // write Buildings
   write_buildings(f, buildings);
   
   // write checksum
   write_checksum(f);

   // print out checksum for debugging
//   uchar* buf= (uchar*) f->get_chksum();
//   printf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
  //       buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7],
    //     buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
    
    // DONE!!!
   return OK;
}


