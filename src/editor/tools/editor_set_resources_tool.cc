/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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
#include "editor_decrease_resources_tool.h"
#include "editor_set_resources_tool.h"
#include "map.h"
#include "field.h"
#include "editorinteractive.h"
#include "world.h"
#include "overlay_manager.h"

/*
=============================

class Editor_Set_Resources_Tool

=============================
*/

/*
===========
Editor_Set_Resources_Tool::handle_click_impl()

sets the resources of the current to a fixed value
===========
*/
int Editor_Set_Resources_Tool::handle_click_impl(FCoords& fc, Map* map, Editor_Interactive* parent)
{
   MapRegion mrc(map, fc, parent->get_fieldsel_radius());
   FCoords c;

   while(mrc.next(&c)) {
      Field* f=map->get_field(c);

      int res=f->get_resources();
      int amount=f->get_resources_amount();
      int max_amount=map->get_world()->get_resource(m_cur_res)->get_max_amount();

      amount=m_set_to;
      if(amount<0) amount=0;
      if(amount>max_amount) amount=max_amount;

      if(Editor_Change_Resource_Tool_Callback(c,map,m_cur_res)) {
         // Ok, we're doing something. First remove the current overlays
         std::string str;
         if(res) {
            str=map->get_world()->get_resource(res)->get_editor_pic(f->get_resources_amount());
            int picid=g_gr->get_picture(PicMod_Menu, str.c_str(), true);
            map->get_overlay_manager()->remove_overlay(c,picid);

         }
         if(!amount) {
            f->set_resources(0,0);
            f->set_starting_res_amount(0);
         } else {
            f->set_resources(m_cur_res,amount);
            f->set_starting_res_amount(amount);
            // set new overlay
            str=map->get_world()->get_resource(m_cur_res)->get_editor_pic(amount);
            int picid=g_gr->get_picture(PicMod_Menu, str.c_str(), true);
            map->get_overlay_manager()->register_overlay(c,picid,4);
            map->recalc_for_field_area(c,0);
         }
      }
   }
   return parent->get_fieldsel_radius();
}
