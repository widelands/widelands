/*
 * Copyright (C) 2002-2004, 2006-2008, 2010 by the Widelands Development Team
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

#include "editor/ui_menus/editor_tool_place_immovable_options_menu.h"

#include <SDL_keysym.h>

#include "editor/editorinteractive.h"
#include "editor/tools/editor_place_immovable_tool.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "logic/map.h"
#include "logic/world.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/textarea.h"
#include "wlapplication.h"


using Widelands::Immovable_Descr;

Editor_Tool_Place_Immovable_Options_Menu::
Editor_Tool_Place_Immovable_Options_Menu
		(Editor_Interactive          & parent,
		 Editor_Place_Immovable_Tool & pit,
		 UI::UniqueWindow::Registry  & registry)
:
Editor_Tool_Options_Menu(parent, registry, 100, 100, _("Immovable Bobs")),
m_tabpanel(this, 0, 0, g_gr->images().get("pics/but1.png")),
m_pit     (pit),
m_click_recursion_protect(false)
{
	int32_t const space  =  5;
	const Widelands::World & world = parent.egbase().map().world();
	const Immovable_Descr::Index nr_immovables = world.get_nr_immovables();
	const uint32_t immovables_in_row = std::min
		(static_cast<uint32_t>
		 (ceil(sqrt(static_cast<float>(nr_immovables)))), 6U);

	set_center_panel(&m_tabpanel);

	uint32_t width = 0, height = 0;
	for (int32_t j = 0; j < nr_immovables; ++j) {
		const Immovable_Descr & descr = *world.get_immovable_descr(j);
		const Image& pic = g_gr->animations().get_animation(descr.main_animation())
				.representative_image(RGBColor(0, 0, 0));
		uint16_t w = pic.width();
		uint16_t h = pic.height();
		if (w > width)
			width  = w;
		if (h > height)
			height = h;
	}

	//box->set_inner_size((immovables_in_row)*(width+1+space)+xstart,
	//                     (immovables_in_row)*(height+1+space)+ystart+yend);
	const Image* tab_icon =
		g_gr->images().get("pics/list_first_entry.png");

	Point pos;
	uint32_t cur_x = immovables_in_row;

	UI::Box * box;
	for (Immovable_Descr::Index i = 0; i < nr_immovables; ++cur_x, ++i) {
		if (cur_x == immovables_in_row) {
			cur_x = 0;
			pos = Point(5, 15);
			box =
				new UI::Box
					(&m_tabpanel, 0, 0, UI::Box::Horizontal,
					 parent.get_inner_w() - 50, parent.get_inner_h() - 50);
			box->set_scrolling(true);
			m_tabpanel.add("immovables", tab_icon, box);
		}
		assert(box);

		UI::Checkbox & cb = *new UI::Checkbox
			(box, pos,
			 &g_gr->animations().get_animation(world.get_immovable_descr(i)->main_animation())
				.representative_image(RGBColor(0, 0, 0)), world.get_immovable_descr(i)->descname());
		cb.set_desired_size(width, height);
		cb.set_state(m_pit.is_enabled(i));
		cb.changedto.connect
			(boost::bind(&Editor_Tool_Place_Immovable_Options_Menu::clicked, this, i, _1));
		m_checkboxes.push_back(&cb);
		box->add(&cb, UI::Align_Left);
		box->add_space(space);
		pos.x += width + 1 + space;
	}

	m_tabpanel.activate(0);
}

/**
 * Cleanup
 */
Editor_Tool_Place_Immovable_Options_Menu::
~Editor_Tool_Place_Immovable_Options_Menu
	()
{}

/**
 * Called when one of the state boxes is toggled
*/
void Editor_Tool_Place_Immovable_Options_Menu::clicked(int32_t n, bool t)
{
	if (m_click_recursion_protect)
		return;

	//  FIXME This code is erroneous. It checks the current key state. What it
	//  FIXME needs is the key state at the time the mouse was clicked. See the
	//  FIXME usage comment for get_key_state.
	const bool multiselect =
		get_key_state(SDLK_LCTRL) | get_key_state(SDLK_RCTRL);
	if (not t and (not multiselect or m_pit.get_nr_enabled() == 1))
		m_checkboxes[n]->set_state(true);
	else {
		if (not multiselect) {
			for (uint32_t i = 0; m_pit.get_nr_enabled(); ++i)
				m_pit.enable(i, false);
			//  Disable all checkboxes
			const int32_t size = m_checkboxes.size();
			m_click_recursion_protect = true;
			for (int32_t i = 0; i < size; ++i) {
				if (i != n)
					m_checkboxes[i]->set_state(false);
			}
			m_click_recursion_protect = false;
		}

		m_pit.enable(n, t);
		select_correct_tool();
	}
}
