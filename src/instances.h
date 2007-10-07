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

#include <SDL_types.h>

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

class RenderTarget;
class Path;
class Player;
struct Widelands_Map_Map_Object_Loader;
namespace UI {struct Tab_Panel;};

//
// Base class for descriptions of worker, files and so on. this must just
// link them together
//
struct Map_Object_Descr {
	friend class DirAnimations;
		typedef Uint8 Index;
		Map_Object_Descr() {}
		virtual ~Map_Object_Descr() {m_anims.clear();}

		struct Animation_Nonexistent {};
		uint32_t get_animation(const char * const name) const {
			std::map<std::string, uint32_t>::const_iterator it = m_anims.find(name);
			if (it == m_anims.end()) throw Animation_Nonexistent();
			return it->second;
		}

		uint32_t main_animation() const throw () {
			return m_anims.begin() != m_anims.end() ? m_anims.begin()->second : 0;
		}

		std::string get_animation_name(const uint32_t anim) const; // This is needed for save games and debug
		bool has_attribute(uint32_t attr) const throw ();
		static uint32_t get_attribute_id(std::string name);

	protected:
		void add_attribute(uint32_t attr);

		void add_animation(const char* name, uint32_t anim);
		bool is_animation_known(const char* name);


	private:
		typedef std::map<std::string, uint32_t> AttribMap;

		Map_Object_Descr & operator=(const Map_Object_Descr &);
		Map_Object_Descr            (const Map_Object_Descr &);

		std::vector<uint32_t>           m_attributes;
		std::map<std::string, uint32_t>  m_anims;
		static uint32_t                 s_dyn_attribhigh; //  highest attribute ID used
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
Use the appropriate type-dependent create() function for creation, and call
delete() the created object for removal
Note that convenient creation functions are defined in class Game.

When you do create a new object yourself (i.e. when you're implementing one
of the create() functions), you need to allocate the object using new,
potentially set it up by calling basic functions like set_position, set_owner,
etc. and then call init(). After that, the object is supposed to be fully
created.
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
	virtual int32_t get_type() const throw () = 0;

	uint32_t get_serial() const {return m_serial;}
	bool has_attribute(const uint32_t attr) const throw ()
	{return descr().has_attribute(attr);}

	void remove(Editor_Game_Base*);
	virtual void destroy(Editor_Game_Base*);

   // The next functions are really only needed in games.
   // Not in Editor
	void schedule_destroy(Game *g);
	uint32_t schedule_act(Game* g, uint32_t tdelta, uint32_t data = 0);
	virtual void act(Game*, uint32_t data);

	// implementation is in game_debug_ui.cc
	virtual void create_debug_panels(Editor_Game_Base* egbase, UI::Tab_Panel* tabs);

	LogSink* get_logsink() {return m_logsink;}
	void set_logsink(LogSink* sink);
	virtual void log_general_info(Editor_Game_Base*); // Called when a new logsink is set, used to give general informations

	// saving and loading
	/**
	 * Header bytes to distinguish between data packages for the different
	 * Map_Object classes.
	 *
	 * Be careful in changing those, since they are written to files.
	 */
	enum {
		header_Map_Object = 1,
		header_Immovable = 2,
		header_Battle = 3,
		header_AttackController = 4
	};

	/**
	 * Static load functions of derived classes will return a pointer to
	 * a Loader class. The caller needs to call the virtual functions
	 * \ref load for all instances loaded that way, after that call
	 * \ref load_pointers for all instances loaded that way and finally
	 * call \ref load_finish for all instances loaded that way.
	 * Those are the three phases of loading. After the last phase,
	 * all Loader objects should be deleted.
	 */
	class Loader {
		Editor_Game_Base* m_egbase;
		Widelands_Map_Map_Object_Loader* m_mol;
		Map_Object* m_object;

	protected:
		Loader() : m_egbase(0), m_mol(0), m_object(0) {}

	public:
		virtual ~Loader() {}

		void init
				(Editor_Game_Base* e,
				 Widelands_Map_Map_Object_Loader* m,
				 Map_Object* object)
		{
			m_egbase = e;
			m_mol = m;
			m_object = object;
		}

		Editor_Game_Base& egbase() {return *m_egbase;}
		Widelands_Map_Map_Object_Loader& mol() {return *m_mol;}
		Map_Object * get_object() {return m_object;}

	protected:
		virtual void load(FileRead&);

	public:
		virtual void load_pointers();
		virtual void load_finish();
	};

	// This is just a fail-safe guard for the time until we fully transition
	// to the new Map_Object saving system
	virtual bool has_new_save_support() {return false;}

	virtual void save(Editor_Game_Base*, Widelands_Map_Map_Object_Saver*, FileWrite&);
	// Pure Map_Objects cannot be loaded
	//static Loader* load(Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);

protected:
	// init for editor and game
	virtual void init(Editor_Game_Base*);
	virtual void cleanup(Editor_Game_Base*);

	void molog(const char* fmt, ...) const __attribute__((format(printf, 2, 3)));

protected:
	const Map_Object_Descr * m_descr;
	uint32_t                     m_serial;
	LogSink                * m_logsink;

private:
	Map_Object & operator=(const Map_Object &);
	Map_Object            (const Map_Object &);
};

inline int32_t get_reverse_dir(int32_t dir) {return 1 + ((dir-1)+3)%6;}


/** class Object_Manager
 *
 * Keeps the list of all objects currently in the game.
 */
struct Object_Manager {
	typedef std::map<uint32_t, Map_Object *> objmap_t;

	Object_Manager() {m_lastserial = 0;}
	~Object_Manager();

	void cleanup(Editor_Game_Base *g);

	Map_Object * get_object(const uint32_t serial) const {
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

	/**
	 * Get the map of all objects for the purpose of iterating over it.
	 * Only provide a const version of the map!
	 */
	const objmap_t & get_objects() const throw () {return m_objects;}

private:
	uint32_t m_lastserial;
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

	uint32_t get_serial() const {return m_serial;}

private:
	uint32_t m_serial;
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

	uint32_t get_serial() const {return m.get_serial();}

private:
	Object_Ptr m;
};

class Cmd_Destroy_Map_Object : public GameLogicCommand {
private:
	int32_t obj_serial;

public:
	Cmd_Destroy_Map_Object() : GameLogicCommand(0) {} // For savegame loading
	Cmd_Destroy_Map_Object (int32_t t, Map_Object* o);
	virtual void execute (Game* g);

	virtual void Write
		(FileWrite             &,
		 Editor_Game_Base               &,
		 Widelands_Map_Map_Object_Saver &);
	virtual void Read
		(FileRead               &,
		 Editor_Game_Base                &,
		 Widelands_Map_Map_Object_Loader &);

	virtual int32_t get_id() {return QUEUE_CMD_DESTROY_MAPOBJECT;} // Get this command id
};

class Cmd_Act : public GameLogicCommand {
private:
	int32_t obj_serial;
	int32_t arg;

public:
	Cmd_Act() : GameLogicCommand(0) {} // For savegame loading
	Cmd_Act (int32_t t, Map_Object* o, int32_t a);

	virtual void execute (Game* g);

	virtual void Write
		(FileWrite             &,
		 Editor_Game_Base               &,
		 Widelands_Map_Map_Object_Saver &);
	virtual void Read
		(FileRead               &,
		 Editor_Game_Base                &,
		 Widelands_Map_Map_Object_Loader &);

	virtual int32_t get_id() {return QUEUE_CMD_ACT;} // Get this command id
};

#endif // __S__INSTANCE_H
