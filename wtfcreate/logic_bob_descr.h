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

#ifndef __S__LOGIC_BOB_DESCR_H
#define __S__LOGIC_BOB_DESCR_H

#include "../src/myfile.h"
#include "../src/profile.h"
#include "../src/helper.h"
#include "file_descr.h"
#include "pic_descr.h"
#include "fabric.h"

/*
 * this is a bob description for 'logic' bobs
 * also diminishing, growings, critters and borings
 */
class Logic_Bob_Descr : virtual public File_Descr {
   
   friend class Fabric<Logic_Bob_Descr>;

   protected:
      enum {
         BOB_GROWING=0,
         BOB_DIMINISHING,
         BOB_BORING,
         BOB_CRITTER
      };
      
      Logic_Bob_Descr(const char* name);
      int create_bob(Profile* p, Section* s, const char* def_suffix, const char* key_name, Bob_Descr* bob); 

   public:
      virtual ~Logic_Bob_Descr(void);
      const char* get_name(void) { return name; }

      virtual int construct(Profile* p, Section *s);
      virtual int write(Binary_file* f);

   private: 
      char name[30];
      ushort w, h;
      ushort hsx, hsy;
      Bob_Descr bob_anim;
};

//
// Growings
// 
class Growing_Bob_Descr : virtual public Logic_Bob_Descr {
   public:
      Growing_Bob_Descr(const char* name);
      virtual ~Growing_Bob_Descr(void);

      virtual int construct(Profile* p, Section *s);
      virtual int write(Binary_file* f);

   private:
      char ends_in[30];
      ushort growing_speed;
};

// 
// Diminishing
// 
class Diminishing_Bob_Descr : virtual public Logic_Bob_Descr {
   public:
      Diminishing_Bob_Descr(const char* name);
      virtual ~Diminishing_Bob_Descr(void);

      virtual int construct(Profile* p, Section *s);
      virtual int write(Binary_file* f);

   private:
      char ends_in[30];
      ushort stock;
};

// 
// Borings
// 
class Boring_Bob_Descr : virtual public Logic_Bob_Descr {
   public:
      Boring_Bob_Descr(const char* name);
      virtual ~Boring_Bob_Descr(void);

      virtual int construct(Profile* p, Section *s);
      virtual int write(Binary_file* f);

   private:
      ushort ttl; // time to life
};

// 
// Critters
// 
class Critter_Bob_Descr : virtual public Logic_Bob_Descr {
   public:
      Critter_Bob_Descr(const char* name);
      virtual ~Critter_Bob_Descr(void);

      virtual int construct(Profile* p, Section *s);
      virtual int write(Binary_file* f);

   private:
      short stock;
      bool swimming;
      Bob_Descr bob_walk_sw, bob_walk_se, bob_walk_w,
       bob_walk_e, bob_walk_ne, bob_walk_nw;
};


// global ware fabric objekt!
extern Fabric<Logic_Bob_Descr> bobf;

#endif

