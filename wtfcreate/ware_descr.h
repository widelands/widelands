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

#ifndef __S__WARE_DESCR_H
#define __S__WARE_DESCR_H

#include "../src/md5file.h"
#include "../src/profile.h"
#include "../src/helper.h"
#include "file_descr.h"
#include "pic_descr.h"
#include "fabric.h"

/*
 * This a ware description. It is created whenever a building or a worker needs a ware
 * through the fabric, which keeps track of wares, so no ware is created twice
 */
class Ware_Descr : virtual public File_Descr {
   friend class Fabric<Ware_Descr>;

   private:
      Ware_Descr(const char* name);
      
   public:
      ~Ware_Descr(void);
      const char* get_name(void) { return name; }

      virtual int construct(Profile* p, Section *s);
      virtual int write(Binary_file* f);
      
   private: 
      char name[30];
      ushort clrkey;
      Pic_Descr* menu_pic;
      Pic_Descr* pic;
};

// global ware fabric objekt!
extern Fabric<Ware_Descr> waref;

// some usefull structs
struct Ware_List {
   Ware_Descr* ware;
   ushort num;
};

struct Need_List : public Ware_List {
   ushort stock;
};

struct Product_List: public Ware_List {
};


#endif

