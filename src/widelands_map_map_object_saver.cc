/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

Widelands_Map_Map_Object_Saver::Widelands_Map_Map_Object_Saver() :
m_nr_roads     (0),
m_nr_flags     (0),
m_nr_buildings (0),
m_nr_bobs      (0),
m_nr_wares     (0),
m_nr_immovables(0),
m_lastserial   (0)
{}


/*
 * Returns true if this object has already been inserted
 */
bool Widelands_Map_Map_Object_Saver::is_object_known
(const Map_Object * const obj) const
{return m_objects.find(obj) != m_objects.end();}

/*
 * Registers this object as a new one
 */
uint Widelands_Map_Map_Object_Saver::register_object
(const Map_Object * const obj)
{
	assert(!is_object_known(obj));

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
		throw wexception
			("Widelands_Map_Map_Object_Saver: Unknown MapObject type : %i",
			 obj->get_type());
	}

	uint fileserial = ++m_lastserial;

	m_objects.insert(std::pair<const Map_Object*, uint>(obj, fileserial));
	m_saved_obj[obj] = false;

	return fileserial;
}

/*
 * Returns the file index for this map object. This is used on load
 * to regenerate the depencies between the objects
 */
uint Widelands_Map_Map_Object_Saver::get_object_file_index
(const Map_Object * const obj)
{
	// This check should rather be an assert(), but we get more information
	// from a throw and time's not soo much an issue here
	if (!is_object_known(obj))
		throw wexception
				("Widelands_Map_Map_Object_Saver::get_object_file_index(): "
				 "Map Object %p (%i) is not known!\n",
				 obj, obj->get_serial());

	return m_objects[obj];
}

/*
 * mark this object as saved
 */
void Widelands_Map_Map_Object_Saver::mark_object_as_saved
		(const Map_Object * const obj)
{
	m_saved_obj[obj] = true;
}

/*
 * Return the number of unsaved objects
 */
uint Widelands_Map_Map_Object_Saver::get_nr_unsaved_objects() const throw ()
{
	std::map<const Map_Object *, bool>::const_iterator i = m_saved_obj.begin();
	uint retval = 0;

	while (i!=m_saved_obj.end()) {
		if (!i->second) retval++;
		++i;
	}

	return retval;
}
