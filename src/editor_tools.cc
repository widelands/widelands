/*
 * Copyright (C) 2003 by The Widelands Development Team
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

#include "widelands.h"
#include "editorinteractive.h"
#include "editor_tools.h"
#include "ui.h"
#include "map.h"
#include "graphic.h"
#include "sw16_graphic.h"
#include "editor.h"
#include <string>

using std::string;

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
int Editor_Info_Tool::handle_click_impl(const Coords* coordinates, Field* f, Map* map, Editor_Interactive* parent) {
   Window* w = new Window(parent, 30, 30, 400, 200, "Field Information");
   Multiline_Textarea* multiline_textarea = new Multiline_Textarea(w, 0, 0, w->get_inner_w(), w->get_inner_h(), 0);

   string buf;
   char buf1[1024];
   
   buf += "1) Field Infos\n";
   sprintf(buf1, " Coordinates: (%i/%i)\n", coordinates->x, coordinates->y); buf+=buf1;
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

/*
=============================

class Editor_Increase_Height_Tool

=============================
*/

/*
===========
Editor_Increase_Height_Tool::handle_click_impl()

increase the height of the current field by one,
this increases the height of the surrounding fields also
if this is needed.
===========
*/
int Editor_Increase_Height_Tool::handle_click_impl(const Coords* coordinates, Field* field, Map* map, Editor_Interactive* parent) {
   MapRegion mrc(map, *coordinates, parent->get_fieldsel_radius());
	Coords c;

   int max, i;
   max=0;
   while(mrc.next(&c)) {
      i=map->change_field_height(c, m_changed_by);
      if(i>max) max=i;
   }
   return parent->get_fieldsel_radius()+max;
}

/*
=============================

class Editor_Decrease_Height_Tool

=============================
*/

/*
===========
Editor_Decrease_Height_Tool::handle_click_impl()

decrease the height of the current field by one,
this decreases the height of the surrounding fields also
if this is needed.
===========
*/
int Editor_Decrease_Height_Tool::handle_click_impl(const Coords* coordinates, Field* field, Map* map, Editor_Interactive* parent) {
   MapRegion mrc(map, *coordinates, parent->get_fieldsel_radius());
   Coords c;

   int max,i;
   max=0;
   while(mrc.next(&c)) {
      i=map->change_field_height(c, -m_changed_by);
      if(i>max) max=i;
   }
   return parent->get_fieldsel_radius()+max;
}

/*
=============================

class Editor_Set_Height_Tool

=============================
*/

/*
===========
Editor_Set_Height_Tool::handle_click_impl()

sets the height of the current to a fixed value,
this decreases the height of the surrounding fields also
if this is needed.
===========
*/
int Editor_Set_Height_Tool::handle_click_impl(const Coords* coordinates, Field* field, Map* map, Editor_Interactive* parent)
{
   MapRegion mrc(map, *coordinates, parent->get_fieldsel_radius());
   Coords c;
   int i, max;
   max=0;
   while(mrc.next(&c)) {
      i=map->set_field_height(c, m_set_to);
      if(i>max) max=i;
   }

   return parent->get_fieldsel_radius()+max;
}

/*
=============================

class Editor_Noise_Height_Tool

=============================
*/

/*
===========
Editor_Noise_Height_Tool::handle_click_impl()

sets the height of the current to a random value,
this decreases the height of the surrounding fields also
if this is needed.
===========
*/
int Editor_Noise_Height_Tool::handle_click_impl(const Coords* coordinates, Field* field, Map* map, Editor_Interactive* parent) {
   MapRegion mrc(map, *coordinates, parent->get_fieldsel_radius());
   Coords c;

   int i, max;
   max=0;
   while(mrc.next(&c)) {
      int j=m_lower_value+(int) ((double)(m_upper_value-m_lower_value)*rand()/(RAND_MAX+1.0));
      i=map->set_field_height(c, j);
      if(i>max) max=i;
   }
   return parent->get_fieldsel_radius()+max;
}

/*
=============================

class Set_Terrain

=============================
*/

/*
===========
Set_Terrain::set_terrain()

This changes the terrain in the given area. It randoms through all selected
terrains.
===========
*/
int Set_Terrain::set_terrain(const Coords* coordinates, Field* field, Map* map, Editor_Interactive* parent, bool right, bool down) {
   if(!get_nr_enabled()) return parent->get_fieldsel_radius();
   MapRegion mrc(map, *coordinates, parent->get_fieldsel_radius());
   Coords c;
   
   int i, j, max;
   max=0;
   while(mrc.next(&c)) {
      i=j=0;
      if(right) 
         i=map->change_field_terrain(c,get_random_enabled(),false,true);
      if(down)
         j=map->change_field_terrain(c,get_random_enabled(),true,false);
      if(i>max) max=i;
      if(j>max) max=j;
   }
   return parent->get_fieldsel_radius()+max;
}

/*
=============================

class Editor_Place_Immovable_Tool

=============================
*/

/*
===========
Editor_Place_Immovable_Tool::handle_click_impl()

choses an object to place randomly from all enabled
and places this on the current field
===========
*/
int Editor_Place_Immovable_Tool::handle_click_impl(const Coords* coordinates, Field* field, Map* map, Editor_Interactive* parent) {
   if(!get_nr_enabled()) return parent->get_fieldsel_radius();

   MapRegion mrc(map, *coordinates, parent->get_fieldsel_radius());
   Coords c;

   while(mrc.next(&c)) {
      Field *f = parent->get_map()->get_field(c);
      if (f->get_immovable()) {
         if(f->get_immovable()->get_size() != BaseImmovable::NONE)
            continue;
      }

		parent->get_editor()->create_immovable(c, get_random_enabled());
   }
   return parent->get_fieldsel_radius()+2;
}

/*
=================================================

class Editor_Delete_Immovable_Tool

=================================================
*/

/*
===========
Editor_Delete_Immovable_Tool::handle_click_impl()

deletes the immovable at the given location
===========
*/
int Editor_Delete_Immovable_Tool::handle_click_impl(const Coords* coordinates, Field* field, Map* map, Editor_Interactive* parent) {
   MapRegion mrc(map, *coordinates, parent->get_fieldsel_radius());
   Coords c;

   while(mrc.next(&c)) {
      Field *f = parent->get_map()->get_field(c);
      BaseImmovable* mim=f->get_immovable();
      if (mim) {
         mim->remove(parent->get_editor());
      }
   }
   return parent->get_fieldsel_radius()+2;
}
