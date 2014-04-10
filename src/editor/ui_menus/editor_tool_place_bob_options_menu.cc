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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "editor/ui_menus/editor_tool_place_bob_options_menu.h"

#include <SDL_keysym.h>

#include "editor/editorinteractive.h"
#include "editor/tools/editor_place_bob_tool.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "logic/critter_bob.h"
#include "logic/map.h"
#include "logic/world.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/textarea.h"
#include "upcast.h"
#include "wlapplication.h"


Editor_Tool_Place_Bob_Options_Menu::Editor_Tool_Place_Bob_Options_Menu
	(Editor_Interactive         & parent,
	 Editor_Place_Bob_Tool      & pit,
	 UI::UniqueWindow::Registry & registry)
:
Editor_Tool_Options_Menu(parent, registry, 100, 100, _("Bobs")),

m_tabpanel          (this, 0, 0, g_gr->images().get("pics/but1.png")),
m_pit               (pit),
m_click_recursion_protect(false)
{
	int32_t const space  =  5;
	const Widelands::World & world = parent.egbase().map().world();
	int32_t const nr_bobs = world.get_nr_bobs();
	const uint32_t bobs_in_row =
		std::max
			(std::min
			 	(static_cast<uint32_t>(ceil(sqrt(static_cast<float>(nr_bobs)))),
			 	 24U),
			 12U);

	set_center_panel(&m_tabpanel);

	uint32_t width = 0, height = 0;
	for (int32_t j = 0; j < nr_bobs; ++j) {
		const Image& pic =
			g_gr->animations().get_animation(world.get_bob_descr(j)->main_animation())
				.representative_image(RGBColor(0, 0, 0));
		uint16_t w = pic.width();
		uint16_t h = pic.height();
		if (w > width)
			width = w;
		if (h > height)
			height = h;
	}

	const Image* tab_icon =
		g_gr->images().get("pics/list_first_entry.png");
	Point pos;
	uint32_t cur_x = bobs_in_row;
	int32_t i = 0;
	UI::Box * box = nullptr;
	while (i < nr_bobs) {
		if (cur_x == bobs_in_row) {
			cur_x = 0;
			pos   = Point(5, 15);
			box = new UI::Box(&m_tabpanel, 0, 0, UI::Box::Horizontal);
			m_tabpanel.add("icons", tab_icon, box);
		}

		const Widelands::BobDescr & descr = *world.get_bob_descr(i);
		upcast(Widelands::Critter_Bob_Descr const, critter_descr, &descr);
		UI::Checkbox & cb = *new UI::Checkbox
			(box,
			 pos,
			 &g_gr->animations().get_animation(descr.main_animation())
				.representative_image(RGBColor(0, 0, 0)),
			 critter_descr ? critter_descr->descname() : std::string());

		cb.set_desired_size(width, height);
		cb.set_state(m_pit.is_enabled(i));
		cb.changedto.connect(boost::bind(&Editor_Tool_Place_Bob_Options_Menu::clicked, this, i, _1));
		m_checkboxes.push_back(&cb);
		box->add(&cb, UI::Align_Left);
		box->add_space(space);
		pos.x += width + 1 + space;
		++cur_x;
		++i;
	}

	m_tabpanel.activate(0);
}


/**
 * This is called when one of the state boxes is toggled
*/
void Editor_Tool_Place_Bob_Options_Menu::clicked
	(int32_t const n, bool const t)
{
	if (m_click_recursion_protect)
		return;

	//  FIXME This code is erroneous. It checks the current key state. What it
	//  FIXME needs is the key state at the time the mouse was clicked. See the
	//  FIXME usage comment for get_key_state.
	const bool multiselect =
		get_key_state(SDLK_LCTRL) | get_key_state(SDLK_RCTRL);
	if (not t and (not multiselect or m_pit.get_nr_enabled() == 1)) {
		m_checkboxes[n]->set_state(true);
		return;
	}

	if (not multiselect) {
		for (uint32_t i = 0; m_pit.get_nr_enabled(); ++i) m_pit.enable(i, false);

		//  disable all checkboxes
		m_click_recursion_protect = true;
		for (uint32_t i = 0; i < m_checkboxes.size(); ++i) {
			if (i != static_cast<uint32_t>(n))
				m_checkboxes[i]->set_state(false);
		}
		m_click_recursion_protect = false;
	}

	m_pit.enable(n, t);
	select_correct_tool();
}
