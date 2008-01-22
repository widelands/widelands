/*
 * Copyright (C) 2002, 2006-2007 by the Widelands Development Team
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

#ifndef __TRIBE_H
#define __TRIBE_H

#include "animation.h"
#include "bob.h"
#include "building.h"
#include "descr_maintainer.h"
#include "immovable.h"
#include "item_ware_descr.h"
#include "worker.h"

#include <map>
#include <vector>

namespace Widelands {

class Building_Descr;
class Editor_Game_Base;
class Item_Ware_Descr;
class Resource_Descr;
class Worker_Descr;
class Warehouse;
struct World;

/*
Tribes
------

Every player chooses a tribe. A tribe has distinct properties such as the
buildings it can build and the associated graphics.
Two players can choose the same tribe.
*/
struct Tribe_Descr {
	enum {
         OK = 0,
         ERR_WRONGVERSION
	};

	Tribe_Descr(const std::string & tribename, const World &);

      // Static function to check for tribes
	static bool exists_tribe(const std::string & name);
      static void get_all_tribenames(std::vector<std::string> &);


	const std::string & name() const throw () {return m_name;}
	__attribute__ ((deprecated)) const std::string & get_name() const throw () {return m_name;}
	const World & world() const throw () {return m_world;}

		int32_t get_nrworkers() const {return m_workers.get_nitems();}
	Worker_Descr * get_worker_descr(Ware_Index const index) const {
		return m_workers.get(index);
	}
	Ware_Index worker_index(char const * const workername) const {
		return m_workers.get_index(workername);
	}
		int32_t get_worker_index(const char * const workername) const
		{return m_workers.get_index(workername);}
		int32_t get_nrwares() const {return m_wares.get_nitems();}
	Ware_Index ware_index(char const * const warename) const {
		return m_wares.get_index(warename);
	}
		int32_t get_safe_ware_index(const char * const warename) const;
		int32_t get_ware_index(const char * const warename) const
		{return m_wares.get_index(warename);}
	Item_Ware_Descr* get_ware_descr(Ware_Index const index) const {
		return m_wares.get(index);
	}
		int32_t get_safe_worker_index(const char * const workername) const;
		int32_t get_nrbuildings() const {return m_buildings.get_nitems();}
		int32_t get_safe_building_index(const char *name) const;
	int32_t get_building_index(const char * const buildingname) const
	{return m_buildings.get_index(buildingname);}
		Building_Descr *get_building_descr(uint32_t idx) const {return m_buildings.get(idx);}
      int32_t get_immovable_index(const char* l) const {return m_immovables.get_index(l);}
      int32_t get_nr_immovables() {return m_immovables.get_nitems();}
		Immovable_Descr* get_immovable_descr(int32_t index) const {return m_immovables.get(index);}
      int32_t get_bob(const char* l) const {return m_bobs.get_index(l);}
		Bob::Descr* get_bob_descr(uint16_t index) const {return m_bobs.get(index);}
      int32_t get_nr_bobs() {return m_bobs.get_nitems();}

	uint32_t get_frontier_anim() const throw () {return m_anim_frontier;}
	uint32_t get_flag_anim    () const throw () {return m_anim_flag;}

	uint32_t get_bob_vision_range() const {return m_bob_vision_range;}

	uint32_t get_resource_indicator
		(const Resource_Descr * const res, const uint32_t amount) const;

		void postload(Editor_Game_Base*);
		void load_graphics();

	void load_warehouse_with_start_wares
		(Editor_Game_Base &, Warehouse &) const;


private:
	const std::string m_name;
	const World & m_world;
	uint32_t m_anim_frontier;
	uint32_t m_anim_flag;
	uint32_t m_bob_vision_range;

      Descr_Maintainer<Worker_Descr> m_workers;
      Descr_Maintainer<Building_Descr> m_buildings;
      Descr_Maintainer<Item_Ware_Descr> m_wares;
      Descr_Maintainer<Immovable_Descr> m_immovables;  // The player immovables
      Descr_Maintainer<Bob::Descr> m_bobs;  // The player critters

	typedef std::map<std::string, int32_t> starting_resources_map;
	starting_resources_map m_startwares;
	starting_resources_map m_startworkers;
	starting_resources_map m_startsoldiers;

      // Parsing the tribe
	EncodeData m_default_encdata;

		void parse_root_conf(const char *directory);
		void parse_buildings(const char *directory);
		void parse_workers(const char *directory);
		void parse_wares(const char *wares);
		void parse_bobs(const char *directory);
};

};

#endif //__TRIBE_H
