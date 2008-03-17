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

#include "critter_bob.h"
#include "editor_place_bob_tool.h"
#include "editorinteractive.h"
#include "graphic.h"
#include "i18n.h"
#include "map.h"
#include "wlapplication.h"
#include "world.h"

#include "ui_box.h"
#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_tabpanel.h"
#include "ui_textarea.h"

#include "upcast.h"

#include <SDL_keysym.h>

Editor_Tool_Place_Bob_Options_Menu::Editor_Tool_Place_Bob_Options_Menu
(Editor_Interactive         & parent,
 Editor_Place_Bob_Tool      & pit,
 UI::UniqueWindow::Registry & registry)
:
Editor_Tool_Options_Menu(parent, registry, 100, 100, _("Bobs Menu")),

m_tabpanel              (this, 0, 0, 1),
m_pit                   (pit)
{
	int32_t const space  =  5;
	int32_t const xstart =  5;
	int32_t const ystart = 15;
	Widelands::World const & world = parent.egbase().map().world();
	int32_t const nr_bobs = world.get_nr_bobs();
	const uint32_t bobs_in_row =
		std::max
		(std::min
		 (static_cast<uint32_t>(ceil(sqrt(static_cast<float>(nr_bobs)))), 24U),
		 12U);

	m_tabpanel.set_snapparent(true);

	uint32_t width = 0, height = 0;
	for (int32_t j = 0; j < nr_bobs; ++j) {
		uint32_t w, h;
		g_gr->get_picture_size
			(g_gr->get_picture
			 (PicMod_Game, world.get_bob_descr(j)->get_picture()), w, h);
		if (w > width)  width  = w;
		if (h > height) height = h;
	}

	const uint32_t tab_icon =
		g_gr->get_picture(PicMod_Game, "pics/list_first_entry.png");
	int32_t ypos = ystart;
	int32_t xpos = xstart;
	uint32_t cur_x = bobs_in_row;
	int32_t i = 0;
	while (i < nr_bobs) {
		UI::Box * box;
		if (cur_x == bobs_in_row) {
			cur_x = 0;
			ypos  = ystart;
			xpos  = xstart;
			box = new UI::Box(&m_tabpanel, 0, 0, UI::Box::Horizontal);
			box->resize();
			m_tabpanel.add(tab_icon, box);
		}

		Widelands::Bob::Descr const & descr = *world.get_bob_descr(i);
		upcast(Widelands::Critter_Bob_Descr const, critter_descr, &descr);
		UI::Checkbox & cb = *new UI::Checkbox
			(box,
			 xpos, ypos,
			 g_gr->get_picture(PicMod_Game, descr.get_picture()),
			 critter_descr ? critter_descr->descname() : std::string());

		cb.set_size(width, height);
		cb.set_id(i);
		cb.set_state(m_pit.is_enabled(i));
		cb.changedtoid.set(this, &Editor_Tool_Place_Bob_Options_Menu::clicked);
		m_checkboxes.push_back(&cb);
		box->add(&cb, Align_Left);
		box->add_space(space);
		xpos += width + 1 + space;
		++cur_x;
		++i;
	}
	ypos += height + 1 + space + 5;

	m_tabpanel.activate(0);
	m_tabpanel.resize();
}


/**
 * This is called when one of the state boxes is toggled
*/
void Editor_Tool_Place_Bob_Options_Menu::clicked(int32_t n, bool t) {
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
		for (uint32_t i = 0; i < m_checkboxes.size(); ++i, i += i == static_cast<uint32_t>(n)) {
			m_checkboxes[i]->changedtoid.set
				(this, &Editor_Tool_Place_Bob_Options_Menu::do_nothing);
			m_checkboxes[i]->set_state(false);
			m_checkboxes[i]->changedtoid.set
				(this, &Editor_Tool_Place_Bob_Options_Menu::clicked);
		}
	}

	m_pit.enable(n, t);
	select_correct_tool();
}

/**
 * Do nothing
*/
void Editor_Tool_Place_Bob_Options_Menu::do_nothing(int32_t, bool) {}
