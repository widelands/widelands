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

#include "editor_objectives_menu.h"
#include "editorinteractive.h"
#include "ui_unique_window.h"
#include "ui_textarea.h"
#include "ui_button.h"
#include "ui_listselect.h"
#include "ui_modal_messagebox.h"
#include "error.h"
#include "map.h"

/*
===============
Editor_Objectives_Menu::Editor_Objectives_Menu

Create all the buttons etc...
===============
*/
Editor_Objectives_Menu::Editor_Objectives_Menu(Editor_Interactive *parent, UIUniqueWindowRegistry *registry)
	: UIUniqueWindow(parent, registry, 400, 300, "Objectives Menu")
{
   m_parent=parent;

   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, "Objectives Menu", Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   const int offsx=5;
   const int offsy=25;
   const int spacing=5;
   int posx=offsx;
   int posy=offsy;

	// Put in the default position, if necessary
	if (get_usedefaultpos())
		center_to_parent();
}

/*
 * destructor
 */
Editor_Objectives_Menu::~Editor_Objectives_Menu( void ) {
}
