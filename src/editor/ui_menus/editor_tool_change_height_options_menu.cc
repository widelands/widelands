/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "editor/ui_menus/editor_tool_change_height_options_menu.h"

#include <cstdio>

#include "editor/editorinteractive.h"
#include "editor/tools/editor_increase_height_tool.h"
#include "editor/tools/editor_set_height_tool.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "ui_basic/button.h"

#define width  20
#define height 20
Editor_Tool_Change_Height_Options_Menu::Editor_Tool_Change_Height_Options_Menu
	(Editor_Interactive          & parent,
	 Editor_Increase_Height_Tool & increase_tool,
	 UI::UniqueWindow::Registry  & registry)
	:
	Editor_Tool_Options_Menu
		(parent, registry, 135, 135, _("Height Tools Options")),
	m_increase_tool(increase_tool),
	m_change_by_label
		(this,
		 hmargin(), vmargin(), get_inner_w() - 2 * hmargin(), height,
		 _("Increase/Decrease Value"), UI::Align_BottomCenter),
	m_change_by_increase
		(this, "incr_change_by",
		 get_inner_w() - hmargin() - width,
		 m_change_by_label.get_y() + m_change_by_label.get_h() + spacing(),
		 width, height,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_up.png"),
		 std::string(),
		 increase_tool.get_change_by() < MAX_FIELD_HEIGHT_DIFF),
	m_change_by_decrease
		(this, "decr_change_by",
		 hmargin(),
		 m_change_by_increase.get_y(),
		 width, height,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_down.png"),
		 std::string(),
		 1 < increase_tool.get_change_by()),
	m_change_by_value
		(this,
		 m_change_by_increase.get_x() + m_change_by_increase.get_w() +
		 hspacing(),
		 m_change_by_increase.get_y(),
		 m_change_by_decrease.get_x() - hspacing()
		 -
		 (m_change_by_increase.get_x() + m_change_by_increase.get_w() +
		  hspacing()),
		 height,
		 UI::Align_BottomCenter),
	m_set_to_label
		(this,
		 vmargin(),
		 m_change_by_increase.get_y() + m_change_by_increase.get_h() +
		 vspacing(),
		 get_inner_w() - 2 * hmargin(), height,
		 _("Set Value"), UI::Align_BottomCenter),
	m_set_to_increase
		(this, "incr_set_to",
		 m_change_by_increase.get_x(),
		 m_set_to_label.get_y() + m_set_to_label.get_h() + vspacing(),
		 width, height,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_up.png"),
		 std::string(),
		 increase_tool.set_tool().get_interval().min < MAX_FIELD_HEIGHT),
	m_set_to_decrease
		(this, "decr_set_to",
		 hmargin(),
		 m_set_to_increase.get_y(),
		 width, height,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_down.png"),
		 std::string(),
		 0 < increase_tool.set_tool().get_interval().min),
	m_set_to_value
		(this,
		 m_change_by_value.get_x(), m_set_to_increase.get_y(),
		 m_change_by_value.get_w(), height,
		 UI::Align_BottomCenter)
{
	m_change_by_increase.sigclicked.connect
		(boost::bind
		 (&Editor_Tool_Change_Height_Options_Menu::clicked_change_by_increment, boost::ref(*this)));
	m_change_by_decrease.sigclicked.connect
		(boost::bind
		 (&Editor_Tool_Change_Height_Options_Menu::clicked_change_by_decrement, boost::ref(*this)));
	m_set_to_increase.sigclicked.connect
		(boost::bind(&Editor_Tool_Change_Height_Options_Menu::clicked_setto_increment, boost::ref(*this)));
	m_set_to_decrease.sigclicked.connect
		(boost::bind(&Editor_Tool_Change_Height_Options_Menu::clicked_setto_decrement, boost::ref(*this)));

	m_change_by_increase.set_repeating(true);
	m_change_by_decrease.set_repeating(true);
	m_set_to_increase   .set_repeating(true);
	m_set_to_decrease   .set_repeating(true);
	update();
}


void Editor_Tool_Change_Height_Options_Menu::clicked_change_by_decrement() {
	int32_t change_by = m_increase_tool.get_change_by();
	assert(change_by == m_increase_tool.decrease_tool().get_change_by());
	assert(1 < change_by);

	--change_by;

	m_increase_tool                .set_change_by(change_by);
	m_increase_tool.decrease_tool().set_change_by(change_by);
	m_change_by_decrease.set_enabled(1 < change_by);
	m_change_by_increase.set_enabled(true);
	select_correct_tool();
	update();
}


void Editor_Tool_Change_Height_Options_Menu::clicked_change_by_increment() {
	int32_t change_by = m_increase_tool.get_change_by();
	assert(change_by == m_increase_tool.decrease_tool().get_change_by());
	assert(change_by < MAX_FIELD_HEIGHT_DIFF);

	++change_by;

	m_increase_tool                .set_change_by(change_by);
	m_increase_tool.decrease_tool().set_change_by(change_by);
	m_change_by_decrease.set_enabled(true);
	m_change_by_increase.set_enabled(change_by < MAX_FIELD_HEIGHT_DIFF);
	select_correct_tool();
	update();
}


void Editor_Tool_Change_Height_Options_Menu::clicked_setto_decrement() {
	Widelands::Field::Height setto =
		m_increase_tool.set_tool().get_interval().min;
	assert(setto == m_increase_tool.set_tool().get_interval().max);
	assert(0 < setto);

	--setto;

	m_increase_tool.set_tool().set_interval
		(interval<Widelands::Field::Height>(setto, setto));
	m_set_to_decrease.set_enabled(0 < setto);
	m_set_to_increase.set_enabled(true);
	select_correct_tool();
	update();
}


void Editor_Tool_Change_Height_Options_Menu::clicked_setto_increment() {
	Widelands::Field::Height setto =
		m_increase_tool.set_tool().get_interval().min;
	assert(setto == m_increase_tool.set_tool().get_interval().max);
	assert(setto < MAX_FIELD_HEIGHT);

	++setto;

	m_increase_tool.set_tool().set_interval
		(interval<Widelands::Field::Height>(setto, setto));
	m_set_to_decrease.set_enabled(true);
	m_set_to_increase.set_enabled(setto < MAX_FIELD_HEIGHT);
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
