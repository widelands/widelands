/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#include "logic/instances.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>

#include "base/log.h"
#include "base/wexception.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/cmd_queue.h"
#include "logic/game.h"
#include "logic/queue_cmd_ids.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

CmdDestroyMapObject::CmdDestroyMapObject
	(int32_t const t, MapObject & o)
	: GameLogicCommand(t), obj_serial(o.serial())
{}

void CmdDestroyMapObject::execute(Game & game)
{
	game.syncstream().unsigned_32(obj_serial);

	if (MapObject * obj = game.objects().get_object(obj_serial))
		obj->destroy (game);
}

constexpr uint16_t kCurrentPacketVersionDestroyMapObject = 1;

void CmdDestroyMapObject::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionDestroyMapObject) {
			GameLogicCommand::read(fr, egbase, mol);
			if (Serial const serial = fr.unsigned_32())
				try {
					obj_serial = mol.get<MapObject>(serial).serial();
				} catch (const WException & e) {
					throw GameDataError("%u: %s", serial, e.what());
				}
			else
				obj_serial = 0;
		} else {
			throw UnhandledVersionError(packet_version, kCurrentPacketVersionDestroyMapObject);
		}
	} catch (const WException & e) {
		throw GameDataError("destroy map object: %s", e.what());
	}
}
void CmdDestroyMapObject::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionDestroyMapObject);

	// Write base classes
	GameLogicCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(obj_serial)));
}

CmdAct::CmdAct(int32_t const t, MapObject & o, int32_t const a) :
	GameLogicCommand(t), obj_serial(o.serial()), arg(a)
{}


void CmdAct::execute(Game & game)
{
	game.syncstream().unsigned_32(obj_serial);

	if (MapObject * const obj = game.objects().get_object(obj_serial))
		obj->act(game, arg);
	// the object must queue the next CMD_ACT itself if necessary
}

constexpr uint16_t kCurrentPacketVersionCmdAct = 1;

void CmdAct::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdAct) {
			GameLogicCommand::read(fr, egbase, mol);
			if (Serial const object_serial = fr.unsigned_32())
				try {
					obj_serial = mol.get<MapObject>(object_serial).serial();
				} catch (const WException & e) {
					throw GameDataError
						("object %u: %s", object_serial, e.what());
				}
			else
				obj_serial = 0;
			arg = fr.unsigned_32();
		} else {
			throw UnhandledVersionError(packet_version, kCurrentPacketVersionCmdAct);
		}
	} catch (const WException & e) {
		throw wexception("act: %s", e.what());
	}
}
void CmdAct::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdAct);

	// Write base classes
	GameLogicCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(obj_serial)));

	// And arg
	fw.unsigned_32(arg);
}


ObjectManager::~ObjectManager()
{
	// better not throw an exception in a destructor...
	if (!m_objects.empty())
		log("ObjectManager: ouch! remaining objects\n");

	log("lastserial: %i\n", m_lastserial);
}

/**
 * Clear all objects
 */
void ObjectManager::cleanup(EditorGameBase & egbase)
{
	while (!m_objects.empty()) {
		MapObjectMap::iterator it = m_objects.begin();
		it->second->remove(egbase);
	}
	m_lastserial = 0;
}

/**
 * Insert the given MapObject into the object manager
 */
void ObjectManager::insert(MapObject * obj)
{
	++m_lastserial;
	assert(m_lastserial);
	obj->m_serial = m_lastserial;
	m_objects[m_lastserial] = obj;
}

/**
 * Remove the MapObject from the manager
 */
void ObjectManager::remove(MapObject & obj)
{
	m_objects.erase(obj.m_serial);
}

/*
 * Return the list of all serials currently in use
 */
std::vector<Serial> ObjectManager::all_object_serials_ordered () const {
	std::vector<Serial> rv;

	for (const std::pair<Serial, MapObject *>& o : m_objects) {
		rv.push_back(o.first);
	}

	std::sort(rv.begin(), rv.end());

	return rv;
}

MapObject * ObjectPointer::get(const EditorGameBase & egbase)
{
	if (!m_serial)
		return nullptr;
	MapObject * const obj = egbase.objects().get_object(m_serial);
	if (!obj)
		m_serial = 0;
	return obj;
}

// This version also returns a pointer to a non-const object,
// because it is logically the pointer that is const, not the object
// that is pointed to.
// That is, a 'const ObjectPointer' behaves like a 'ObjectPointer * const'.
MapObject * ObjectPointer::get(const EditorGameBase & egbase) const {
	return m_serial ? egbase.objects().get_object(m_serial) : nullptr;
}




/*
==============================================================================

MapObjectDescr IMPLEMENTATION

==============================================================================
*/

uint32_t MapObjectDescr::s_dyn_attribhigh =
	MapObject::HIGHEST_FIXED_ATTRIBUTE;
MapObjectDescr::AttribMap MapObjectDescr::s_dyn_attribs;

/**
 * Add this animation for this map object under this name
 */
