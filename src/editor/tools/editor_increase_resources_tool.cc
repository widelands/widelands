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

#include "editor_increase_resources_tool.h"

#include "editor/editorinteractive.h"
#include "field.h"
#include "graphic/graphic.h"
#include "map.h"
#include "world.h"
#include "graphic/overlay_manager.h"
#include "worlddata.h"

using Widelands::TCoords;

int32_t Editor_Change_Resource_Tool_Callback
	(TCoords<Widelands::FCoords> const c, void * data, int32_t curres)
{
	Widelands::Map   & map   = *static_cast<Widelands::Map *>(data);
	Widelands::World & world = map.world();
	Widelands::FCoords f(c, &map[c]);

	Widelands::FCoords f1;
	int32_t count = 0;

	//  this field
	count += world.terrain_descr(f.field->terrain_r()).resource_value(curres);
	count += world.terrain_descr(f.field->terrain_d()).resource_value(curres);


	//  If one of the neighbours is unpassable, count its resource stronger.
	//  top left neigbour
	map.get_neighbour(f, Widelands::Map_Object::WALK_NW, &f1);
	count += world.terrain_descr(f1.field->terrain_r()).resource_value(curres);
	count += world.terrain_descr(f1.field->terrain_d()).resource_value(curres);

	//  top right neigbour
	map.get_neighbour(f, Widelands::Map_Object::WALK_NE, &f1);
	count += world.terrain_descr(f1.field->terrain_d()).resource_value(curres);

	//  left neighbour
	map.get_neighbour(f, Widelands::Map_Object::WALK_W, &f1);
	count += world.terrain_descr(f1.field->terrain_r()).resource_value(curres);

	return count <= 3 ? 0 : f.field->get_caps();
}

/*
===========
Editor_Increase_Resources_Tool::handle_click_impl()

decrease the resources of the current field by one if
there is not already another resource there.
===========
*/
int32_t Editor_Increase_Resources_Tool::handle_click_impl
	(Widelands::Map               &       map,
	 Widelands::Node_and_Triangle<> const center,
	 Editor_Interactive           &       parent)
{
	Widelands::World const & world = map.world();
	Overlay_Manager & overlay_manager = map.overlay_manager();
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
		(map,
		 Widelands::Area<Widelands::FCoords>
		 	(map.get_fcoords(center.node), parent.get_sel_radius()));
	do {
		int32_t res        = mr.location().field->get_resources();
		int32_t amount     = mr.location().field->get_resources_amount();
		int32_t max_amount =
			map.get_world()->get_resource(m_cur_res)->get_max_amount();

		amount += m_change_by;
		if (amount > max_amount)
			amount = max_amount;


		if
			((res == m_cur_res or not res)
			 and
			 Editor_Change_Resource_Tool_Callback(mr.location(), &map, m_cur_res))
		{
			//  Ok, we're doing something. First remove the current overlays.
			uint32_t picid =
				g_gr->get_picture
					(PicMod_Menu,
					 world.get_resource(res)->get_editor_pic
					 	(mr.location().field->get_resources_amount()).c_str());
			overlay_manager.remove_overlay(mr.location(), picid);

			if (not amount) {
				mr.location().field->set_resources(0, 0);
				mr.location().field->set_starting_res_amount(0);
			} else {
				mr.location().field->set_resources(m_cur_res, amount);
				mr.location().field->set_starting_res_amount(amount);
				//  set new overlay
				picid = g_gr->get_picture
					(PicMod_Menu,
					 world.get_resource(m_cur_res)->get_editor_pic(amount).c_str());
				overlay_manager.register_overlay(mr.location(), picid, 4);
				map.recalc_for_field_area
					(Widelands::Area<Widelands::FCoords>(mr.location(), 0));
			}
		}
	} while (mr.advance(map));
	return mr.radius();
}
