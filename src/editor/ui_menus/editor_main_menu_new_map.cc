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

#include "editor_main_menu_new_map.h"

#include "graphic.h"
#include "i18n.h"
#include "editorinteractive.h"
#include "map.h"
#include "profile.h"
#include "world.h"
#include "editor_game_base.h"

#include "ui_button.h"
#include "ui_progresswindow.h"
#include "ui_textarea.h"
#include "ui_window.h"

#include <string>
#include <cstring>
#include <vector>

#include <stdio.h>

using Widelands::NUMBER_OF_MAP_DIMENSIONS;

/**
 * Create all the buttons etc...
*/
Main_Menu_New_Map::Main_Menu_New_Map(Editor_Interactive *parent)
:
UI::Window
	(parent,
	 (parent->get_w() - 140) / 2, (parent->get_h() - 150) / 2, 140, 150,
	 _("New Map")),
m_parent(parent), //  FIXME redundant (base stores parent pointer)
m_currentworld(0)
{
	char buffer[250];
	int32_t const offsx   =  5;
	int32_t const offsy   = 30;
	int32_t const spacing =  5;
	int32_t const width   = get_inner_w() - offsx * 2;
	int32_t const height  = 20;
	int32_t       posx    = offsx;
	int32_t       posy    = offsy;
	Widelands::Map const & map = parent->egbase().map();
	{
		Widelands::Extent const map_extent = map.extent();
		for (m_w = 0; Widelands::MAP_DIMENSIONS[m_w] < map_extent.w; ++m_w) {}
		for (m_h = 0; Widelands::MAP_DIMENSIONS[m_h] < map_extent.h; ++m_h) {}
	}
	snprintf
		(buffer, sizeof(buffer), _("Width: %u"), Widelands::MAP_DIMENSIONS[m_w]);
	m_width =
		new UI::Textarea(this, posx + spacing + 20, posy, buffer, Align_Left);

	new UI::IDButton<Main_Menu_New_Map, int32_t>
		(this,
		 posx, posy, 20, 20,
		 1,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png"),
		 &Main_Menu_New_Map::button_clicked, this, 0);

	new UI::IDButton<Main_Menu_New_Map, int32_t>
		(this,
		 get_inner_w()-spacing - 20, posy, 20, 20,
		 1,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png"),
		 &Main_Menu_New_Map::button_clicked, this, 1);

	posy+=20+spacing+spacing;

	snprintf
		(buffer, sizeof(buffer),
		 _("Height: %u"), Widelands::MAP_DIMENSIONS[m_h]);
	m_height =
		new UI::Textarea(this, posx + spacing + 20, posy, buffer, Align_Left);

	new UI::IDButton<Main_Menu_New_Map, int32_t>
		(this,
		 posx, posy, 20, 20,
		 1,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png"),
		 &Main_Menu_New_Map::button_clicked, this, 2);

	new UI::IDButton<Main_Menu_New_Map, int32_t>
		(this,
		 get_inner_w() - spacing - 20, posy, 20, 20,
		 1,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png"),
		 &Main_Menu_New_Map::button_clicked, this, 3);

	posy += 20 + spacing + spacing;

	Widelands::World::get_all_worlds(m_worlds);

	assert(m_worlds.size());
	while (strcmp(map.get_world_name(), m_worlds[m_currentworld].c_str()))
		++m_currentworld;

	m_world = new UI::IDButton<Main_Menu_New_Map, int32_t>
		(this,
		 posx, posy, width, height,
		 1,
		 &Main_Menu_New_Map::button_clicked, this, 4,
		 Widelands::World::World(m_worlds[m_currentworld].c_str()).get_name());

	posy += height + spacing + spacing + spacing;

	new UI::Button<Main_Menu_New_Map>
		(this,
		 posx, posy, width, height,
		 0,
		 &Main_Menu_New_Map::clicked_create_map, this,
		 _("Create Map"));

	posy += height + spacing;
}


/**
 * Called, when button get clicked
*/
void Main_Menu_New_Map::button_clicked(int32_t n) {
	switch (n) {
	case 0: ++m_w; break;
	case 1: --m_w; break;
	case 2: ++m_h; break;
	case 3: --m_h; break;
	case 4:
		++m_currentworld;
		if (m_currentworld == m_worlds.size()) m_currentworld=0;
		m_world->set_title
			(Widelands::World::World(m_worlds[m_currentworld].c_str()).get_name
			 	());
		break;
	}

	char buffer[200];
	if (m_w <  0)                        m_w = 0;
	if (m_w >= NUMBER_OF_MAP_DIMENSIONS) m_w = NUMBER_OF_MAP_DIMENSIONS - 1;
	if (m_h <  0)                        m_h = 0;
	if (m_h >= NUMBER_OF_MAP_DIMENSIONS) m_h = NUMBER_OF_MAP_DIMENSIONS - 1;
	snprintf
		(buffer, sizeof(buffer),
		 _("Width: %u"),  Widelands::MAP_DIMENSIONS[m_w]);
	m_width ->set_text(buffer);
	snprintf
		(buffer, sizeof(buffer),
		 _("Height: %u"), Widelands::MAP_DIMENSIONS[m_h]);
	m_height->set_text(buffer);
}

void Main_Menu_New_Map::clicked_create_map() {
	Widelands::Editor_Game_Base & egbase = m_parent->egbase();
	Widelands::Map              & map    = egbase.map();
	UI::ProgressWindow loader;

	// Clean all the stuff up, so we can load
	egbase.cleanup_for_load(true, false);

	if (strcmp(map.get_world_name(), m_worlds[m_currentworld].c_str()))
		m_parent->change_world();
	map.create_empty_map
		(Widelands::MAP_DIMENSIONS[m_w], Widelands::MAP_DIMENSIONS[m_h],
		 m_worlds[m_currentworld],
		 _("No Name"),
		 g_options.pull_section("global")->get_string("realname", _("Unknown")));

	// Postload the world which provides all the immovables found on a map
	map.world().postload(&egbase);

	egbase.postload     ();
	egbase.load_graphics(loader);

	map.recalc_whole_map();

	m_parent->set_need_save(true);
	m_parent->need_complete_redraw();

	die();
}
