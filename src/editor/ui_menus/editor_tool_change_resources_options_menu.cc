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

#include "editor_tool_change_resources_options_menu.h"

#include "editorinteractive.h"
#include "i18n.h"
#include "ui_button.h"
#include "editor_increase_resources_tool.h"
#include "editor_set_resources_tool.h"
#include "map.h"
#include "world.h"
#include "graphic.h"
#include "overlay_manager.h"

#include <stdio.h>

using Widelands::Resource_Descr;

#define width  20
#define height 20
Editor_Tool_Change_Resources_Options_Menu::Editor_Tool_Change_Resources_Options_Menu
(Editor_Interactive             & parent,
 Editor_Increase_Resources_Tool & increase_tool,
 UI::UniqueWindow::Registry     & registry)
:
Editor_Tool_Options_Menu
(parent, registry, 164, 120, _("Resources Tools Options")),

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
 &Editor_Tool_Change_Resources_Options_Menu::clicked_button,
 this, Change_By_Increase),

m_change_by_decrease
(this,
 get_inner_w() - hmargin() - width, m_change_by_increase.get_y(), width, height,
 1,
 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
 &Editor_Tool_Change_Resources_Options_Menu::clicked_button,
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
 &Editor_Tool_Change_Resources_Options_Menu::clicked_button,
 this, Set_To_Increase),

m_set_to_decrease
(this,
 m_change_by_decrease.get_x(), m_set_to_increase.get_y(), width, height,
 1,
 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
 &Editor_Tool_Change_Resources_Options_Menu::clicked_button,
 this, Set_To_Decrease),

m_set_to_value
(this,
 m_change_by_value.get_x(), m_set_to_increase.get_y(),
 m_change_by_value.get_w(), height,
 Align_BottomCenter),

m_cur_selection
(this, 0, 0, _("Current Selection"), Align_BottomCenter),

m_increase_tool(increase_tool)
{
	Widelands::World const & world = parent.egbase().map().world();
	const Resource_Descr::Index nr_resources = world.get_nr_resources();

	//  Find the maximal width and height for the resource pictures.
	uint32_t resource_pic_max_width = 0, resource_pic_max_height = 0;
	for (Resource_Descr::Index i = 0; i < nr_resources; ++i) {
		uint32_t w, h;
		g_gr->get_picture_size
			(g_gr->get_picture
			 	(PicMod_Game,
			 	 world.get_resource(i)->get_editor_pic(100000).c_str()),
			 w, h);
		resource_pic_max_width  = std::max(resource_pic_max_width,  w);
		resource_pic_max_height = std::max(resource_pic_max_height, h);
	}

	const uint32_t resources_in_row =
		(get_inner_w() - 2 * hmargin() + spacing())
		/
		(resource_pic_max_width + spacing());

	m_radiogroup.changed.set
		(this, &Editor_Tool_Change_Resources_Options_Menu::selected);
	m_radiogroup.clicked.set
		(this, &Editor_Tool_Change_Resources_Options_Menu::selected);

	uint32_t cur_x = 0;
	Point pos
		(hmargin(), m_set_to_value.get_y() + m_set_to_value.get_h() + vspacing());
	for
		(Resource_Descr::Index i = 0;
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
			 pos.x, pos.y,
			 g_gr->get_picture
			 	(PicMod_Game,
			 	 world.get_resource(i)->get_editor_pic(100000).c_str()));
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
	case Change_By_Increase:
		change_by += change_by < std::numeric_limits<int32_t>::max(); break;
	case Change_By_Decrease: change_by -= 1 < change_by;         break;
	case    Set_To_Increase:
		set_to += set_to < std::numeric_limits<int32_t>::max();       break;
	case    Set_To_Decrease: set_to    -= 0 < set_to;
	default:
		assert(false);
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

	Widelands::Map & map = dynamic_cast<Editor_Interactive &>(*get_parent())
		.egbase().map();
	map.overlay_manager().register_overlay_callback_function
		(&Editor_Change_Resource_Tool_Callback, static_cast<void *>(&map), n);
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
		(m_increase_tool.set_tool().get_cur_res() ?
		 dynamic_cast<Editor_Interactive &>(*get_parent()).egbase().map().world()
		 .get_resource(m_increase_tool.set_tool().get_cur_res())->name()
		 :
		 "");
	m_cur_selection.set_pos
		(Point
		 	((get_inner_w() - m_cur_selection.get_w()) / 2, get_inner_h() - 20));
}
