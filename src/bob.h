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

// #include "worldfiletypes.h"
#include "graphic.h"
#include "pic.h"
#include "myfile.h"
#include "instances.h"

#define CRITTER_WALKING_SPEED 20      // frames needed to cross one field
#define CRITTER_MAX_WAIT_TIME_BETWEEN_WALK 50 // wait maximal n frames before doing something 
#define CRITTER_PER_DEFINITION   1

// class World;
//class Pic;

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

      Animation(void) { npics=0; pics=0;}
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

      int read(Binary_file*);

      inline Animation_Pic* get_pic(ushort n) { assert(n<npics); return &pics[n]; }
      inline ushort get_npics(void) { return npics; }

   private:
      uint flags;
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

      virtual int read(Binary_file*);
      virtual int create_instance(Instance*)=0;
      const char* get_name(void) { return name; }

      inline Animation* get_anim(void) { return &anim; }

   protected:
      char name[30];
      Animation anim;
};

//
// Growings
// 
class Growing_Bob_Descr : virtual public Logic_Bob_Descr {
   public:
      Growing_Bob_Descr(void) { ends_in=0; growing_speed=0; }
      virtual ~Growing_Bob_Descr(void) {  }

      virtual int read(Binary_file* f);
      int create_instance(Instance*);

   private:
      Logic_Bob_Descr* ends_in;
      ushort growing_speed;
};

// 
// Diminishing
// 
class Diminishing_Bob_Descr : virtual public Logic_Bob_Descr {
   public:
      Diminishing_Bob_Descr(void) { ends_in=0; stock=0; }
      virtual ~Diminishing_Bob_Descr(void) { }

      virtual int read(Binary_file* f);
      int create_instance(Instance*);

   private:
      Logic_Bob_Descr* ends_in;
      ushort stock;
};

// 
// Borings
// 
class Boring_Bob_Descr : virtual public Logic_Bob_Descr {
   public:
      Boring_Bob_Descr(void) { ttl=0; }
      virtual ~Boring_Bob_Descr(void) { } 

      virtual int read(Binary_file* f);
      int create_instance(Instance*);

   private:
      ushort ttl; // time to life
};

// 
// Critters
// 
class Critter_Bob_Descr : virtual public Logic_Bob_Descr {
   public:
      Critter_Bob_Descr(void) { stock=swimming=0; }
      virtual ~Critter_Bob_Descr(void) { } 

      virtual int read(Binary_file* f);
      int create_instance(Instance*);

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

//
// This class describes a in-game Boring bob
//
class Boring_Bob : public Map_Object {
   public:
      Boring_Bob(Boring_Bob_Descr *d) { descr=d; cur_pic=d->get_anim()->get_pic(0); type=Map_Object::BORING_BOB; } 
      virtual ~Boring_Bob(void) { }

      int act(Game* g);

   private:
      Boring_Bob_Descr* descr;
};

//
// This class describes a in-game Critter bob
//
class Critter_Bob : public Map_Object {
   public:
      Critter_Bob(Critter_Bob_Descr *d) { descr=d; cur_pic=d->get_anim()->get_pic(0); state=IDLE; type=Map_Object::CRITTER_BOB;  } 
      virtual ~Critter_Bob(void) { }

      int act(Game* g);

   private:
      enum {
         IDLE, 
         WALK_NE,
         WALK_E,
         WALK_SE,
         WALK_SW,
         WALK_W,
         WALK_NW
      } state;
      Critter_Bob_Descr* descr;
      float vx, vy;
      uint steps;
};


//
// This class describes a in-game Diminishing bob
//
class Diminishing_Bob : public Map_Object {
   public:
      Diminishing_Bob(Diminishing_Bob_Descr* d)  { descr=d;  cur_pic=d->get_anim()->get_pic(0); type=Map_Object::DIMINISHING_BOB; } 
      virtual ~Diminishing_Bob(void) { }

      int act(Game* g);

   private:
      Diminishing_Bob_Descr* descr;
};


#endif

