/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_LOGIC_MAP_OBJECTS_MAP_OBJECT_H
#define WL_LOGIC_MAP_OBJECTS_MAP_OBJECT_H

#include <cstring>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <boost/function.hpp>
#include <boost/unordered_map.hpp>
#include <boost/signals2.hpp>

#include "base/log.h"
#include "base/macros.h"
#include "graphic/color.h"
#include "graphic/image.h"
#include "logic/cmd_queue.h"
#include "logic/widelands.h"
#include "scripting/lua_table.h"

class FileRead;
class RenderTarget;
struct DirAnimations;
namespace UI {struct TabPanel;}

namespace Widelands {

class EditorCategory;
class MapObjectLoader;
class Player;
struct Path;

// This enum lists the available classes of Map Objects.
enum class MapObjectType : uint8_t {
	MAPOBJECT = 0,  // Root superclass

	WARE,  //  class WareInstance
	BATTLE,
	FLEET,

	BOB = 10,  // Bob
	CRITTER,   // Bob -- Critter
	SHIP,      // Bob -- Ship
	WORKER,    // Bob -- Worker
	CARRIER,   // Bob -- Worker -- Carrier
	SOLDIER,   // Bob -- Worker -- Soldier

	// everything below is at least a BaseImmovable
	IMMOVABLE = 30,

	// everything below is at least a PlayerImmovable
	FLAG = 40,
	ROAD,
	PORTDOCK,

	// everything below is at least a Building
	BUILDING = 100,    // Building
	CONSTRUCTIONSITE,  // Building -- Constructionsite
	DISMANTLESITE,     // Building -- Dismantlesite
	WAREHOUSE,         // Building -- Warehouse
	PRODUCTIONSITE,    // Building -- Productionsite
	MILITARYSITE,      // Building -- Productionsite -- Militarysite
	TRAININGSITE       // Building -- Productionsite -- Trainingsite
};

// Returns a string representation for 'type'.
std::string to_string(MapObjectType type);

/**
 * Base class for descriptions of worker, files and so on. This must just
 * link them together
 */
struct MapObjectDescr {

	enum class OwnerType {
		kWorld,
		kTribe
	};

	MapObjectDescr(const MapObjectType init_type,
						const std::string& init_name,
						const std::string& init_descname);
	MapObjectDescr(const MapObjectType init_type,
						const std::string& init_name,
						const std::string& init_descname,
						const LuaTable& table);
	virtual ~MapObjectDescr();

	const std::string &     name() const {return m_name;}
	const std::string &     descname() const {return m_descname;}

	// Type of the MapObjectDescr.
	MapObjectType type() const {return m_type;}

	struct AnimationNonexistent {};
	uint32_t get_animation(char const * const anim) const {
		std::map<std::string, uint32_t>::const_iterator it = m_anims.find(anim);
		if (it == m_anims.end())
			throw AnimationNonexistent();
		return it->second;
	}
	uint32_t get_animation(const std::string & animname) const {
		return get_animation(animname.c_str());
	}

	uint32_t main_animation() const {
		return !m_anims.empty()? m_anims.begin()->second : 0;
	}

	std::string get_animation_name(uint32_t) const; ///< needed for save, debug
	bool has_attribute(uint32_t) const;
	static uint32_t get_attribute_id(const std::string & name, bool add_if_not_exists = false);
	static std::string get_attribute_name(uint32_t id);

	bool is_animation_known(const std::string & name) const;
	void add_animation(const std::string & name, uint32_t anim);

	/// Sets the directional animations in 'anims' with the animations '<prefix>_(ne|e|se|sw|w|nw)'.
	void add_directional_animation(DirAnimations* anims, const std::string& prefix);

	/// Returns the image for the first frame of the idle animation if the MapObject has animations,
	/// nullptr otherwise
	const Image* representative_image(const RGBColor* player_color = nullptr) const;
	/// Returns the image fileneme for first frame of the idle animation if the MapObject has animations,
	/// is empty otherwise
	const std::string& representative_image_filename() const;

