/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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
typedef std::map<Map_Object*, uint> Map_Object_Map;

class Widelands_Map_Map_Object_Saver {
   public:
      Widelands_Map_Map_Object_Saver(void);
      ~Widelands_Map_Map_Object_Saver(void);

      bool is_object_known(Map_Object*);
      uint register_object(Map_Object*);

      uint get_object_file_index(Map_Object*);

      void mark_object_as_saved(Map_Object*);

      // Information functions
      int get_nr_unsaved_objects(void);
      int get_nr_roads(void) { return m_nr_roads; }
      int get_nr_flags(void) { return m_nr_flags; }
      int get_nr_buildings(void) { return m_nr_buildings; }
      int get_nr_wares(void) { return m_nr_wares; }
      int get_nr_bobs(void) { return m_nr_bobs; }
      int get_nr_immovables(void) { return m_nr_immovables; }

      bool is_object_saved(Map_Object* obj) { return m_saved_obj[obj]; }

   private:
      std::map<Map_Object*, bool> m_saved_obj;
      Map_Object_Map m_objects;
      uint m_nr_roads, m_nr_flags, m_nr_buildings, m_nr_bobs,
           m_nr_wares, m_nr_immovables;
};



#endif
