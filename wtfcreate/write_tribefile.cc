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


#include <iostream.h>
#include "logic_bob_descr.h"
#include "write_tribefile.h"
#include "tribedata.h"
#include "need_list.h"

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

int write_tribefile(MD5_Binary_file* f,  Buildings_Descr* buildings, Tribe_Header* header, Regent_Descr* regent) {
  
   // header out
   write_header(f, header);

   // write needlist
   needl.write(f);
  
   // write regent data
   regent->write(f);

   // write bobs
   write_bobs(f);
   
   // print out checksum for debugging
   uchar* buf= (uchar*) f->get_chksum();
   printf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
         buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7],
         buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
   return OK;
}