	/// Returns the menu image if the MapObject has one, nullptr otherwise
	const Image* icon() const;
	/// Returns the image fileneme for the menu image if the MapObject has one, is empty otherwise
	const std::string& icon_filename() const;

protected:
	// Add all the special attributes to the attribute list. Only the 'allowed_special'
	// attributes are allowed to appear - i.e. resi are fine for immovables.
	void add_attributes(const std::vector<std::string>& attributes, const std::set<uint32_t>& allowed_special);
	void add_attribute(uint32_t attr);

private:
	using Anims = std::map<std::string, uint32_t>;
	using AttribMap = std::map<std::string, uint32_t>;
	using Attributes = std::vector<uint32_t>;

	const MapObjectType m_type;           /// Subclasses pick from the enum above
	std::string const   m_name;           /// The name for internal reference
	std::string const   m_descname;       /// A localized Descriptive name
	Attributes          m_attributes;
	Anims               m_anims;
	static uint32_t     s_dyn_attribhigh; ///< highest attribute ID used
	static AttribMap    s_dyn_attribs;
	std::string representative_image_filename_; // Image for big represenations, e.g. on buttons
	std::string icon_filename_; // Filename for the menu icon

	DISALLOW_COPY_AND_ASSIGN(MapObjectDescr);
};


/**
 * \par Notes on MapObject
 *
 * MapObject is the base class for everything that can be on the map:
 * buildings, animals, decorations, etc... most of the time, however, you'll
 * deal with one of the derived classes, BaseImmovable or Bob.
 *
 * Every MapObject has a unique serial number. This serial number is used as
 * key in the ObjectManager map, and in the safe ObjectPointer.
 *
 * Unless you're perfectly sure about when an object can be destroyed you
 * should use an ObjectPointer or, better yet, the type safe OPtr template.
 * This is not necessary when the relationship and lifetime between objects
 * is well-defined, such as in the relationship between Building and Flag.
 *
 * MapObjects can also have attributes. They are mainly useful for finding
 * objects of a given type (e.g. trees) within a certain radius.
 *
 * \warning DO NOT allocate/free MapObjects directly. Use the appropriate
 * type-dependent create() function for creation, and call die() for removal.
 *
 * \note Convenient creation functions are defined in class Game.
 *
 * When you do create a new object yourself (i.e. when you're implementing one
 * of the create() functions), you need to allocate the object using new,
 * potentially set it up by calling basic functions like set_position(),
 * set_owner(), etc. and then call init(). After that, the object is supposed to
 * be fully created.
*/

/// If you find a better way to do this that doesn't cost a virtual function
/// or additional member variable, go ahead
#define MO_DESCR(type)                     \
public: const type & descr() const {       \
		return dynamic_cast<const type&>(*m_descr); \
   }                                                                          \

class MapObject {
	friend struct ObjectManager;
	friend struct ObjectPointer;

	MO_DESCR(MapObjectDescr)

public:
	/// Some default, globally valid, attributes.
	/// Other attributes (such as "harvestable corn") could be
	/// allocated dynamically (?)
	enum Attribute {
		CONSTRUCTIONSITE = 1, ///< assume BUILDING
		WORKER,               ///< assume BOB
		SOLDIER,              ///<  assume WORKER
		RESI,                 ///<  resource indicator, assume IMMOVABLE

		HIGHEST_FIXED_ATTRIBUTE
	};

	struct LogSink {
		virtual void log(std::string str) = 0;
		virtual ~LogSink() {}
	};

	virtual void load_finish(EditorGameBase &) {}

	virtual const Image* representative_image() const;

protected:
	MapObject(MapObjectDescr const * descr);
	virtual ~MapObject() {}

public:
	Serial serial() const {return m_serial;}

	/**
	 * Is called right before the object will be removed from
	 * the game. No conncetion is handled in this class.
	 * \param serial : the object serial
	 */
	boost::signals2::signal<void(uint32_t)> removed;

	/**
	 * Attributes are fixed boolean properties of an object.
	 * An object either has a certain attribute or it doesn't.
	 * See the \ref Attribute enume.
	 *
	 * \return whether this object has the given attribute
	 */
	bool has_attribute(uint32_t const attr) const {
		return descr().has_attribute(attr);
	}

