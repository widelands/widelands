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

#include "editor_delete_bob_tool.h"
#include "field.h"
#include "map.h"
#include "editorinteractive.h"
#include "editor.h"
#include "bob.h"

/*
=================================================

class Editor_Delete_Bob_Tool

=================================================
*/

/*
===========
Editor_Delete_Bob_Tool::handle_click_impl()

deletes the bob at the given location
===========
*/
int Editor_Delete_Bob_Tool::handle_click_impl(FCoords& fc, Map* map, Editor_Interactive* parent) {
   MapRegion mrc(map, fc, parent->get_fieldsel_radius());
   Coords c;

   while(mrc.next(&c)) {
      Field *f = parent->get_map()->get_field(c);
      Bob* bob=f->get_first_bob();
      if (bob) {
         bob->remove(parent->get_editor());
      }
   }
   return parent->get_fieldsel_radius()+2;
}
