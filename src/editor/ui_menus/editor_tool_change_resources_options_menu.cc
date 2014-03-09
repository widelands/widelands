/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#include "editor/ui_menus/editor_tool_change_resources_options_menu.h"

#include <cstdio>

#include "editor/editorinteractive.h"
#include "editor/tools/editor_increase_resources_tool.h"
#include "editor/tools/editor_set_resources_tool.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "logic/map.h"
#include "logic/widelands.h"
#include "logic/world.h"
#include "ui_basic/button.h"
#include "wui/overlay_manager.h"

const static int BUTTON_WIDTH = 20;
const static int BUTTON_HEIGHT = 20;

Editor_Tool_Change_Resources_Options_Menu::
Editor_Tool_Change_Resources_Options_Menu
		(Editor_Interactive             & parent,
		 Editor_Increase_Resources_Tool & increase_tool,
		 UI::UniqueWindow::Registry     & registry)
	:
	Editor_Tool_Options_Menu
		(parent, registry, 164, 120, _("Resources")),
	m_change_by_label
		(this,
		 hmargin(), vmargin(), get_inner_w() - 2 * hmargin(), BUTTON_HEIGHT,
		 _("Increase/Decrease Value"), UI::Align_BottomCenter),
	m_change_by_increase
		(this, "incr_change_by",
		 get_inner_w() - hmargin() - BUTTON_WIDTH,
		 m_change_by_label.get_y() + m_change_by_label.get_h() + spacing(),
		 BUTTON_WIDTH, BUTTON_HEIGHT,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_up.png")),
	m_change_by_decrease
		(this, "decr_change_by",
		 hmargin(),
		 m_change_by_increase.get_y(),
		 BUTTON_WIDTH, BUTTON_HEIGHT,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_down.png")),
	m_change_by_value
		(this,
		 m_change_by_increase.get_x() + m_change_by_increase.get_w() +
		 hspacing(),
		 m_change_by_increase.get_y(),
		 m_change_by_decrease.get_x() - hspacing()
		 -
		 (m_change_by_increase.get_x() + m_change_by_increase.get_w() +
		  hspacing()),
		 BUTTON_HEIGHT,
		 UI::Align_BottomCenter),
	m_set_to_label
		(this,
		 vmargin(),
		 m_change_by_increase.get_y() + m_change_by_increase.get_h() + vspacing(),
		 get_inner_w() - 2 * hmargin(), BUTTON_HEIGHT,
		 _("Set Value"), UI::Align_BottomCenter),
	m_set_to_increase
		(this, "incr_set_to",
		 m_change_by_increase.get_x(),
		 m_set_to_label.get_y() + m_set_to_label.get_h() + vspacing(),
		 BUTTON_WIDTH, BUTTON_HEIGHT,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_up.png")),
	m_set_to_decrease
		(this, "decr_set_to",
		 hmargin(),
		 m_set_to_increase.get_y(), BUTTON_WIDTH, BUTTON_HEIGHT,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_down.png")),
	m_set_to_value
		(this,
		 m_change_by_value.get_x(), m_set_to_increase.get_y(),
		 m_change_by_value.get_w(), BUTTON_HEIGHT,
		 UI::Align_BottomCenter),
	m_cur_selection(this, 0, 0, _("Current Selection"), UI::Align_BottomCenter),
	m_increase_tool(increase_tool)
{
	m_change_by_increase.sigclicked.connect
		(boost::bind
			(&Editor_Tool_Change_Resources_Options_Menu::clicked_button,
			 boost::ref(*this),
			 Change_By_Increase));
	m_change_by_decrease.sigclicked.connect
		(boost::bind
			(&Editor_Tool_Change_Resources_Options_Menu::clicked_button,
			 boost::ref(*this),
			 Change_By_Decrease));
	m_set_to_increase.sigclicked.connect
		(boost::bind
			(&Editor_Tool_Change_Resources_Options_Menu::clicked_button,
			 boost::ref(*this),
			 Set_To_Increase));
	m_set_to_decrease.sigclicked.connect
		(boost::bind
			(&Editor_Tool_Change_Resources_Options_Menu::clicked_button,
			 boost::ref(*this),
			 Set_To_Decrease));

	m_change_by_increase.set_repeating(true);
	m_change_by_decrease.set_repeating(true);
	m_set_to_increase   .set_repeating(true);
	m_set_to_decrease   .set_repeating(true);
	const Widelands::World & world = parent.egbase().map().world();
	Widelands::Resource_Index const nr_resources = world.get_nr_resources();

	//  Find the maximal width and height for the resource pictures.
	uint16_t resource_pic_max_width = 0, resource_pic_max_height = 0;
	for (Widelands::Resource_Index i = 0; i < nr_resources; ++i) {
		const Image* pic = g_gr->images().get(world.get_resource(i)->get_editor_pic(100000));
		resource_pic_max_width  = std::max(resource_pic_max_width,  pic->width());
		resource_pic_max_height = std::max(resource_pic_max_height, pic->height());
	}

	const uint16_t resources_in_row =
		(get_inner_w() - 2 * hmargin() + spacing())
		/
		(resource_pic_max_width + spacing());

	m_radiogroup.changed.connect
		(boost::bind(&Editor_Tool_Change_Resources_Options_Menu::selected, this));
	m_radiogroup.clicked.connect
		(boost::bind(&Editor_Tool_Change_Resources_Options_Menu::selected, this));

	uint16_t cur_x = 0;
	Point pos
		(hmargin(), m_set_to_value.get_y() + m_set_to_value.get_h() + vspacing());
	for
		(Widelands::Resource_Index i = 0;
		 i < nr_resources;
		 pos.x += resource_pic_max_width + hspacing(), ++cur_x, ++i)
	{
		if (cur_x == resources_in_row) {
			cur_x = 0;
			pos.x = hmargin();
			pos.y += resource_pic_max_height + vspacing();
		}
		m_radiogroup.add_button
			(this,
			 pos,
			 g_gr->images().get(world.get_resource(i)->get_editor_pic(100000)));
	}
	pos.y += resource_pic_max_height + vspacing();

	set_inner_size(get_inner_w(), pos.y + m_cur_selection.get_h() + vmargin());
	m_cur_selection.set_pos(Point(get_inner_w() / 2, pos.y + hspacing()));

	m_radiogroup.set_state(m_increase_tool.get_cur_res());

	update();
}


void Editor_Tool_Change_Resources_Options_Menu::clicked_button(Button const n)
{
	assert
		(m_increase_tool.get_change_by()
		 ==
		 m_increase_tool.decrease_tool().get_change_by());

	int32_t change_by = m_increase_tool.get_change_by();
	int32_t set_to    = m_increase_tool.set_tool().get_set_to();

	switch (n) {
	case Change_By_Increase: change_by += change_by < 63; break;
	case Change_By_Decrease: change_by -= 1 < change_by;  break;
	case    Set_To_Increase: set_to    += set_to    < 63; break;
	case    Set_To_Decrease: set_to    -= 0 < set_to;     break;
	default:
		assert(false);
		break;
	}
	m_increase_tool.set_change_by(change_by);
	m_increase_tool.decrease_tool().set_change_by(change_by);
	m_increase_tool.set_tool().set_set_to(set_to);

	select_correct_tool();
	update();
}

/**
 * called when a resource has been selected
 */
void Editor_Tool_Change_Resources_Options_Menu::selected() {
	const int32_t n = m_radiogroup.get_state();

	m_increase_tool.set_tool().set_cur_res(n);
	m_increase_tool.set_cur_res(n);
	m_increase_tool.decrease_tool().set_cur_res(n);

	Widelands::Map & map = ref_cast<Editor_Interactive, UI::Panel>(*get_parent())
		.egbase().map();
	map.overlay_manager().register_overlay_callback_function(
	   boost::bind(&Editor_Change_Resource_Tool_Callback, _1, boost::ref(map), n));
	map.recalc_whole_map();
	select_correct_tool();

	update();
}

/**
 * Update all the textareas, so that they represent the correct values
*/
void Editor_Tool_Change_Resources_Options_Menu::update() {
	char buf[250];
	sprintf(buf, "%i", m_increase_tool.get_change_by());
	m_change_by_value.set_text(buf);
	sprintf(buf, "%i", m_increase_tool.set_tool().get_set_to());
	m_set_to_value.set_text(buf);

	m_cur_selection.set_text
		(ref_cast<Editor_Interactive, UI::Panel>(*get_parent()).egbase().map()
		 .world().get_resource(m_increase_tool.set_tool().get_cur_res())->descname());
	m_cur_selection.set_pos
		(Point
		 	((get_inner_w() - m_cur_selection.get_w()) / 2, get_inner_h() - 20));

	{
		int32_t const change_by = m_increase_tool.get_change_by();
		m_change_by_decrease.set_enabled(1 < change_by);
		m_change_by_increase.set_enabled    (change_by < 63);
	}
	{
		int32_t const set_to   = m_increase_tool.set_tool().get_set_to();
		m_set_to_decrease   .set_enabled(0 < set_to);
		m_set_to_increase   .set_enabled    (set_to    < 63);
	}
}
