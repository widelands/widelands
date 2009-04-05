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

using Widelands::Bob;

/**
 * Choses an object to place randomly from all enabled
 * and places this on the current field
*/
int32_t Editor_Place_Bob_Tool::handle_click_impl
	(Widelands::Map               &       map,
	 Widelands::Node_and_Triangle<> const center,
	 Editor_Interactive           &       parent)
{
	Widelands::Editor_Game_Base & egbase = parent.egbase();
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
		(map,
		 Widelands::Area<Widelands::FCoords>
		 	(map.get_fcoords(center.node), parent.get_sel_radius()));
	if (get_nr_enabled()) {
		do {
			Bob::Descr const & descr =
				*map.world().get_bob_descr(get_random_enabled());
			if (mr.location().field->get_caps() & descr.movecaps()) {
				if (Bob * const bob = mr.location().field->get_first_bob())
					bob->remove(egbase); //  There is already a bob. Remove it.
				descr.create(egbase, 0, mr.location());
			}
		} while (mr.advance(map));
		return mr.radius() + 2;
	} else
		return mr.radius();
}
