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

#include "editor_increase_height_tool.h"
#include "editor_decrease_height_tool.h"
#include "editor_set_height_tool.h"
#include "editor_noise_height_tool.h"
#include "map.h"
#include "field.h"
#include "editorinteractive.h"

/*
=============================

class Editor_Noise_Height_Tool

=============================
*/

/*
===========
Editor_Noise_Height_Tool::handle_click_impl()

sets the height of the current to a random value,
this decreases the height of the surrounding fields also
if this is needed.
===========
*/
int Editor_Noise_Height_Tool::handle_click_impl(FCoords& fc, Map* map, Editor_Interactive* parent) {
   MapRegion mrc(map, fc, parent->get_fieldsel_radius());
   Coords c;

   int i, max;
   max=0;
   while(mrc.next(&c)) {
      int j=m_lower_value+(int) ((double)(m_upper_value-m_lower_value)*rand()/(RAND_MAX+1.0));
      i=map->set_field_height(c, j);
      if(i>max) max=i;
   }
   return parent->get_fieldsel_radius()+max;
}
