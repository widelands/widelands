/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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
#include "mapregion.h"
#include "editorinteractive.h"
#include "editor_game_base.h"
#include "bob.h"

/**
 * Choses an object to place randomly from all enabled
 * and places this on the current field
*/
int32_t Editor_Place_Bob_Tool::handle_click_impl
(Widelands::Map                     & map,
 Widelands::Node_and_Triangle<> const center,
 Editor_Interactive                 & parent)
{
	Widelands::Editor_Game_Base & editor = parent.egbase();
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
		(map,
		 Widelands::Area<Widelands::FCoords>
		 	(map.get_fcoords(center.node), parent.get_sel_radius()));
	if (get_nr_enabled()) {
		do {
			if (Widelands::Bob * const bob = mr.location().field->get_first_bob())
				// There is already a bob. Remove it first
				bob->remove(&editor);
			editor.create_bob(mr.location(), get_random_enabled());
		} while (mr.advance(map));
		return mr.radius() + 2;
	} else return mr.radius();
}
