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

#include "widelands.h"
#include "instances.h"
#include "game.h"
#include "map.h"
#include "player.h"

#include <stdarg.h>


/** Object_Manager::~Object_Manager()
 *
 * Remove all map objects
 */
Object_Manager::~Object_Manager(void)
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
	while(!m_objects.empty()) {
		objmap_t::iterator it = m_objects.begin();
      it->second->remove(g);
	}
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
Map_Object* Object_Ptr::get(Editor_Game_Base* game)
{
	if (!m_serial) return 0;
	Map_Object* obj = game->get_objects()->get_object(m_serial);
	if (!obj)
		m_serial = 0;
	return obj;
}


/*
==============================================================================

Map_Object_Descr IMPLEMENTATION

==============================================================================
*/

uint Map_Object_Descr::s_dyn_attribhigh = Map_Object::HIGHEST_FIXED_ATTRIBUTE;
Map_Object_Descr::AttribMap Map_Object_Descr::s_dyn_attribs;


/*
===============
Map_Object_Descr::has_attribute

Search for the attribute in the attribute list
===============
*/
bool Map_Object_Descr::has_attribute(uint attr)
{
	for(uint i = 0; i < m_attributes.size(); i++) {
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
Map_Object::Map_Object(Map_Object_Descr* descr)
{
	m_descr = descr;
	m_serial = 0;
}

/*
===============
Map_Object::~Map_Object

Cleanup an object.
===============
*/
Map_Object::~Map_Object()
{
}


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
void Map_Object::destroy(Editor_Game_Base *g)
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
	g->get_cmdqueue()->queue(g->get_gametime(), SENDER_MAPOBJECT, CMD_DESTROY, m_serial);
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
	g->get_objects()->insert(this);
}

/*
===============
Map_Object::cleanup

Make sure you call this from derived classes!
===============
*/
void Map_Object::cleanup(Editor_Game_Base *g)
{
	g->get_objects()->remove(this);
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
	uint time = g->get_gametime() + tdelta;

	g->get_cmdqueue()->queue(time, SENDER_MAPOBJECT, CMD_ACT, m_serial, data, 0);

	return time;
}


/*
===============
Map_Object::act

Called when a CMD_ACT triggers.
===============
*/
void Map_Object::act(Game* g, uint data)
{
}


/*
===============
Map_Object::molog

Prints a log message prepended by the object's serial number.
===============
*/
void Map_Object::molog(const char* fmt, ...)
{
	va_list va;
	char buf[2048];

	va_start(va, fmt);
	vsnprintf(buf, sizeof(buf), fmt, va);
	va_end(va);

	log("MO(%u): %s", m_serial, buf);
}
