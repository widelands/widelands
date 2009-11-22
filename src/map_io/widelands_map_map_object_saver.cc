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

#include "logic/areawatcher.h"
#include "logic/battle.h"
#include "logic/bob.h"
#include "logic/building.h"
#include "container_iterate.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "economy/ware_instance.h"
#include "logic/item_ware_descr.h"
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
Map_Map_Object_Saver::MapObjectRec &
Map_Map_Object_Saver::get_object_record(Map_Object const & obj)
{
	Map_Object_Map::iterator it = m_objects.find(&obj);

	if (it != m_objects.end())
		return it->second;

	MapObjectRec rec;
#ifndef NDEBUG
	rec.description  = obj.type_name();
	rec.description += " (";
	rec.description += obj.serial();
	rec.description += ')';
#endif
	rec.fileserial = ++m_lastserial;
	rec.registered = false;
	rec.saved = false;
	return
		m_objects.insert(std::pair<Map_Object const *, MapObjectRec>(&obj, rec))
		.first->second;
}


/**
 * Returns true if this object has already been registered.
 * \deprecated since get_object_file_index supports unregistered objects now
 */
bool Map_Map_Object_Saver::is_object_known(Map_Object const & obj) const
{
	Map_Object_Map::const_iterator it = m_objects.find(&obj);

	if (it == m_objects.end())
		return false;

	return it->second.registered;
}

bool Map_Map_Object_Saver::is_object_saved(Map_Object const & obj) throw ()
{
	return get_object_record(obj).saved;
}


/*
 * Registers this object as a new one
 */
Serial Map_Map_Object_Saver::register_object(Map_Object const & obj) {
	MapObjectRec & rec = get_object_record(obj);

	assert(!rec.registered);

	if      (dynamic_cast<AreaWatcher  const *>(&obj));
	else if (dynamic_cast<Flag         const *>(&obj)) ++m_nr_flags;
	else if (dynamic_cast<Road         const *>(&obj)) ++m_nr_roads;
	else if (dynamic_cast<Building     const *>(&obj)) ++m_nr_buildings;
	else if (dynamic_cast<Immovable    const *>(&obj)) ++m_nr_immovables;
	else if (dynamic_cast<WareInstance const *>(&obj)) ++m_nr_wares;
	else if (dynamic_cast<Bob          const *>(&obj)) ++m_nr_bobs;
	else if (dynamic_cast<Battle       const *>(&obj)) ++m_nr_battles;
	else
		throw wexception("Map_Map_Object_Saver: Unknown MapObject type");

	rec.registered = true;
	return rec.fileserial;
}

/*
 * Returns the file index for this map object. This is used on load
 * to regenerate the dependencies between the objects.
 */
uint32_t Map_Map_Object_Saver::get_object_file_index(Map_Object const & obj)
{
	return get_object_record(obj).fileserial;
}

/*
 * mark this object as saved
 */
void Map_Map_Object_Saver::mark_object_as_saved(Map_Object const & obj) {
	MapObjectRec & rec = get_object_record(obj);
	assert(rec.registered);
	rec.saved = true;
}

#ifndef NDEBUG
/*
 * Return the number of unsaved objects
 */
void Map_Map_Object_Saver::detect_unsaved_objects() const {
	container_iterate_const(Map_Object_Map, m_objects, i) {
		if (!i.current->second.saved) {
			throw wexception
				("%s has not been saved", i.current->second.description.c_str());
		}
	}
}
#endif

}
