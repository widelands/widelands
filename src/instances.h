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

#ifndef __S__INSTANCE_H
#define __S__INSTANCE_H

#include "field.h"

class Game;
struct Animation_Pic;

//
// Base class for descriptions of worker, files and so on. this must just
// link them together
// 
class Map_Object_Descr {
   public:
      Map_Object_Descr(void) { }
      virtual ~Map_Object_Descr(void) { }
};

//
// Map_Object is a class representing a base class for all objects. like buildings, animals
// ... so on. Every 'Instance' has one of those classes
// 
class Map_Object {
   friend class Instance;

   public:
      enum Type {
         DIMINISHING_BOB,
         CRITTER_BOB,
         GROWING_BOB,
         BORING_BOB,
         BIG_BUILDING
      };
      
      
      Map_Object(void) { pic_idx=0; rel_pos_x=0; rel_pos_y=0; };
      virtual ~Map_Object(void) { }

      inline Animation_Pic* get_cur_pic(void) { return cur_pic; }
   private:
      virtual int act(Game*)=0;
      inline void set_owned_by(uchar plnum) { owned_by=plnum; }
 //     uint handle_click(void); // is this good here?
 
   protected:
      Animation_Pic* cur_pic;
      uchar pic_idx;
      uchar owned_by; // player number
      Field* field;
      ushort px, py;
      float rel_pos_x, rel_pos_y;
      Type type;
};

//
// an Instance is a representation of every object on the map, like animals
// buildings, people, trees....
// 
class Instance {
   friend class Instance_Handler;
   
   public:
      enum State {
         UNUSED,
         USED
      };
     
      inline State get_state(void) { return state; }

      Instance(void) { obj=0; state=UNUSED; }
      virtual ~Instance(void) { }
      
      inline Map_Object::Type get_type(void) { assert(obj); return obj->type; }
      
      inline Animation_Pic* get_cur_pic(void) { assert(obj);  return obj->get_cur_pic(); }
      inline int act(Game* g) { assert(obj); return obj->act(g); }
      inline void set_owned_by(uchar plnum) { assert(obj); obj->set_owned_by(plnum); }
      inline float get_rel_pos_x(void) { assert(obj); return obj->rel_pos_x; }
      inline float get_rel_pos_y(void) { assert(obj); return obj->rel_pos_y; }
      
      inline void hook_field(ushort x, ushort y, Field* f) {
         obj->px=x;
         obj->py=y;
         obj->field=f;
         f->hook_instance(this);
      }

      // Is this a good idea, making this thinggy public?
      Map_Object* obj;

   private:
      State state;
};
      
class Instance_Handler {
   public:
      Instance_Handler(uint max) { inst=new Instance[max]; nobj=max; }
      ~Instance_Handler(void) { delete[] inst; }

      inline Instance* get_inst(uint n) { assert(n<nobj); return &inst[n]; }
      inline uint get_free_inst_id(void) { 
         uint i; for(i=0; i<nobj; i++) {
            if(inst[i].state==Instance::UNUSED) {
               // we guess, now it _is_ used
               inst[i].state=Instance::USED;
               return i;
            }
         }
         assert(0); // this is bad!
         return ((uint) -1);
      }

      inline void free_inst(uint n) {
         if(inst[n].obj) delete inst[n].obj; 
         inst[n].obj=0;
         inst[n].state=Instance::UNUSED;
      }
     
      private:
      uint nobj;
      Instance* inst;
};
         
#endif // __S__INSTANCE_H

