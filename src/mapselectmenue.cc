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

#include "widelands.h"
#include "ui.h"
#include "mapselectmenue.h"
#include "font.h"
#include "input.h"
#include "menuecommon.h"
#include "map.h"
#include "singlepmenue.h"
#include "game.h"
#include "mainmenue.h"

/*
==============================================================================

MapSelectMenu

==============================================================================
*/


class MapSelectMenu : public BaseMenu {
	Game		*game;

	Listselect *list;
	Textarea *taname;
	Textarea *taauthor;
	Textarea *tasize;
	Textarea *taworld;
	Textarea *tanplayers;
	Multiline_Textarea *tadescr;
	Button *m_ok;

public:
	MapSelectMenu(Game *g);

	const char *get_map() { return list->get_selection(); }

	void ok();
	void map_selected(int id);
};

MapSelectMenu::MapSelectMenu(Game *g)
	: BaseMenu("splash.bmp")
{
	game = g;

	// Text
	new Textarea(this, MENU_XRES/2, 140, "Choose your map!", Textarea::H_CENTER);

	// Buttons
	Button* b;

	b = new Button(this, 410, 406, 174, 24, 0, 0);
	b->clickedid.set(this, &MapSelectMenu::end_modal);
	b->set_pic(g_fh.get_string("Back", 0));

	m_ok = new Button(this, 410, 436, 174, 24, 2, 0);
	m_ok->clicked.set(this, &MapSelectMenu::ok);
	m_ok->set_pic(g_fh.get_string("OK", 0));
	m_ok->set_enabled(false);

	// Create the list area
	list = new Listselect(this, 20, 170, 360, 290);
	list->selected.set(this, &MapSelectMenu::map_selected);

	// Fill it with the files: Widelands map files
	filenameset_t mapfiles;
	
	g_fs->FindFiles("maps", "*"WLMF_SUFFIX, &mapfiles);
	g_fs->FindFiles("maps", "*"S2MF_SUFFIX, &mapfiles);
	
	for(filenameset_t::iterator pname = mapfiles.begin(); pname != mapfiles.end(); pname++) {
		const char *name = pname->c_str();
		const char *slash = strrchr(name, '/');
		const char *backslash = strrchr(name, '\\');
		
		if (backslash && (!slash || backslash > slash))
			slash = backslash;

      cerr << "\"" << name << "\"" << endl;

		list->add_entry(slash?slash+1:name, name);
	}

	// Info fields
	new Textarea(this, 450, 210, "Name:", Textarea::H_RIGHT);
	taname = new Textarea(this, 460, 210, 0);
	new Textarea(this, 450, 230, "Author:", Textarea::H_RIGHT);
	taauthor = new Textarea(this, 460, 230, 0);
	new Textarea(this, 450, 250, "Size:", Textarea::H_RIGHT);
	tasize = new Textarea(this, 460, 250, 0);
	new Textarea(this, 450, 270, "World:", Textarea::H_RIGHT);
	taworld = new Textarea(this, 460, 270, 0);
	new Textarea(this, 450, 290, "Players:", Textarea::H_RIGHT);
	tanplayers = new Textarea(this, 460, 290, 0);
	new Textarea(this, 450, 310, "Descr:", Textarea::H_RIGHT);
	tadescr = new Multiline_Textarea(this, 460, 310, 180, 80, 0);
}

void MapSelectMenu::ok()
{
	game->set_mapname(get_map());
	end_modal(1);
}

void MapSelectMenu::map_selected(int id)
{
	if (get_map())
	{
		Map m;
		if (m.load_map_header(get_map()) == RET_OK) {
			char buf[256];
			taname->set_text(m.get_name());
			taauthor->set_text(m.get_author());
			sprintf(buf, "%-4ix%4i", m.get_w(), m.get_h());
			tasize->set_text(buf);
			sprintf(buf, "%i", m.get_nplayers());
			tanplayers->set_text(buf);
			tadescr->set_text(m.get_descr());
			taworld->set_text(m.get_world_name());
         m_ok->set_enabled(true);
		} else {
			taname->set_text("(bad map file)");
			taauthor->set_text(0);
			tasize->set_text(0);
			tanplayers->set_text(0);
			tadescr->set_text(0);
			m_ok->set_enabled(false);
		}
	}
}

/** void map_select_menue(Game *g)
 *
 * Here, you chose a map out of a given listbox
 *
 * Args:	g	the game
 */
void map_select_menue(Game *g)
{
	MapSelectMenu ms(g);
	ms.run();
}
