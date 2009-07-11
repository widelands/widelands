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

#include "editor_tool_place_bob_options_menu.h"

#include "logic/critter_bob.h"
#include "editor/tools/editor_place_bob_tool.h"
#include "editor/editorinteractive.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "map.h"
#include "wlapplication.h"
#include "world.h"

#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/textarea.h"

#include "upcast.h"

#include <SDL_keysym.h>

Editor_Tool_Place_Bob_Options_Menu::Editor_Tool_Place_Bob_Options_Menu
	(Editor_Interactive         & parent,
	 Editor_Place_Bob_Tool      & pit,
	 UI::UniqueWindow::Registry & registry)
:
Editor_Tool_Options_Menu(parent, registry, 100, 100, _("Bobs Menu")),

m_tabpanel          (this, 0, 0, g_gr->get_picture(PicMod_UI, "pics/but1.png")),
m_pit               (pit)
{
	int32_t const space  =  5;
	Widelands::World const & world = parent.egbase().map().world();
	int32_t const nr_bobs = world.get_nr_bobs();
	const uint32_t bobs_in_row =
		std::max
			(std::min
			 	(static_cast<uint32_t>(ceil(sqrt(static_cast<float>(nr_bobs)))),
			 	 24U),
			 12U);

	m_tabpanel.set_snapparent(true);

	uint32_t width = 0, height = 0;
	for (int32_t j = 0; j < nr_bobs; ++j) {
		uint32_t w, h;
		g_gr->get_picture_size
			(g_gr->get_picture
			 	(PicMod_Game, world.get_bob_descr(j)->get_picture()),
			 w, h);
		if (w > width)
			width  = w;
		if (h > height)
			height = h;
	}

	const PictureID tab_icon =
		g_gr->get_picture(PicMod_Game, "pics/list_first_entry.png");
	Point pos;
	uint32_t cur_x = bobs_in_row;
	int32_t i = 0;
	UI::Box * box;
	while (i < nr_bobs) {
		if (cur_x == bobs_in_row) {
			cur_x = 0;
			pos   = Point(5, 15);
			box = new UI::Box(&m_tabpanel, 0, 0, UI::Box::Horizontal);
			box->resize();
			m_tabpanel.add(tab_icon, box);
		}

		Widelands::Bob::Descr const & descr = *world.get_bob_descr(i);
		upcast(Widelands::Critter_Bob_Descr const, critter_descr, &descr);
		UI::Checkbox & cb = *new UI::Checkbox
			(box,
			 pos,
			 g_gr->get_picture(PicMod_Game, descr.get_picture()),
			 critter_descr ? critter_descr->descname() : std::string());

		cb.set_size(width, height);
		cb.set_id(i);
		cb.set_state(m_pit.is_enabled(i));
		cb.changedtoid.set(this, &Editor_Tool_Place_Bob_Options_Menu::clicked);
		m_checkboxes.push_back(&cb);
		box->add(&cb, UI::Align_Left);
		box->add_space(space);
		pos.x += width + 1 + space;
		++cur_x;
		++i;
	}

	m_tabpanel.activate(0);
	m_tabpanel.resize();
}


/**
 * This is called when one of the state boxes is toggled
*/
void Editor_Tool_Place_Bob_Options_Menu::clicked
	(int32_t const n, bool const t)
{
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
		//  TODO The uint32_t cast is ugly!
		for
			(uint32_t i = 0;
			 i < m_checkboxes.size();
			 ++i, i += i == static_cast<uint32_t>(n))
			{
				m_checkboxes[i]->changedtoid.set
					(this,
					 static_cast
					 <void (Editor_Tool_Place_Bob_Options_Menu::*)(int32_t, bool)>
					 (0));
				m_checkboxes[i]->set_state(false);
				m_checkboxes[i]->changedtoid.set
					(this, &Editor_Tool_Place_Bob_Options_Menu::clicked);
			}
	}

	m_pit.enable(n, t);
	select_correct_tool();
}
