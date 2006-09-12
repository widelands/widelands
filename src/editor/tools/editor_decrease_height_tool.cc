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
#include "editor_set_height_tool.h"
#include "editor_decrease_height_tool.h"
#include "map.h"
#include "field.h"
#include "editorinteractive.h"

/*
=============================

class Editor_Decrease_Height_Tool

=============================
*/

/*
===========
Editor_Decrease_Height_Tool::handle_click_impl()

decrease the height of the current field by one,
this decreases the height of the surrounding fields also
if this is needed.
===========
*/
int Editor_Decrease_Height_Tool::handle_click_impl(FCoords& fc, Map* map, Editor_Interactive* parent) {
   MapRegion mrc(map, fc, parent->get_fieldsel_radius());
   Coords c;

   int max,i;
   max=0;
   while(mrc.next(&c)) {
      i=map->change_field_height(c, -m_changed_by);
      if(i>max) max=i;
   }
   return parent->get_fieldsel_radius()+max;
}
