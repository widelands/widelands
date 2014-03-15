/*
 * Copyright (C) 2002-2004, 2006-2008, 2010, 2012 by the Widelands Development Team
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

#include "editor/tools/editor_increase_resources_tool.h"

#include "editor/editorinteractive.h"
#include "graphic/graphic.h"
#include "logic/field.h"
#include "logic/mapregion.h"
#include "logic/world.h"
#include "logic/worlddata.h"
#include "wui/overlay_manager.h"

using Widelands::TCoords;

int32_t Editor_Change_Resource_Tool_Callback
	(const TCoords<Widelands::FCoords>& c, Widelands::Map& map, int32_t const curres)
{
	Widelands::World & world = map.world();
	Widelands::FCoords f(c, &map[c]);

	Widelands::FCoords f1;
	int32_t count = 0;

	//  this field
	count += world.terrain_descr(f.field->terrain_r()).resource_value(curres);
	count += world.terrain_descr(f.field->terrain_d()).resource_value(curres);


	//  If one of the neighbours is unpassable, count its resource stronger.
	//  top left neigbour
	map.get_neighbour(f, Widelands::WALK_NW, &f1);
	count += world.terrain_descr(f1.field->terrain_r()).resource_value(curres);
	count += world.terrain_descr(f1.field->terrain_d()).resource_value(curres);

	//  top right neigbour
	map.get_neighbour(f, Widelands::WALK_NE, &f1);
	count += world.terrain_descr(f1.field->terrain_d()).resource_value(curres);

	//  left neighbour
	map.get_neighbour(f, Widelands::WALK_W, &f1);
	count += world.terrain_descr(f1.field->terrain_r()).resource_value(curres);

	return count <= 3 ? 0 : f.field->nodecaps();
}

/*
===========
Editor_Increase_Resources_Tool::handle_click_impl()

increase the resources of the current field by one if
there is not already another resource there.
===========
*/
int32_t Editor_Increase_Resources_Tool::handle_click_impl
	(Widelands::Map           &           map,
	Widelands::Node_and_Triangle<> const center,
	Editor_Interactive         &         /* parent */,
	Editor_Action_Args         &         args)
{
	const Widelands::World & world = map.world();
	Overlay_Manager & overlay_manager = map.overlay_manager();
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
		(map,
			Widelands::Area<Widelands::FCoords>
				(map.get_fcoords(center.node), args.sel_radius));
	do {
		int32_t res        = mr.location().field->get_resources();
		int32_t amount     = mr.location().field->get_resources_amount();
		int32_t max_amount =
		    map.get_world()->get_resource(args.cur_res)->get_max_amount();

		amount += args.change_by;
		if (amount > max_amount)
			amount = max_amount;

		args.orgResT.push_back(res);
		args.orgRes.push_back(mr.location().field->get_resources_amount());

		if
		((res == args.cur_res or not mr.location().field->get_resources_amount())
		        and
		        Editor_Change_Resource_Tool_Callback(mr.location(), map, args.cur_res))
		{
			//  Ok, we're doing something. First remove the current overlays.
			const Image* pic =
			    g_gr->images().get
			    (world.get_resource(res)->get_editor_pic
			     (mr.location().field->get_resources_amount()));
			overlay_manager.remove_overlay(mr.location(), pic);

			if (not amount) {
				mr.location().field->set_resources(0, 0);
				mr.location().field->set_starting_res_amount(0);
			} else {
				mr.location().field->set_resources(args.cur_res, amount);
				mr.location().field->set_starting_res_amount(amount);
				//  set new overlay
				pic = g_gr->images().get
				        (world.get_resource(args.cur_res)->get_editor_pic(amount));
				overlay_manager.register_overlay(mr.location(), pic, 4);
				map.recalc_for_field_area
				(Widelands::Area<Widelands::FCoords>(mr.location(), 0));
			}
		}
	} while (mr.advance(map));
	return mr.radius();
}

int32_t Editor_Increase_Resources_Tool::handle_undo_impl
	(Widelands::Map & map,
	Widelands::Node_and_Triangle< Widelands::Coords > center,
	Editor_Interactive & parent, Editor_Action_Args & args)
{
	return m_set_tool.handle_undo_impl(map, center, parent, args);
}

Editor_Action_Args Editor_Increase_Resources_Tool::format_args_impl(Editor_Interactive & parent)
{
	Editor_Action_Args a(parent);
	a.change_by = m_change_by;
	a.cur_res = m_cur_res;
	return a;
}
