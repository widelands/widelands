/*
 * Copyright (C) 2006-2008 by the Widelands Development Team
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


#include "editor_set_terrain_tool.h"

#include "editorinteractive.h"
#include "maptriangleregion.h"

using Widelands::TCoords;

int32_t Editor_Set_Terrain_Tool::handle_click_impl
	(Widelands::Map               &       map,
	 Widelands::Node_and_Triangle<> const center,
	 Editor_Interactive           &       parent)
{
	assert
		(center.triangle.t == TCoords<>::D or center.triangle.t == TCoords<>::R);
	uint16_t const radius = parent.get_sel_radius();
	if (get_nr_enabled()) {
		int32_t max = 0;
		Widelands::World const & world = map.world();
		Widelands::MapTriangleRegion<TCoords<Widelands::FCoords> > mr
			(map,
			 Widelands::Area<TCoords<Widelands::FCoords> >
			 	(TCoords<Widelands::FCoords>
			 	 	(Widelands::FCoords(map.get_fcoords(center.triangle)),
			 	 	 static_cast<TCoords<Widelands::FCoords>::TriangleIndex>
			 	 	 	(center.triangle.t)),
			 	 radius));
		do {
			Widelands::Terrain_Descr::Index const new_terrain_index =
				get_random_enabled();
				max = std::max
					(max, map.change_terrain(mr.location(), new_terrain_index));
		} while (mr.advance(map));
		return radius + max;
	} else return radius;
}
