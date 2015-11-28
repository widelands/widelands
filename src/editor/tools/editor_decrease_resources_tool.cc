/*
 * Copyright (C) 2002-2004, 2006-2008, 2010-2015 by the Widelands Development Team
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

#include "editor/tools/editor_decrease_resources_tool.h"

#include "editor/editorinteractive.h"
#include "editor/tools/editor_increase_resources_tool.h"
#include "editor/tools/editor_set_resources_tool.h"
#include "graphic/graphic.h"
#include "logic/field.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/map_objects/world/world.h"
#include "logic/mapregion.h"
#include "logic/widelands_geometry.h"
#include "wui/overlay_manager.h"


/**
 * Decrease the resources of the current field by one if
 * there is not already another resource there.
*/
int32_t
EditorDecreaseResourcesTool::handle_click_impl(Widelands::Map& map,
                                                  const Widelands::World& world,
                                                  Widelands::NodeAndTriangle<> const center,
                                                  EditorInteractive& /* parent */,
                                                  EditorActionArgs& args) {
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
	(map,
	Widelands::Area<Widelands::FCoords>
	(map.get_fcoords(center.node), args.sel_radius));
	do {
		int32_t res    = mr.location().field->get_resources();
		int32_t amount = mr.location().field->get_resources_amount();

		amount -= args.change_by;
		if (amount < 0)
			amount = 0;

		args.orgResT.push_back(res);
		args.orgRes.push_back(mr.location().field->get_resources_amount());

		if (res == args.cur_res &&
		    editor_change_resource_tool_callback(mr.location(), map, world, args.cur_res)) {
			//  Ok, we're doing something. First remove the current overlays.
			std::string str =
			    world.get_resource(res)->get_editor_pic
			    (mr.location().field->get_resources_amount());
			const Image* pic = g_gr->images().get(str);
			map.overlay_manager().remove_overlay(mr.location(), pic);
			if (!amount) {
				mr.location().field->set_resources(0, 0);
				mr.location().field->set_initial_res_amount(0);
			} else {
				mr.location().field->set_resources(args.cur_res, amount);
				mr.location().field->set_initial_res_amount(amount);
				//  set new overlay
				str = world.get_resource(args.cur_res)->get_editor_pic(amount);
				pic = g_gr->images().get(str);
				map.overlay_manager().register_overlay(mr.location(), pic, 4);
				map.recalc_for_field_area(
				   world, Widelands::Area<Widelands::FCoords>(mr.location(), 0));
			}
		}
	} while (mr.advance(map));
	return mr.radius();
}

int32_t EditorDecreaseResourcesTool::handle_undo_impl(
   Widelands::Map& map,
   const Widelands::World& world,
   Widelands::NodeAndTriangle<Widelands::Coords> center,
   EditorInteractive& parent,
   EditorActionArgs& args) {
	return parent.tools.set_resources.handle_undo_impl(map, world, center, parent, args);
}

EditorActionArgs EditorDecreaseResourcesTool::format_args_impl(EditorInteractive & parent)
{
	EditorActionArgs a(parent);
	a.change_by = m_change_by;
	a.cur_res = m_cur_res;
	return a;
}
