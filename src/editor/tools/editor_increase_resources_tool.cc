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
#include "logic/world/resource_description.h"
#include "logic/world/terrain_description.h"
#include "logic/world/world.h"
#include "wui/overlay_manager.h"

using Widelands::TCoords;

namespace  {

int32_t resource_value(const Widelands::TerrainDescription& terrain,
                       const Widelands::DescriptionIndex resource) {
	if (!terrain.is_resource_valid(resource)) {
		return -1;
	}
	if (terrain.get_is() & Widelands::TerrainDescription::Type::kImpassable) {
		return 8;
	}
	return 1;
}

}  // namespace


int32_t editor_change_resource_tool_callback
	(const TCoords<Widelands::FCoords>& c, Widelands::Map& map,
	 const Widelands::World& world, int32_t const curres)
{
	Widelands::FCoords f(c, &map[c]);

	Widelands::FCoords f1;
	int32_t count = 0;

	//  this field
	count += resource_value(world.terrain_descr(f.field->terrain_r()), curres);
	count += resource_value(world.terrain_descr(f.field->terrain_d()), curres);

	//  If one of the neighbours is impassable, count its resource stronger.
	//  top left neigbour
	map.get_neighbour(f, Widelands::WALK_NW, &f1);
	count += resource_value(world.terrain_descr(f1.field->terrain_r()), curres);
	count += resource_value(world.terrain_descr(f1.field->terrain_d()), curres);

	//  top right neigbour
	map.get_neighbour(f, Widelands::WALK_NE, &f1);
	count += resource_value(world.terrain_descr(f1.field->terrain_d()), curres);

	//  left neighbour
	map.get_neighbour(f, Widelands::WALK_W, &f1);
	count += resource_value(world.terrain_descr(f1.field->terrain_r()), curres);

	return count <= 3 ? 0 : f.field->nodecaps();
}

/*
===========
EditorIncreaseResourcesTool::handle_click_impl()

increase the resources of the current field by one if
there is not already another resource there.
===========
*/
int32_t
EditorIncreaseResourcesTool::handle_click_impl(Widelands::Map& map,
                                                  const Widelands::World& world,
                                                  Widelands::NodeAndTriangle<> const center,
                                                  EditorInteractive& /* parent */,
                                                  EditorActionArgs& args) {
	OverlayManager & overlay_manager = map.overlay_manager();
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
		(map,
			Widelands::Area<Widelands::FCoords>
				(map.get_fcoords(center.node), args.sel_radius));
	do {
		int32_t res = mr.location().field->get_resources();
		int32_t amount = mr.location().field->get_resources_amount();
		int32_t max_amount = world.get_resource(args.cur_res)->max_amount();

		amount += args.change_by;
		if (amount > max_amount)
			amount = max_amount;

		args.orgResT.push_back(res);
		args.orgRes.push_back(mr.location().field->get_resources_amount());

		if
		((res == args.cur_res || !mr.location().field->get_resources_amount())
		        &&
				  editor_change_resource_tool_callback(mr.location(), map, world, args.cur_res))
		{
			//  Ok, we're doing something. First remove the current overlays.
			const Image* pic;
			if (res != Widelands::kNoResource) {
				pic = g_gr->images().get(world.get_resource(res)->get_editor_pic(
						mr.location().field->get_resources_amount()));
				overlay_manager.remove_overlay(mr.location(), pic);
			}

			if (!amount) {
				mr.location().field->set_resources(Widelands::kNoResource, 0);
				mr.location().field->set_initial_res_amount(0);
			} else {
				mr.location().field->set_resources(args.cur_res, amount);
				mr.location().field->set_initial_res_amount(amount);
				//  set new overlay
				pic = g_gr->images().get
				        (world.get_resource(args.cur_res)->get_editor_pic(amount));
				overlay_manager.register_overlay(mr.location(), pic, 4);
				map.recalc_for_field_area(
				   world, Widelands::Area<Widelands::FCoords>(mr.location(), 0));
			}
		}
	} while (mr.advance(map));
	return mr.radius();
}

int32_t EditorIncreaseResourcesTool::handle_undo_impl(
   Widelands::Map& map,
   const Widelands::World& world,
   Widelands::NodeAndTriangle<Widelands::Coords> center,
   EditorInteractive& parent,
   EditorActionArgs& args) {
	return m_set_tool.handle_undo_impl(map, world, center, parent, args);
}

EditorActionArgs EditorIncreaseResourcesTool::format_args_impl(EditorInteractive & parent)
{
	EditorActionArgs a(parent);
	a.change_by = m_change_by;
	a.cur_res = m_cur_res;
	return a;
}
