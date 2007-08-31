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

#include "instances.h"

#include "cmd_queue.h"
#include "error.h"
#include "fileread.h"
#include "filewrite.h"
#include "game.h"
#include "queue_cmd_ids.h"
#include "wexception.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"

#include <cstdarg>
#include <string>

#include <stdio.h>


Cmd_Destroy_Map_Object::Cmd_Destroy_Map_Object(int t, Map_Object* o)
	: GameLogicCommand(t)
{
	obj_serial = o->get_serial();
}

void Cmd_Destroy_Map_Object::execute(Game* g)
{
	g->syncstream().Unsigned32(obj_serial);

	Map_Object* obj = g->objects().get_object(obj_serial);

	if (obj)
		obj->destroy (g);
}

#define CMD_DESTROY_MAP_OBJECT_VERSION 1
void Cmd_Destroy_Map_Object::Read
(FileRead               & fr,
 Editor_Game_Base                & egbase,
 Widelands_Map_Map_Object_Loader & mol)
{
	const Uint16 packet_version = fr.Unsigned16();
	if (packet_version == CMD_DESTROY_MAP_OBJECT_VERSION) {
		// Read Base Commands
		GameLogicCommand::Read(fr, egbase, mol);

      // Serial
		if (const Uint32 fileserial = fr.Unsigned32()) {
			assert(mol.is_object_known(fileserial)); //  FIXME NEVER USE assert TO VALIDATE INPUT!!!
			obj_serial = mol.get_object_by_file_index(fileserial)->get_serial();
		} else
			obj_serial = 0;
	} else
		throw wexception
			("Unknown version in Cmd_Destroy_Map_Object::Read: %u",
			 packet_version);
}
void Cmd_Destroy_Map_Object::Write
(FileWrite             & fw,
 Editor_Game_Base               & egbase,
 Widelands_Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(CMD_DESTROY_MAP_OBJECT_VERSION);

	// Write base classes
	GameLogicCommand::Write(fw, egbase, mos);

	// Now serial
	if (const Map_Object * const obj = egbase.objects().get_object(obj_serial)) {
		// The object might have vanished
		assert(mos.is_object_known(obj));
		fw.Unsigned32(mos.get_object_file_index(obj));
	} else
		fw.Unsigned32(0);

}

Cmd_Act::Cmd_Act(int t, Map_Object* o, int a) : GameLogicCommand(t)
{
	obj_serial = o->get_serial();
	arg = a;
}


void Cmd_Act::execute(Game* g)
{
	g->syncstream().Unsigned32(obj_serial);

	Map_Object* obj = g->objects().get_object(obj_serial);
	if (obj)
		obj->act(g, arg);
	// the object must queue the next CMD_ACT itself if necessary
}

#define CMD_ACT_VERSION 1
void Cmd_Act::Read
(FileRead               & fr,
 Editor_Game_Base                & egbase,
 Widelands_Map_Map_Object_Loader & mol)
{
	const Uint16 packet_version = fr.Unsigned16();
	if (packet_version == CMD_ACT_VERSION) {
		// Read Base Commands
		GameLogicCommand::Read(fr, egbase, mol);

		// Serial
		if (const Uint32 fileserial = fr.Unsigned32()) {
			assert(mol.is_object_known(fileserial)); //  FIXME NEVER USE assert TO VALIDATE INPUT!!!
			obj_serial = mol.get_object_by_file_index(fileserial)->get_serial();
		} else
			obj_serial = 0;

		// arg
		arg = fr.Unsigned32();
	} else
		throw wexception("Unknown version in Cmd_Act::Read: %u", packet_version);
}
void Cmd_Act::Write
(FileWrite             & fw,
 Editor_Game_Base               & egbase,
 Widelands_Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(CMD_ACT_VERSION);

	// Write base classes
	GameLogicCommand::Write(fw, egbase, mos);

	// Now serial
	if (const Map_Object * const obj = egbase.objects().get_object(obj_serial)) {
		// Object might have dissappeared
		assert(mos.is_object_known(obj));
		fw.Unsigned32(mos.get_object_file_index(obj));
	} else
		fw.Unsigned32(0);

	// And arg
	fw.Unsigned32(arg);
}


/** Object_Manager::~Object_Manager()
 *
 * Remove all map objects
 */
Object_Manager::~Object_Manager()
{
	// better not throw an exception in a destructor...
	if (!m_objects.empty())
		log("Object_Manager: ouch! remaining objects\n");

	log("lastserial: %i\n", m_lastserial);
}

/*
===============
Object_Manager::cleanup

Clear all objects
===============
*/
void Object_Manager::cleanup(Editor_Game_Base *g)
{
	while (!m_objects.empty()) {
		objmap_t::iterator it = m_objects.begin();
		it->second->remove(g);
	}
	m_lastserial = 0;
}

/*
===============
Object_Manager::insert

Insert the given Map_Object into the object manager
===============
*/
void Object_Manager::insert(Map_Object *obj)
{
	m_lastserial++;
	assert(m_lastserial);
	obj->m_serial = m_lastserial;
	m_objects[m_lastserial] = obj;
}

/*
===============
Object_Manager::remove

Remove the Map_Object from the manager
===============
*/
void Object_Manager::remove(Map_Object *obj)
{
	m_objects.erase(obj->m_serial);
}

/*
===============
Object_Ptr::get
===============
*/
Map_Object * Object_Ptr::get(const Editor_Game_Base * const game)
{
	if (!m_serial) return 0;
	Map_Object* obj = game->objects().get_object(m_serial);
	if (!obj)
		m_serial = 0;
	return obj;
}

/*
===============
Object_Ptr::get
===============
*/
const Map_Object * Object_Ptr::get(const Editor_Game_Base * const game) const
{return m_serial ? game->objects().get_object(m_serial) : 0;}




