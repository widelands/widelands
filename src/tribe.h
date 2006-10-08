/*
 * Copyright (C) 2002, 2006 by the Widelands Development Team
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

#include <map>
#include <vector>
#include "animation.h"
#include "bob.h"
#include "building.h"
#include "descr_maintainer.h"
#include "immovable.h"
#include "ware.h"
#include "worker.h"

class Building_Descr;
class Editor_Game_Base;
class Item_Ware_Descr;
class Resource_Descr;
class Worker_Descr;
class Warehouse;

/*
Tribes
------

Every player chooses a tribe. A tribe has distinct properties such as the
buildings it can build and the associated graphics.
Two players can choose the same tribe.
*/
class Tribe_Descr {
   public:
      enum {
         OK = 0,
         ERR_WRONGVERSION
      };

      Tribe_Descr(const char* name);

      // Static function to check for tribes
      static bool exists_tribe(std::string);
      static void get_all_tribes(std::vector<std::string>*);


      inline const char *get_name() const { return m_name; }

		inline int get_nrworkers() { return m_workers.get_nitems(); }
		Worker_Descr * get_worker_descr(const uint idx) const
		{return m_workers.get(idx);}
		int get_worker_index(const char * const name) const
		{return m_workers.get_index(name);}
		inline int get_nrwares() { return m_wares.get_nitems(); }
		int get_safe_ware_index(const char * const name) const;
		int get_ware_index(const char * const name) const
		{return m_wares.get_index(name);}
      inline Item_Ware_Descr* get_ware_descr(uint idx) { return m_wares.get(idx); }
 Item_Ware_Descr & get_ware_descr(const uint idx) const
	{return *m_wares.get(idx);}
		int get_safe_worker_index(const char * const name) const;
		int get_nrbuildings() const { return m_buildings.get_nitems(); }
		int get_safe_building_index(const char *name);
	int get_building_index(const char * const name) const
	{return m_buildings.get_index(name);}
		Building_Descr *get_building_descr(uint idx) const { return m_buildings.get(idx); }
      inline int get_immovable_index(const char* l) { return m_immovables.get_index(l); }
      inline int get_nr_immovables(void) { return m_immovables.get_nitems(); }
		inline Immovable_Descr* get_immovable_descr(int index) { return m_immovables.get(index); }
      inline int get_bob(const char* l) { return m_bobs.get_index(l); }
		inline Bob_Descr* get_bob_descr(ushort index) { return m_bobs.get(index); }
      inline int get_nr_bobs(void) { return m_bobs.get_nitems(); }

      inline uint get_frontier_anim(void) { return m_anim_frontier; }
		inline uint get_flag_anim(void) { return m_anim_flag; }

      int get_resource_indicator(Resource_Descr* res, uint amount);

		void postload(Editor_Game_Base*);
		void load_graphics();

	void load_warehouse_with_start_wares
		(Editor_Game_Base &, Warehouse &) const;

   private:
      char m_name[30];

      uint m_anim_frontier;
		uint m_anim_flag;

      Descr_Maintainer<Worker_Descr> m_workers;
      Descr_Maintainer<Building_Descr> m_buildings;
      Descr_Maintainer<Item_Ware_Descr> m_wares;
      Descr_Maintainer<Immovable_Descr> m_immovables;  // The player immovables
      Descr_Maintainer<Bob_Descr> m_bobs;  // The player critters

	typedef std::map<std::string, int> starting_resources_map;
	starting_resources_map m_startwares;
	starting_resources_map m_startworkers;
	starting_resources_map m_startsoldiers;

      // Parsing the tribe
		EncodeData	m_default_encdata;

		void parse_root_conf(const char *directory);
		void parse_buildings(const char *directory);
		void parse_workers(const char *directory);
		void parse_wares(const char *wares);
		void parse_bobs(const char *directory);


};

#endif //__TRIBE_H
