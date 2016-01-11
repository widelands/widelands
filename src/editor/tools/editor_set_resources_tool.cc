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
#include "logic/map_objects/world/resource_description.h"
#include "logic/map_objects/world/world.h"
#include "logic/mapregion.h"
#include "wui/field_overlay_manager.h"

int32_t EditorSetResourcesTool::handle_click_impl(const Widelands::World& world,
                                                  Widelands::NodeAndTriangle<> const center,
                                                  EditorInteractive& parent,
                                                  EditorActionArgs* args,
                                                  Widelands::Map* map) {
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
	(*map,
	 Widelands::Area<Widelands::FCoords>
	 (map->get_fcoords(center.node), args->sel_radius));
	do {
		int32_t amount     = args->set_to;
		int32_t max_amount = args->cur_res != Widelands::kNoResource ?
							 world.get_resource(args->cur_res)->max_amount() : 0;
		if (amount < 0)
			amount = 0;
		else if (amount > max_amount)
			amount = max_amount;

		if (map->is_resource_valid(world, mr.location(), args->cur_res)) {
			args->orgResT.push_back(mr.location().field->get_resources());
			args->orgRes.push_back(mr.location().field->get_resources_amount());
			set_res_and_overlay(world, amount, args->cur_res, mr.location(),
			                    parent.mutable_field_overlay_manager(), map);
		}
	} while (mr.advance(*map));
	return mr.radius();
}

int32_t
EditorSetResourcesTool::handle_undo_impl(const Widelands::World& world,
                                         Widelands::NodeAndTriangle<Widelands::Coords> center,
                                         EditorInteractive& parent,
                                         EditorActionArgs* args,
                                         Widelands::Map* map) {
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
	(*map,
	 Widelands::Area<Widelands::FCoords>
	 (map->get_fcoords(center.node), args->sel_radius));
	std::list<uint8_t>::iterator ir = args->orgRes.begin(), it = args->orgResT.begin();
	do {
		int32_t amount     = *ir;
		int32_t max_amount = world.get_resource(args->cur_res)->max_amount();

		if (amount < 0)
			amount = 0;
		if (amount > max_amount)
			amount = max_amount;

		set_res_and_overlay(
		   world, amount, *ir, mr.location(), parent.mutable_field_overlay_manager(), map);

		++ir;
		++it;
	} while (mr.advance(*map));
	args->orgRes.clear();
	args->orgResT.clear();
	return mr.radius();
}

EditorActionArgs EditorSetResourcesTool::format_args_impl(EditorInteractive & parent)
{
	EditorActionArgs a(parent);
	a.cur_res = m_cur_res;
	a.set_to = m_set_to;
	return a;
}

void EditorSetResourcesTool::set_res_and_overlay(const Widelands::World& world,
											int32_t amount, uint8_t new_res,
											const Widelands::FCoords& fcoords,
											FieldOverlayManager* field_overlay_manager,
											Widelands::Map* map) {
	int32_t old_res = fcoords.field->get_resources();


	//  Ok, we're doing something. First remove the current overlays.
	if (old_res != Widelands::kNoResource) {
		std::string str = world.get_resource(old_res)->get_editor_pic(
				fcoords.field->get_resources_amount());
		const Image* pic = g_gr->images().get(str);
		field_overlay_manager->remove_overlay(fcoords, pic);
	}

	if (!amount) {
		fcoords.field->set_resources(Widelands::kNoResource, 0);
		fcoords.field->set_initial_res_amount(0);
	} else {
		fcoords.field->set_resources(new_res, amount);
		fcoords.field->set_initial_res_amount(amount);
		//  set new overlay
		std::string str = world.get_resource(new_res)->get_editor_pic(amount);
		const Image* pic = g_gr->images().get(str);
		field_overlay_manager->register_overlay(fcoords, pic, 0);
		map->recalc_for_field_area(world, Widelands::Area<Widelands::FCoords>(fcoords, 0));
	}
}
