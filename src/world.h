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

class Section;

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
      Terrain_Descr(void) { ntex=0; tex=0; curtex=0; m_frametime = FRAME_LENGTH; }
      ~Terrain_Descr(void) { 
         if(ntex) {
            uint i;
            for(i=0; i<ntex; i++) 
               delete tex[i];
            free(tex);
         }
      }

      inline Pic* get_texture(void) { return tex[curtex]; }
      inline uchar get_is(void) { return is; }

      void read(const char *directory, Section* s);

      
   private:
      inline void animate(uint time) {
			curtex = (time / m_frametime) % ntex;
      }

      char name[30];
		uint m_frametime;
      uchar is;
      Pic** tex;
      ushort ntex;
      ushort curtex;
};

/** class World
  *
  * This class provides information on a worldtype usable to create a map;
  * it can read a world file.
  */
class World
{
   public:
      enum {
         OK = 0,
         ERR_WRONGVERSION
      };

      World(void);
		~World();
      void load_world(const char*);
      
      inline const char* get_name(void) { return hd.name; }
      inline const char* get_author(void) { return hd.author; }
      inline const char* get_descr(void) { return hd.descr; }
      
      inline Terrain_Descr* get_terrain(uint i) { assert(i<ters.get_nitems()); return ters.get(i); }
      inline int get_bob(const char* l) { return bobs.get_index(l); }
      inline Logic_Bob_Descr* get_bob_descr(ushort index) { return bobs.get(index); }

      inline void animate(int time) {
         uint i;
         for(i=0; i<ters.get_nitems(); i++)
				ters.get(i)->animate(time);
      }
 
   private:
      Descr_Maintainer<Logic_Bob_Descr> bobs;
      Descr_Maintainer<Resource_Descr> res;
      Descr_Maintainer<Terrain_Descr> ters;
      World_Descr_Header hd;

      // Functions
      void parse_root_conf(const char *directory, const char *name);
      void parse_resources(const char *directory);
      void parse_terrains(const char *directory);
      void parse_bobs(const char *directory);
};

#endif

