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

#include "widelands.h"
#include "pic.h"
#include "myfile.h"
#include "ware.h"

// 
// Need List
// 
int Need_List::read(Binary_file* f) {
   f->read(&nneeds, sizeof(short));
   if(!nneeds) {
      // we're done, this guy is for free
      return RET_OK;
   } else if(nneeds==-1) {
      // this guy can't be assembled in stores
      return RET_OK;
   }

   list=(List*) malloc(sizeof(List)*nneeds);
   
   int i;
   for(i=0; i< nneeds; i++) {
      f->read(&list[i].count, sizeof(ushort));
      f->read(&list[i].index, sizeof(ushort));
   }

   return RET_OK;
}
