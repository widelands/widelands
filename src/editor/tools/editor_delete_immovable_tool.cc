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

#include "editor_delete_immovable_tool.h"

#include "field.h"
#include "editorinteractive.h"
#include "immovable.h"
#include "mapregion.h"


/*
===========
Editor_Delete_Immovable_Tool::handle_click_impl()

deletes the immovable at the given location
===========
*/
int32_t Editor_Delete_Immovable_Tool::handle_click_impl
(Map & map, const Node_and_Triangle<> center, Editor_Interactive & parent)
{
	MapRegion<Area<FCoords> > mr
		(map,
		 Area<FCoords>(map.get_fcoords(center.node), parent.get_sel_radius()));
	do if
		(Immovable * const immovable =
		 dynamic_cast<Immovable *>(mr.location().field->get_immovable()))
		immovable->remove(&parent.editor()); //  Delete no buildings or stuff.
	while (mr.advance(map));
	return mr.radius() + 2;
}
