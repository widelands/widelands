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

#ifndef __WARE_DESCR_H
#define __WARE_DESCR_H

struct Ware_Descr {
   char name[30];
   Pic menu_pic;
   Pic pic;
};

class Need_List {
   public:
      Need_List(void) { list=0; }
      ~Need_List(void) { if(list) free(list); }

      int read(Binary_file *f);
      
   private:
      struct List {
         ushort index;
         ushort count;
      };
      short nneeds;
      List* list;
};


#endif

