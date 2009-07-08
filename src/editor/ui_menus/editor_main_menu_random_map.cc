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

#include "editor_main_menu_random_map.h"

#include "graphic/graphic.h"
#include "i18n.h"
#include "editor/editorinteractive.h"
#include "map.h"
#include "profile/profile.h"
#include "world.h"
#include "logic/editor_game_base.h"

#include "ui_basic/button.h"
#include "ui_basic/progresswindow.h"
#include "ui_basic/textarea.h"
#include "ui_basic/window.h"

#include <string>
#include <cstring>
#include <vector>
#include <sstream>

#include "random.h"

using Widelands::NUMBER_OF_MAP_DIMENSIONS;

Main_Menu_New_Random_Map::Main_Menu_New_Random_Map
	(Editor_Interactive * const parent)
	:
	UI::Window
		(parent,
		 (parent->get_w() - 220) / 2, (parent->get_h() - 250) / 2, 220, 250,
		 _("New Random Map")),
	m_currentworld(0)
{
	char buffer[250];
	int32_t const offsx   =  5;
	int32_t const offsy   =  5;
	int32_t const spacing =  5;
	int32_t const width   = get_inner_w() - offsx * 2;
	int32_t const height  = 20;
	int32_t       posx    = offsx;
	int32_t       posy    = offsy;
	m_waterval     = 20;
	m_landval      = 60;
	m_wastelandval = 0;

	// ---------- Random map number edit ----------

	new UI::Textarea(this, posx, posy, _("Random Number:"), Align_Left);
	posy += height + spacing;

	m_nrEditbox =
		new UI::EditBox
			(this,
			 posx, posy,
			 width, 20,
			 g_gr->get_picture(PicMod_UI, "pics/but1.png"), 0);
	m_nrEditbox->changed.set
		(this, & Main_Menu_New_Random_Map::nr_edit_box_changed);
	RNG rng;
	rng.seed(clock());
	rng.rand();
	m_mapNumber = rng.rand();
	snprintf
		(buffer, sizeof(buffer), "%u",
		 static_cast<unsigned int>(m_mapNumber));
	m_nrEditbox->setText(buffer);
	posy += height + spacing + spacing + spacing;


	// ---------- Width  ----------


	Widelands::Map const & map = parent->egbase().map();
	{
		Widelands::Extent const map_extent = map.extent();
		for (m_w = 0; Widelands::MAP_DIMENSIONS[m_w] < map_extent.w; ++m_w) {}
		for (m_h = 0; Widelands::MAP_DIMENSIONS[m_h] < map_extent.h; ++m_h) {}
	}


	new UI::Callback_IDButton<Main_Menu_New_Random_Map, int32_t>
		(this,
		 posx, posy, 20, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png"),
		 &Main_Menu_New_Random_Map::button_clicked, *this, 0);

	new UI::Callback_IDButton<Main_Menu_New_Random_Map, int32_t>
		(this,
		 get_inner_w() - spacing - 20, posy, 20, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png"),
		 &Main_Menu_New_Random_Map::button_clicked, *this, 1);

	snprintf
		(buffer, sizeof(buffer), _("Width: %u"), Widelands::MAP_DIMENSIONS[m_w]);
	m_width =
		new UI::Textarea(this, posx + spacing + 20, posy, buffer, Align_Left);

	posy += 20 + spacing + spacing;

	// ---------- Height  ----------

	snprintf
		(buffer, sizeof(buffer),
		 _("Height: %u"), Widelands::MAP_DIMENSIONS[m_h]);
	m_height =
		new UI::Textarea(this, posx + spacing + 20, posy, buffer, Align_Left);

	new UI::Callback_IDButton<Main_Menu_New_Random_Map, int32_t>
		(this,
		 posx, posy, 20, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png"),
		 &Main_Menu_New_Random_Map::button_clicked, *this, 2);

	new UI::Callback_IDButton<Main_Menu_New_Random_Map, int32_t>
		(this,
		 get_inner_w() - spacing - 20, posy, 20, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png"),
		 &Main_Menu_New_Random_Map::button_clicked, *this, 3);

	posy += 20 + spacing + spacing;


	// ---------- Water -----------

	new UI::Callback_IDButton<Main_Menu_New_Random_Map, int32_t>
		(this,
		 posx, posy, 20, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png"),
		 &Main_Menu_New_Random_Map::button_clicked, *this, 4);

	new UI::Callback_IDButton<Main_Menu_New_Random_Map, int32_t>
		(this,
		 get_inner_w() - spacing - 20, posy, 20, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png"),
		 &Main_Menu_New_Random_Map::button_clicked, *this, 5);

	snprintf
		(buffer, sizeof(buffer), _("Water: %u %%"), m_waterval);
	m_water =
		new UI::Textarea(this, posx + spacing + 20, posy, buffer, Align_Left);

	posy += 20 + spacing + spacing;



	// ---------- Land -----------

	new UI::Callback_IDButton<Main_Menu_New_Random_Map, int32_t>
		(this,
		 posx, posy, 20, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png"),
		 &Main_Menu_New_Random_Map::button_clicked, *this, 6);

	new UI::Callback_IDButton<Main_Menu_New_Random_Map, int32_t>
		(this,
		 get_inner_w() - spacing - 20, posy, 20, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png"),
		 &Main_Menu_New_Random_Map::button_clicked, *this, 7);

	snprintf
		(buffer, sizeof(buffer), _("Land: %u %%"), m_landval);
	m_land =
		new UI::Textarea(this, posx + spacing + 20, posy, buffer, Align_Left);

	posy += 20 + spacing + spacing;



	// ---------- Wasteland -----------

	new UI::Callback_IDButton<Main_Menu_New_Random_Map, int32_t>
		(this,
		 posx, posy, 20, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png"),
		 &Main_Menu_New_Random_Map::button_clicked, *this, 10);

	new UI::Callback_IDButton<Main_Menu_New_Random_Map, int32_t>
		(this,
		 get_inner_w() - spacing - 20, posy, 20, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png"),
		 &Main_Menu_New_Random_Map::button_clicked, *this, 11);

	snprintf
		(buffer, sizeof(buffer), _("Wasteland: %u %%"), m_wastelandval);
	m_wasteland =
		new UI::Textarea(this, posx + spacing + 20, posy, buffer, Align_Left);

	posy += 20 + spacing + spacing;



	// ---------- Mountains -----------

	snprintf
		(buffer, sizeof(buffer), _("Mountains: %u %%"),
		 100 - m_waterval - m_landval);
	m_mountains =
		new UI::Textarea(this, posx + spacing + 20, posy, buffer, Align_Left);

	posy += 20 + spacing + spacing;



	// ---------- Island mode ----------

	Point pos(get_inner_w() - spacing - 20, posy);
	m_island_mode = new UI::Checkbox(this, pos);
	m_island_mode->set_state(true);
	m_island_mode->set_id(9);
	m_island_mode->changed.set
		(this, & Main_Menu_New_Random_Map::nr_edit_box_changed);

	new UI::Textarea
		(this, posx + spacing + 20, posy, _("Island mode:"), Align_Left);
	posy += height + spacing;



	// ---------- Worlds ----------

	Widelands::World::get_all_worlds(m_worlds);

	assert(m_worlds.size());
	while (strcmp(map.get_world_name(), m_worlds[m_currentworld].c_str()))
		++m_currentworld;

	m_world = new UI::Callback_IDButton<Main_Menu_New_Random_Map, int32_t>
		(this,
		 posx, posy, width, height,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 &Main_Menu_New_Random_Map::button_clicked, *this, 8,
		 Widelands::World::World(m_worlds[m_currentworld].c_str()).get_name());

	posy += height + spacing + spacing + spacing;

	// ---------- Map ID String edit ----------

	new UI::Textarea
		(this, posx + spacing + 20, posy, _("Map ID:"), Align_Left);
	posy += height + spacing;

	m_idEditbox =
		new UI::EditBox
			(this,
			 posx, posy,
			 width, 20,
			 g_gr->get_picture(PicMod_UI, "pics/but1.png"), 0);
	m_idEditbox->setText("abcd-efgh-ijkl-mnop");
	m_idEditbox->changed.set
		(this, & Main_Menu_New_Random_Map::id_edit_box_changed);
	posy += height + spacing + spacing + spacing;

	// ---------- "Generate Map" button ----------

	m_goButton = new UI::Callback_Button<Main_Menu_New_Random_Map>
		(this,
		 posx, posy, width, height,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 &Main_Menu_New_Random_Map::clicked_create_map, *this,
		 _("Generate Map"));
	posy += height + spacing;

	set_inner_size(get_inner_w(), posy);


	nr_edit_box_changed();
}


