/*
 * Copyright (C) 2002-2004, 2006-2011, 2013 by the Widelands Development Team
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

#include "editor/ui_menus/editor_main_menu_random_map.h"

#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include "editor/editorinteractive.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "logic/world.h"
#include "map_generator.h"
#include "profile/profile.h"
#include "random.h"
#include "ui_basic/button.h"
#include "ui_basic/progresswindow.h"
#include "ui_basic/textarea.h"
#include "ui_basic/window.h"

using namespace Widelands;

Main_Menu_New_Random_Map::Main_Menu_New_Random_Map
	(Editor_Interactive & parent)
	:
	UI::Window
		(&parent, "random_map_menu",
		 (parent.get_w() - 260) / 2, (parent.get_h() - 450) / 2, 260, 490,
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
	m_pn = 1;

	// ---------- Random map number edit ----------

	new UI::Textarea(this, posx, posy, _("Random Number:"));
	posy += height + spacing;

	m_nrEditbox =
		new UI::EditBox
			(this,
			 posx, posy,
			 width, 20,
			 g_gr->images().get("pics/but1.png"));
	m_nrEditbox->changed.connect
		(boost::bind(&Main_Menu_New_Random_Map::nr_edit_box_changed, this));
	RNG rng;
	rng.seed(clock());
	rng.rand();
	m_mapNumber = rng.rand();
	snprintf
		(buffer, sizeof(buffer), "%u", static_cast<unsigned int>(m_mapNumber));
	m_nrEditbox->setText(buffer);
	posy += height + spacing + spacing + spacing;


	// ---------- Width  ----------


	const Widelands::Map & map = parent.egbase().map();
	{
		Widelands::Extent const map_extent = map.extent();

		for (m_w = 0; Widelands::MAP_DIMENSIONS[m_w] < map_extent.w; ++m_w) {}
		for (m_h = 0; Widelands::MAP_DIMENSIONS[m_h] < map_extent.h; ++m_h) {}
	}

	UI::Button * widthupbtn = new UI::Button
		(this, "width_up",
		 get_inner_w() - spacing - 20, posy, 20, 20,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_up.png"));
	widthupbtn->sigclicked.connect
		(boost::bind(&Main_Menu_New_Random_Map::button_clicked, this, MAP_W_PLUS));

	UI::Button * widthdownbtn = new UI::Button
		(this, "width_down",
		 posx, posy, 20, 20,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_down.png"));
	widthdownbtn->sigclicked.connect
		(boost::bind(&Main_Menu_New_Random_Map::button_clicked, this, MAP_W_MINUS));

	snprintf
		(buffer, sizeof(buffer), _("Width: %u"), Widelands::MAP_DIMENSIONS[m_w]);
	m_width =
		new UI::Textarea(this, posx + spacing + 20, posy, buffer);

	posy += 20 + spacing + spacing;

	// ---------- Height  ----------

	snprintf
		(buffer, sizeof(buffer),
		 _("Height: %u"), Widelands::MAP_DIMENSIONS[m_h]);
	m_height = new UI::Textarea(this, posx + spacing + 20, posy, buffer);

	UI::Button * heightupbtn = new UI::Button
		(this, "height_up",
		 get_inner_w() - spacing - 20, posy, 20, 20,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_up.png"));
	heightupbtn->sigclicked.connect
		(boost::bind(&Main_Menu_New_Random_Map::button_clicked, this, MAP_H_PLUS));

	UI::Button * heightdownbtn = new UI::Button
		(this, "height_down",
		 posx, posy, 20, 20,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_down.png"));
	heightdownbtn->sigclicked.connect
		(boost::bind(&Main_Menu_New_Random_Map::button_clicked, this, MAP_H_MINUS));

	posy += 20 + spacing + spacing;


	// ---------- Water -----------

	UI::Button * waterupbtn = new UI::Button
		(this, "water_up",
		 get_inner_w() - spacing - 20, posy, 20, 20,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_up.png"));
	waterupbtn->sigclicked.connect
		(boost::bind(&Main_Menu_New_Random_Map::button_clicked, this, WATER_PLUS));

	UI::Button * waterdownbtn = new UI::Button
		(this, "water_down",
		 posx, posy, 20, 20,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_down.png"));
	waterdownbtn->sigclicked.connect
		(boost::bind(&Main_Menu_New_Random_Map::button_clicked, this, WATER_MINUS));

	snprintf(buffer, sizeof(buffer), _("Water: %u %%"), m_waterval);
	m_water = new UI::Textarea(this, posx + spacing + 20, posy, buffer);

	posy += 20 + spacing + spacing;



	// ---------- Land -----------

	UI::Button * landupbtn = new UI::Button
		(this, "land_up",
		 get_inner_w() - spacing - 20, posy, 20, 20,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_up.png"));
	landupbtn->sigclicked.connect
		(boost::bind(&Main_Menu_New_Random_Map::button_clicked, this, LAND_PLUS));

	UI::Button * landdownbtn = new UI::Button
		(this, "land_down",
		 posx, posy, 20, 20,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_down.png"));
	landdownbtn->sigclicked.connect
		(boost::bind(&Main_Menu_New_Random_Map::button_clicked, this, LAND_MINUS));

	snprintf
		(buffer, sizeof(buffer), _("Land: %u %%"), m_landval);
	m_land = new UI::Textarea(this, posx + spacing + 20, posy, buffer);

	posy += 20 + spacing + spacing;



	// ---------- Wasteland -----------

	UI::Button * wastelandupbtn = new UI::Button
		(this, "wasteland_up",
		 get_inner_w() - spacing - 20, posy, 20, 20,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_up.png"));
	wastelandupbtn->sigclicked.connect
		(boost::bind(&Main_Menu_New_Random_Map::button_clicked, this, WASTE_PLUS));

	UI::Button * wastelanddownbtn = new UI::Button
		(this, "wasteland_down",
		 posx, posy, 20, 20,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_down.png"));
	wastelanddownbtn->sigclicked.connect
		(boost::bind(&Main_Menu_New_Random_Map::button_clicked, this, WASTE_MINUS));

	snprintf
		(buffer, sizeof(buffer), _("Wasteland: %u %%"), m_wastelandval);
	m_wasteland = new UI::Textarea(this, posx + spacing + 20, posy, buffer);

	posy += 20 + spacing + spacing;



	// ---------- Mountains -----------

	snprintf
		(buffer, sizeof(buffer), _("Mountains: %u %%"),
		 100 - m_waterval - m_landval);
	m_mountains = new UI::Textarea(this, posx + spacing + 20, posy, buffer);

	posy += 20 + spacing + spacing;



	// ---------- Island mode ----------

	Point pos(get_inner_w() - spacing - 20, posy);
	m_island_mode = new UI::Checkbox(this, pos);
	m_island_mode->set_state(true);
	m_island_mode->changed.connect
		(boost::bind(&Main_Menu_New_Random_Map::button_clicked, this, SWITCH_ISLAND_MODE));

	new UI::Textarea(this, posx, posy, _("Island mode:"));
	posy += height + spacing;



	// ---------- Amount of Resources (Low/Medium/High) ----------

	new UI::Textarea(this, posx, posy, _("Resources:"));
	posy += height + spacing;

	m_res_amounts.push_back(_("Low"));
	m_res_amounts.push_back(_("Medium"));
	m_res_amounts.push_back(_("High"));

	m_res_amount = 2;

	m_res = new UI::Button
		(this, "resources",
		 posx, posy, width, height,
		 g_gr->images().get("pics/but1.png"),
		 m_res_amounts[m_res_amount].c_str());
	m_res->sigclicked.connect(boost::bind(&Main_Menu_New_Random_Map::button_clicked, this, SWITCH_RES));

	posy += height + spacing + spacing + spacing;



	// ---------- Worlds ----------

	Widelands::World::get_all_worlds(m_worlds);

	assert(m_worlds.size());
	while (strcmp(map.get_world_name(), m_worlds[m_currentworld].c_str()))
		++m_currentworld;

	m_world = new UI::Button
		(this, "world",
		 posx, posy, width, height,
		 g_gr->images().get("pics/but1.png"),
		 Widelands::World(m_worlds[m_currentworld].c_str()).get_name());
	m_world->sigclicked.connect
		(boost::bind(&Main_Menu_New_Random_Map::button_clicked, this, SWITCH_WORLD));

	posy += height + spacing + spacing + spacing;



	// ---------- Map ID String edit ----------

	new UI::Textarea(this, posx + spacing + 20, posy, _("Map ID:"));
	posy += height + spacing;

	m_idEditbox =
		new UI::EditBox
			(this,
			 posx, posy,
			 width, 20,
			 g_gr->images().get("pics/but1.png"));
	m_idEditbox->setText("abcd-efgh-ijkl-mnop");
	m_idEditbox->changed.connect
		(boost::bind(&Main_Menu_New_Random_Map::id_edit_box_changed, this));
	posy += height + spacing + spacing + spacing;



	// ---------- Players -----------

	UI::Button * playerupbtn = new UI::Button
		(this, "player_up",
		 get_inner_w() - spacing - 20, posy, 20, 20,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_up.png"));
	playerupbtn->sigclicked.connect
		(boost::bind(&Main_Menu_New_Random_Map::button_clicked, this, PLAYER_PLUS));

	UI::Button * playerdownbtn = new UI::Button
		(this, "player_down",
		 posx, posy, 20, 20,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_down.png"));
	playerdownbtn->sigclicked.connect
		(boost::bind(&Main_Menu_New_Random_Map::button_clicked, this, PLAYER_MINUS));

	snprintf(buffer, sizeof(buffer), _("Players: %u"), m_pn);
	m_players = new UI::Textarea(this, posx + spacing + 20, posy, buffer);

	posy += 20 + spacing + spacing;



	// ---------- "Generate Map" button ----------

	m_goButton = new UI::Button
		(this, "generate_map",
		 posx, posy, width, height,
		 g_gr->images().get("pics/but0.png"),
		 _("Generate Map"));
	m_goButton->sigclicked.connect(boost::bind(&Main_Menu_New_Random_Map::clicked_create_map, this));
	posy += height + spacing;

	set_inner_size(get_inner_w(), posy);


	nr_edit_box_changed();
}


/**
 * Called, when button get clicked
*/
void Main_Menu_New_Random_Map::button_clicked(Main_Menu_New_Random_Map::ButtonID n) {
	switch (n) {
	case MAP_W_PLUS: ++m_w; break;
	case MAP_W_MINUS:
		--m_w;
		if (m_w >= 0 && m_pn > m_w + 2)
			--m_pn;
		break;
	case MAP_H_PLUS: ++m_h; break;
	case MAP_H_MINUS:
		--m_h;
		if (m_h >= 0 && m_pn > m_h + 2)
			--m_pn;
		break;
	case PLAYER_PLUS:
		// Only higher the player number, if there is enough space
		if (m_pn < MAX_PLAYERS && m_pn < m_w + 2 && m_pn < m_h + 2)
			++m_pn;
		break;
	case PLAYER_MINUS:
		if (m_pn > 1)
			--m_pn;
		break;
	case WATER_PLUS:
		if (m_waterval < 60)
			m_waterval += 5;
		if (m_landval + m_waterval > 100)
			m_landval -= 5;
		break;
	case WATER_MINUS:
		if (m_waterval > 0)
			m_waterval -= 5;
		break;
	case LAND_PLUS:
		if (m_landval < 100)
			m_landval += 5;
		if (m_waterval + m_landval > 100)
			m_waterval -= 5;
		break;
	case LAND_MINUS:
		if (m_landval > 0)
			m_landval -= 5;
		break;
	case SWITCH_WORLD:
		++ m_currentworld;
		if (m_currentworld == m_worlds.size())
			m_currentworld = 0;
		m_world->set_title
			(Widelands::World(m_worlds[m_currentworld].c_str()).get_name());
		break;
	case SWITCH_ISLAND_MODE:
		break;
	case WASTE_PLUS:
		if (m_wastelandval < 70)
			m_wastelandval += 10;
		break;
	case WASTE_MINUS:
		if (m_wastelandval > 0)
			m_wastelandval -= 10;
		break;
	case SWITCH_RES:
		++ m_res_amount;
		if (m_res_amount == m_res_amounts.size())
			m_res_amount = 0;
		m_res->set_title(m_res_amounts[m_res_amount].c_str());
		break;
	default:
		assert(false);
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
		(buffer, sizeof(buffer), _("Water: %u %%"), m_waterval);
	m_water->set_text(buffer);
	snprintf
		(buffer, sizeof(buffer), _("Land: %u %%"), m_landval);
	m_land->set_text(buffer);
	snprintf
		(buffer, sizeof(buffer), _("Wasteland: %u %%"), m_wastelandval);
	m_wasteland->set_text(buffer);
	snprintf
		(buffer, sizeof(buffer), _("Mountains: %u %%"),
		 100 - m_waterval - m_landval);
	m_mountains->set_text(buffer);
	snprintf(buffer, sizeof(buffer), _("Players: %u"), m_pn);
	m_players->set_text(buffer);

	nr_edit_box_changed();  // Update ID String
}

void Main_Menu_New_Random_Map::clicked_create_map() {
	Editor_Interactive & eia =
		ref_cast<Editor_Interactive, UI::Panel>(*get_parent());
	Widelands::Editor_Game_Base & egbase = eia.egbase();
	Widelands::Map              & map    = egbase.map();
	UI::ProgressWindow loader;

	egbase.cleanup_for_load();

	if (strcmp(map.get_world_name(), m_worlds[m_currentworld].c_str()))
		eia.change_world();

	UniqueRandomMapInfo mapInfo;
	set_map_info(mapInfo);

	std::stringstream sstrm;
	sstrm << "Random generated map\nRandom number = "
		<< mapInfo.mapNumber << "\n"
		<< "World = " << m_world->get_title() << "\n"
		<< "Water = " << m_waterval << " %\n"
		<< "Land = " << m_landval << " %\n"
		<< "Wasteland = " << m_wastelandval << " %\n"
		<< "Resources = " << m_res->get_title() << "\n"
		<< "ID = " << m_idEditbox->text() << "\n";

	MapGenerator gen(map, mapInfo, egbase);
	map.create_empty_map
		(mapInfo.w, mapInfo.h,
		 m_worlds[m_currentworld].c_str(), _("No Name"),
		 g_options.pull_section("global").get_string("realname", _("Unknown")),
		 sstrm.str().c_str());
	loader.step(_("Generating random map..."));
	gen.create_random_map();

	egbase.postload     ();
	egbase.load_graphics(loader);

	map.recalc_whole_map();

	eia.set_need_save(true);
	eia.need_complete_redraw();
	eia.register_overlays();

	die();
}

void Main_Menu_New_Random_Map::id_edit_box_changed()
{
	UniqueRandomMapInfo mapInfo;

	std::string str = m_idEditbox->text();

	if (!UniqueRandomMapInfo::setFromIdString(mapInfo, str, m_worlds))
		m_goButton->set_enabled(false);
	else {
		std::stringstream sstrm;
		sstrm << mapInfo.mapNumber;
		m_nrEditbox->setText(sstrm.str());

		m_h = 0;
		for (uint32_t ix = 0; ix < NUMBER_OF_MAP_DIMENSIONS; ++ix)
			if (MAP_DIMENSIONS[ix] == mapInfo.h)
				m_h = ix;

		m_w = 0;
		for (uint32_t ix = 0; ix < NUMBER_OF_MAP_DIMENSIONS; ++ix)
			if (MAP_DIMENSIONS[ix] == mapInfo.w)
				m_w = ix;

		m_landval  = mapInfo.landRatio  * 100.0 + 0.49;
		m_waterval = mapInfo.waterRatio * 100.0 + 0.49;
		m_res_amount = mapInfo.resource_amount;

		m_res->set_title(m_res_amounts[m_res_amount].c_str());

		// Get world

		m_currentworld = 0;
		while
			(strcmp(mapInfo.worldName.c_str(), m_worlds[m_currentworld].c_str()))
			++m_currentworld;
		m_world->set_title
			(Widelands::World(m_worlds[m_currentworld].c_str()).get_name());

		// Update other values in UI as well
		button_clicked(static_cast<ButtonID>(-1));

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
			set_map_info(mapInfo);

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

void Main_Menu_New_Random_Map::set_map_info
	(Widelands::UniqueRandomMapInfo & mapInfo) const
{
	mapInfo.h = Widelands::MAP_DIMENSIONS[m_h];
	mapInfo.w = Widelands::MAP_DIMENSIONS[m_w];
	mapInfo.waterRatio = static_cast<double>(m_waterval) / 100.0;
	mapInfo.landRatio  = static_cast<double>(m_landval) / 100.0;
	mapInfo.wastelandRatio = static_cast<double>(m_wastelandval) / 100.0;
	mapInfo.mapNumber = m_mapNumber;
	mapInfo.islandMode = m_island_mode->get_state();
	mapInfo.numPlayers = m_pn;
	mapInfo.resource_amount = static_cast
		<Widelands::UniqueRandomMapInfo::Resource_Amount>
			(m_res_amount);
	mapInfo.worldName = m_worlds[m_currentworld];
}
