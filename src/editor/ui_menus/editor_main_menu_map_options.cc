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

#include <stdio.h>
#include "i18n.h"
#include "map.h"
#include "ui_textarea.h"
#include "ui_multilinetextarea.h"
#include "ui_multilineeditbox.h"
#include "ui_editbox.h"
#include "editor_main_menu_map_options.h"
#include "editorinteractive.h"

/*
===============
Main_Menu_Map_Options::Main_Menu_Map_Options

Create all the buttons etc...
===============
*/
Main_Menu_Map_Options::Main_Menu_Map_Options(Editor_Interactive *parent)
	: UIWindow(parent, (parent->get_w()-200)/2, (parent->get_h()-300)/2, 200, 300, _("Map Options"))
{
   m_parent=parent;

   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, _("New Map Options"), Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   // UIButtons
   const int offsx=5;
   const int offsy=30;
   const int spacing=3;
   const int height=20;
   int posx=offsx;
   int posy=offsy;
   UITextarea* ta= new UITextarea(this, posx, posy+5, _("Map Name:"), Align_Left);
   m_name=new UIEdit_Box(this, posx+ta->get_w()+spacing, posy, get_inner_w()-(posx+ta->get_w()+spacing)-spacing, 20, 1, 0);
   m_name->changedid.set(this, &Main_Menu_Map_Options::changed);
   posy+=height+spacing;
   ta=new UITextarea(this, posx, posy+5, _("Size:"));
   m_size=new UITextarea(this, posx+ta->get_w()+spacing, posy+5, "512x512", Align_Left);
   posy+=height+spacing;
   ta=new UITextarea(this, posx, posy+5, _("Nr Players:"));
   m_nrplayers=new UITextarea(this, posx+ta->get_w()+spacing, posy+5, "4", Align_Left);
   posy+=height+spacing;
   ta=new UITextarea(this, posx, posy+5, _("World:"));
   m_world=new UITextarea(this, posx+ta->get_w()+spacing, posy+5, "\"Greenland\"", Align_Left);
   posy+=height+spacing;
   ta=new UITextarea(this, posx, posy+5, _("Author:"), Align_Left);
   m_author=new UIEdit_Box(this, posx+ta->get_w()+spacing, posy, get_inner_w()-(posx+ta->get_w()+spacing)-spacing, 20, 1, 1);
   m_author->changedid.set(this, &Main_Menu_Map_Options::changed);
   posy+=height+spacing;
   m_descr=new UIMultiline_Editbox(this, posx, posy, get_inner_w()-spacing-posx, get_inner_h()-spacing-posy, _("Nothing defined!"));
   m_descr->changed.set(this, &Main_Menu_Map_Options::editbox_changed);
   update();
}

/*
===============
Main_Menu_Map_Options::update()

Updates all UITextareas in the UIWindow to represent currently
set values
==============
*/
void Main_Menu_Map_Options::update(void) {
   Map* map=m_parent->get_map();

   char buf[200];
   sprintf(buf, "%ix%i", map->get_width(), map->get_height());
   m_size->set_text(buf);
   m_author->set_text(map->get_author());
   m_name->set_text(map->get_name());
   sprintf(buf, "%i", map->get_nrplayers());
   m_nrplayers->set_text(buf);
   m_world->set_text(map->get_world_name());
   m_descr->set_text(map->get_description());
}



/*
===============
Main_Menu_Map_Options::~Main_Menu_Map_Options

Unregister from the registry pointer
===============
*/
Main_Menu_Map_Options::~Main_Menu_Map_Options()
{
}

/*
===========
Main_Menu_Map_Options::changed()

called when one of the editboxes are changed
===========
*/
void Main_Menu_Map_Options::changed(int id) {
   if(id==0) {
      // name
      m_parent->get_map()->set_name(m_name->get_text());
   } else if(id==1) {
      // author
      m_parent->get_map()->set_author(m_author->get_text());
   }
   update();
}

/*
 * called when the editbox has changed
 */
void Main_Menu_Map_Options::editbox_changed(void) {
   m_parent->get_map()->set_description(m_descr->get_text().c_str());
}

