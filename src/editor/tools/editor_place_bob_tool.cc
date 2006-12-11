/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#include "editor_place_bob_tool.h"
#include "field.h"
#include "map.h"
#include "editorinteractive.h"
#include "editor.h"
#include "bob.h"

/*
===========
Editor_Place_Bob_Tool::handle_click_impl()

choses an object to place randomly from all enabled
and places this on the current field
===========
*/
int Editor_Place_Bob_Tool::handle_click_impl
(Map & map, const Node_and_Triangle center, Editor_Interactive & parent)
{
	const int radius = parent.get_sel_radius();
	if (not get_nr_enabled()) return radius;
	MapRegion mr(map, center.node, radius);
   FCoords fc;
	while (mr.next(fc)) {
		if (Bob * const bob = fc.field->get_first_bob())
         // There is already a bob. Remove it first
         bob->remove(parent.get_editor());
		parent.get_editor()->create_bob(fc, get_random_enabled());
   }
   return radius + 2;
}
