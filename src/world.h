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
#ifndef __WORLD_H
#define __WORLD_H

#include "graphic.h"
#include "descr_maintainer.h"
#include "bob.h"
#include "worlddata.h"
#include "immovable.h"
#include "ware.h"

class Section;
class Game;

struct World_Descr_Header {
   char name[30];
   char author[30];
   char descr[1024];
};

class Resource_Descr {
   friend class Descr_Maintainer<Resource_Descr>;

   public:
      Resource_Descr(void) { }
      ~Resource_Descr(void) { }

      void parse(Section *s);
		
   private:
      char name[30];
      uchar minh, maxh, importance;
};

class Terrain_Descr {
   friend class Descr_Maintainer<Terrain_Descr>;
   friend class World;
   
   public:
      Terrain_Descr(const char* directory, Section* s);
      ~Terrain_Descr(void);

		void load_graphics();
		
      inline uint get_texture(void) { return m_texture; }
      inline uchar get_is(void) { return m_is; }

   private:
      char		m_name[30];
		char*		m_picnametempl;
		uint		m_frametime;
      uchar		m_is;

		uint		m_texture; // renderer's texture
};

/** class World
  *
  * This class provides information on a worldtype usable to create a map;
  * it can read a world file.
  */
class World
{
	friend class Game;

   public:
      enum {
         OK = 0,
         ERR_WRONGVERSION
      };

      World(const char* name);
		~World();
      
		void postload(Game*);
		void load_graphics();
		
      inline const char* get_name(void) { return hd.name; }
      inline const char* get_author(void) { return hd.author; }
      inline const char* get_descr(void) { return hd.descr; }
      
      inline Terrain_Descr* get_terrain(uint i) { assert(i<ters.get_nitems()); return ters.get(i); }
      inline int get_bob(const char* l) { return bobs.get_index(l); }
		inline Bob_Descr* get_bob_descr(ushort index) { return bobs.get(index); }
      inline int get_immovable_index(const char* l) { return immovables.get_index(l); }
		inline Immovable_Descr* get_immovable_descr(int index) { return immovables.get(index); }
		
		void parse_wares(Descr_Maintainer<Ware_Descr> *wares);
		
   private:
      World_Descr_Header hd;
      Descr_Maintainer<Bob_Descr> bobs;
		Descr_Maintainer<Immovable_Descr> immovables;
      Descr_Maintainer<Resource_Descr> res;
      Descr_Maintainer<Terrain_Descr> ters;

      // Functions
      void parse_root_conf(const char *directory, const char *name);
      void parse_resources(const char *directory);
      void parse_terrains(const char *directory);
      void parse_bobs(const char *directory);
};

#endif