	/**
	 * \return the value of the given \ref TrainingAttribute. -1 if this object
	 * doesn't have this kind of attribute.
	 * The default behaviour returns \c -1 for all attributes.
	 */
	virtual int32_t get_training_attribute(uint32_t attr) const;

	void remove(EditorGameBase &);
	virtual void destroy(EditorGameBase &);

	//  The next functions are really only needed in games, not in the editor.
	void schedule_destroy(Game &);
	uint32_t schedule_act(Game &, uint32_t tdelta, uint32_t data = 0);
	virtual void act(Game &, uint32_t data);

	// implementation is in game_debug_ui.cc
	virtual void create_debug_panels
		(const EditorGameBase & egbase, UI::TabPanel & tabs);

	LogSink * get_logsink() {return m_logsink;}
	void set_logsink(LogSink *);

	/// Called when a new logsink is set. Used to give general information.
	virtual void log_general_info(const EditorGameBase &);

	// Header bytes to distinguish between data packages for the different
	// MapObject classes. Be careful in changing those, since they are written
	// to files.
	enum {
		HeaderMapObject = 1,
		HeaderImmovable = 2,
		// 3 was battle object.
		// 4 was attack controller.
		HeaderBattle = 5,
		HeaderCritter = 6,
		HeaderWorker = 7,
		HeaderWareInstance = 8,
		HeaderShip = 9,
		HeaderPortDock = 10,
		HeaderFleet = 11,
	};

	public:

	/**
	 * Returns whether this immovable was reserved by a worker.
	 */
	bool is_reserved_by_worker() const;

	/**
	 * Change whether this immovable is marked as reserved by a worker.
	 */
	void set_reserved_by_worker(bool reserve);


	/**
	 * Static load functions of derived classes will return a pointer to
	 * a Loader class. The caller needs to call the virtual functions
	 * \ref load for all instances loaded that way, after that call
	 * \ref load_pointers for all instances loaded that way and finally
	 * call \ref load_finish for all instances loaded that way.
	 * Those are the three phases of loading. After the last phase,
	 * all Loader objects should be deleted.
	 */
	struct Loader {
		EditorGameBase      * m_egbase;
		MapObjectLoader * m_mol;
		MapObject            * m_object;

	protected:
		Loader() : m_egbase(nullptr), m_mol(nullptr), m_object(nullptr) {}

	public:
		virtual ~Loader() {}

		void init
			(EditorGameBase & e, MapObjectLoader & m, MapObject & object)
		{
			m_egbase = &e;
			m_mol    = &m;
			m_object = &object;
		}

		EditorGameBase      & egbase    () {return *m_egbase;}
		MapObjectLoader & mol   () {return *m_mol;}
		MapObject            * get_object() {return m_object;}
		template<typename T> T & get() {
			return dynamic_cast<T&>(*m_object);
		}

	protected:
		void load(FileRead &);

	public:
		virtual void load_pointers();
		virtual void load_finish();
	};

	/// This is just a fail-safe guard for the time until we fully transition
	/// to the new MapObject saving system
	virtual bool has_new_save_support() {return false;}

	virtual void save(EditorGameBase &, MapObjectSaver &, FileWrite &);
	// Pure MapObjects cannot be loaded

protected:
	/// Called only when the oject is logically created in the simulation. If
	/// called again, such as when the object is loaded from a savegame, it will
	/// cause bugs.
	virtual void init(EditorGameBase &);

	virtual void cleanup(EditorGameBase &);

	void molog(char const * fmt, ...) const
		__attribute__((format(printf, 2, 3)));

	const MapObjectDescr * m_descr;
	Serial                   m_serial;
	LogSink                * m_logsink;

