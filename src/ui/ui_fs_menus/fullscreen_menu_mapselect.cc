/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#include "editor_game_base.h"
#include "error.h"
#include "fullscreen_menu_mapselect.h"
#include "map.h"
#include "ui_button.h"
#include "ui_textarea.h"
#include "ui_multilinetextarea.h"
#include "ui_checkbox.h"
#include "map_loader.h"

/*
==============================================================================

Fullscreen_Menu_MapSelect

==============================================================================
*/


Fullscreen_Menu_MapSelect::Fullscreen_Menu_MapSelect(Editor_Game_Base *g, Map_Loader** ml)
	: Fullscreen_Menu_Base("choosemapmenu.jpg")
{
	egbase = g;
   m_map = new Map;
   m_is_scenario = false;
   m_ml=ml;
   if(*m_ml) delete *m_ml;
   *m_ml=0;

	// Text
   UITextarea* title= new UITextarea(this, MENU_XRES/2, 90, "Choose your map!", Align_HCenter);
   title->set_font(UI_FONT_BIG, UI_FONT_CLR_FG);

   // Checkbox
   UITextarea* ta=new UITextarea(this, MENU_XRES-300, 130, "Load Map as scenario: ", Align_VCenter);
   UICheckbox* cb=new UICheckbox(this, (ta->get_x()+ta->get_w()+10), 120);
   cb->changedto.set(this, &Fullscreen_Menu_MapSelect::changed);
   cb->set_state(m_is_scenario);

	// UIButtons
	UIButton* b;

	b = new UIButton(this, 410, 356, 174, 24, 0, 0);
	b->clickedid.set(this, &Fullscreen_Menu_MapSelect::end_modal);
	b->set_title("Back");

	m_ok = new UIButton(this, 410, 386, 174, 24, 2, 0);
	m_ok->clicked.set(this, &Fullscreen_Menu_MapSelect::ok);
	m_ok->set_title("OK");
	m_ok->set_enabled(false);

	// Create the list area
	list = new UIListselect(this, 12, 163, 363, 297);
	list->selected.set(this, &Fullscreen_Menu_MapSelect::map_selected);

	// Fill it with the files: Widelands map files
	g_fs->FindFiles("maps", "*"WLMF_SUFFIX, &m_mapfiles);
	g_fs->FindFiles("maps", "*"S2MF_SUFFIX, &m_mapfiles);


   Map* map=new Map();
	for(filenameset_t::iterator pname = m_mapfiles.begin(); pname != m_mapfiles.end(); pname++) {
		const char *name = pname->c_str();

      Map_Loader* m_ml = map->get_correct_loader(name);
      try {
         m_ml->preload_map(true);
         list->add_entry(map->get_name(), reinterpret_cast<void*>(const_cast<char*>(name)));
      } catch(wexception& ) {
         // we simply skip illegal entries
      }
	   delete m_ml;

   }
   list->sort();
   delete map;

	// Info fields
	new UITextarea(this, 450, 160, "Name:", Align_Right);
	taname = new UITextarea(this, 460, 160, "");
	new UITextarea(this, 450, 180, "Author:", Align_Right);
	taauthor = new UITextarea(this, 460, 180, "");
	new UITextarea(this, 450, 200, "Size:", Align_Right);
	tasize = new UITextarea(this, 460, 200, "");
	new UITextarea(this, 450, 220, "World:", Align_Right);
	taworld = new UITextarea(this, 460, 220, "");
	new UITextarea(this, 450, 240, "Players:", Align_Right);
	tanplayers = new UITextarea(this, 460, 240, "");
	new UITextarea(this, 450, 260, "Descr:", Align_Right);
	tadescr = new UIMultiline_Textarea(this, 460, 260, 160, 80, "");
}

Fullscreen_Menu_MapSelect::~Fullscreen_Menu_MapSelect()
{
   if(m_map) {
      // upsy, obviously ok was not pressed
      delete m_map;
      m_map=0;
   }
}

/*
 * Gets called when the Checkbox changes
 */
void Fullscreen_Menu_MapSelect::changed(bool t) {
   m_is_scenario=t;
}

void Fullscreen_Menu_MapSelect::ok()
{
	if (m_map)
	{
		assert(*m_ml);

		egbase->set_map((*m_ml)->get_map());
		(*m_ml)->preload_map(m_is_scenario);
		m_map = 0;
	}

   if(m_is_scenario)
      end_modal(2);
   else
      end_modal(1);
}

void Fullscreen_Menu_MapSelect::map_selected(int id)
{
	if (*m_ml) {
		delete *m_ml;
      *m_ml = 0;
	}

	if (get_mapname())
	{
		assert(m_map);

      try {
         *m_ml = m_map->get_correct_loader(get_mapname());
	      (*m_ml)->preload_map(m_is_scenario);

			char buf[256];
			taname->set_text(m_map->get_name());
			taauthor->set_text(m_map->get_author());
			sprintf(buf, "%-4ix%4i", m_map->get_width(), m_map->get_height());
			tasize->set_text(buf);
			sprintf(buf, "%i", m_map->get_nrplayers());
			tanplayers->set_text(buf);
			tadescr->set_text(m_map->get_description());
			taworld->set_text(m_map->get_world_name());
         m_ok->set_enabled(true);
		} catch(std::exception& e) {
			log("Failed to load map %s: %s\n", get_mapname(), e.what());

			taname->set_text("(bad map file)");
			taauthor->set_text("");
			tasize->set_text("");
			tanplayers->set_text("");
			tadescr->set_text("");
			taworld->set_text("");
			m_ok->set_enabled(false);
		}
	}
}
