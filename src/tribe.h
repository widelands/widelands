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

#ifndef __TRIBE_H
#define __TRIBE_H

#include "pic.h"
#include "bob.h"
#include "ware.h"
#include "worker.h"
#include "building.h"
#include "descr_maintainer.h"

/*
 * this class represents a tribe file as it is read out of 
 * a file + the distributing of informations and bobs of 
 * it
 */
class Tribe_Descr {
   public:
      enum {
         OK = 0,
         ERR_WRONGVERSION
      };
      
      Tribe_Descr(void);
      ~Tribe_Descr(void);

      void load(const char* name);

      inline Ware_Descr* get_ware_descr(uint idx) { return wares.get(idx); }
      inline Soldier_Descr* get_soldier_descr(uint idx) { return soldiers.get(idx); }
      inline Worker_Descr* get_worker_descr(uint idx) { return workers.get(idx); }
      inline Building_Descr* get_building_descr(uint idx) { return buildings.get(idx); }
      inline Animation* get_frontier_anim(void) { return &frontier; }

   private:
      Animation frontier;
      char name[30];
      Descr_Maintainer<Ware_Descr> wares;
      Descr_Maintainer<Soldier_Descr> soldiers;
      Descr_Maintainer<Worker_Descr> workers;
      Descr_Maintainer<Building_Descr> buildings;

      // Functions
      void parse_header(FileRead* f);
      void parse_regent(FileRead* f);
      void parse_bobs(FileRead* f);
      void parse_wares(FileRead* f);
      void parse_soldiers(FileRead* f);
      void parse_workers(FileRead* f);
      void parse_buildings(FileRead* f);
};

int read_anim(Animation* a, FileRead* f);

#endif //__TRIBE_H