bool MapObjectDescr::is_animation_known(const std::string & animname) const {
	for (const std::pair<std::string, uint32_t>& anim : m_anims) {
		if (anim.first == animname) {
			return true;
		}
	}
	return false;
}

void MapObjectDescr::add_animation
	(const std::string & animname, uint32_t const anim)
{
#ifndef NDEBUG
	for (const std::pair<std::string, uint32_t>& temp_anim : m_anims) {
		if (temp_anim.first == animname) {
			throw wexception
				("adding already existing animation \"%s\"", animname.c_str());
		}
	}
#endif
	m_anims.insert(std::pair<std::string, uint32_t>(animname, anim));
}

std::string MapObjectDescr::get_animation_name(uint32_t const anim) const {

	for (const std::pair<std::string, uint32_t>& temp_anim : m_anims) {
		if (temp_anim.second == anim) {
			return temp_anim.first;
		}
	}

	// Never here
	assert(false);
	return "";
}


/**
 * Search for the attribute in the attribute list
 */
bool MapObjectDescr::has_attribute(uint32_t const attr) const {
	for (const uint32_t& attrib : m_attributes) {
		if (attrib == attr) {
			return true;
		}
	}
	return false;
}


/**
 * Add an attribute to the attribute list if it's not already there
 */
void MapObjectDescr::add_attribute(uint32_t const attr)
{
	if (!has_attribute(attr))
		m_attributes.push_back(attr);
}

void MapObjectDescr::add_attributes(const std::vector<std::string>& attributes,
									  const std::set<uint32_t>& allowed_special) {
	for (const std::string& attribute : attributes) {
		uint32_t const attrib = get_attribute_id(attribute);
		if (attrib < MapObject::HIGHEST_FIXED_ATTRIBUTE) {
			if (!allowed_special.count(attrib)) {
				throw GameDataError("bad attribute \"%s\"", attribute.c_str());
			}
		}
		add_attribute(attrib);
	}
}

/**
 * Lookup an attribute by name. If the attribute name hasn't been encountered
 * before, we add it to the map.
 */
uint32_t MapObjectDescr::get_attribute_id(const std::string & name) {
	AttribMap::iterator it = s_dyn_attribs.find(name);

	if (it != s_dyn_attribs.end())
		return it->second;

	if      (name == "worker")
		return MapObject::WORKER;
	else if (name == "resi")
		return MapObject::RESI;

	++s_dyn_attribhigh;
	s_dyn_attribs[name] = s_dyn_attribhigh;

	assert(s_dyn_attribhigh != 0); // wrap around seems *highly* unlikely ;)

	return s_dyn_attribhigh;
}

/**
 * Lookup an attribute by id. If the attribute isn't found,
 * returns an emtpy string.
 */
std::string MapObjectDescr::get_attribute_name(uint32_t id) {
	for
		(AttribMap::iterator iter = s_dyn_attribs.begin();
		 iter != s_dyn_attribs.end(); ++iter)
	{
		if (iter->second == id)
			return iter->first;
	}
	return "";
}

/*
==============================================================================

MapObject IMPLEMENTATION

==============================================================================
*/

/**
 * Zero-initialize a map object
 */
MapObject::MapObject(const MapObjectDescr * const the_descr) :
m_descr(the_descr),
m_serial(0),
m_logsink(nullptr),
m_reserved_by_worker(false)
{}


/**
 * Call this function if you want to remove the object immediately, without
 * any effects.
 */
void MapObject::remove(EditorGameBase & egbase)
{
	removed(m_serial); // Signal call
	cleanup(egbase);
	delete this;
}

/**
 * Destroy the object immediately. Unlike remove(), special actions may be
 * performed:
 * \li Create a decaying skeleton (humans)
 * \li Create a burning fire (buildings)
 * \li ...
 *
 * \warning This function will immediately delete the memory allocated for
 * the object. Therefore, it may be safer to call schedule_destroy()
 * instead.
 */
void MapObject::destroy(EditorGameBase & egbase)
{
	remove(egbase);
}

/**
 * Schedule the object for immediate destruction.
 * This can be used to safely destroy the object from within an act function.
 */
void MapObject::schedule_destroy(Game & game)
{
	game.cmdqueue().enqueue
		(new CmdDestroyMapObject(game.get_gametime(), *this));
}

/**
 * Initialize the object by adding it to the object manager.
 *
 * \warning Make sure you call this from derived classes!
 */
void MapObject::init(EditorGameBase & egbase)
{
	egbase.objects().insert(this);
}

/**
 * \warning Make sure you call this from derived classes!
 */
void MapObject::cleanup(EditorGameBase & egbase)
{
	egbase.objects().remove(*this);
}

/**
 * Default implementation
 */
int32_t MapObject::get_training_attribute(uint32_t) const
{
	return -1;
}


/**
 * Queue a CMD_ACT tdelta milliseconds from now, using the given data.
 *
 * \return The absolute gametime at which the CMD_ACT will occur.
 */
