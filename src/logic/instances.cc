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

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>

#include "base/deprecated.h"
#include "base/log.h"
#include "base/wexception.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/cmd_queue.h"
#include "logic/game.h"
#include "logic/queue_cmd_ids.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "map_io/widelands_map_map_object_saver.h"

namespace Widelands {

Cmd_Destroy_Map_Object::Cmd_Destroy_Map_Object
	(int32_t const t, MapObject & o)
	: GameLogicCommand(t), obj_serial(o.serial())
{}

void Cmd_Destroy_Map_Object::execute(Game & game)
{
	game.syncstream().Unsigned32(obj_serial);

	if (MapObject * obj = game.objects().get_object(obj_serial))
		obj->destroy (game);
}

#define CMD_DESTROY_MAP_OBJECT_VERSION 1
void Cmd_Destroy_Map_Object::Read
	(FileRead & fr, Editor_Game_Base & egbase, MapMapObjectLoader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CMD_DESTROY_MAP_OBJECT_VERSION) {
			GameLogicCommand::Read(fr, egbase, mol);
			if (Serial const serial = fr.Unsigned32())
				try {
					obj_serial = mol.get<Map_Object>(serial).serial();
				} catch (const _wexception & e) {
					throw game_data_error("%u: %s", serial, e.what());
				}
			else
				obj_serial = 0;
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("destroy map object: %s", e.what());
	}
}
void Cmd_Destroy_Map_Object::Write
	(FileWrite & fw, Editor_Game_Base & egbase, MapMapObjectSaver & mos)
{
	// First, write version
	fw.Unsigned16(CMD_DESTROY_MAP_OBJECT_VERSION);

	// Write base classes
	GameLogicCommand::Write(fw, egbase, mos);

	// Now serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(obj_serial)));
}

Cmd_Act::Cmd_Act(int32_t const t, MapObject & o, int32_t const a) :
	GameLogicCommand(t), obj_serial(o.serial()), arg(a)
{}


void Cmd_Act::execute(Game & game)
{
	game.syncstream().Unsigned32(obj_serial);

	if (MapObject * const obj = game.objects().get_object(obj_serial))
		obj->act(game, arg);
	// the object must queue the next CMD_ACT itself if necessary
}

#define CMD_ACT_VERSION 1
void Cmd_Act::Read
	(FileRead & fr, Editor_Game_Base & egbase, MapMapObjectLoader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CMD_ACT_VERSION) {
			GameLogicCommand::Read(fr, egbase, mol);
			if (Serial const object_serial = fr.Unsigned32())
				try {
					obj_serial = mol.get<Map_Object>(object_serial).serial();
				} catch (const _wexception & e) {
					throw game_data_error
						("object %u: %s", object_serial, e.what());
				}
			else
				obj_serial = 0;
			arg = fr.Unsigned32();
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw wexception("act: %s", e.what());
	}
}
void Cmd_Act::Write
	(FileWrite & fw, Editor_Game_Base & egbase, MapMapObjectSaver & mos)
{
	// First, write version
	fw.Unsigned16(CMD_ACT_VERSION);

	// Write base classes
	GameLogicCommand::Write(fw, egbase, mos);

	// Now serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(obj_serial)));

	// And arg
	fw.Unsigned32(arg);
}


Object_Manager::~Object_Manager()
{
	// better not throw an exception in a destructor...
	if (!m_objects.empty())
		log("Object_Manager: ouch! remaining objects\n");

	log("lastserial: %i\n", m_lastserial);
}

/**
 * Clear all objects
 */
void Object_Manager::cleanup(Editor_Game_Base & egbase)
{
	while (!m_objects.empty()) {
		objmap_t::iterator it = m_objects.begin();
		it->second->remove(egbase);
	}
	m_lastserial = 0;
}

/**
 * Insert the given Map_Object into the object manager
 */
void Object_Manager::insert(MapObject * obj)
{
	++m_lastserial;
	assert(m_lastserial);
	obj->m_serial = m_lastserial;
	m_objects[m_lastserial] = obj;
}

/**
 * Remove the Map_Object from the manager
 */
void Object_Manager::remove(MapObject & obj)
{
	m_objects.erase(obj.m_serial);
}

/*
 * Return the list of all serials currently in use
 */
std::vector<Serial> Object_Manager::all_object_serials_ordered () const {
	std::vector<Serial> rv;

	for (const std::pair<Serial, MapObject *>& o : m_objects) {
		rv.push_back(o.first);
	}

	std::sort(rv.begin(), rv.end());

	return rv;
}

MapObject * Object_Ptr::get(const Editor_Game_Base & egbase)
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
// That is, a 'const Object_Ptr' behaves like a 'Object_Ptr * const'.
MapObject * Object_Ptr::get(const Editor_Game_Base & egbase) const {
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
				throw game_data_error("bad attribute \"%s\"", attribute.c_str());
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

Map_Object IMPLEMENTATION

==============================================================================
*/

/**
 * Zero-initialize a map object
 */
MapObject::MapObject(const MapObjectDescr * const the_descr) :
m_descr(the_descr), m_serial(0), m_logsink(nullptr)
{}


/**
 * Call this function if you want to remove the object immediately, without
 * any effects.
 */
void MapObject::remove(Editor_Game_Base & egbase)
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
void MapObject::destroy(Editor_Game_Base & egbase)
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
		(new Cmd_Destroy_Map_Object(game.get_gametime(), *this));
}

/**
 * Initialize the object by adding it to the object manager.
 *
 * \warning Make sure you call this from derived classes!
 */
void MapObject::init(Editor_Game_Base & egbase)
{
	egbase.objects().insert(this);
}

/**
 * \warning Make sure you call this from derived classes!
 */
void MapObject::cleanup(Editor_Game_Base & egbase)
{
	egbase.objects().remove(*this);
}

/**
 * Default implementation
 */
int32_t MapObject::get_tattribute(uint32_t) const
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
	if (tdelta < Forever()) {
		uint32_t const time = game.get_gametime() + tdelta;

		game.cmdqueue().enqueue (new Cmd_Act(time, *this, data));

		return time;
	} else
		return Never();
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


void MapObject::log_general_info(const Editor_Game_Base &) {}

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


#define CURRENT_SAVEGAME_VERSION 1

/**
 * Load the entire data package from the given file.
 * This will be called from the Map_Object's derived class static load function.
 *
 * Derived functions must read all data into member variables, even if
 * it is used only later in \ref load_pointers or \ref load_finish .
 *
 * Derived functions must call ancestor's function in the appropriate place.
 */
void MapObject::Loader::load(FileRead & fr)
{
	try {
		uint8_t const header = fr.Unsigned8();
		if (header != header_Map_Object)
			throw wexception
				("header is %u, expected %u", header, header_Map_Object);

		uint8_t const version = fr.Unsigned8();
		if (version != CURRENT_SAVEGAME_VERSION)
			throw game_data_error("unknown/unhandled version %u", version);

		Serial const serial = fr.Unsigned32();
		try {
			mol().register_object<Map_Object>(serial, *get_object());
		} catch (const _wexception & e) {
			throw wexception("%u: %s", serial, e.what());
		}
	} catch (const _wexception & e) {
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
 * Save the Map_Object to the given file.
 */
void MapObject::save
	(Editor_Game_Base &, MapMapObjectSaver & mos, FileWrite & fw)
{
	fw.Unsigned8(header_Map_Object);
	fw.Unsigned8(CURRENT_SAVEGAME_VERSION);

	fw.Unsigned32(mos.get_object_file_index(*this));
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
