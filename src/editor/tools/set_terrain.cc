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

#include "set_terrain.h"
#include "field.h"
#include "map.h"
#include "editorinteractive.h"

/*
=============================

class Set_Terrain

=============================
*/

/*
===========
Set_Terrain::set_terrain()

This changes the terrain in the given area. It randoms through all selected
terrains.
===========
*/
int Set_Terrain::set_terrain(FCoords& fc, Map* map, Editor_Interactive* parent, bool right, bool down) {
   if(!get_nr_enabled()) return parent->get_fieldsel_radius();

   MapRegion mrc;
   Coords c;

   int i, j, max;
   max=0;
   if(right && down && parent->get_fieldsel_radius()) {
      // Beatify the clicked are so that it has a nice round border
      Coords curc;
      int radius=parent->get_fieldsel_radius()-1;

      // ok, click here
      mrc.init(map,fc,radius);
      while(mrc.next(&curc)) {
         i=map->change_field_terrain(curc,get_random_enabled(),true,true);
         if(i>max) max=i;
      }

      // click tln
      FCoords nb;
      map->get_tln(fc,&nb);
      mrc.init(map,nb,radius);
      while(mrc.next(&curc)) {
         i=map->change_field_terrain(curc,get_random_enabled(),true,true);
         if(i>max) max=i;
      }

      // click trn
      map->get_trn(fc,&nb);
      mrc.init(map,nb,radius);
      while(mrc.next(&curc)) {
         i=map->change_field_terrain(curc,get_random_enabled(),true,false);
      }


      // click ln
      map->get_ln(fc,&nb);
      mrc.init(map,nb,radius);
      while(mrc.next(&curc)) {
         i=map->change_field_terrain(curc,get_random_enabled(),false, true);
         if(i>max) max=i;
      }
   } else {
      mrc.init(map, fc, parent->get_fieldsel_radius());
      while(mrc.next(&c)) {
         i=j=0;
         if(right)
            i=map->change_field_terrain(c,get_random_enabled(),false,true);
         if(down)
            j=map->change_field_terrain(c,get_random_enabled(),true,false);
         if(i>max) max=i;
         if(j>max) max=j;
      }
   }
   return parent->get_fieldsel_radius()+max;
}


