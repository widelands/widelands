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

#ifndef __S__TRIBE_DESCR_H
#define __S__TRIBE_DESCR_H

#include "../src/md5file.h"
#include "../src/profile.h"
#include "../src/helper.h"
#include "file_descr.h"
#include "pic_descr.h"
#include "bob_descr.h"

/*
 * This is the tribe header
 */
class Tribe_Header : virtual public File_Descr {
   public:
      Tribe_Header(void);
      virtual ~Tribe_Header(void);

      virtual int construct(Profile* p, Section *s);
      virtual int write(Binary_file* f);
      const char* get_name(void) { return name; }
   
   private: 
      char name[30];
      char author[30];
      char descr[1024];
      Bob_Descr frontier;
};

/* 
 * This is the regent description
 */
class Regent_Descr : virtual public File_Descr {
   public:
      Regent_Descr(void);
      virtual ~Regent_Descr(void);
      
      virtual int write(Binary_file* f);
      virtual int construct(Profile* p, Section *s);

   private:
      char name[30];
      ushort clrkey;
      Pic_Descr* small_pic;
      Pic_Descr* big_pic;
};
#endif

