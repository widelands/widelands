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

		// TODO: maybe this could be implemented as an array of available attributes (bit_vector even?)		
		virtual bool has_attribute(uint attr) { return false; }
};


//
// Map_Object is a class representing a base class for all objects. like buildings, animals
// ... so on.
// Every Map_Object has a unique serial number. This serial number is used as key in the
// Object_Manager map, and in the safe Object_Ptr.
// Since Map_Objects can be destroyed at pretty much any time, you shouldn't point to a
// Map_Object directly; use an Object_Ptr instead.
//
// DO NOT allocate/free Map_Objects directly; use the Object_Manager for this.
// Note that convenient creation functions are defined in class Game.
// 

// If you find a better way to do this that doesn't cost a virtual function or additional
// member variable, go ahead
// Note that if you abused virtual inheritance in the *_Descr, you need to use MO_VIRTUAL_DESCR
// and set that variable in the constructor!
#define MO_DESCR(type) \
protected: inline type* get_descr() { return static_cast<type*>(m_descr); }

#define MO_VIRTUAL_DESCR(type) \
protected: type* m_descr; inline type* get_descr() { return m_descr; }

class Map_Object {
   friend class Object_Manager;
	friend class Object_Ptr;

		MO_DESCR(Map_Object_Descr)

	public:
		// Some default, globally valid, attributes.
		// Other attributes (such as "harvestable corn") could be allocated dynamically (?)
      enum Attribute {
			// This Map_Object can move (animals, humans, ships)
			MOVABLE = 0,
			
			// (only valid when !MOVABLE): this Map_Object cannot be killed by
			// placing something else on it.
			// This applies to e.g. buildings, trees, but it doesn't apply to purely
			// aesthetic objects such as pebbles.
			ROBUST = 1,
			
			// (only valid when ROBUST): cannot walk onto this Map_Object
			UNPASSABLE = 2,
			
			// (assert ROBUST && !UNPASSABLE): this is a flag (can be used as road endpoint)
			FLAG = 3,
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
		
	protected:
      Map_Object(Map_Object_Descr *descr);
      virtual ~Map_Object(void);

	public:
		virtual uint get_movecaps() { return 0; }
		inline bool has_attribute(uint attr) { return m_descr->has_attribute(attr); }
		
		// init() is called from Instance::create, just after the constructor
		// use it to trigger initial CMD_ACTs
		virtual void init(Game*) { }

		// act() is called whenever a CMD_ACT triggers.
		// Some bobs may not want to act (e.g. borings)
		virtual void act(Game*) { }
		
		// Let the Map_Object draw itself
		virtual void draw(Game* game, Bitmap* dst, int posx, int posy);

      inline void set_owned_by(char plnum) { m_owned_by = plnum; }
		inline char get_owned_by() { return m_owned_by; }

		void set_position(Game* g, int x, int y, Field* f = 0);
		inline bool get_position(int *px, int *py, Field **pf = 0) {
			if (!m_field) return false;
			if (pf) *pf = m_field;
			*px = m_pos.x;
			*py = m_pos.y;
			return true;
		}
		inline Map_Object* get_next_object(void) { return m_linknext; }

   protected:
		void set_animation(Game* g, Animation* anim);

		bool start_walk(Game* g, WalkingDir dir, Animation* a);
		bool act_walk(Game* g);
		bool is_walking();

	protected:
		Map_Object_Descr *m_descr;
		uint m_serial;
      char m_owned_by; // player number, or -1 if neutral (like animals, trees)
      
		Field* m_field; // where are we right now?
      Coords m_pos;
		Map_Object* m_linknext; // next object on this field
		Map_Object** m_linkpprev;
		
		Animation* m_anim;
		int m_animstart; // gametime when the animation was started

		WalkingDir m_walking;
		int m_walkstart; // start and end time used for interpolation
		int m_walkend;
};


/** class Object_Manager
 *
 * Keeps the list of all objects currently in the game.
 * You must use create_object() and free_object() to allocate/free Map_Objects!
 */
class Object_Manager {
		typedef std::map<uint, Map_Object *> objmap_t;

   public:
      Object_Manager() { m_lastserial = 0; }
      ~Object_Manager(void);

		inline Map_Object* get_object(uint serial) {
			objmap_t::iterator it = m_objects.find(serial);
			if (it == m_objects.end())
				return 0;
			return it->second;
		}
		Map_Object* create_object(Game* g, Map_Object_Descr* d, int owner, int x, int y);
		void free_object(Game* g, Map_Object* obj);
		
	private:
		uint m_lastserial;
		objmap_t m_objects;
};

/** class Object_Ptr
 *
 * Provides a safe pointer to a Map_Object
 */
class Object_Ptr {
	public:
		inline Object_Ptr() { m_serial = 0; }
		inline Object_Ptr(Map_Object* obj) { m_serial = obj->m_serial; }
		// can use standard copy constructor and assignment operator
	
		inline void set(Map_Object* obj) { m_serial = obj->m_serial; }
		inline Object_Ptr& operator = (Map_Object* obj) { m_serial = obj->m_serial; return *this; }
		
		// dammit... without a Game object, we can't implement a Map_Object* operator
		// (would be _really_ nice)
		Map_Object* get(Game* g);
		
	private:
		uint m_serial;
};

#endif // __S__INSTANCE_H
