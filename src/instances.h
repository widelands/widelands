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
class Path;
class Player;

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

		bool has_attribute(uint attr);
		
	protected:
		void add_attribute(uint attr);
	
	private:
		vector<uint>	m_attributes;
};


/*
Notes on Map_Object
-------------------

Map_Object is a class representing a base class for all objects. like buildings, animals
... so on.
Every Map_Object has a unique serial number. This serial number is used as key in the
Object_Manager map, and in the safe Object_Ptr.
Since Map_Objects can be destroyed at pretty much any time, you shouldn't point to a
Map_Object directly; use an Object_Ptr instead.

DO NOT allocate/free Map_Objects directly; use the Object_Manager for this.
Note that convenient creation functions are defined in class Game.


Attributes
----------
Attributes are used to flag certain types of object. They should be used to
find all objects of a certain class (e.g. all trees around a lumberjack
or all flags in the nearby area).

Some attributes are relevant for building capabilities.
ROBUST indicates that this stationary map object cannot be simply removed.
  Decorative items (bushes, pebbles) can be removed simply by building something 
  on them. They are not robust.

SMALL and BIG define the size of a stationary object. Their exact meaning 
  depends on the context.
  
  For normal objects:
   Small: up to medium-sized houses can be built next to this object
   Normal: small houses and flags can be built next to this obejct
   Big: only flags can be built next to this object

  For buildings, it is simply the building size. Note however, that:
   Small: up to normal objects can be placed next to it (e.g. trees)
	Normal: up to small objects can be placed next to it (e.g. flags)
	Big: no robust objects can be placed next to it (however, non-robust objects
	     such as pebbles as a result of catapults are possible)

*/

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
			MOVABLE = 0,		// This Map_Object can move (animals, humans, ships)
			
			ROBUST,				// not overwritable (assert !MOVABLE)
			BUILDING,			// is a building (assert ROBUST)
			
			SMALL,				// object / building sizes (assert ROBUST)
			BIG,
			
			UNPASSABLE,			// cannot walk into this (assert ROBUST)
			FLAG,					// flag; can be used as road endpoint (assert ROBUST && !UNPASSABLE)
		};
		
		// the enums tell us where we are going
      enum WalkingDir {
         IDLE = 0,
         WALK_NE = 1,
         WALK_E = 2,
         WALK_SE = 3,
         WALK_SW = 4,
         WALK_W = 5,
         WALK_NW = 6,
      };
		
		enum {
			TASK_NONE = 0,
			
			// Do nothing. Use start_task_idle() to invoke this task.
			TASK_IDLE = 1,
			
			// Move along a path. Use start_task_movepath() to invoke this task
			TASK_MOVEPATH = 2,
		
			// descendants of Map_Objects must use task IDs greater than this
			TASK_FIRST_USER = 10,
		};
		
	protected:
      Map_Object(Map_Object_Descr *descr);
      virtual ~Map_Object(void);

	public:
		void die(Game*);
	
		virtual uint get_movecaps() { return 0; }
		inline bool has_attribute(uint attr) { return m_descr->has_attribute(attr); }
		
		// init() is called from Instance::create, just after the constructor
		// use it to trigger initial CMD_ACTs
		// make sure to always call Map_Object::init()!
		virtual void init(Game*);
		virtual void cleanup(Game*);

		// act() is called whenever a CMD_ACT triggers.
		// Some bobs may not want to act (e.g. borings)
		void act(Game*);
		
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
		inline const Coords &get_position() const { return m_pos; }
		inline Map_Object* get_next_object(void) { return m_linknext; }

	protected: // default tasks
		void start_task_idle(Game*, Animation* anim, int timeout);
		bool start_task_movepath(Game*, Coords dest, int persist, Animation **anims);
		
	protected: // higher level handling (task-based)
		inline int get_current_task() { return m_task; }
		void start_task(Game*, uint task);
		void end_task(Game*, bool success, uint nexttask);
		
		// handler functions
		virtual int task_begin(Game*);
		virtual int task_act(Game*);
		virtual void task_end(Game*);
		
		/** Map_Object::task_start_best(Game*, uint prev, bool success) [virtual]
		 *
		 * prev is the task that was last run (can be 0 on initial startup).
		 * success is the success parameter passed to end_task().
		 * nexthint is the nexttask parameter passed to end_task().
		 *
		 * You must call start_task() (directly or indirectly) from this function.
		 * Therefor, you MUST override this function in derived classes.
		 */
		virtual void task_start_best(Game*, uint prev, bool success, uint nexthint) = 0;

	private:
		void do_next_task(Game*);
		void do_start_task(Game*);
		
   protected: // low level handling
		void set_animation(Game* g, Animation* anim);

		int start_walk(Game* g, WalkingDir dir, Animation* a);
		void end_walk(Game* g);
		bool is_walking();

	protected:
		Map_Object_Descr *m_descr;
		uint m_serial;
		int m_owned_by; // 0 = neutral, otherwise player number
      
		Field* m_field; // where are we right now?
      Coords m_pos;
		Map_Object* m_linknext; // next object on this field
		Map_Object** m_linkpprev;
		
		Animation* m_anim;
		int m_animstart; // gametime when the animation was started

		WalkingDir m_walking;
		int m_walkstart; // start and end time used for interpolation
		int m_walkend;

		// Task framework variables		
		uint m_task; // the task we are currently performing
		bool m_task_acting;
		bool m_task_switching;
		bool m_lasttask_success;
		uint m_lasttask;
		uint m_nexttask;
		
		// Variables used by the default tasks
		union {
			struct {
				int timeout;
			} idle;
			struct {
				int step;
				Animation* anims[6];
				Path* path;
			} movepath;
		} task;
};

inline int get_reverse_dir(int dir) { return 1 + ((dir-1)+3)%6; }


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

		void cleanup(Game *g);
		
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
