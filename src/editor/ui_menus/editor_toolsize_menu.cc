/*
 * Copyright (C) 2002-2009 by the Widelands Development Team
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
#include "editor/editorinteractive.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "editor/tools/editor_tool.h"

#include <cstdio>

inline Editor_Interactive & Editor_Toolsize_Menu::eia() {
	return dynamic_cast<Editor_Interactive &>(*get_parent());
}


/**
 * Create all the buttons etc...
*/
Editor_Toolsize_Menu::Editor_Toolsize_Menu
		(Editor_Interactive *parent, UI::UniqueWindow::Registry *registry)
:
UI::UniqueWindow(parent, registry, 160, 50, _("Toolsize Menu")),
m_textarea(this, 5, 5, 150, 10, std::string(), Align_HCenter),
m_increase
	(this,
	 60, 25, 20, 20,
	 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
	 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
	 &Editor_Toolsize_Menu::increase_radius, *this,
	 std::string(),
	 parent->get_sel_radius() < MAX_TOOL_AREA),
m_decrease
	(this,
	 80, 25, 20, 20,
	 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
	 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
	 &Editor_Toolsize_Menu::decrease_radius, *this,
	 std::string(),
	 0 < parent->get_sel_radius())
{
	m_increase.set_repeating(true);
	m_decrease.set_repeating(true);
	update(parent->get_sel_radius());

	if (get_usedefaultpos())
		center_to_parent();
}


void Editor_Toolsize_Menu::update(uint32_t const val) {
	eia().set_sel_radius(val);
	m_decrease.set_enabled(0 < val);
	m_increase.set_enabled    (val < MAX_TOOL_AREA);
	char buffer[250];
	snprintf(buffer, sizeof(buffer), _("Current Size: %u"), val + 1);
	m_textarea.set_text(buffer);
}


void Editor_Toolsize_Menu::decrease_radius() {
	assert(0 < eia().get_sel_radius());
	update(eia().get_sel_radius() - 1);
}
void Editor_Toolsize_Menu::increase_radius() {
	assert(eia().get_sel_radius() < MAX_TOOL_AREA);
	update(eia().get_sel_radius() + 1);
}
