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
int Set_Terrain::set_terrain(const Coords* coordinates, Field* field, Map* map, Editor_Interactive* parent, bool right, bool down) {
   if(!get_nr_enabled()) return parent->get_fieldsel_radius();
   MapRegion mrc(map, *coordinates, parent->get_fieldsel_radius());
   Coords c;

   int i, j, max;
   max=0;
   while(mrc.next(&c)) {
      i=j=0;
      if(right)
         i=map->change_field_terrain(c,get_random_enabled(),false,true);
      if(down)
         j=map->change_field_terrain(c,get_random_enabled(),true,false);
      if(i>max) max=i;
      if(j>max) max=j;
   }
   return parent->get_fieldsel_radius()+max;
}


