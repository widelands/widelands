/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_LOGIC_MAP_OBJECTS_MAP_OBJECT_H
#define WL_LOGIC_MAP_OBJECTS_MAP_OBJECT_H

#include <atomic>

#include "base/macros.h"
#include "graphic/animation/animation.h"
#include "graphic/animation/diranimations.h"
#include "graphic/color.h"
#include "graphic/image.h"
#include "logic/cmd_queue.h"
#include "logic/map_objects/info_to_draw.h"
#include "logic/map_objects/map_object_type.h"
#include "logic/map_objects/tribes/training_attribute.h"
#include "logic/widelands.h"
#include "notifications/signal.h"
#include "scripting/lua_table.h"

class RenderTarget;

namespace Widelands {

class MapObject;
class Player;

/**
 * Base class for descriptions of worker, files and so on. This must just
 * link them together
 */
class MapObjectDescr {
public:
	using AttributeIndex = uint32_t;
	using Attributes = std::vector<AttributeIndex>;

	enum class OwnerType { kWorld, kTribe };

	MapObjectDescr(const MapObjectType init_type,
	               const std::string& init_name,
	               const std::string& init_descname);
	MapObjectDescr(const MapObjectType init_type,
	               const std::string& init_name,
	               const std::string& init_descname,
	               const LuaTable& table);
	virtual ~MapObjectDescr();

	const std::string& name() const {
		return name_;
	}
	const std::string& descname() const {
		return descname_;
	}

	// Type of the MapObjectDescr.
	MapObjectType type() const {
		return type_;
	}

	virtual uint32_t get_animation(const std::string& animname, const MapObject* mo) const;

	uint32_t main_animation() const;
	std::string get_animation_name(uint32_t) const;  ///< needed for save, debug

	bool is_animation_known(const std::string& name) const;

	/// Preload animation graphics at default scale
	void load_graphics() const;

	/// Returns the image for the first frame of the idle animation if the MapObject has animations,
	/// nullptr otherwise
	const Image* representative_image(const RGBColor* player_color = nullptr) const;

	/// Returns the menu image if the MapObject has one, nullptr otherwise
	const Image* icon() const;
	/// Returns the image fileneme for the menu image if the MapObject has one, is empty otherwise
	const std::string& icon_filename() const;

	bool has_attribute(AttributeIndex) const;
	const MapObjectDescr::Attributes& attributes() const;
	static AttributeIndex get_attribute_id(const std::string& name, bool add_if_not_exists = false);

	/// Sets a tribe-specific ware or immovable helptext for this MapObject
	void set_helptexts(const std::string& tribename,
	                   std::map<std::string, std::string> localized_helptext);
	/// Gets the tribe-specific ware or immovable helptext for the given tribe. Fails if it doesn't
	/// exist.
	const std::map<std::string, std::string>& get_helptexts(const std::string& tribename) const;
	/// Returns whether a tribe-specific helptext exists for the given tribe
	bool has_helptext(const std::string& tribename) const;

protected:
	// Add attributes to the attribute list
	void add_attributes(const std::vector<std::string>& attribs);
	void add_attribute(AttributeIndex attr);

	/// Sets the directional animations in 'anims' with the animations
	/// '&lt;basename&gt;_(ne|e|se|sw|w|nw)'.
	void assign_directional_animation(DirAnimations* anims, const std::string& basename);

private:
	void add_animations(const LuaTable& table,
	                    const std::string& animation_directory,
	                    Animation::Type anim_type);

	/// Throws an exception if the MapObjectDescr has no representative image
	void check_representative_image();

	using Anims = std::map<std::string, uint32_t>;

	static std::map<std::string, AttributeIndex> attribute_names_;
	Attributes attribute_ids_;

	const MapObjectType type_;    /// Subclasses pick from the enum above
	std::string const name_;      /// The name for internal reference
	std::string const descname_;  /// A localized Descriptive name

	/// Tribe-specific helptexts. Format: <tribename, <category, localized_text>>
	std::map<std::string, std::map<std::string, std::string>> helptexts_;

