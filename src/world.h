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

#include "bob.h"
#include "descr_maintainer.h"
#include "immovable.h"
#include "worlddata.h"

class Section;
class Editor_Game_Base;

#define WORLD_NAME_LEN 30
#define WORLD_AUTHOR_LEN 30
#define WORLD_DESCR_LEN 1024
struct World_Descr_Header {
   char name[WORLD_NAME_LEN];
   char author[WORLD_AUTHOR_LEN];
   char descr[WORLD_DESCR_LEN];
};

class Resource_Descr {
public:
	Resource_Descr() { }
	~Resource_Descr() { }

	void parse(Section* s);

	std::string get_name() const { return m_name; }
	std::string get_indicator(uint amount) const;

private:
	struct Indicator {
		std::string		bobname;
		int				upperlimit;
	};

	std::string					m_name;
	std::vector<Indicator>	m_indicators;
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
      inline const char* get_name() { return m_name; }

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

      // TODO: Resource names should not be 
      // hardcoded!!!!!
		enum Resource {
			Resource_None = 0,
			Resource_Coal = 1,
			Resource_Iron = 2,
			Resource_Gold = 3,

			Num_Resources = 4
		};

      World(const char* name);
		~World();
      
      // Check if a world really exists
      static bool exists_world(std::string);
      static void get_all_worlds(std::vector<std::string>*);

		void postload(Editor_Game_Base*);
		void load_graphics();

      inline const char* get_name(void) { return hd.name; }
      inline const char* get_author(void) { return hd.author; }
      inline const char* get_descr(void) { return hd.descr; }

      inline Terrain_Descr* get_terrain(uint i) { assert(i<ters.get_nitems()); return ters.get(i); }
      inline Terrain_Descr* get_terrain(char* str ) { int i=ters.get_index(str); if(i==-1) return 0; return ters.get(i); }
      inline int get_nr_terrains(void) { return ters.get_nitems(); }
      inline int get_bob(const char* l) { return bobs.get_index(l); }
		inline Bob_Descr* get_bob_descr(ushort index) { return bobs.get(index); }
      inline int get_nr_bobs(void) { return bobs.get_nitems(); }
      inline int get_immovable_index(const char* l) { return immovables.get_index(l); }
      inline int get_nr_immovables(void) { return immovables.get_nitems(); }
		inline Immovable_Descr* get_immovable_descr(int index) { return immovables.get(index); }

		const Resource_Descr* get_resource(Resource res) const
		{ assert(res < Num_Resources); return &m_resources[res]; }

   private:
		std::string				m_basedir;	// base directory, where the main conf file resides
      World_Descr_Header	hd;

      Descr_Maintainer<Bob_Descr> bobs;
		Descr_Maintainer<Immovable_Descr> immovables;
      Descr_Maintainer<Terrain_Descr> ters;

		Resource_Descr	m_resources[Num_Resources];

      // Functions
      void parse_root_conf(const char *name);
      void parse_resources();
      void parse_terrains();
      void parse_bobs();
};

#endif

