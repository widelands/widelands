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
class Bitmap;
class Animation;
struct Animation_Pic;

//
// Base class for descriptions of worker, files and so on. this must just
// link them together
// 
class Map_Object;

class Map_Object_Descr {
	public:
      Map_Object_Descr(void) { }
      virtual ~Map_Object_Descr(void) { }
		
		virtual Map_Object *create_object() = 0;
};

enum MoveCaps {
	MOVECAPS_WALK = 1,
	MOVECAPS_SWIM = 2
};

//
// Map_Object is a class representing a base class for all objects. like buildings, animals
// ... so on. Every 'Instance' has one of those classes
// 
class Map_Object {
   friend class Instance;

   public:
		//TODO: do we really need enum Type? implementation details should be hidden,
		// that's what generic interfaces are for
      enum Type {
         DIMINISHING_BOB,
         CRITTER_BOB,
         GROWING_BOB,
         BORING_BOB,
         BIG_BUILDING
      };
      
		// the enums tell us where we are going
      enum WalkingDir {
         IDLE, 
         WALK_NE,
         WALK_E,
         WALK_SE,
         WALK_SW,
         WALK_W,
         WALK_NW
      };
      
      Map_Object(Type t);
      virtual ~Map_Object(void) { }

		virtual uint get_movecaps() { return 0; }
		
		// init() is called from Instance::create, just after the constructor
		// use it to trigger initial CMD_ACTs
		virtual void init(Game*, Instance*) { }

		// act() is called whenever a CMD_ACT triggers.
		// Some bobs may not want to act (e.g. borings)
		virtual void act(Game*, Instance*) { }
		
		// Let the Map_Object draw itself
		virtual void draw(Game* game, Bitmap* dst, int posx, int posy);
		
      inline void set_owned_by(uchar plnum) { owned_by=plnum; }
 //     uint handle_click(void); // is this good here?
 
   protected:
		void set_animation(Game* g, Animation* anim);
	
		void end_walk();
		bool start_walk(Game* g, Instance* i, WalkingDir dir, Animation* a);
		
      Type type;
      Field* field;		// where are we right now?
      ushort px, py;
      uchar owned_by; // player number

		Animation* m_anim;
		int m_animstart; // gametime when the animation was started

		WalkingDir m_walking;
		int m_walkstart; // start and end time used for interpolation
		int m_walkend;
};

//
// an Instance is a representation of every object on the map, like animals
// buildings, people, trees....
// 
class Instance {
   friend class Instance_Handler;
	friend class Field;
   
   public:
      enum State {
         UNUSED,
         USED
      };
     
      Instance(void) { obj=0; state=UNUSED; }
      virtual ~Instance(void) { }
      
      inline State get_state(void) { return state; }
		void create(Game *g, Map_Object_Descr *d);

      inline Map_Object::Type get_type(void) { assert(obj); return obj->type; }
      
		inline void act(Game* g) { assert(obj); obj->act(g, this); }
      inline void set_owned_by(uchar plnum) { assert(obj); obj->set_owned_by(plnum); }
		inline void draw(Game* game, Bitmap* dst, int posx, int posy) { assert(obj); obj->draw(game, dst, posx, posy); }
      
      inline void hook_field(ushort x, ushort y, Field* f) {
         obj->px=x;
         obj->py=y;
         obj->field=f;
         f->hook_instance(this);
      }

   private:
		Map_Object* obj;
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

