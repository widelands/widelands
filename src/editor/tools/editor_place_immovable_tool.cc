/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "editor_place_immovable_tool.h"
#include "field.h"
#include "immovable.h"
#include "map.h"
#include "editorinteractive.h"
#include "editor.h"

/*
===========
Editor_Place_Immovable_Tool::handle_click_impl()

choses an object to place randomly from all enabled
and places this on the current field
===========
*/
int Editor_Place_Immovable_Tool::handle_click_impl
(Map & map, const Node_and_Triangle center, Editor_Interactive & parent)
{
	const int radius = parent.get_sel_radius();
	if (not get_nr_enabled()) return radius;
	MapRegion mr(map, center.node, radius);
	FCoords fc;
	Editor & editor = parent.editor();
	while (mr.next(fc)) {
		if (fc.field->get_immovable()) {
			if (fc.field->get_immovable()->get_size() != BaseImmovable::NONE)
            continue;
      }

		editor.create_immovable(fc, get_random_enabled(), 0);
   }
   return radius + 2;
}
