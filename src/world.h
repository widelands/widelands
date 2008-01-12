/*
 * Copyright (C) 2002, 2006-2008 by the Widelands Development Team
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

#include "widelands.h"

class Section;

namespace Widelands {

class Editor_Game_Base;

#define WORLD_NAME_LEN 30
#define WORLD_AUTHOR_LEN 30
#define WORLD_DESCR_LEN 1024

struct World_Descr_Header {
   char name[WORLD_NAME_LEN];
   char author[WORLD_AUTHOR_LEN];
   char descr[WORLD_DESCR_LEN];
};

struct Resource_Descr {
	typedef Resource_Index Index;
	Resource_Descr() {}
	~Resource_Descr() {}

	void parse(Section* s, std::string);

	const std::string & name     () const throw () {return m_name;}
	__attribute__ ((deprecated)) const char * get_name() const throw () {return m_name.c_str();}
	const std::string & descrname() const throw () {return m_descrname;}
	__attribute__ ((deprecated)) const char * get_descrname() const throw () {return m_descrname.c_str();}

	bool is_detectable() const throw () {return m_is_detectable;}
	int32_t get_max_amount() const throw () {return m_max_amount;}

	const std::string & get_editor_pic(const uint32_t amount) const;

private:
	struct Indicator {
		std::string bobname;
		int32_t         upperlimit;
	};
	struct Editor_Pic {
      std::string    picname;
      int32_t            upperlimit;
	};

   bool                    m_is_detectable;
   int32_t                     m_max_amount;
	std::string             m_name;
	std::string             m_descrname;
	std::vector<Editor_Pic> m_editor_pics;
};

struct Terrain_Descr {
   friend class Descr_Maintainer<Terrain_Descr>;
   friend class World;

	typedef Terrain_Index Index;
      Terrain_Descr(const char* directory, Section* s, Descr_Maintainer<Resource_Descr>*);
      ~Terrain_Descr();

		void load_graphics();

	uint32_t         get_texture() const throw () {return m_texture;}
	uint8_t        get_is     () const throw () {return m_is;}
	const std::string & name() const throw () {return m_name;}
	__attribute__ ((deprecated)) const char * get_name   () const throw () {return m_name.c_str();}
	int32_t resource_value(const Resource_Index resource) const throw () {
		return
			resource == get_default_resources() or is_resource_valid(resource) ?
			(get_is() & TERRAIN_UNPASSABLE ? 8 : 1) : -1;
	}

	bool is_resource_valid(const int32_t res) const throw () {
		for (int32_t i = 0; i < m_nr_valid_resources; ++i)
            if (m_valid_resources[i]==res) return true;
         return false;
		}
	int8_t get_default_resources() const
		throw ()
	{return m_default_resources;}
	int32_t get_default_resources_amount() const throw () {return m_default_amount;}

private:
	const std::string m_name;
	char  * m_picnametempl;
	uint32_t    m_frametime;
	uint8_t   m_is;

      uint8_t*   m_valid_resources;
      uint8_t    m_nr_valid_resources;
      int8_t     m_default_resources;
      int32_t      m_default_amount;
	uint32_t    m_texture; //  renderer's texture
};

/** class World
  *
  * This class provides information on a worldtype usable to create a map;
  * it can read a world file.
  */
struct World {
	friend class Game;

	enum {
         OK = 0,
         ERR_WRONGVERSION
	};

      World(const std::string name);

      // Check if a world really exists
      static bool exists_world(std::string);
      static void get_all_worlds(std::vector<std::string>*);

		void postload(Editor_Game_Base*);
		void load_graphics();

	const char * get_name  () const throw () {return hd.name;}
	const char * get_author() const throw () {return hd.author;}
	const char * get_descr () const throw () {return hd.descr;}

	Terrain_Descr ::Index index_of_terrain   (const char * const name) const
	{return ters       .get_index(name);}
	Terrain_Descr & terrain_descr    (const Terrain_Descr  ::Index i) const
		throw ()
	{return *ters.get(i);}
	const Terrain_Descr & get_ter(const Terrain_Descr::Index i) const
	{assert(i < ters.get_nitems()); return *ters.get(i);}
	const Terrain_Descr * get_ter(const char * const name) const
	{const int32_t i = ters.get_index(name); return i != -1 ? ters.get(i) : 0;}
      int32_t get_nr_terrains() const {return ters.get_nitems();}
      int32_t get_bob(const char* l) {return bobs.get_index(l);}
      Bob::Descr* get_bob_descr(uint16_t index) const {return bobs.get(index);}
      int32_t get_nr_bobs() const {return bobs.get_nitems();}
      int32_t get_immovable_index(const char* l)const {return immovables.get_index(l);}
      int32_t get_nr_immovables() const {return immovables.get_nitems();}
		Immovable_Descr* get_immovable_descr(int32_t index) const {return immovables.get(index);}

	int32_t get_resource(const char * const name) const {return m_resources.get_index(name);}
	Resource_Descr * get_resource(const Resource_Index res) const throw ()
		{assert(res < m_resources.get_nitems()); return m_resources.get(res);}
      int32_t get_nr_resources() const {return m_resources.get_nitems();}

private:
	std::string m_basedir; //  base directory, where the main conf file resides
	World_Descr_Header                hd;

      Descr_Maintainer<Bob::Descr> bobs;
		Descr_Maintainer<Immovable_Descr> immovables;
      Descr_Maintainer<Terrain_Descr> ters;
	Descr_Maintainer<Resource_Descr>  m_resources;

      // Functions
      void parse_root_conf(const char *name);
      void parse_resources();
      void parse_terrains();
      void parse_bobs();
};

};

#endif
