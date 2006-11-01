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

#include "editor_tool_options_menu.h"
#include "editorinteractive.h"

/*
=================================================

class Editor_Tool_Options_Menu

=================================================
*/

/*
===========
Editor_Tool_Options_Menu::Editor_Tool_Options_Menu()

constructor
===========
*/
Editor_Tool_Options_Menu::Editor_Tool_Options_Menu(Editor_Interactive* parent, int index,
													UI::UniqueWindow::Registry* registry, const char* title) :
   UI::UniqueWindow(parent, registry, 100, 100, title)
{
   m_parent=parent;
   m_index=index;

	if (get_usedefaultpos())
		move_to_mouse();
}

/*
===========
Editor_Tool_Options_Menu::~Editor_Tool_Options_Menu()

destructor
===========
*/
Editor_Tool_Options_Menu::~Editor_Tool_Options_Menu()
{
}

/*
 * Selects the correct tool from the parent.
 * This is needed when a selection was made in the
 * options menus
 */
void Editor_Tool_Options_Menu::select_correct_tool(void) {
   m_parent->select_tool(m_index, 0);
}
