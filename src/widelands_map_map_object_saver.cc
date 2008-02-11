/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#include "widelands_map_map_object_saver.h"

#include "bob.h"
#include "instances.h"
#include "wexception.h"

namespace Widelands {

Map_Map_Object_Saver::Map_Map_Object_Saver() :
m_nr_roads     (0),
m_nr_flags     (0),
m_nr_buildings (0),
m_nr_bobs      (0),
m_nr_wares     (0),
m_nr_immovables(0),
m_lastserial   (0)
{}


/**
 * Return a pointer to the record for the given object.
 * Create a record if that hasn't been done yet.
 */
Map_Map_Object_Saver::MapObjectRec*
Map_Map_Object_Saver::get_object_record(const Map_Object* obj)
{
	Map_Object_Map::iterator it = m_objects.find(obj);

	if (it != m_objects.end())
		return &it->second;

	MapObjectRec rec;
	rec.fileserial = ++m_lastserial;
	rec.registered = false;
	rec.saved = false;
	return &m_objects.insert(std::pair<const Map_Object*, MapObjectRec>(obj, rec)).first->second;
}


/**
 * Returns true if this object has already been registered.
 * \deprecated since get_object_file_index supports unregistered objects now
 */
bool Map_Map_Object_Saver::is_object_known(const Map_Object * const obj) const
{
	Map_Object_Map::const_iterator it = m_objects.find(obj);

	if (it == m_objects.end())
		return false;

	return it->second.registered;
}

bool Map_Map_Object_Saver::is_object_saved(const Map_Object * const obj) throw ()
{
	MapObjectRec* rec = get_object_record(obj);
	return rec->saved;
}


/*
 * Registers this object as a new one
 */
uint32_t Map_Map_Object_Saver::register_object(const Map_Object * const obj) {
	MapObjectRec* rec = get_object_record(obj);

	assert(!rec->registered);

	switch (obj->get_type()) {
	case Map_Object::FLAG:             ++m_nr_flags;              break;
	case Map_Object::ROAD:             ++m_nr_roads;              break;
	case Map_Object::BUILDING:         ++m_nr_buildings;          break;
	case Map_Object::IMMOVABLE:        ++m_nr_immovables;         break;
	case Map_Object::WARE:             ++m_nr_wares;              break;
	case Map_Object::BOB:              ++m_nr_bobs;               break;
	case Map_Object::BATTLE:           ++m_nr_battles;            break;
	case Map_Object::ATTACKCONTROLLER: ++m_nr_attack_controllers; break;
	default:
		throw wexception("Map_Map_Object_Saver: Unknown MapObject type");
	}

	rec->registered = true;
	return rec->fileserial;
}

/*
 * Returns the file index for this map object. This is used on load
 * to regenerate the depencies between the objects
 */
uint32_t Map_Map_Object_Saver::get_object_file_index
(const Map_Object * const obj)
{
	MapObjectRec* rec = get_object_record(obj);
	return rec->fileserial;
}

/*
 * mark this object as saved
 */
void Map_Map_Object_Saver::mark_object_as_saved(Map_Object const * const obj) {
	MapObjectRec* rec = get_object_record(obj);
	assert(rec->registered);
	rec->saved = true;
}

/*
 * Return the number of unsaved objects
 */
uint32_t Map_Map_Object_Saver::get_nr_unsaved_objects() const throw () {
	uint retval = 0;

	for
		(Map_Object_Map::const_iterator it = m_objects.begin();
		 it != m_objects.end();
		 ++it)
	{
		if (!it->second.saved)
			retval++;
	}

	return retval;
}

};