	/**
	 * MapObjects like trees are reserved by a worker that is walking
	 * towards them, so that e.g. two lumberjacks don't attempt to
	 * work on the same tree simultaneously or two hunters try to hunt
	 * the same animal.
	 */
	bool m_reserved_by_worker;

private:
	DISALLOW_COPY_AND_ASSIGN(MapObject);
};

inline int32_t get_reverse_dir(int32_t const dir) {
	return 1 + ((dir - 1) + 3) % 6;
}


/**
 *
 * Keeps the list of all objects currently in the game.
 */
struct ObjectManager  {
	using MapObjectMap = boost::unordered_map<Serial, MapObject *>;

	ObjectManager() {m_lastserial = 0;}
	~ObjectManager();

	void cleanup(EditorGameBase &);

	MapObject * get_object(Serial const serial) const {
		const MapObjectMap::const_iterator it = m_objects.find(serial);
		return it != m_objects.end() ? it->second : nullptr;
	}

	void insert(MapObject *);
	void remove(MapObject &);

	bool object_still_available(const MapObject * const t) const {
		if (!t)
			return false;
		MapObjectMap::const_iterator it = m_objects.begin();
		while (it != m_objects.end()) {
			if (it->second == t)
				return true;
			++it;
		}
		return false;
	}

	/**
	 * When saving the map object, ordere matters. Return a vector of all ids
	 * that are currently available;
	 */
	std::vector<Serial> all_object_serials_ordered () const;

private:
	Serial   m_lastserial;
	MapObjectMap m_objects;

	DISALLOW_COPY_AND_ASSIGN(ObjectManager);
};

/**
 * Provides a safe pointer to a MapObject
 */
struct ObjectPointer {
	// Provide default constructor to shut up cppcheck.
	ObjectPointer() {m_serial = 0;}
	ObjectPointer(MapObject * const obj) {m_serial = obj ? obj->m_serial : 0;}
	// can use standard copy constructor and assignment operator

	ObjectPointer & operator= (MapObject * const obj) {
		m_serial = obj ? obj->m_serial : 0;
		return *this;
	}

	bool is_set() const {return m_serial;}

	// TODO(unknown): dammit... without an EditorGameBase object, we can't implement a
	// MapObject* operator (would be _really_ nice)
	MapObject * get(const EditorGameBase &);
	MapObject * get(const EditorGameBase & egbase) const;

	bool operator<  (const ObjectPointer & other) const {
		return m_serial < other.m_serial;
	}
	bool operator== (const ObjectPointer & other) const {
		return m_serial == other.m_serial;
	}
	bool operator!= (const ObjectPointer & other) const {
		return m_serial != other.m_serial;
	}

	uint32_t serial() const {return m_serial;}

private:
	uint32_t m_serial;
};

template<class T>
struct OPtr {
	OPtr(T * const obj = 0) : m(obj) {}

	OPtr & operator= (T * const obj) {
		m = obj;
		return *this;
	}

	bool is_set() const {return m.is_set();}

	T * get(const EditorGameBase &       egbase) {
		return static_cast<T *>(m.get(egbase));
	}
	T * get(const EditorGameBase &       egbase) const {
		return static_cast<T *>(m.get(egbase));
	}

	bool operator<  (const OPtr<T> & other) const {return m <  other.m;}
	bool operator== (const OPtr<T> & other) const {return m == other.m;}
	bool operator!= (const OPtr<T> & other) const {return m != other.m;}

	Serial serial() const {return m.serial();}

private:
	ObjectPointer m;
};

struct CmdDestroyMapObject : public GameLogicCommand {
	CmdDestroyMapObject() : GameLogicCommand(0), obj_serial(0) {} ///< For savegame loading
	CmdDestroyMapObject (uint32_t t, MapObject &);
	void execute (Game &) override;

	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_DESTROY_MAPOBJECT;}

private:
	Serial obj_serial;
};

struct CmdAct : public GameLogicCommand {
	CmdAct() : GameLogicCommand(0), obj_serial(0), arg(0) {} ///< For savegame loading
	CmdAct (uint32_t t, MapObject &, int32_t a);

	void execute (Game &) override;

	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	uint8_t id() const override {return QUEUE_CMD_ACT;}

private:
	Serial obj_serial;
	int32_t arg;
};

}

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_MAP_OBJECT_H
