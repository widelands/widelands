/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#include "editor_tool_change_height_options_menu.h"

#include "editor_increase_height_tool.h"
#include "editor_set_height_tool.h"
#include "editorinteractive.h"
#include "graphic.h"
#include "i18n.h"

#include "ui_button.h"

#include <stdio.h>


#define width  20
#define height 20
Editor_Tool_Change_Height_Options_Menu::Editor_Tool_Change_Height_Options_Menu
(Editor_Interactive          & parent,
 Editor_Increase_Height_Tool & increase_tool,
 UI::UniqueWindow::Registry  & registry)
:
Editor_Tool_Options_Menu
(parent, registry, 135, 135, _("Height Tools Options").c_str()),

m_change_by_label
(this,
 hmargin(), vmargin(), get_inner_w() - 2 * hmargin(), height,
 _("In-/Decrease Value"), Align_BottomCenter),

m_change_by_increase
(this,
 hmargin(), m_change_by_label.get_y() + m_change_by_label.get_h() + spacing(),
 width, height,
 1,
 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
 &Editor_Tool_Change_Height_Options_Menu::clicked_button,
 this, Change_By_Increase),

m_change_by_decrease
(this,
 get_inner_w() - hmargin() - width, m_change_by_increase.get_y(), width, height,
 1,
 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
 &Editor_Tool_Change_Height_Options_Menu::clicked_button,
 this, Change_By_Decrease),

m_change_by_value
(this,
 m_change_by_increase.get_x() + m_change_by_increase.get_w() + hspacing(),
 m_change_by_increase.get_y(),
 m_change_by_decrease.get_x() - hspacing()
 -
 (m_change_by_increase.get_x() + m_change_by_increase.get_w() + hspacing()),
 height,
 Align_BottomCenter),

m_set_to_label
(this,
 vmargin(),
 m_change_by_increase.get_y() + m_change_by_increase.get_h() + vspacing(),
 get_inner_w() - 2 * hmargin(), height,
 _("Set Value"), Align_BottomCenter),

m_set_to_increase
(this,
 hmargin(), m_set_to_label.get_y() + m_set_to_label.get_h() + vspacing(),
 width, height,
 1,
 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
 &Editor_Tool_Change_Height_Options_Menu::clicked_button,
 this, Set_To_Increase),

m_set_to_decrease
(this,
 m_change_by_decrease.get_x(), m_set_to_increase.get_y(), width, height,
 1,
 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
 &Editor_Tool_Change_Height_Options_Menu::clicked_button,
 this, Set_To_Decrease),

m_set_to_value
(this,
 m_change_by_value.get_x(), m_set_to_increase.get_y(),
 m_change_by_value.get_w(), height,
 Align_BottomCenter),

m_increase_tool(increase_tool)
{update();}


void Editor_Tool_Change_Height_Options_Menu::clicked_button(const Button n) {
	assert
		(m_increase_tool                .get_change_by()
		 ==
		 m_increase_tool.decrease_tool().get_change_by());

	int32_t change_by = m_increase_tool.get_change_by();
	Widelands::Field::Height set_to =
		m_increase_tool.set_tool().get_interval().min;
	switch (n) {
	case Change_By_Increase:
		change_by +=     change_by < MAX_FIELD_HEIGHT_DIFF;
		break;
	case Change_By_Decrease: change_by -= 1 < change_by;                 break;
	case    Set_To_Increase: set_to    +=     set_to < MAX_FIELD_HEIGHT; break;
	case    Set_To_Decrease: set_to    -= 0 < set_to;
	default:
		assert(false);
	}

	m_increase_tool                .set_change_by(change_by);
	m_increase_tool.decrease_tool().set_change_by(change_by);
	m_increase_tool.set_tool()
		.set_interval(interval<Widelands::Field::Height>(set_to, set_to));

	select_correct_tool();
	update();
}

/// Update all the textareas, so that they represent the correct values.
void Editor_Tool_Change_Height_Options_Menu::update() {
	char buf[250];
	sprintf(buf, "%i", m_increase_tool.get_change_by());
	m_change_by_value.set_text(buf);
	sprintf(buf, "%i", m_increase_tool.set_tool().get_interval().min);
	m_set_to_value.set_text(buf);
}
