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

#ifndef __S__TERRAIN_DESCR_H
#define __S__TERRAIN_DESCR_H

#include "../src/myfile.h"
#include "../src/profile.h"
#include "../src/helper.h"
#include "../wtfcreate/fabric.h"
#include "../wtfcreate/file_descr.h"
#include "../wtfcreate/pic_descr.h"
#include "../src/worlddata.h" 
//
// This is the class used to describe resources
class Terrain_Descr : virtual public File_Descr {
   friend class Fabric<Terrain_Descr>;

   protected:
      Terrain_Descr(const char* name);

   public:
      virtual ~Terrain_Descr(void);
      const char* get_name(void) { return name; }

      virtual int construct(Profile* p, Section *s);
      virtual int write(Binary_file* f);

   private: 
      char name[30];
      uchar is;
      ushort minh, maxh;
      short def_res;
      short def_stock;
      uchar nres;
      uchar* res;
      ushort ntextures;
      Pic_Descr** tex;
};

// global ware fabric objekt!
extern Fabric<Terrain_Descr> terf;

#endif

