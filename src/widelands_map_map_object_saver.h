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

#ifndef __S__WIDELANDS_MAP_MAP_OBJECT_SAVER_H
#define __S__WIDELANDS_MAP_MAP_OBJECT_SAVER_H

#include <map>

class Map_Object;

/*
 * This class helps to
 *   - keep track of map objects on the map
 *   - translate Map_Object* Pointer into the index used in the saved file
 */
class Widelands_Map_Map_Object_Saver {
public:
	Widelands_Map_Map_Object_Saver();

	bool is_object_known(const Map_Object * const) const;
	uint register_object(const Map_Object * const);

	uint get_object_file_index(const Map_Object * const);

	void mark_object_as_saved(const Map_Object * const);

	// Information functions
	uint get_nr_unsaved_objects() const throw ();
	uint get_nr_roads          () const throw () {return m_nr_roads;}
	uint get_nr_flags          () const throw () {return m_nr_flags;}
	uint get_nr_buildings      () const throw () {return m_nr_buildings;}
	uint get_nr_wares          () const throw () {return m_nr_wares;}
	uint get_nr_bobs           () const throw () {return m_nr_bobs;}
	uint get_nr_immovables     () const throw () {return m_nr_immovables;}
	uint get_nr_battles        () const throw () {return m_nr_battles;}
	uint get_nr_attack_controllers() const throw () {return m_nr_attack_controllers;}

	bool is_object_saved(const Map_Object * const obj) throw ()
	{return m_saved_obj[obj];}

private:
	typedef std::map<const Map_Object *, const uint> Map_Object_Map;

	std::map<const Map_Object *, bool> m_saved_obj;
	Map_Object_Map m_objects;
	uint m_nr_roads;
	uint m_nr_flags;
	uint m_nr_buildings;
	uint m_nr_bobs;
	uint m_nr_wares;
	uint m_nr_immovables;
	uint m_nr_battles;
	uint m_nr_attack_controllers;
	uint m_lastserial;
};



#endif
