/*
 * Copyright (C) 2002-2008 by the Widelands Development Team
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

#include <stdio.h>

inline Editor_Interactive & Editor_Toolsize_Menu::eia() {
	return dynamic_cast<Editor_Interactive &>(*get_parent());
}


inline static void update_label_size(UI::Textarea & ta, uint32_t const val) {
	char buffer[250];
	snprintf(buffer, sizeof(buffer), _("Current Size: %u"), val + 1);
	ta.set_text(buffer);
}

/**
 * Create all the buttons etc...
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
	 &Editor_Toolsize_Menu::increase_radius, this,
	 std::string(),
	 parent->get_sel_radius() < MAX_TOOL_AREA),
m_decrease
	(this,
	 80, 25, 20, 20,
	 0,
	 g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png"),
	 &Editor_Toolsize_Menu::decrease_radius, this,
	 std::string(),
	 0 < parent->get_sel_radius())
{
	update_label_size(m_textarea, parent->get_sel_radius());

	if (get_usedefaultpos())
		center_to_parent();
}


void Editor_Toolsize_Menu::decrease_radius() {
	assert(0 < eia().get_sel_radius());
	uint32_t const val = eia().get_sel_radius() - 1;
	m_decrease.set_enabled(0 < val);
	m_increase.set_enabled(true);
	eia().set_sel_radius(val);
	update_label_size(m_textarea, val);
}
void Editor_Toolsize_Menu::increase_radius() {
	assert(eia().get_sel_radius() < MAX_TOOL_AREA);
	uint32_t const val = eia().get_sel_radius() + 1;
	m_decrease.set_enabled(true);
	m_increase.set_enabled(val < MAX_TOOL_AREA);
	eia().set_sel_radius(val);
	update_label_size(m_textarea, val);
}