/*
==============================================================================

Map_Object_Descr IMPLEMENTATION

==============================================================================
*/

uint Map_Object_Descr::s_dyn_attribhigh = Map_Object::HIGHEST_FIXED_ATTRIBUTE;
Map_Object_Descr::AttribMap Map_Object_Descr::s_dyn_attribs;

/*
 * Add this animation for this map object under this name
 */
bool Map_Object_Descr::is_animation_known(const char* name) {
	std::map<std::string, uint>::iterator i=m_anims.begin();
	while (i!=m_anims.end()) {
		if (i->first==name)
			return true;
		++i;
	}
	return false;
}

void Map_Object_Descr::add_animation(const char* name, uint anim) {
	std::string use_name=name;
	std::map<std::string, uint>::iterator i=m_anims.begin();

	while (i!=m_anims.end()) {
		assert(i->first!=name);
		++i;
	}
	m_anims.insert(std::pair<std::string, uint>(use_name, anim));
}

std::string Map_Object_Descr::get_animation_name(uint anim) const {
	for
		(std::map<std::string, uint>::const_iterator it = m_anims.begin();
		 it != m_anims.end();
		 ++it)
	{
		if (it->second == anim)
			return it->first;
	}

	// Never here
	assert(0);
	return "";
}


/*
===============
Map_Object_Descr::has_attribute

Search for the attribute in the attribute list
===============
*/
bool Map_Object_Descr::has_attribute(uint attr) const throw () {
	for (uint i = 0; i < m_attributes.size(); i++) {
		if (m_attributes[i] == attr)
			return true;
	}

	return false;
}


/*
===============
Map_Object_Descr::add_attribute

Add an attribute to the attribute list if it's not already there
===============
*/
void Map_Object_Descr::add_attribute(uint attr)
{
	if (!has_attribute(attr))
		m_attributes.push_back(attr);
}


/*
===============
Map_Object_Descr::get_attribute_id [static]

Lookup an attribute by name. If the attribute name hasn't been encountered
before, we add it to the map.
===============
*/
uint Map_Object_Descr::get_attribute_id(std::string name)
{
	AttribMap::iterator it = s_dyn_attribs.find(name);

	if (it != s_dyn_attribs.end())
		return it->second;

	if (name == "warehouse")
		return Map_Object::WAREHOUSE;
	else if (name == "worker")
		return Map_Object::WORKER;
	else if (name == "resi")
		return Map_Object::RESI;

	s_dyn_attribhigh++;
	s_dyn_attribs[name] = s_dyn_attribhigh;

	assert(s_dyn_attribhigh != 0); // wrap around seems *highly* unlikely ;)

	return s_dyn_attribhigh;
}


/*
==============================================================================

Map_Object IMPLEMENTATION

==============================================================================
*/

/*
===============
Map_Object::Map_Object

Zero-initialize a map object
===============
*/
Map_Object::Map_Object(const Map_Object_Descr * const the_descr) :
m_descr(the_descr), m_serial(0), m_logsink(0)
{}


/*
===============
Map_Object::remove

Call this function if you want to remove the object immediately, without
any effects.
===============
*/
void Map_Object::remove(Editor_Game_Base *g)
{
	cleanup(g);
	delete this;
}

/*
===============
Map_Object::destroy [virtual]

Destroy the object immediately. Unlike remove(), special actions may be
performed:
- create a decaying skeleton (humans)
- create a burning fire (buildings)
...
===============
*/
void Map_Object::destroy(Editor_Game_Base* g)
{
	remove(g);
}

/*
===============
Map_Object::schedule_destroy

Schedule the object for immediate destruction.
This can be used to safely destroy the object from within an act function.
===============
*/
void Map_Object::schedule_destroy(Game *g)
{
	g->get_cmdqueue()->enqueue (new Cmd_Destroy_Map_Object(g->get_gametime(), this));
}

/*
===============
Map_Object::init

Make sure you call this from derived classes!

Initialize the object by adding it to the object manager.
===============
*/
void Map_Object::init(Editor_Game_Base* g)
{
	g->objects().insert(this);
}

/*
===============
Map_Object::cleanup

Make sure you call this from derived classes!
===============
*/
void Map_Object::cleanup(Editor_Game_Base *g)
{
	g->objects().remove(this);
}


/*
===============
Map_Object::schedule_act

Queue a CMD_ACT tdelta milliseconds from now, using the given data.
Returns the absolute gametime at which the CMD_ACT will occur.
===============
*/
uint Map_Object::schedule_act(Game* g, uint tdelta, uint data)
{
	if (tdelta < Editor_Game_Base::Forever()) {
		uint time = g->get_gametime() + tdelta;

		g->get_cmdqueue()->enqueue (new Cmd_Act(time, this, data));

		return time;
	} else
		return Editor_Game_Base::Never();
}


/*
===============
Map_Object::act

Called when a CMD_ACT triggers.
===============
*/
void Map_Object::act(Game *, uint)
{
}


/*
===============
Map_Object::set_logsink

Set the logsink. This should only be used by the debugging facilities.
===============
*/
void Map_Object::set_logsink(LogSink* sink)
{
	m_logsink = sink;
}

/*
 * General infos, nothing todo for a no object
 */
void Map_Object::log_general_info(Editor_Game_Base*) {
}

/*
===============
Map_Object::molog

Prints a log message prepended by the object's serial number.
===============
*/
void Map_Object::molog(const char* fmt, ...) const
{
	va_list va;
	char buffer[2048];

	va_start(va, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, va);
	va_end(va);

	if (m_logsink) m_logsink->log(buffer);

	log("MO(%u): %s", m_serial, buffer);
}
