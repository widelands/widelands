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

#include "container_iterate.h"
#include "log.h"
#include "logic/cmd_queue.h"
#include "logic/game.h"
#include "logic/queue_cmd_ids.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "map_io/widelands_map_map_object_saver.h"
#include "wexception.h"

namespace Widelands {

Cmd_Destroy_Map_Object::Cmd_Destroy_Map_Object
	(int32_t const t, Map_Object & o)
	: GameLogicCommand(t), obj_serial(o.serial())
{}

void Cmd_Destroy_Map_Object::execute(Game & game)
{
	game.syncstream().Unsigned32(obj_serial);

	if (Map_Object * obj = game.objects().get_object(obj_serial))
		obj->destroy (game);
}

#define CMD_DESTROY_MAP_OBJECT_VERSION 1
void Cmd_Destroy_Map_Object::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
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
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(CMD_DESTROY_MAP_OBJECT_VERSION);

	// Write base classes
	GameLogicCommand::Write(fw, egbase, mos);

	// Now serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(obj_serial)));
}

Cmd_Act::Cmd_Act(int32_t const t, Map_Object & o, int32_t const a) :
	GameLogicCommand(t), obj_serial(o.serial()), arg(a)
{}


void Cmd_Act::execute(Game & game)
{
	game.syncstream().Unsigned32(obj_serial);

	if (Map_Object * const obj = game.objects().get_object(obj_serial))
		obj->act(game, arg);
	// the object must queue the next CMD_ACT itself if necessary
}

#define CMD_ACT_VERSION 1
void Cmd_Act::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
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
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
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
void Object_Manager::insert(Map_Object * obj)
{
	++m_lastserial;
	assert(m_lastserial);
	obj->m_serial = m_lastserial;
	m_objects[m_lastserial] = obj;
}

/**
 * Remove the Map_Object from the manager
 */
void Object_Manager::remove(Map_Object & obj)
{
	m_objects.erase(obj.m_serial);
}

/*
 * Return the list of all serials currently in use
 */
std::vector<Serial> Object_Manager::all_object_serials_ordered () const {
	std::vector<Serial> rv;

	container_iterate_const(objmap_t, m_objects, o)
		rv.push_back(o->first);

	std::sort(rv.begin(), rv.end());

	return rv;
}

Map_Object * Object_Ptr::get(const Editor_Game_Base & egbase)
{
	if (!m_serial)
		return nullptr;
	Map_Object * const obj = egbase.objects().get_object(m_serial);
	if (!obj)
		m_serial = 0;
	return obj;
}

// This version also returns a pointer to a non-const object,
// because it is logically the pointer that is const, not the object
// that is pointed to.
// That is, a 'const Object_Ptr' behaves like a 'Object_Ptr * const'.
Map_Object * Object_Ptr::get(const Editor_Game_Base & egbase) const {
	return m_serial ? egbase.objects().get_object(m_serial) : nullptr;
}




/*
==============================================================================

Map_Object_Descr IMPLEMENTATION

==============================================================================
*/

uint32_t Map_Object_Descr::s_dyn_attribhigh =
	Map_Object::HIGHEST_FIXED_ATTRIBUTE;
Map_Object_Descr::AttribMap Map_Object_Descr::s_dyn_attribs;

/**
 * Add this animation for this map object under this name
 */
bool Map_Object_Descr::is_animation_known(const std::string & animname) const {
	container_iterate_const(Anims, m_anims, i)
		if (i.current->first == animname)
			return true;
	return false;
}

void Map_Object_Descr::add_animation
	(const std::string & animname, uint32_t const anim)
{
#ifndef NDEBUG
	container_iterate_const(Anims, m_anims, i)
		if (i.current->first == animname)
			throw wexception
				("adding already existing animation \"%s\"", animname.c_str());
#endif
	m_anims.insert(std::pair<std::string, uint32_t>(animname, anim));
}


std::string Map_Object_Descr::get_animation_name(uint32_t const anim) const {
	container_iterate_const(Anims, m_anims, i)
		if (i.current->second == anim)
			return i.current->first;

	// Never here
	assert(false);
	return "";
}


