/*
 * Copyright (C) 2006-2008, 2012 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */


#include "editor/tools/editor_set_terrain_tool.h"

#include "editor/editorinteractive.h"
#include "logic/maptriangleregion.h"

using Widelands::TCoords;

int32_t Editor_Set_Terrain_Tool::handle_click_impl
	(Widelands::Map           &          map,
	Widelands::Node_and_Triangle<> const center,
	Editor_Interactive        &          /* parent */, Editor_Action_Args & args)
{
	assert
	(center.triangle.t == TCoords<>::D or center.triangle.t == TCoords<>::R);
	uint16_t const radius = args.sel_radius;
	int32_t max = 0;

	if (get_nr_enabled() && args.terrainType.empty()) {
		Widelands::MapTriangleRegion<TCoords<Widelands::FCoords> > mr
		(map, Widelands::Area<TCoords<Widelands::FCoords> >
		 (TCoords<Widelands::FCoords>
		  (Widelands::FCoords(map.get_fcoords(center.triangle)),
		   static_cast<TCoords<Widelands::FCoords>::TriangleIndex>(center.triangle.t)),
		  radius));
		do {
			args.origTerrainType.push_back
				((mr.location().t == TCoords<Widelands::FCoords>::D)
					? mr.location().field->terrain_d() : mr.location().field->terrain_r());
			args.terrainType.push_back(get_random_enabled());
		} while (mr.advance(map));
	}

	if (not args.terrainType.empty()) {
		Widelands::MapTriangleRegion<TCoords<Widelands::FCoords> > mr
		(map, Widelands::Area<TCoords<Widelands::FCoords> >
		 (TCoords<Widelands::FCoords>
		  (Widelands::FCoords(map.get_fcoords(center.triangle)),
		   static_cast<TCoords<Widelands::FCoords>::TriangleIndex>(center.triangle.t)),
		    radius));
		std::list<Widelands::Terrain_Index>::iterator i = args.terrainType.begin();
		do {
			max = std::max
			      (max, map.change_terrain(mr.location(), *i));
			++i;
		} while (mr.advance(map));
	}
	return radius + max;
}

int32_t Editor_Set_Terrain_Tool::handle_undo_impl
	(Widelands::Map & map, Widelands::Node_and_Triangle< Widelands::Coords > center,
	Editor_Interactive & /* parent */, Editor_Action_Args & args)
{
	assert
	(center.triangle.t == TCoords<>::D or center.triangle.t == TCoords<>::R);
	uint16_t const radius = args.sel_radius;
	if (not args.terrainType.empty()) {
		int32_t max = 0;
		Widelands::MapTriangleRegion<TCoords<Widelands::FCoords> > mr
		(map,
		 Widelands::Area<TCoords<Widelands::FCoords> >
		 (TCoords<Widelands::FCoords>
		  (Widelands::FCoords(map.get_fcoords(center.triangle)),
		   static_cast<TCoords<Widelands::FCoords>::TriangleIndex>
		   (center.triangle.t)),
		  radius));

		std::list<Widelands::Terrain_Index>::iterator i = args.origTerrainType.begin();
		do {
			max = std::max
			      (max, map.change_terrain(mr.location(), *i));
			++i;
		} while (mr.advance(map));
		return radius + max;
	} else return radius;
}

Editor_Action_Args Editor_Set_Terrain_Tool::format_args_impl(Editor_Interactive & parent)
{
	return Editor_Tool::format_args_impl(parent);
}
