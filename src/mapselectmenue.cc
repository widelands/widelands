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

	filenameset_t	m_mapfiles;
	
	Map*			m_map;
	
public:
	MapSelectMenu(Game *g);
	~MapSelectMenu();

	const char *get_mapname() { return (const char*)list->get_selection(); }

	void ok();
	void map_selected(int id);
};

MapSelectMenu::MapSelectMenu(Game *g)
	: BaseMenu("choosemapmenu.bmp")
{
	game = g;
	m_map = 0;

	// Text
	new Textarea(this, MENU_XRES/2, 140, "Choose your map!", Align_HCenter);

	// Buttons
	Button* b;

	b = new Button(this, 410, 406, 174, 24, 0, 0);
	b->clickedid.set(this, &MapSelectMenu::end_modal);
	b->set_title("Back");

	m_ok = new Button(this, 410, 436, 174, 24, 2, 0);
	m_ok->clicked.set(this, &MapSelectMenu::ok);
	m_ok->set_title("OK");
	m_ok->set_enabled(false);

	// Create the list area
	list = new Listselect(this, 20, 170, 360, 290);
	list->selected.set(this, &MapSelectMenu::map_selected);

	// Fill it with the files: Widelands map files
	g_fs->FindFiles("maps", "*"WLMF_SUFFIX, &m_mapfiles);
	g_fs->FindFiles("maps", "*"S2MF_SUFFIX, &m_mapfiles);
	
	for(filenameset_t::iterator pname = m_mapfiles.begin(); pname != m_mapfiles.end(); pname++) {
		const char *name = pname->c_str();
		const char *slash = strrchr(name, '/');
		const char *backslash = strrchr(name, '\\');
		
		if (backslash && (!slash || backslash > slash))
			slash = backslash;

		list->add_entry(slash?slash+1:name, (void*)name);
	}

	// Info fields
	new Textarea(this, 450, 210, "Name:", Align_Right);
	taname = new Textarea(this, 460, 210, 0);
	new Textarea(this, 450, 230, "Author:", Align_Right);
	taauthor = new Textarea(this, 460, 230, 0);
	new Textarea(this, 450, 250, "Size:", Align_Right);
	tasize = new Textarea(this, 460, 250, 0);
	new Textarea(this, 450, 270, "World:", Align_Right);
	taworld = new Textarea(this, 460, 270, 0);
	new Textarea(this, 450, 290, "Players:", Align_Right);
	tanplayers = new Textarea(this, 460, 290, 0);
	new Textarea(this, 450, 310, "Descr:", Align_Right);
	tadescr = new Multiline_Textarea(this, 460, 310, 180, 80, 0);
}

MapSelectMenu::~MapSelectMenu()
{
	if (m_map) {
		delete m_map;
		m_map = 0;
	}
}


void MapSelectMenu::ok()
{
	game->set_map(m_map);
	m_map = 0;
	
	end_modal(1);
}

void MapSelectMenu::map_selected(int id)
{
	if (m_map) {
		delete m_map;
		m_map = 0;
	}
	
	if (get_mapname())
	{
		try {
			m_map = new Map(get_mapname());
		
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
			taauthor->set_text(0);
			tasize->set_text(0);
			tanplayers->set_text(0);
			tadescr->set_text(0);
			taworld->set_text(0);
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
