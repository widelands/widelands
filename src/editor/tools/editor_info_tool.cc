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

#include "editor_info_tool.h"
#include "ui_multilinetextarea.h"
#include "ui_window.h"
#include "editorinteractive.h"
#include "world.h"
#include "map.h"

/*
=============================

class Editor_Info_Tool

=============================
*/

/*
===========
Editor_Info_Tool::handle_click()

show a simple info dialog with infos about this field
===========
*/
int Editor_Info_Tool::handle_click_impl(FCoords& fc,Map* map, Editor_Interactive* parent) {
   UIWindow* w = new UIWindow(parent, 30, 30, 400, 200, "Field Information");
   UIMultiline_Textarea* multiline_textarea = new UIMultiline_Textarea(w, 0, 0, w->get_inner_w(), w->get_inner_h(), 0);

   Field* f=fc.field;

   std::string buf;
   char buf1[1024];

   buf += "1) Field Infos\n";
   sprintf(buf1, " Coordinates: (%i/%i)\n", fc.x, fc.y); buf+=buf1;
   sprintf(buf1, " Height: %i\n", f->get_height()); buf+=buf1;
   buf+=" Caps: ";
   switch((f->get_caps() & BUILDCAPS_SIZEMASK)) {
      case BUILDCAPS_SMALL: buf+="small"; break;
      case BUILDCAPS_MEDIUM: buf+="medium"; break;
      case BUILDCAPS_BIG: buf+="big"; break;
      default: break;
   }
   if(f->get_caps() & BUILDCAPS_FLAG) buf+=" flag";
   if(f->get_caps() & BUILDCAPS_MINE) buf+=" mine";
   if(f->get_caps() & BUILDCAPS_PORT) buf+=" port";
   if(f->get_caps() & MOVECAPS_WALK) buf+=" walk";
   if(f->get_caps() & MOVECAPS_SWIM) buf+=" swim";
   buf+="\n";
   sprintf(buf1, " Owned by: %i\n", f->get_owned_by()); buf+=buf1;
   sprintf(buf1, " Has base immovable: %s (TODO! more info)\n", f->get_immovable() ? "Yes" : "No"); buf+=buf1;
   sprintf(buf1, " Has bobs: %s (TODO: more informations)\n", f->get_first_bob() ? "Yes" : "No"); buf+=buf1;
   int res=f->get_resources();
   int amount=f->get_resources_amount();
   if(res==0 && amount==0) {
      sprintf(buf1, " Has resources: No\n");
   } else {
      sprintf(buf1, " Has resources: Yes, %i amount of '%s'\n", amount, map->get_world()->get_resource(res)->get_name());
   }
   buf+=buf1;
   
   sprintf(buf1, " Start resources amount: %i\n", f->get_starting_res_amount());
   buf+=buf1;

   sprintf(buf1, " Roads: TODO!\n"); buf+=buf1;

   buf += "\n";
   Terrain_Descr* ter=f->get_terr();
   buf += "2) Right Terrain Info\n";
   sprintf(buf1, " Name: %s\n", ter->get_name()); buf+=buf1;
   sprintf(buf1, " Texture Number: %i\n", ter->get_texture()); buf+=buf1;

   buf += "\n";
   ter=f->get_terd();
   buf += "3) Down Terrain Info\n";
   sprintf(buf1, " Name: %s\n", ter->get_name()); buf+=buf1;
   sprintf(buf1, " Texture Number: %i\n", ter->get_texture()); buf+=buf1;

   buf += "\n";
   buf += "4) Map Info";
   sprintf(buf1, " Name: %s\n", map->get_name()); buf+=buf1;
   sprintf(buf1, " Size: %ix%i\n", map->get_width(), map->get_height()); buf+=buf1;
   sprintf(buf1, " Author: %s\n", map->get_author()); buf+=buf1;
   sprintf(buf1, " Descr: %s\n", map->get_description()); buf+=buf1;
   sprintf(buf1, " Number of Players: %i\n", map->get_nrplayers()); buf+=buf1;
   sprintf(buf1, " TODO: more information (number of resources, number of terrains...)\n"); buf+=buf1;

   buf += "\n";
   buf += "5) World Info";
   sprintf(buf1, " Name: %s\n", map->get_world()->get_name()); buf+=buf1;
   sprintf(buf1, " Author: %s\n", map->get_world()->get_author()); buf+=buf1;
   sprintf(buf1, " Descr: %s\n", map->get_world()->get_descr()); buf+=buf1;
   sprintf(buf1, " TODO -- More information (Number of bobs/number of wares...)\n"); buf+=buf1;

   buf += "\n";
   buf += "\n";
   buf += "\n";
   buf += "\n";
   buf += "\n";

   multiline_textarea->set_text(buf.c_str());

   return 0;
}


