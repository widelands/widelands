/*
 * Copyright (C) 2002-2007 by the Widelands Development Team
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
#include "graphic.h"
#include "i18n.h"
#include "editor_tool.h"

#include <stdint.h>
#include <stdio.h>

/*
===============
Editor_Toolsize_Menu::Editor_Toolsize_Menu

Create all the buttons etc...
===============
*/
Editor_Toolsize_Menu::Editor_Toolsize_Menu(Editor_Interactive *parent, UI::UniqueWindow::Registry *registry)
:
UI::UniqueWindow(parent, registry, 160, 50, _("Toolsize Menu")),

m_textarea(this, 5, 5, 150, 10, std::string(), Align_HCenter),

m_increase
(this,
 60, 25, 20, 20,
 0,
 g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png"),
 &Editor_Toolsize_Menu::change_radius, this, true),

m_decrease
(this,
 80, 25, 20, 20,
 0,
 g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png"),
 &Editor_Toolsize_Menu::change_radius, this, false)

{
   char buf[250];
	sprintf(buf, _("Current Size: %u").c_str(), parent->get_sel_radius() + 1);
	m_textarea.set_text(buf);

	if (get_usedefaultpos())
		center_to_parent();
}


/*
===========
Editor_Toolsize_Menu::button_clicked()

called, when one of the up/down buttons is pressed
id: 0 is up, 1 is down
===========
*/
void Editor_Toolsize_Menu::change_radius(const bool increase) {
	Interactive_Base & intbase = dynamic_cast<Interactive_Base &>(*get_parent());
	const uint32_t val = intbase.get_sel_radius() +
		(increase ? 1 : std::numeric_limits<uint32_t>::max());
	if (val <= MAX_TOOL_AREA) {
		intbase.set_sel_radius(val);
   char buf[250];
		sprintf(buf, _("Current Size: %u").c_str(), val + 1);
		m_textarea.set_text(buf);
	}
}
