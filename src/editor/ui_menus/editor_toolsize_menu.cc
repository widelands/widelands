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

#include "editor_toolsize_menu.h"
#include "editorinteractive.h"
#include "ui_textarea.h"
#include "ui_button.h"
#include "editor_tool.h"

/*
===============
Editor_Toolsize_Menu::Editor_Toolsize_Menu

Create all the buttons etc...
===============
*/
Editor_Toolsize_Menu::Editor_Toolsize_Menu(Editor_Interactive *parent, UIUniqueWindowRegistry *registry)
	: UIUniqueWindow(parent, registry, 160, 65, "Toolsize Menu")
{
   m_parent=parent;

   new UITextarea(this, 15, 5, "Set Tool Size Menu", Align_Left);
   char buf[250];
   sprintf(buf, "Current Size: %i", m_parent->get_fieldsel_radius()+1);
   m_textarea=new UITextarea(this, 25, 25, buf);

   int bx=60;
   UIButton* b = new UIButton(this, bx, 40, 20, 20, 0, 0);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Editor_Toolsize_Menu::button_clicked);
   b=new UIButton(this, bx+20, 40, 20, 20, 0, 1);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Editor_Toolsize_Menu::button_clicked);

	if (get_usedefaultpos())
		center_to_parent();
}

/*
===============
Editor_Toolsize_Menu::~Editor_Toolsize_Menu

Unregister from the registry pointer
===============
*/
Editor_Toolsize_Menu::~Editor_Toolsize_Menu()
{
}

/*
===========
Editor_Toolsize_Menu::button_clicked()

called, when one of the up/down buttons is pressed
id: 0 is up, 1 is down
===========
*/
void Editor_Toolsize_Menu::button_clicked(int n) {
   int val=m_parent->get_fieldsel_radius();
   if(n==0) {
      ++val;
      if(val>MAX_TOOL_AREA) val=MAX_TOOL_AREA;
   } else if(n==1) {
      --val;
      if(val<0) val=0;
   }
   m_parent->set_fieldsel_radius(val);

   char buf[250];
   sprintf(buf, "Current Size: %i", m_parent->get_fieldsel_radius()+1);
   m_textarea->set_text(buf);
}

