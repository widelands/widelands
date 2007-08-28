/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "editor_tool_place_immovable_options_menu.h"

#include "editorinteractive.h"
#include "editor_place_immovable_tool.h"
#include "graphic.h"
#include "i18n.h"
#include "keycodes.h"
#include "map.h"
#include "wlapplication.h"
#include "world.h"

#include "ui_box.h"
#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_textarea.h"


/*
===========
Editor_Tool_Place_Immovable_Options_Menu::Editor_Tool_Place_Immovable_Options_Menu

constructor
===========
*/
Editor_Tool_Place_Immovable_Options_Menu::Editor_Tool_Place_Immovable_Options_Menu
(Editor_Interactive          & parent,
 Editor_Place_Immovable_Tool & pit,
 UI::UniqueWindow::Registry  & registry)
:
Editor_Tool_Options_Menu
(parent, registry, 100, 100, _("Immovable Bobs Menu").c_str()),
m_tabpanel(this, 0, 0, 1),
m_pit     (pit)
{
   const int space=5;
   const int xstart=5;
   const int ystart=15;
	const World & world = parent.egbase().map().world();
	const Immovable_Descr::Index nr_immovables = world.get_nr_immovables();
	const uint immovables_in_row = std::min
		(static_cast<uint>
		 (ceil(sqrt(static_cast<float>(nr_immovables)))),
		 6U);


   m_tabpanel.set_snapparent(true);


	uint width = 0, height = 0;
   for (int j=0; j<nr_immovables; j++) {
		uint w, h;
		Immovable_Descr* descr = world.get_immovable_descr(j);
		g_gr->get_picture_size
			(g_gr->get_picture(PicMod_Game, descr->get_picture()), w, h);
      if (w>width) width=w;
      if (h>height) height=h;
	}

	//box->set_inner_size((immovables_in_row)*(width+1+space)+xstart, (immovables_in_row)*(height+1+space)+ystart+yend);
	const uint tab_icon =
		g_gr->get_picture(PicMod_Game, "pics/list_first_entry.png");

   int ypos=ystart;
   int xpos=xstart;
	uint cur_x = immovables_in_row;
	for (Immovable_Descr::Index i = 0; i < nr_immovables; ++cur_x, ++i) {
		UI::Box * box;
		if (cur_x==immovables_in_row) {
         cur_x=0;
         ypos=ystart;
         xpos=xstart;
			box = new UI::Box(&m_tabpanel, 0, 0, UI::Box::Horizontal);
         box->resize();
			m_tabpanel.add(tab_icon, box);
		}

		UI::Checkbox & cb = *new UI::Checkbox
			(box, xpos, ypos,
			 g_gr->get_picture
			 (PicMod_Game, world.get_immovable_descr(i)->get_picture()));

		cb.set_size(width, height);
		cb.set_id(i);
		cb.set_state(m_pit.is_enabled(i));
		cb.changedtoid.set(this, &Editor_Tool_Place_Immovable_Options_Menu::clicked);
		m_checkboxes.push_back(&cb);
		box->add(&cb, Align_Left);
      box->add_space(space);
      xpos+=width+1+space;
	}
   ypos+=height+1+space+5;

	m_tabpanel.activate(0);
	m_tabpanel.resize();
}

/*
 * Cleanup
 */
Editor_Tool_Place_Immovable_Options_Menu::~Editor_Tool_Place_Immovable_Options_Menu() {
}

/*
===========
   void Editor_Tool_Place_Immovable_Options_Menu::clicked()

this is called when one of the state boxes is toggled
===========
*/
void Editor_Tool_Place_Immovable_Options_Menu::clicked(int n, bool t) {
	//  FIXME This code is erroneous. It checks the current key state. What it
	//  FIXME needs is the key state at the time the mouse was clicked.
	const bool multiselect =
		WLApplication::get()->get_key_state(KEY_LCTRL)
		|
		WLApplication::get()->get_key_state(KEY_RCTRL);
	if (not t and (not multiselect or m_pit.get_nr_enabled() == 1))
		m_checkboxes[n]->set_state(true);
	else {
		if (not multiselect) {
			for (uint i = 0; m_pit.get_nr_enabled(); ++i) m_pit.enable(i, false);
      // Disable all checkboxes
			const uint size = m_checkboxes.size();
			//TODO: the uint cast is ugly!
			for (uint i = 0; i < size; ++i, i += i == (uint)n) {
         m_checkboxes[i]->changedtoid.set(this, &Editor_Tool_Place_Immovable_Options_Menu::do_nothing);
         m_checkboxes[i]->set_state(false);
         m_checkboxes[i]->changedtoid.set(this, &Editor_Tool_Place_Immovable_Options_Menu::clicked);
			}
		}

		m_pit.enable(n, t);
   select_correct_tool();
	}
}

/* do nothing */
void Editor_Tool_Place_Immovable_Options_Menu::do_nothing(int, bool) {}
