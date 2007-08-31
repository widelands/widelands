/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "cmd_queue.h"
#include "error.h"
#include "types.h"

#include <SDL_types.h>

#include <map>
#include <string>
#include <vector>

class RenderTarget;
class Path;
class Player;
namespace UI {struct Tab_Panel;};

//
// Base class for descriptions of worker, files and so on. this must just
// link them together
//
struct Map_Object_Descr {
	friend class DirAnimations;
	public:
		typedef Uint8 Index;
		Map_Object_Descr() {}
		virtual ~Map_Object_Descr() {m_anims.clear();}

		struct Animation_Nonexistent {};
		uint get_animation(const char * const name) const {
			std::map<std::string, uint>::const_iterator it = m_anims.find(name);
			if (it == m_anims.end()) throw Animation_Nonexistent();
			return it->second;
		}

		uint main_animation() const throw () {
			return (m_anims.begin() != m_anims.end()) ?
					m_anims.begin()->second : 0;
		}

		std::string get_animation_name(const uint anim) const; // This is needed for save games and debug
		bool has_attribute(uint attr) const throw ();
		static uint get_attribute_id(std::string name);

	protected:
		void add_attribute(uint attr);

		void add_animation(const char* name, uint anim);
		bool is_animation_known(const char* name);


	private:
		typedef std::map<std::string, uint> AttribMap;

		Map_Object_Descr & operator=(const Map_Object_Descr &);
		Map_Object_Descr            (const Map_Object_Descr &);

		std::vector<uint>           m_attributes;
		std::map<std::string, uint>  m_anims;
		static uint                 s_dyn_attribhigh; //  highest attribute ID used
		static AttribMap            s_dyn_attribs;

};

/**
 * dummy instance because Map_Object needs a description
 * \todo move this to another header??
 */
extern Map_Object_Descr g_flag_descr;

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
public: __attribute__ ((deprecated)) inline const type* get_descr() const {return dynamic_cast<const type *>(m_descr);}\
public: const type & descr() const {return dynamic_cast<const type &>(*m_descr);}

// would be necessary for virtual inheritance stuff
//#define MO_VIRTUAL_DESCR(type)
//protected: type* m_descr; inline type* get_descr() const {return m_descr;}

class Map_Object {
	friend class Object_Manager;
	friend class Object_Ptr;

	MO_DESCR(Map_Object_Descr);

public:
	enum {
		RESOURCE_REGENERATOR,
		AREAWATCHER,
		BOB,  //  class Bob

		WARE, //  class WareInstance

		// everything below is at least a BaseImmovable
		IMMOVABLE,
		BATTLE,
		ATTACKCONTROLLER,

		// everything below is at least a PlayerImmovable
		BUILDING,
		FLAG,
		ROAD
	};
	// Some default, globally valid, attributes.
	// Other attributes (such as "harvestable corn") could be allocated dynamically (?)
	enum Attribute {
		WAREHOUSE        = 1, //  assume BUILDING
		CONSTRUCTIONSITE, // assume BUILDING
		WORKER,               //  assume BOB
		SOLDIER,              //  assume WORKER
		RESI,                 //  resource indicator, assume IMMOVABLE

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
		virtual ~LogSink() {}
	};

protected:
	Map_Object(const Map_Object_Descr * const descr);
	virtual ~Map_Object() {}

public:
	virtual int get_type() const throw () = 0;

	inline uint get_serial() const {return m_serial;}
	bool has_attribute(const uint attr) const throw ()
	{return descr().has_attribute(attr);}

	void remove(Editor_Game_Base*);
	virtual void destroy(Editor_Game_Base*);

   // The next functions are really only needed in games.
   // Not in Editor
	void schedule_destroy(Game *g);
	uint schedule_act(Game* g, uint tdelta, uint data = 0);
	virtual void act(Game*, uint data);

	// implementation is in game_debug_ui.cc
	virtual void create_debug_panels(Editor_Game_Base* egbase, UI::Tab_Panel* tabs);

	LogSink* get_logsink() {return m_logsink;}
	void set_logsink(LogSink* sink);
	virtual void log_general_info(Editor_Game_Base*); // Called when a new logsink is set, used to give general informations

protected:
	// init for editor and game
	virtual void init(Editor_Game_Base*);
	virtual void cleanup(Editor_Game_Base*);