uint32_t MapObject::schedule_act
	(Game & game, uint32_t const tdelta, uint32_t const data)
{
	if (tdelta < endless()) {
		uint32_t const time = game.get_gametime() + tdelta;

		game.cmdqueue().enqueue (new CmdAct(time, *this, data));

		return time;
	} else
		return never();
}


/**
 * Called when a CMD_ACT triggers.
 */
void MapObject::act(Game &, uint32_t) {}


/**
 * Set the logsink. This should only be used by the debugging facilities.
 */
void MapObject::set_logsink(LogSink * const sink)
{
	m_logsink = sink;
}


void MapObject::log_general_info(const EditorGameBase &) {}

/**
 * Prints a log message prepended by the object's serial number.
 */
void MapObject::molog(char const * fmt, ...) const
{
	if (!g_verbose && !m_logsink)
		return;

	va_list va;
	char buffer[2048];

	va_start(va, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, va);
	va_end(va);

	if (m_logsink)
		m_logsink->log(buffer);

	log("MO(%u,%s): %s", m_serial, descr().name().c_str(), buffer);
}

bool MapObject::is_reserved_by_worker() const
{
	return m_reserved_by_worker;
}

void MapObject::set_reserved_by_worker(bool reserve)
{
	m_reserved_by_worker = reserve;
}


constexpr uint8_t kCurrentPacketVersionMapObject = 2;

/**
 * Load the entire data package from the given file.
 * This will be called from the MapObject's derived class static load function.
 *
 * Derived functions must read all data into member variables, even if
 * it is used only later in \ref load_pointers or \ref load_finish .
 *
 * Derived functions must call ancestor's function in the appropriate place.
 */
void MapObject::Loader::load(FileRead & fr)
{
	try {
		uint8_t const header = fr.unsigned_8();
		if (header != HeaderMapObject)
			throw wexception
				("header is %u, expected %u", header, HeaderMapObject);

		uint8_t const packet_version = fr.unsigned_8();
		if (packet_version <= 0 || packet_version > kCurrentPacketVersionMapObject) {
			throw UnhandledVersionError(packet_version, kCurrentPacketVersionMapObject);
		}

		Serial const serial = fr.unsigned_32();
		try {
			mol().register_object<MapObject>(serial, *get_object());
		} catch (const WException & e) {
			throw wexception("%u: %s", serial, e.what());
		}

		if (packet_version == kCurrentPacketVersionMapObject) {
			get_object()->m_reserved_by_worker = fr.unsigned_8();
		}
	} catch (const WException & e) {
		throw wexception("map object: %s", e.what());
	}

	egbase().objects().insert(get_object());
}


/**
 * This will be called after all instances have been loaded.
 *
 * This is where pointers to other instances should be established, possibly
 * using data that was previously stored in a member variable by \ref load .
 *
 * Derived functions must call ancestor's function in the appropriate place.
 */
void MapObject::Loader::load_pointers() {}


/**
 * This will be called after all instances have been load_pointer'ed.
 *
 * This is where dependent data (e.g. ware requests) should be checked and
 * configured.
 *
 * Derived functions must call ancestor's function in the appropriate place.
 */
void MapObject::Loader::load_finish()
{
}

/**
 * Save the MapObject to the given file.
 */
void MapObject::save
	(EditorGameBase &, MapObjectSaver & mos, FileWrite & fw)
{
	fw.unsigned_8(HeaderMapObject);
	fw.unsigned_8(kCurrentPacketVersionMapObject);

	fw.unsigned_32(mos.get_object_file_index(*this));
	fw.unsigned_8(m_reserved_by_worker);
}

std::string to_string(const MapObjectType type) {
	switch (type) {
	case MapObjectType::BOB:
		return "bob";
	case MapObjectType::CRITTER:
		return "critter";
	case MapObjectType::SHIP:
		return "ship";
	case MapObjectType::WORKER:
		return "worker";
	case MapObjectType::CARRIER:
		return "carrier";
	case MapObjectType::SOLDIER:
		return "soldier";
	case MapObjectType::WARE:
		return "ware";
	case MapObjectType::BATTLE:
		return "battle";
	case MapObjectType::FLEET:
		return "fleet";
	case MapObjectType::IMMOVABLE:
		return "immovable";
	case MapObjectType::FLAG:
		return "flag";
	case MapObjectType::ROAD:
		return "road";
	case MapObjectType::PORTDOCK:
		return "portdock";
	case MapObjectType::BUILDING:
		return "building";
	case MapObjectType::CONSTRUCTIONSITE:
		return "constructionsite";
	case MapObjectType::DISMANTLESITE:
		return "dismantlesite";
	case MapObjectType::WAREHOUSE:
		return "warehouse";
	case MapObjectType::PRODUCTIONSITE:
		return "productionsite";
	case MapObjectType::MILITARYSITE:
		return "militarysite";
	case MapObjectType::TRAININGSITE:
		return "trainingsite";
	default:
		throw wexception("Unknown MapObjectType %d.", static_cast<int>(type));
	}
}

}
