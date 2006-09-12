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

#ifndef __S__WIDELANDS_MAP_LOADER_H
#define __S__WIDELANDS_MAP_LOADER_H

#include <string>
#include "map_loader.h"
#include "map.h"

class FileSystem;
class Editor_Game_Base;
class Widelands_Map_Map_Object_Loader;

class Widelands_Map_Loader : public Map_Loader {
   public:
      Widelands_Map_Loader(FileSystem*, Map*);
      virtual ~Widelands_Map_Loader(void);

      virtual int get_type(void) { return Map_Loader::WLML; }

      virtual int preload_map(bool);
      virtual int load_map_complete(Editor_Game_Base*, bool);

      inline Widelands_Map_Map_Object_Loader* get_map_object_loader(void) { return m_mol; }

      static bool is_widelands_map( std::string filename ) {
         if( !strcasecmp( &filename.c_str()[filename.size()-4], WLMF_SUFFIX))
            return true;
         return false;
      }

   private:
      FileSystem* m_fs;
      std::string m_filename;
      Widelands_Map_Map_Object_Loader* m_mol;
};

#endif