	void molog(const char* fmt, ...) const __attribute__((format(printf, 2, 3)));

protected:
	const Map_Object_Descr * m_descr;
	uint                     m_serial;
	LogSink                * m_logsink;

private:
	Map_Object & operator=(const Map_Object &);
	Map_Object            (const Map_Object &);
};

inline int get_reverse_dir(int dir) {return 1 + ((dir-1)+3)%6;}


/** class Object_Manager
 *
 * Keeps the list of all objects currently in the game.
 */
class Object_Manager {
	typedef std::map<uint, Map_Object *> objmap_t;

public:
	Object_Manager() {m_lastserial = 0;}
	~Object_Manager();

	void cleanup(Editor_Game_Base *g);

	Map_Object * get_object(const uint serial) const {
		const objmap_t::const_iterator it = m_objects.find(serial);
		return it != m_objects.end() ? it->second : 0;
	}

	void insert(Map_Object *obj);
	void remove(Map_Object *obj);

	bool object_still_available(const Map_Object * const t) const {
		objmap_t::const_iterator it = m_objects.begin();
		while (it!=m_objects.end()) {
			if (it->second==t) return true;
			++it;
		}
		return false;
	}

private:
	uint m_lastserial;
	objmap_t m_objects;

	Object_Manager & operator=(const Object_Manager &);
	Object_Manager            (const Object_Manager &);
};

/** class Object_Ptr
 *
 * Provides a safe pointer to a Map_Object
 */
struct Object_Ptr {
	Object_Ptr(Map_Object * const obj = 0) {m_serial = obj ? obj->m_serial : 0;}
	// can use standard copy constructor and assignment operator

	Object_Ptr & operator=(Map_Object * const obj)
	{m_serial = obj ? obj->m_serial : 0; return *this;}

	bool is_set() const {return m_serial;}

	// dammit... without a Editor_Game_Base object, we can't implement a Map_Object* operator
	// (would be _really_ nice)
	Map_Object * get(const Editor_Game_Base * const g);
	const Map_Object * get(const Editor_Game_Base * const g) const;

	bool operator<(const Object_Ptr other) const throw ()
	{return m_serial < other.m_serial;}

	uint get_serial() const {return m_serial;}

private:
	uint m_serial;
};

template<class T>
struct OPtr {
	OPtr(T* const obj = 0)
		: m(obj)
	{
	}

	OPtr& operator=(T* const obj)
	{
		m = obj;
		return *this;
	}

	bool is_set() const {return m.is_set();}

	T* get(const Editor_Game_Base* const g)
	{
		return static_cast<T*>(m.get(g));
	}

	const T* get(const Editor_Game_Base* const g) const
	{
		return static_cast<const T*>(m.get(g));
	}

	bool operator<(const OPtr<T>& other) const throw ()
	{
		return m < other.m;
	}

	uint get_serial() const {return m.get_serial();}

private:
	Object_Ptr m;
};

class Cmd_Destroy_Map_Object : public GameLogicCommand {
private:
	int obj_serial;

public:
	Cmd_Destroy_Map_Object() : GameLogicCommand(0) {} // For savegame loading
	Cmd_Destroy_Map_Object (int t, Map_Object* o);
	virtual void execute (Game* g);

	virtual void Write
		(WidelandsFileWrite             &,
		 Editor_Game_Base               &,
		 Widelands_Map_Map_Object_Saver &);
	virtual void Read
		(WidelandsFileRead               &,
		 Editor_Game_Base                &,
		 Widelands_Map_Map_Object_Loader &);

	virtual int get_id() {return QUEUE_CMD_DESTROY_MAPOBJECT;} // Get this command id
};

class Cmd_Act : public GameLogicCommand {
private:
	int obj_serial;
	int arg;

public:
	Cmd_Act() : GameLogicCommand(0) {} // For savegame loading
	Cmd_Act (int t, Map_Object* o, int a);

	virtual void execute (Game* g);

	virtual void Write
		(WidelandsFileWrite             &,
		 Editor_Game_Base               &,
		 Widelands_Map_Map_Object_Saver &);
	virtual void Read
		(WidelandsFileRead               &,
		 Editor_Game_Base                &,
		 Widelands_Map_Map_Object_Loader &);

	virtual int get_id() {return QUEUE_CMD_ACT;} // Get this command id
};

#endif // __S__INSTANCE_H
