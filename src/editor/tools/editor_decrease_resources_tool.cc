/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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
#include "editor_set_resources_tool.h"
#include "editor_decrease_resources_tool.h"
#include "geometry.h"
#include "graphic.h"
#include "map.h"
#include "field.h"
#include "editorinteractive.h"
#include "world.h"
#include "overlay_manager.h"

/*
=============================

class Editor_Decrease_Resources_Tool

=============================
*/

/*
===========
Editor_Decrease_Resources_Tool::handle_click_impl()

decrease the resources of the current field by one if
there is not already another resource there.
===========
*/
int Editor_Decrease_Resources_Tool::handle_click_impl
(Map & map, const Node_and_Triangle center, Editor_Interactive & parent)
{
	const int radius = parent.get_sel_radius();
	MapRegion mr(map, center.node, radius);
	FCoords fc;
	while (mr.next(fc)) {
		int res    =fc.field->get_resources();
		int amount =fc.field->get_resources_amount();

      amount-=m_changed_by;
      if(amount<0) amount=0;


		if
			(res == m_cur_res
			 and
			 Editor_Change_Resource_Tool_Callback(fc, &map, m_cur_res))
		{
			int picid;
         // Ok, we're doing something. First remove the current overlays
         std::string str;
			str = map.world().get_resource(res)->get_editor_pic
				(fc.field->get_resources_amount());
         picid=g_gr->get_picture( PicMod_Menu,  str.c_str() );
			map.get_overlay_manager()->remove_overlay(fc, picid);
         if(!amount) {
				fc.field->set_resources(0,0);
				fc.field->set_starting_res_amount(0);
         } else {
				fc.field->set_resources(m_cur_res,amount);
				fc.field->set_starting_res_amount(amount);
            // set new overlay
				str = map.world().get_resource(m_cur_res)->get_editor_pic(amount);
            picid=g_gr->get_picture( PicMod_Menu,  str.c_str() );
				map.get_overlay_manager()->register_overlay(fc, picid, 4);
				map.recalc_for_field_area(fc, 0);
         }
      }
   }
	return radius;
}
