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

#include "editor_place_immovable_tool.h"

#include "logic/editor_game_base.h"
#include "editor/editorinteractive.h"
#include "logic/field.h"
#include "logic/immovable.h"
#include "logic/mapregion.h"

/**
 * Choses an object to place randomly from all enabled
 * and places this on the current field
*/
int32_t Editor_Place_Immovable_Tool::handle_click_impl
	(Widelands::Map               &       map,
	 Widelands::Node_and_Triangle<> const center,
	 Editor_Interactive           &       parent)
{
	const int32_t radius = parent.get_sel_radius();
	if (not get_nr_enabled())
		return radius;
	Widelands::Editor_Game_Base & egbase = parent.egbase();
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
		(map,
		 Widelands::Area<Widelands::FCoords>
		 	(map.get_fcoords(center.node), radius));
	do if
		(not mr.location().field->get_immovable()
		 and
		 mr.location().field->get_caps() & Widelands::MOVECAPS_WALK)
		egbase.create_immovable(mr.location(), get_random_enabled(), 0);
	while (mr.advance(map));
	return radius + 2;
}
