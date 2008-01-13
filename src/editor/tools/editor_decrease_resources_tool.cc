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

#include "editor_decrease_resources_tool.h"

#include "editor_increase_resources_tool.h"
#include "editor_set_resources_tool.h"
#include "widelands_geometry.h"
#include "graphic.h"
#include "map.h"
#include "field.h"
#include "editorinteractive.h"
#include "world.h"
#include "overlay_manager.h"


/*
===========
Editor_Decrease_Resources_Tool::handle_click_impl()

decrease the resources of the current field by one if
there is not already another resource there.
===========
*/
int32_t Editor_Decrease_Resources_Tool::handle_click_impl
(Widelands::Map                     & map,
 Widelands::Node_and_Triangle<> const center,
 Editor_Interactive                 & parent)
{
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
		(map,
		 Widelands::Area<Widelands::FCoords>
		 (map.get_fcoords(center.node), parent.get_sel_radius()));
	do {
		int32_t res    = mr.location().field->get_resources();
		int32_t amount = mr.location().field->get_resources_amount();

		amount -= m_change_by;
		if (amount < 0)
			amount = 0;


		if
			(res == m_cur_res
			 and
			 Editor_Change_Resource_Tool_Callback(mr.location(), &map, m_cur_res))
		{
			int32_t picid;
         // Ok, we're doing something. First remove the current overlays
         std::string str;
			str = map.world().get_resource(res)->get_editor_pic
				(mr.location().field->get_resources_amount());
         picid=g_gr->get_picture(PicMod_Menu,  str.c_str());
			map.overlay_manager().remove_overlay(mr.location(), picid);
			if (!amount) {
				mr.location().field->set_resources(0, 0);
				mr.location().field->set_starting_res_amount(0);
			} else {
				mr.location().field->set_resources(m_cur_res, amount);
				mr.location().field->set_starting_res_amount(amount);
            // set new overlay
				str = map.world().get_resource(m_cur_res)->get_editor_pic(amount);
            picid=g_gr->get_picture(PicMod_Menu,  str.c_str());
				map.overlay_manager().register_overlay(mr.location(), picid, 4);
				map.recalc_for_field_area
					(Widelands::Area<Widelands::FCoords>(mr.location(), 0));
			}
		}
	} while (mr.advance(map));
	return mr.radius();
}
