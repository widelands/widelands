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
#ifndef __BOB_H
#define __BOB_H

#include "graphic.h"
#include "pic.h"
#include "instances.h"

// class World;
//class Pic;

// Eh? Is this used for anything? If it is what I think it is, attributes
// are better to solve this because it's transparent wrt buildings etc...
enum {
   NOTHING = 0,
   SMALL = 1,
   BIG = 2
};   // for the occupies vars in Bobs

class Animation;

struct Animation_Pic {
   ushort *data;
   Animation* parent;
};

class Animation {
   public:
      enum {
         HAS_TRANSPARENCY = 1,
         HAS_SHADOW = 2,
         HAS_PL_CLR = 3
      };

      Animation(void) { npics=0; pics=0; frametime = FRAME_LENGTH; }
      ~Animation(void) { 
         if(npics) {
            uint i; 
            for(i=0; i<npics; i++) {
               free(pics[i].data);
            }
            free(pics);
         }
      }
   
      inline ushort get_w(void) { return w; }
      inline ushort get_h(void) { return h; }
      inline ushort get_hsx(void) { return hsx; }
      inline ushort get_hsy(void) { return hsy; }

      void add_pic(ushort size, ushort* data) {
         if(!pics) {
            pics=(Animation_Pic*) malloc(sizeof(Animation_Pic));
            npics=1;
         } else {
            ++npics;
            pics=(Animation_Pic*) realloc(pics, sizeof(Animation_Pic)*npics);
         }
         pics[npics-1].data=(ushort*)malloc(size);
         pics[npics-1].parent=this;
         memcpy(pics[npics-1].data, data, size);
      }

      void set_flags(uint mflags) { flags=mflags; }
      void set_dimensions(ushort mw, ushort mh) { w=mw; h=mh; }
      void set_hotspot(ushort x, ushort y) { hsx=x; hsy=y; }

      int read(FileRead*);

      inline Animation_Pic* get_pic(ushort n) { assert(n<npics); return &pics[n]; }
      inline ushort get_npics(void) { return npics; }

		inline uint get_duration() { return frametime * npics; }
		inline Animation_Pic* get_time_pic(uint time) { return &pics[(time / frametime) % npics]; }
		
   private:
      uint flags;
		uint frametime;
      ushort w, h;
      ushort hsx, hsy;
      ushort npics;
      Animation_Pic *pics;
};

class Logic_Bob_Descr : public Map_Object_Descr {
   public:
      enum {
         BOB_GROWING=0,
         BOB_DIMINISHING,
         BOB_BORING,
         BOB_CRITTER
      };

      Logic_Bob_Descr(void) { }
      virtual ~Logic_Bob_Descr(void) { }

      virtual int read(FileRead*);
      const char* get_name(void) { return name; }

      inline Animation* get_anim(void) { return &anim; }

   protected:
      char name[30];
      Animation anim;
};

//
// Growings
// 
class Growing_Bob_Descr : public Logic_Bob_Descr {
   public:
      Growing_Bob_Descr(void) { ends_in=0; growing_speed=0; }
      virtual ~Growing_Bob_Descr(void) {  }

      virtual int read(FileRead* f);
      Map_Object *create_object();

   private:
      Logic_Bob_Descr* ends_in;
      ushort growing_speed;
      uchar occupies;
};

// 
// Diminishing
// 
class Diminishing_Bob_Descr : public Logic_Bob_Descr {
   public:
      Diminishing_Bob_Descr(void) { ends_in=0; stock=0; }
      virtual ~Diminishing_Bob_Descr(void) { }

      virtual int read(FileRead* f);
      Map_Object *create_object();

   private:
      Logic_Bob_Descr* ends_in;
      ushort stock;
      uchar occupies;
};

// 
// Borings
// 
class Boring_Bob_Descr : public Logic_Bob_Descr {
   public:
      Boring_Bob_Descr(void) { ttl=0; }
      virtual ~Boring_Bob_Descr(void) { } 

      virtual int read(FileRead* f);
      Map_Object *create_object();

   private:
      ushort ttl; // time to life
      uchar occupies;
};

// 
// Critters
// 
class Critter_Bob_Descr : public Logic_Bob_Descr {
   public:
      Critter_Bob_Descr(void) { stock=swimming=0; }
      virtual ~Critter_Bob_Descr(void) { } 

      virtual int read(FileRead* f);
      Map_Object *create_object();

      inline bool is_swimming(void) { return swimming; }
      inline Animation* get_walk_ne_anim(void) { return &walk_ne; }
      inline Animation* get_walk_nw_anim(void) { return &walk_nw; }
      inline Animation* get_walk_se_anim(void) { return &walk_se; }
      inline Animation* get_walk_sw_anim(void) { return &walk_sw; }
      inline Animation* get_walk_w_anim(void) { return &walk_w; }
      inline Animation* get_walk_e_anim(void) { return &walk_e; }

   private:
      ushort stock;
      bool swimming;
      Animation walk_ne;
      Animation walk_nw;
      Animation walk_e;
      Animation walk_w;
      Animation walk_se;
      Animation walk_sw;
};


#endif