	Anims anims_;
	std::string icon_filename_;  // Filename for the menu icon

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
 * potentially set it up by calling basic functions like set_position(), etc.
 * and then call init(). After that, the object is supposed to
 * be fully created.
 */

/// If you find a better way to do this that doesn't cost a virtual function
/// or additional member variable, go ahead
#define MO_DESCR(type)                                                                             \
public:                                                                                            \
	const type& descr() const {                                                                     \
		return dynamic_cast<const type&>(*descr_);                                                   \
	}

class MapObject {
	friend struct ObjectManager;
	friend struct ObjectPointer;

	MO_DESCR(MapObjectDescr)

public:
	struct LogSink {
		virtual void log(const std::string& str) = 0;
		virtual ~LogSink() {
		}
	};

	virtual void load_finish(EditorGameBase&) {
	}

	virtual const Image* representative_image() const;

protected:
	explicit MapObject(MapObjectDescr const* descr);
	virtual ~MapObject() {
	}

public:
	Serial serial() const {
		return serial_;
	}

	/**
	 * Is called right before the object will be removed from
	 * the game. No connection is handled in this class.
	 *
	 * param serial : the object serial (cannot use param comment as this is a callback)
	 */
	Notifications::Signal<uint32_t /* serial */> removed;

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
	virtual int32_t get_training_attribute(TrainingAttribute attr) const;

	void remove(EditorGameBase&);
	virtual void destroy(EditorGameBase&);

	//  The next functions are really only needed in games, not in the editor.
	void schedule_destroy(Game&);
	Time schedule_act(Game&, const Duration& tdelta, uint32_t data = 0);
	virtual void act(Game&, uint32_t data);

	LogSink* get_logsink() {
		return logsink_;
	}
	void set_logsink(LogSink*);

	/// Called when a new logsink is set. Used to give general information.
	virtual void log_general_info(const EditorGameBase&) const;

	Player* get_owner() const {
		return owner_;
	}

	const Player& owner() const;

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
		HeaderShipFleet = 11,
		HeaderFerryFleet = 12,
	};

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
		EditorGameBase* egbase_;
		MapObjectLoader* mol_;
		MapObject* object_;

	protected:
		Loader() : egbase_(nullptr), mol_(nullptr), object_(nullptr) {
		}

	public:
		virtual ~Loader() {
		}

		void init(EditorGameBase& e, MapObjectLoader& m, MapObject& object) {
			egbase_ = &e;
			mol_ = &m;
			object_ = &object;
		}

		EditorGameBase& egbase() {
			return *egbase_;
		}
		MapObjectLoader& mol() {
			return *mol_;
		}
		MapObject* get_object() {
			return object_;
		}
		template <typename T> T& get() {
			return dynamic_cast<T&>(*object_);
		}

	protected:
		void load(FileRead&);

	public:
		virtual void load_pointers();
		virtual void load_finish();
	};

	/// This is just a fail-safe guard for the time until we fully transition
	/// to the new MapObject saving system
	virtual bool has_new_save_support() {
		return false;
	}

	virtual void save(EditorGameBase&, MapObjectSaver&, FileWrite&);
	// Pure MapObjects cannot be loaded

protected:
	/// Called only when the oject is logically created in the simulation. If
	/// called again, such as when the object is loaded from a savegame, it will
	/// cause bugs.
	virtual bool init(EditorGameBase&);

	virtual void cleanup(EditorGameBase&);

	/// Draws census and statistics on screen
	void do_draw_info(const InfoToDraw& info_to_draw,
	                  const std::string& census,
	                  const std::string& statictics,
	                  const Vector2f& field_on_dst,
	                  const float scale,
	                  RenderTarget* dst) const;

	void molog(const Time& gametime, char const* fmt, ...) const PRINTF_FORMAT(3, 4);

	const MapObjectDescr* descr_;
	Serial serial_;
	LogSink* logsink_;
	std::atomic<Player*> owner_;

