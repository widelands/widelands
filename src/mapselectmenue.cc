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
#include "fileloc.h"
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

enum {
	ms_go,
	ms_back
};

class MapSelectMenu : public BaseMenu {
	Listselect *list;
	Textarea *taname;
	Textarea *taauthor;
	Textarea *tasize;
	Textarea *taworld;
	Textarea *tanplayers;
	Multiline_Textarea *tadescr;
	Button *ok;

public:
	MapSelectMenu();

	inline const char *get_map() { return list->get_selection(); }

	void map_selected(int id);
};

MapSelectMenu::MapSelectMenu()
	: BaseMenu("splash.bmp")
{
	// Text
	new Textarea(this, MENU_XRES/2, 140, "Choose your map!", Textarea::H_CENTER);

	// Buttons
	Button* b;

	b = new Button(this, 410, 406, 174, 24, 0, ms_back);
	b->clickedid.set(this, &MapSelectMenu::end_modal);
	b->set_pic(g_fh.get_string("Back", 0));

	ok = new Button(this, 410, 436, 174, 24, 2, ms_go);
	ok->clickedid.set(this, &MapSelectMenu::end_modal);
	ok->set_pic(g_fh.get_string("OK", 0));
	ok->set_enabled(false);

	// Create the list area
	list = new Listselect(this, 20, 170, 360, 290);
	list->selected.set(this, &MapSelectMenu::map_selected);

	// Fill it with the files: Widelands map files
	g_fileloc.init_filelisting(TYPE_MAP, WLMF_SUFFIX);
	const char* name;
	uint i;
	while(g_fileloc.get_state() != File_Locator::LA_NOMOREFILES) {
		name = g_fileloc.get_next_file();
		if(!name) continue;

		i = strlen(name);
		while(name[i] != '/' && name[i] != '\\') --i;
		++i;
		list->add_entry(name+i, name);
	}
	g_fileloc.end_filelisting();

	// Fill it with more files: Settlers2 map files
	g_fileloc.init_filelisting(TYPE_MAP, S2MF_SUFFIX);
	while(g_fileloc.get_state() != File_Locator::LA_NOMOREFILES) {
		name=g_fileloc.get_next_file();
		if(!name) continue;

		i=strlen(name);
		while(name[i] != '/' && name[i] != '\\') --i;
		++i;
		list->add_entry(name+i, name);
	}
	g_fileloc.end_filelisting();

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
         ok->set_enabled(true);
		} else {
			taname->set_text("(bad map file)");
			taauthor->set_text(0);
			tasize->set_text(0);
			tanplayers->set_text(0);
			tadescr->set_text(0);
			ok->set_enabled(false);
		}
	}
}

/** void map_select_menue(void)
 *
 *	Here, you chose a map out of a given listbox
 *
 * Args:	None
 * Returns:	true if a game was played
 */
bool map_select_menue(void)
{
	MapSelectMenu *ms = new MapSelectMenu;
	int code = ms->run();
	char *map = 0;
	if (code == ms_go) {
		const char *sel = ms->get_map();
		if (sel)
			map = strdup(sel);
	}
	delete ms;

	if (map) {
		Game *g = new Game;
		g->set_map(map);
		free(map);
      g->run();
		delete g;
		return true;
	}

	return false;
}
