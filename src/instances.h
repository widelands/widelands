/*
 * Copyright (C) 2002-2004 by the Widelands Development Team
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

#include <map>
#include <string>
#include <vector>
#include "cmd_queue.h"
#include "types.h"

class Editor_Game_Base;
class Game;
class Object_Manager;
class RenderTarget;
class Animation;
class Path;
class Player;
class UITab_Panel;


//
// Base class for descriptions of worker, files and so on. this must just
// link them together
//
class Map_Object;

class Map_Object_Descr {
public:
	Map_Object_Descr(void) { }
   virtual ~Map_Object_Descr(void) { 
      m_anims.clear();
   }

   inline uint get_animation(const char* name) {
      std::map<std::string,uint>::iterator i=m_anims.find(name);
      assert(i!=m_anims.end());
      return i->second;
   }

	bool has_attribute(uint attr);
   
   std::string get_animation_name(uint anim); // This is needed for save games and debug

protected:
	void add_attribute(uint attr);
   
   void add_animation(const char* name, uint anim);
   bool is_animation_known(const char* name);


private:
	std::vector<uint>	m_attributes;
   std::map<std::string,uint> m_anims;
   

public:
	static uint get_attribute_id(std::string name);

private:
	typedef std::map<std::string, uint> AttribMap;

	static uint			s_dyn_attribhigh;		// highest attribute ID used
	static AttribMap	s_dyn_attribs;
};


/*
Notes on Map_Object
-------------------

Map_Object is the base class for everything that can be on the map: buildings, animals,
decorations, etc... most of the time, however, you'll deal with one of the derived
classes, BaseImmovable or Bob.

Every Map_Object has a unique serial number. This serial number is used as key in the
Object_Manager map, and in the safe Object_Ptr.
Unless you're perfectly sure about when an object can be destroyed you should use
an Object_Ptr. (there are some well-defined exceptions, such as Building<->Flag
relationships)

Map_Objects can also have attributes. They are mainly useful for finding objects of a
given type (e.g. trees) within a certain radius.

DO NOT allocate/free Map_Objects directly.
Use the appropriate type-dependent create() function for creation, and call die()
for removal.
Note that convenient creation functions are defined in class Game.
*/

// If you find a better way to do this that doesn't cost a virtual function or additional
// member variable, go ahead
#define MO_DESCR(type) \
protected: inline type* get_descr() const { return static_cast<type*>(m_descr); }

// would be necessary for virtual inheritance stuff
//#define MO_VIRTUAL_DESCR(type)
//protected: type* m_descr; inline type* get_descr() const { return m_descr; }

class Map_Object {
   friend class Object_Manager;
	friend class Object_Ptr;

	MO_DESCR(Map_Object_Descr)

public:
	enum {
		BOB,			// class Bob

		WARE,			// class WareInstance

		// everything below is at least a BaseImmovable
		IMMOVABLE,
      BATTLE,

		// everything below is at least a PlayerImmovable
		BUILDING,
		FLAG,
		ROAD
	};
	// Some default, globally valid, attributes.
	// Other attributes (such as "harvestable corn") could be allocated dynamically (?)
	enum Attribute {
		WAREHOUSE = 1,		// assume BUILDING
		CONSTRUCTIONSITE, // assume BUILDING
      WORKER,				// assume BOB
		SOLDIER,				// assume WORKER
		RESI,					// resource indicator, assume IMMOVABLE

		HIGHEST_FIXED_ATTRIBUTE
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

	struct LogSink {
		virtual void log(std::string str) = 0;
	};

protected:
	Map_Object(Map_Object_Descr *descr);
	virtual ~Map_Object(void);

public:
	virtual int get_type() = 0;

   inline uint get_file_serial(void) const { return m_file_serial; }
	inline uint get_serial(void) const { return m_serial; }
	inline bool has_attribute(uint attr) { return m_descr->has_attribute(attr); }

	void remove(Editor_Game_Base*);
	virtual void destroy(Editor_Game_Base*);

   // The next functions are really only needed in games.
   // Not in Editor
	void schedule_destroy(Game *g);
	uint schedule_act(Game* g, uint tdelta, uint data = 0);
	virtual void act(Game*, uint data);

	// implementation is in game_debug_ui.cc
	virtual void create_debug_panels(Editor_Game_Base* egbase, UITab_Panel* tabs);

	LogSink* get_logsink() { return m_logsink; }
	void set_logsink(LogSink* sink);
   virtual void log_general_info(Editor_Game_Base*); // Called when a new logsink is set, used to give general informations
protected:
   // init for editor and game
	virtual void init(Editor_Game_Base*);
	virtual void cleanup(Editor_Game_Base*);

	void molog(const char* fmt, ...);

protected:
	Map_Object_Descr*		m_descr;
	uint						m_serial;
	uint						m_file_serial;
	LogSink*					m_logsink;
};

inline int get_reverse_dir(int dir) { return 1 + ((dir-1)+3)%6; }


/** class Object_Manager
 *
 * Keeps the list of all objects currently in the game.
 */
class Object_Manager {
	typedef std::map<uint, Map_Object *> objmap_t;

public:
	Object_Manager() { m_lastserial = m_last_file_serial = 0; }
	~Object_Manager(void);

	void cleanup(Editor_Game_Base *g);

	inline Map_Object* get_object(uint serial) {
		objmap_t::iterator it = m_objects.find(serial);
		if (it == m_objects.end())
			return 0;
		return it->second;
	}

	void insert(Map_Object *obj);
	void remove(Map_Object *obj);

   inline bool object_still_available(Map_Object* t) {
      objmap_t::iterator it = m_objects.begin();
      while(it!=m_objects.end()) {
         if(it->second==t) return true;
         ++it;
      }
      return false;
   }

   void overwrite_file_serial(Map_Object*, uint);

private:
	uint m_lastserial;
	uint m_last_file_serial;
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

	inline void set(Map_Object* obj) { if (obj) m_serial = obj->m_serial; else m_serial = 0; }
	inline Object_Ptr& operator = (Map_Object* obj) { set(obj); return *this; }

   inline bool is_set(void) { return m_serial; }

	// dammit... without a Editor_Game_Base object, we can't implement a Map_Object* operator
	// (would be _really_ nice)
	Map_Object* get(Editor_Game_Base* g);
	const Map_Object* get(Editor_Game_Base* g) const;

private:
	uint m_serial;
};


class Cmd_Destroy_Map_Object:public BaseCommand {
	private:
		int obj_serial;

	public:
      Cmd_Destroy_Map_Object(void) : BaseCommand(0) { } // For savegame loading
		Cmd_Destroy_Map_Object (int t, Map_Object* o);	
		virtual void execute (Game* g);
  
      // Write these commands to a file (for savegames)
      virtual void Write(FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
      virtual void Read(FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);

      virtual int get_id(void) { return QUEUE_CMD_DESTROY_MAPOBJECT; } // Get this command id
};

class Cmd_Act:public BaseCommand {
	private:
		int obj_serial;
		int arg;

	public:
      Cmd_Act(void) : BaseCommand(0) { } // For savegame loading
		Cmd_Act (int t, Map_Object* o, int a); 
		
		virtual void execute (Game* g);
      
      // Write these commands to a file (for savegames)
      virtual void Write(FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
      virtual void Read(FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);

      virtual int get_id(void) { return QUEUE_CMD_ACT; } // Get this command id
};

#endif // __S__INSTANCE_H