	/**
	 * MapObjects like trees are reserved by a worker that is walking
	 * towards them, so that e.g. two lumberjacks don't attempt to
	 * work on the same tree simultaneously or two hunters try to hunt
	 * the same animal.
	 */
	bool reserved_by_worker_;

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
struct ObjectManager {
	using MapObjectMap = std::unordered_map<Serial, MapObject*>;

	ObjectManager() : lastserial_(0), is_cleaning_up_(false) {
	}
	~ObjectManager();

	void cleanup(EditorGameBase&);

	MapObject* get_object(Serial const serial) const {
		const MapObjectMap::const_iterator it = objects_.find(serial);
		return it != objects_.end() ? it->second : nullptr;
	}

	void insert(MapObject*);
	void remove(MapObject&);

	/**
	 * When saving the map object, ordere matters. Return a vector of all ids
	 * that are currently available;
	 */
	std::vector<Serial> all_object_serials_ordered() const;

	bool is_cleaning_up() const {
		return is_cleaning_up_;
	}

private:
	Serial lastserial_;
	MapObjectMap objects_;

	bool is_cleaning_up_;

	DISALLOW_COPY_AND_ASSIGN(ObjectManager);
};

/**
 * Provides a safe pointer to a MapObject
 * Make sure the MapObject is initialized (has a serial) before using it in ObjectPointer!
 */
struct ObjectPointer {
	// Provide default constructor to shut up cppcheck.
	ObjectPointer() {
		serial_ = 0;
	}
	ObjectPointer(const MapObject* const obj) {
		assert(obj == nullptr || obj->serial_ != 0);
		serial_ = obj ? obj->serial_ : 0;
	}
	// can use standard copy constructor and assignment operator

	ObjectPointer& operator=(const MapObject* const obj) {
		assert(obj == nullptr || obj->serial_ != 0);
		serial_ = obj ? obj->serial_ : 0;
		return *this;
	}

	bool is_set() const {
		return serial_;
	}

	// TODO(unknown): dammit... without an EditorGameBase object, we can't implement a
	// MapObject* operator (would be really nice)
	MapObject* get(const EditorGameBase&);
	MapObject* get(const EditorGameBase& egbase) const;

	bool operator<(const ObjectPointer& other) const {
		return serial_ < other.serial_;
	}
	bool operator==(const ObjectPointer& other) const {
		return serial_ == other.serial_;
	}
	bool operator!=(const ObjectPointer& other) const {
		return serial_ != other.serial_;
	}

	uint32_t serial() const {
		return serial_;
	}

private:
	uint32_t serial_;
};

template <class T> struct OPtr {
	OPtr(T* const obj = nullptr) : m(obj) {
	}

	OPtr& operator=(T* const obj) {
		m = obj;
		return *this;
	}

	bool is_set() const {
		return m.is_set();
	}

	T* get(const EditorGameBase& egbase) {
		return static_cast<T*>(m.get(egbase));
	}
	T* get(const EditorGameBase& egbase) const {
		return static_cast<T*>(m.get(egbase));
	}

	bool operator<(const OPtr<T>& other) const {
		return m < other.m;
	}
	bool operator==(const OPtr<T>& other) const {
		return m == other.m;
	}
	bool operator!=(const OPtr<T>& other) const {
		return m != other.m;
	}

	Serial serial() const {
		return m.serial();
	}

private:
	ObjectPointer m;
};

struct CmdDestroyMapObject : public GameLogicCommand {
	CmdDestroyMapObject() : GameLogicCommand(Time()), obj_serial(0) {
	}  ///< For savegame loading
	CmdDestroyMapObject(const Time&, MapObject&);
	void execute(Game&) override;

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kDestroyMapObject;
	}

private:
	Serial obj_serial;
};

struct CmdAct : public GameLogicCommand {
	CmdAct() : GameLogicCommand(Time()), obj_serial(0), arg(0) {
	}  ///< For savegame loading
	CmdAct(const Time& t, MapObject&, int32_t a);

	void execute(Game&) override;

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kAct;
	}

private:
	Serial obj_serial;
	int32_t arg;
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_MAP_OBJECT_H
