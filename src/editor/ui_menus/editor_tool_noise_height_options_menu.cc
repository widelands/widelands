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

#include "editor_tool_noise_height_options_menu.h"

#include "editorinteractive.h"
#include "editor_increase_height_tool.h"
#include "editor_decrease_height_tool.h"
#include "editor_noise_height_tool.h"
#include "graphic.h"
#include "i18n.h"

#include <stdio.h>

using Widelands::Field;

#define width  20
#define height 20
Editor_Tool_Noise_Height_Options_Menu::Editor_Tool_Noise_Height_Options_Menu
	(Editor_Interactive         & parent,
	 Editor_Noise_Height_Tool   & noise_tool,
	 UI::UniqueWindow::Registry & registry)
:
Editor_Tool_Options_Menu
	(parent, registry, 200, 115, _("Noise Height Options")),
m_lower_label
	(this,
	 hmargin(),
	 vmargin(), (get_inner_w() - 2 * hmargin() - spacing()) / 2, height,
	 Align_BottomCenter),
m_upper_label
	(this,
	 m_lower_label.get_x() + m_lower_label.get_w() + spacing(),
	 m_lower_label.get_y(),
	 m_lower_label.get_w(), height,
	 Align_BottomCenter),
m_lower_increase
	(this,
	 hmargin() + (get_inner_w() - 2 * hmargin() - hspacing() - 4 * width) / 4,
	 m_lower_label.get_y() + m_lower_label.get_h() + vspacing(),
	 width, height,
	 0,
	 g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png"),
	 &Editor_Tool_Noise_Height_Options_Menu::clicked_button, this,
	 Lower_Increase),
m_lower_decrease
	(this,
	 m_lower_increase.get_x() + m_lower_increase.get_w(),
	 m_lower_increase.get_y(),
	 width, height,
	 0,
	 g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png"),
	 &Editor_Tool_Noise_Height_Options_Menu::clicked_button, this,
	 Lower_Decrease),
m_upper_increase
	(this,
	 m_lower_decrease.get_x() + width
	 +
	 (get_inner_w() - 2 * hmargin() - hspacing() - 4 * width) / 2 + hspacing(),
	 m_lower_decrease.get_y(),
	 width, height,
	 0,
	 g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png"),
	 &Editor_Tool_Noise_Height_Options_Menu::clicked_button, this,
	 Upper_Increase),
m_upper_decrease
	(this,
	 m_upper_increase.get_x() + m_upper_increase.get_w(),
	 m_upper_increase.get_y(),
	 width, height,
	 0,
	 g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png"),
	 &Editor_Tool_Noise_Height_Options_Menu::clicked_button, this,
	 Upper_Decrease),
m_set_label
	(this,
	 hspacing(),
	 m_upper_decrease.get_y() + m_upper_decrease.get_h() + vspacing(),
	 get_inner_w() - 2 * hspacing(), height,
	 Align_BottomCenter),
m_set_increase
	(this,
	 get_inner_w() / 2 - width,
	 m_set_label.get_y() + m_set_label.get_h() + vspacing(),
	 width, height,
	 1,
	 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
	 &Editor_Tool_Noise_Height_Options_Menu::clicked_button, this,
	 Set_To_Increase),
m_set_decrease
	(this,
	 get_inner_w() / 2, m_set_increase.get_y(), width, height,
	 1,
	 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
	 &Editor_Tool_Noise_Height_Options_Menu::clicked_button, this,
	 Set_To_Decrease),
m_noise_tool(noise_tool)
{update();}

/**
 * Update all textareas
*/
void Editor_Tool_Noise_Height_Options_Menu::update() {
	char buffer[200];
	const interval<Field::Height> height_interval = m_noise_tool.get_interval();
	snprintf(buffer, sizeof(buffer), _("Minimum: %u"), height_interval.min);
	m_lower_label.set_text(buffer);
	snprintf(buffer, sizeof(buffer), _("Maximum: %u"), height_interval.max);
	m_upper_label.set_text(buffer);

	snprintf
		(buffer, sizeof(buffer),
		 _("Set value: %u"), m_noise_tool.set_tool().get_interval().min);
	m_set_label.set_text(buffer);

	select_correct_tool();
}


void Editor_Tool_Noise_Height_Options_Menu::clicked_button(const Button n) {
	interval<Field::Height> height_interval = m_noise_tool.get_interval();
	Field::Height set_to = m_noise_tool.set_tool().get_interval().min;
	switch (n) {
	case Lower_Increase:
		height_interval.min += height_interval.min  < MAX_FIELD_HEIGHT;
		height_interval.max = std::max(height_interval.min, height_interval.max);
		break;
	case Lower_Decrease: height_interval.min -= 0 < height_interval.min; break;
	case Upper_Increase:
		height_interval.max += height_interval.max  < MAX_FIELD_HEIGHT;
		break;
	case Upper_Decrease:
		if (0 < height_interval.max) {
			if (--height_interval.max < height_interval.min)
				height_interval.min = height_interval.max;
		}
		break;
	case Set_To_Increase: set_to +=     set_to < MAX_FIELD_HEIGHT; break;
	case Set_To_Decrease: set_to -= 0 < set_to;                    break;
	default:
		assert(false);
	}

	m_noise_tool.set_interval(height_interval);
	m_noise_tool.set_tool()
		.set_interval(interval<Field::Height>(set_to, set_to));

	update();
}
