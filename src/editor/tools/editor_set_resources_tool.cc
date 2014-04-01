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

#include "editor/tools/editor_set_resources_tool.h"

#include "editor/editorinteractive.h"
#include "editor/tools/editor_decrease_resources_tool.h"
#include "editor/tools/editor_increase_resources_tool.h"
#include "graphic/graphic.h"
#include "logic/field.h"
#include "logic/mapregion.h"
#include "logic/world.h"
#include "wui/overlay_manager.h"

/**
 * Sets the resources of the current to a fixed value
*/
int32_t Editor_Set_Resources_Tool::handle_click_impl
	(Widelands::Map           &          map,
	Widelands::Node_and_Triangle<> const center,
	Editor_Interactive        &          /* parent */,
	Editor_Action_Args        &          args)
{
	const Widelands::World & world = map.world();
	Overlay_Manager & overlay_manager = map.overlay_manager();
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
	(map,
	 Widelands::Area<Widelands::FCoords>
	 (map.get_fcoords(center.node), args.sel_radius));
	do {
		int32_t res        = mr.location().field->get_resources();
		int32_t amount     = args.set_to;
		int32_t max_amount = world.get_resource(args.cur_res)->get_max_amount();

		if (amount < 0)
			amount = 0;
		if (amount > max_amount)
			amount = max_amount;

		args.orgResT.push_back(res);
		args.orgRes.push_back(mr.location().field->get_resources_amount());

		if (Editor_Change_Resource_Tool_Callback(mr.location(), map, args.cur_res)) {
			//  Ok, we're doing something. First remove the current overlays.
			const Image* pic = g_gr->images().get
				(world.get_resource(res)->get_editor_pic (mr.location().field->get_resources_amount()));
			overlay_manager.remove_overlay(mr.location(), pic);

			if (not amount) {
				mr.location().field->set_resources(0, 0);
				mr.location().field->set_starting_res_amount(0);
			} else {
				mr.location().field->set_resources(args.cur_res, amount);
				mr.location().field->set_starting_res_amount(amount);
				//  set new overlay
				pic =
				    g_gr->images().get(world.get_resource(args.cur_res)->get_editor_pic(amount));
				overlay_manager.register_overlay(mr.location(), pic, 4);
				map.recalc_for_field_area
				(Widelands::Area<Widelands::FCoords>(mr.location(), 0));
			}
		}
	} while (mr.advance(map));
	return mr.radius();
}

int32_t Editor_Set_Resources_Tool::handle_undo_impl
	(Widelands::Map & map, Widelands::Node_and_Triangle< Widelands::Coords > center,
	Editor_Interactive & /* parent */, Editor_Action_Args & args)
{
	const Widelands::World & world = map.world();
	Overlay_Manager & overlay_manager = map.overlay_manager();
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
	(map,
	 Widelands::Area<Widelands::FCoords>
	 (map.get_fcoords(center.node), args.sel_radius));
	std::list<uint8_t>::iterator ir = args.orgRes.begin(), it = args.orgResT.begin();
	do {
		int32_t res        = mr.location().field->get_resources();
		int32_t amount     = *ir;
		int32_t max_amount = world.get_resource(args.cur_res)->get_max_amount();

		if (amount < 0)
			amount = 0;
		if (amount > max_amount)
			amount = max_amount;

		const Image* pic =
		    g_gr->images().get
		    (world.get_resource(res)->get_editor_pic(mr.location().field->get_resources_amount()));
		overlay_manager.remove_overlay(mr.location(), pic);

		if (not amount) {
			mr.location().field->set_resources(0, 0);
			mr.location().field->set_starting_res_amount(0);
		} else {
			mr.location().field->set_resources(*it, amount);
			mr.location().field->set_starting_res_amount(amount);
			//  set new overlay
			pic = g_gr->images().get(world.get_resource(*it)->get_editor_pic(amount));
			overlay_manager.register_overlay(mr.location(), pic, 4);
			map.recalc_for_field_area
			(Widelands::Area<Widelands::FCoords>(mr.location(), 0));
		}
		++ir;
		++it;
	} while (mr.advance(map));
	args.orgRes.clear();
	args.orgResT.clear();
	return mr.radius();
}

Editor_Action_Args Editor_Set_Resources_Tool::format_args_impl(Editor_Interactive & parent)
{
	Editor_Action_Args a(parent);
	a.cur_res = m_cur_res;
	a.set_to = m_set_to;
	return a;
}