/**
 * Search for the attribute in the attribute list
 */
bool Map_Object_Descr::has_attribute(uint32_t const attr) const {
	container_iterate_const(Attributes, m_attributes, i)
		if (*i.current == attr)
			return true;
	return false;
}


/**
 * Add an attribute to the attribute list if it's not already there
 */
void Map_Object_Descr::add_attribute(uint32_t const attr)
{
	if (!has_attribute(attr))
		m_attributes.push_back(attr);
}


/**
 * Lookup an attribute by name. If the attribute name hasn't been encountered
 * before, we add it to the map.
 */
uint32_t Map_Object_Descr::get_attribute_id(const std::string & name) {
	AttribMap::iterator it = s_dyn_attribs.find(name);

	if (it != s_dyn_attribs.end())
		return it->second;

	if      (name == "worker")
		return Map_Object::WORKER;
	else if (name == "resi")
		return Map_Object::RESI;

	++s_dyn_attribhigh;
	s_dyn_attribs[name] = s_dyn_attribhigh;

	assert(s_dyn_attribhigh != 0); // wrap around seems *highly* unlikely ;)

	return s_dyn_attribhigh;
}

/**
 * Lookup an attribute by id. If the attribute isn't found,
 * returns an emtpy string.
 */
std::string Map_Object_Descr::get_attribute_name(uint32_t id) {
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
Map_Object::Map_Object(const Map_Object_Descr * const the_descr) :
m_descr(the_descr), m_serial(0), m_logsink(nullptr)
{}


/**
 * Call this function if you want to remove the object immediately, without
 * any effects.
 */
void Map_Object::remove(Editor_Game_Base & egbase)
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
void Map_Object::destroy(Editor_Game_Base & egbase)
{
	remove(egbase);
}

/**
 * Schedule the object for immediate destruction.
 * This can be used to safely destroy the object from within an act function.
 */
void Map_Object::schedule_destroy(Game & game)
{
	game.cmdqueue().enqueue
		(new Cmd_Destroy_Map_Object(game.get_gametime(), *this));
}

/**
 * Initialize the object by adding it to the object manager.
 *
 * \warning Make sure you call this from derived classes!
 */
void Map_Object::init(Editor_Game_Base & egbase)
{
	egbase.objects().insert(this);
}

/**
 * \warning Make sure you call this from derived classes!
 */
void Map_Object::cleanup(Editor_Game_Base & egbase)
{
	egbase.objects().remove(*this);
}

/**
 * Default implementation
 */
int32_t Map_Object::get_tattribute(uint32_t) const
{
	return -1;
}


/**
 * Queue a CMD_ACT tdelta milliseconds from now, using the given data.
 *
 * \return The absolute gametime at which the CMD_ACT will occur.
 */
uint32_t Map_Object::schedule_act
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
void Map_Object::act(Game &, uint32_t) {}


/**
 * Set the logsink. This should only be used by the debugging facilities.
 */
void Map_Object::set_logsink(LogSink * const sink)
{
	m_logsink = sink;
}


void Map_Object::log_general_info(const Editor_Game_Base &) {}

/**
 * Prints a log message prepended by the object's serial number.
 */
void Map_Object::molog(char const * fmt, ...) const
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
void Map_Object::Loader::load(FileRead & fr)
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
void Map_Object::Loader::load_pointers() {}


/**
 * This will be called after all instances have been load_pointer'ed.
 *
 * This is where dependent data (e.g. ware requests) should be checked and
 * configured.
 *
 * Derived functions must call ancestor's function in the appropriate place.
 */
void Map_Object::Loader::load_finish()
{
}

/**
 * Save the Map_Object to the given file.
 */
void Map_Object::save
	(Editor_Game_Base &, Map_Map_Object_Saver & mos, FileWrite & fw)
{
	fw.Unsigned8(header_Map_Object);
	fw.Unsigned8(CURRENT_SAVEGAME_VERSION);

	fw.Unsigned32(mos.get_object_file_index(*this));
}

}
