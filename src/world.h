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

      int read(FileRead* f) {
         memcpy(name, f->Data(30), 30);
         minh = f->Unsigned8();
         maxh = f->Unsigned8();
         importance = f->Unsigned8();
         return RET_OK;
      }
   private:
      char name[30];
      uchar minh, maxh, importance;
};

class Terrain_Descr {
   friend class Descr_Maintainer<Terrain_Descr>;
   friend class World;
   
   public:
      Terrain_Descr(void) { ntex=0; tex=0; curtex=0; }
      ~Terrain_Descr(void) { 
         if(ntex) {
            uint i;
            for(i=0; i<ntex; i++) 
               delete tex[i];
            delete[] tex;
         }
      }

      inline Pic* get_texture(void) { return tex[curtex]; }
      inline uchar get_is(void) { return is; }

      void read(FileRead* f);

      
   private:
      inline void animate(int time) {
			curtex = (time / FRAME_LENGTH) % ntex;
      }

      char name[30];
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
      inline ushort get_bob(const char* l) { return bobs.get_index(l); }
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
      void parse_header(FileRead* f);
      void parse_resources(FileRead* f);
      void parse_bobs(FileRead* f);
      void parse_terrains(FileRead* f);
};

#endif