/**
 * Called, when button get clicked
*/
void Main_Menu_New_Random_Map::button_clicked(int32_t n) {
	switch (n) {
	case 0: ++m_w; break;
	case 1: --m_w; break;
	case 2: ++m_h; break;
	case 3: --m_h; break;
	case 4:
		if (m_waterval < 60)
			m_waterval += 5;
		if (m_landval + m_waterval > 100)
			m_landval -= 5;
		break;
	case 5:
		if (m_waterval > 0)
			m_waterval -= 5;
		break;
	case 6:
		if (m_landval < 100)
			m_landval += 5;
		if (m_waterval + m_landval > 100)
			m_waterval -= 5;
		break;
	case 7:
		if (m_landval > 0)
			m_landval -= 5;
		break;
	case 8:
		++ m_currentworld;
		if (m_currentworld == m_worlds.size())
			m_currentworld = 0;
		m_world->set_title
			(Widelands::World::World(m_worlds[m_currentworld].c_str()).get_name());
		break;
	case 9:
		break;
	case 10:
		if (m_wastelandval < 70)
			m_wastelandval += 10;
		break;
	case 11:
		if (m_wastelandval > 0)
			m_wastelandval -= 10;
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

	snprintf
		(buffer, sizeof(buffer), _("Water: %i %%"), m_waterval);
	m_water->set_text(buffer);
	snprintf
		(buffer, sizeof(buffer), _("Land: %i %%"), m_landval);
	m_land->set_text(buffer);
	snprintf
		(buffer, sizeof(buffer), _("Wasteland: %i %%"), m_wastelandval);
	m_wasteland->set_text(buffer);
	snprintf
		(buffer, sizeof(buffer), _("Mountains: %i %%"),
		 100 - m_waterval - m_landval);
	m_mountains->set_text(buffer);

	nr_edit_box_changed();  // Update ID String
}

void Main_Menu_New_Random_Map::clicked_create_map() {
	Editor_Interactive & eia =
		dynamic_cast<Editor_Interactive &>(*get_parent());
	Widelands::Editor_Game_Base & egbase = eia.egbase();
	Widelands::Map              & map    = egbase.map();
	UI::ProgressWindow loader;

	// Clean all the stuff up, so we can load
	egbase.cleanup_for_load(true, false);

	if (strcmp(map.get_world_name(), m_worlds[m_currentworld].c_str()))
		eia.change_world();

	Widelands::UniqueRandomMapInfo mapInfo;
	mapInfo.w = Widelands::MAP_DIMENSIONS[m_w];
	mapInfo.h = Widelands::MAP_DIMENSIONS[m_h];
	mapInfo.waterRatio = static_cast<double>(m_waterval) / 100.0;
	mapInfo.landRatio  = static_cast<double>(m_landval) / 100.0;
	mapInfo.wastelandRatio = static_cast<double>(m_wastelandval) / 100.0;
	mapInfo.mapNumber = m_mapNumber;
	mapInfo.islandMode = m_island_mode->get_state();
	mapInfo.numPlayers = 1;

	std::stringstream sstrm;
	sstrm << "Random generated map\nRandom number = "
		<< mapInfo.mapNumber << "\n"
		<< "Water = " << m_waterval << " %\n"
		<< "Land = " << m_landval << " %\n"
		<< "Wasteland = " << m_wastelandval << " %\n"
		<< "ID = " << m_idEditbox->text() << "\n";


	map.create_random_map
		(mapInfo, m_worlds[m_currentworld].c_str(), _("No Name"),
		 g_options.pull_section("global").get_string("realname", _("Unknown")),
		 sstrm.str().c_str());

	egbase.postload     ();
	egbase.load_graphics(loader);

	map.recalc_whole_map();

	eia.set_need_save(true);
	eia.need_complete_redraw();

	die();
}

void Main_Menu_New_Random_Map::id_edit_box_changed()
{
	Widelands::UniqueRandomMapInfo mapInfo;

	std::string str = m_idEditbox->text();

	if (!Widelands::UniqueRandomMapInfo::setFromIdString(mapInfo, str))
		m_goButton->set_enabled(false);
	else {
		std::stringstream sstrm;
		sstrm << mapInfo.mapNumber;
		m_nrEditbox->setText(sstrm.str());

		m_h = 0;
		for (uint32_t ix = 0; ix < NUMBER_OF_MAP_DIMENSIONS; ++ix)
			if (Widelands::MAP_DIMENSIONS[ix] == mapInfo.h)
				m_h = ix;

		m_w = 0;
		for (uint32_t ix = 0; ix < NUMBER_OF_MAP_DIMENSIONS; ++ix)
			if (Widelands::MAP_DIMENSIONS[ix] == mapInfo.w)
				m_w = ix;

		m_landval  = mapInfo.landRatio  * 100.0 + 0.49;
		m_waterval = mapInfo.waterRatio * 100.0 + 0.49;

		button_clicked(-1);  // Update other values in UI as well

		m_goButton->set_enabled(true);
	}
}

void Main_Menu_New_Random_Map::nr_edit_box_changed()
{

	try {
		std::string const text = m_nrEditbox->text();
		std::stringstream sstrm(text);
		unsigned int number;
		sstrm >> number;

		if (!sstrm.fail()) {
			m_mapNumber = number;

			Widelands::UniqueRandomMapInfo mapInfo;

			mapInfo.h = Widelands::MAP_DIMENSIONS[m_h];
			mapInfo.w = Widelands::MAP_DIMENSIONS[m_w];
			mapInfo.waterRatio = static_cast<double>(m_waterval) / 100.0;
			mapInfo.landRatio  = static_cast<double>(m_landval) / 100.0;
			mapInfo.wastelandRatio = static_cast<double>(m_wastelandval) / 100.0;
			mapInfo.mapNumber = m_mapNumber;
			mapInfo.islandMode = m_island_mode->get_state();
			mapInfo.numPlayers = 1;

			std::string idStr;
			Widelands::UniqueRandomMapInfo::generateIdString(idStr, mapInfo);

			m_idEditbox->setText(idStr);

			m_goButton->set_enabled(true);
		} else
			m_goButton->set_enabled(false);
	} catch (...) {
		m_goButton->set_enabled(false);
	}
}
