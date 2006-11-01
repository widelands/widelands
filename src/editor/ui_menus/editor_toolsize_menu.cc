/*
 * Copyright (C) 2002-2006 by the Widelands Development Team
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
#include "editor_toolsize_menu.h"
#include "editorinteractive.h"
#include "graphic.h"
#include "i18n.h"
#include "ui_textarea.h"
#include "ui_button.h"
#include "editor_tool.h"

/*
===============
Editor_Toolsize_Menu::Editor_Toolsize_Menu

Create all the buttons etc...
===============
*/
Editor_Toolsize_Menu::Editor_Toolsize_Menu(Editor_Interactive *parent, UI::UniqueWindow::Registry *registry)
	: UI::UniqueWindow(parent, registry, 160, 65, _("Toolsize Menu"))
{
   m_parent=parent;

   new UI::Textarea(this, 15, 0, _("Set Tool Size Menu"), Align_Left);
   char buf[250];
   sprintf(buf, _("Current Size: %i").c_str(), m_parent->get_fieldsel_radius()+1);
   m_textarea=new UI::Textarea(this, 25, 20, buf);

   int bx=60;
   UI::Button* b = new UI::Button(this, bx, 40, 20, 20, 0, 0);
   b->set_pic(g_gr->get_picture( PicMod_UI,  "pics/scrollbar_up.png" ));
   b->clickedid.set(this, &Editor_Toolsize_Menu::button_clicked);
   b=new UI::Button(this, bx+20, 40, 20, 20, 0, 1);
   b->set_pic(g_gr->get_picture( PicMod_UI,  "pics/scrollbar_down.png" ));
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
   sprintf(buf, "%s: %i", _("Current Size").c_str(), m_parent->get_fieldsel_radius()+1);
   m_textarea->set_text(buf);
}
