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

#include <boost/format.hpp>

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "editor/map_generator.h"
#include "graphic/graphic.h"
#include "logic/constants.h"
#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "logic/world/world.h"
#include "profile/profile.h"
#include "random/random.h"
#include "ui_basic/button.h"
#include "ui_basic/progresswindow.h"
#include "ui_basic/textarea.h"
#include "ui_basic/window.h"

using namespace Widelands;
// TODO(GunChleoc): Arabic: buttons need more height for Arabic.
MainMenuNewRandomMap::MainMenuNewRandomMap(EditorInteractive& parent) :
	UI::Window(&parent,
				  "random_map_menu",
				  (parent.get_w() - 260) / 2,
				  (parent.get_h() - 450) / 2,
				  305,
				  500,
				  _("New Random Map")),
   // TRANSLATORS: The next are world names for the random map generator.
	m_world_descriptions(
	{
		{"greenland", _("Summer")},
		{"winterland", _("Winter")},
		{"desert", _("Desert")},
		{"blackland", _("Wasteland")},
	}),
	m_current_world(0) {
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
	m_mountainsval = 100 - m_waterval - m_landval - m_wastelandval;
	m_pn = 1;

	// ---------- Random map number edit ----------

	new UI::Textarea(this, posx, posy, _("Random Number:"));
	posy += height + spacing;

	m_nrEditbox =
		new UI::EditBox
			(this,
			 posx, posy,
			 width, height,
			 g_gr->images().get("pics/but1.png"));
	m_nrEditbox->changed.connect
		(boost::bind(&MainMenuNewRandomMap::nr_edit_box_changed, this));
	RNG rng;
	rng.seed(clock());
	rng.rand();
	m_mapNumber = rng.rand();
	m_nrEditbox->set_text(std::to_string(static_cast<unsigned int>(m_mapNumber)));
	posy += height + 3 * spacing;


	// ---------- Width  ----------


	const Widelands::Map & map = parent.egbase().map();
	{
		Widelands::Extent const map_extent = map.extent();

		for (m_w = 0; Widelands::MAP_DIMENSIONS[m_w] < map_extent.w; ++m_w) {}
		for (m_h = 0; Widelands::MAP_DIMENSIONS[m_h] < map_extent.h; ++m_h) {}
	}

	UI::Button * widthupbtn = new UI::Button
		(this, "width_up",
		 get_inner_w() - spacing - height, posy, height, height,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_up.png"));
	widthupbtn->sigclicked.connect
		(boost::bind(&MainMenuNewRandomMap::button_clicked, this, ButtonId::MAP_W_PLUS));

	UI::Button * widthdownbtn = new UI::Button
		(this, "width_down",
		 posx, posy, height, height,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_down.png"));
	widthdownbtn->sigclicked.connect
		(boost::bind(&MainMenuNewRandomMap::button_clicked, this, ButtonId::MAP_W_MINUS));

	m_width = new UI::Textarea(this, posx + spacing + height, posy,
										(boost::format(_("Width: %u"))
										 % Widelands::MAP_DIMENSIONS[m_w]).str());

	posy += height + 2 * spacing;

	// ---------- Height  ----------

	m_height = new UI::Textarea(this, posx + spacing + height, posy,
										 (boost::format(_("Height: %u"))
										  % Widelands::MAP_DIMENSIONS[m_h]).str());

	UI::Button * heightupbtn = new UI::Button
		(this, "height_up",
		 get_inner_w() - spacing - height, posy, height, height,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_up.png"));
	heightupbtn->sigclicked.connect
		(boost::bind(&MainMenuNewRandomMap::button_clicked, this, ButtonId::MAP_H_PLUS));

	UI::Button * heightdownbtn = new UI::Button
		(this, "height_down",
		 posx, posy, height, height,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_down.png"));
	heightdownbtn->sigclicked.connect
		(boost::bind(&MainMenuNewRandomMap::button_clicked, this, ButtonId::MAP_H_MINUS));

	posy += height + 4 * spacing;


	// ---------- Water -----------

	UI::Button * waterupbtn = new UI::Button
		(this, "water_up",
		 get_inner_w() - spacing - height, posy, height, height,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_up.png"));
	waterupbtn->sigclicked.connect
		(boost::bind(&MainMenuNewRandomMap::button_clicked, this, ButtonId::WATER_PLUS));

	UI::Button * waterdownbtn = new UI::Button
		(this, "water_down",
		 posx, posy, height, height,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_down.png"));
	waterdownbtn->sigclicked.connect
		(boost::bind(&MainMenuNewRandomMap::button_clicked, this, ButtonId::WATER_MINUS));

	m_water = new UI::Textarea(this, posx + spacing + height, posy,
										(boost::format(_("Water: %i %%")) % m_waterval).str());

	posy += height + 2 * spacing;



	// ---------- Land -----------

	UI::Button * landupbtn = new UI::Button
		(this, "land_up",
		 get_inner_w() - spacing - height, posy, height, height,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_up.png"));
	landupbtn->sigclicked.connect
		(boost::bind(&MainMenuNewRandomMap::button_clicked, this, ButtonId::LAND_PLUS));

	UI::Button * landdownbtn = new UI::Button
		(this, "land_down",
		 posx, posy, height, height,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_down.png"));
	landdownbtn->sigclicked.connect
		(boost::bind(&MainMenuNewRandomMap::button_clicked, this, ButtonId::LAND_MINUS));

	m_land = new UI::Textarea(this, posx + spacing + height, posy,
									  (boost::format(_("Land: %i %%")) % m_landval).str());

	posy += height + 2 * spacing;



	// ---------- Wasteland -----------

	UI::Button * wastelandupbtn = new UI::Button
		(this, "wasteland_up",
		 get_inner_w() - spacing - height, posy, height, height,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_up.png"));
	wastelandupbtn->sigclicked.connect
		(boost::bind(&MainMenuNewRandomMap::button_clicked, this, ButtonId::WASTE_PLUS));

	UI::Button * wastelanddownbtn = new UI::Button
		(this, "wasteland_down",
		 posx, posy, height, height,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_down.png"));
	wastelanddownbtn->sigclicked.connect
		(boost::bind(&MainMenuNewRandomMap::button_clicked, this, ButtonId::WASTE_MINUS));

	m_wasteland = new UI::Textarea(this, posx + spacing + height, posy,
											 (boost::format(_("Wasteland: %i %%")) % m_wastelandval).str());

	posy += height + 2 * spacing;



	// ---------- Mountains -----------

	m_mountains = new UI::Textarea(this, posx + spacing + height, posy,
											 (boost::format(_("Mountains: %i %%")) % m_mountainsval).str());

	posy += height + 2 * spacing;


	// ---------- Island mode ----------

	m_island_mode = new UI::Checkbox(this, Point(posx, posy), _("Island mode"));
	m_island_mode->set_state(true);
	m_island_mode->changed.connect
		(boost::bind(&MainMenuNewRandomMap::button_clicked, this, ButtonId::SWITCH_ISLAND_MODE));
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
	m_res->sigclicked.connect(boost::bind(&MainMenuNewRandomMap::button_clicked, this, ButtonId::SWITCH_RES));

	posy += height + 3 * spacing;

	// ---------- Worlds ----------
	m_world = new UI::Button
		(this, "world",
		 posx, posy, width, height,
		 g_gr->images().get("pics/but1.png"),
		 m_world_descriptions[m_current_world].descrname);
	m_world->sigclicked.connect
		(boost::bind(&MainMenuNewRandomMap::button_clicked, this, ButtonId::SWITCH_WORLD));

	posy += height + 3 * spacing;

	// ---------- Map ID String edit ----------

	new UI::Textarea(this, posx, posy, _("Map ID:"));
	posy += height + spacing;

	m_idEditbox =
		new UI::EditBox
			(this,
			 posx, posy,
			 width, height,
			 g_gr->images().get("pics/but1.png"));
	m_idEditbox->set_text("abcd-efgh-ijkl-mnop");
	m_idEditbox->changed.connect
		(boost::bind(&MainMenuNewRandomMap::id_edit_box_changed, this));
	posy += height + 3 * spacing;



	// ---------- Players -----------

	UI::Button * playerupbtn = new UI::Button
		(this, "player_up",
		 get_inner_w() - spacing - height, posy, height, height,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_up.png"));
	playerupbtn->sigclicked.connect
		(boost::bind(&MainMenuNewRandomMap::button_clicked, this, ButtonId::PLAYER_PLUS));

	UI::Button * playerdownbtn = new UI::Button
		(this, "player_down",
		 posx, posy, height, height,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_down.png"));
	playerdownbtn->sigclicked.connect
		(boost::bind(&MainMenuNewRandomMap::button_clicked, this, ButtonId::PLAYER_MINUS));

	m_players = new UI::Textarea(this, posx + spacing + height, posy,
										  (boost::format(_("Players: %u"))
											% static_cast<unsigned int>(m_pn)).str());

	posy += height + 2 * spacing;



	// ---------- "Generate Map" button ----------

	m_goButton = new UI::Button
		(this, "generate_map",
		 posx, posy, width, height,
		 g_gr->images().get("pics/but5.png"),
		 _("Generate Map"));
	m_goButton->sigclicked.connect(boost::bind(&MainMenuNewRandomMap::clicked_create_map, this));
	posy += height + spacing;

	set_inner_size(get_inner_w(), posy);


	nr_edit_box_changed();
}


/**
 * Called, when button get clicked
*/
void MainMenuNewRandomMap::button_clicked(MainMenuNewRandomMap::ButtonId n) {
	switch (n) {
	case ButtonId::MAP_W_PLUS: ++m_w; break;
	case ButtonId::MAP_W_MINUS:
		--m_w;
		if (m_w >= 0 && m_pn > m_w + 2)
			--m_pn;
		break;
	case ButtonId::MAP_H_PLUS: ++m_h; break;
	case ButtonId::MAP_H_MINUS:
		--m_h;
		if (m_h >= 0 && m_pn > m_h + 2)
			--m_pn;
		break;
	case ButtonId::PLAYER_PLUS:
		// Only higher the player number, if there is enough space
		if (m_pn < MAX_PLAYERS && m_pn < m_w + 2 && m_pn < m_h + 2)
			++m_pn;
		break;
	case ButtonId::PLAYER_MINUS:
		if (m_pn > 1)
			--m_pn;
		break;
	case ButtonId::WATER_PLUS:
		if (m_waterval < 60)
			m_waterval += 5;
		normalize_landmass(n);
		break;
	case ButtonId::WATER_MINUS:
		if (m_waterval >= 5)
			m_waterval -= 5;
		else
			m_waterval = 0;
		normalize_landmass(n);
		break;
	case ButtonId::LAND_PLUS:
		if (m_landval < 100)
			m_landval += 5;
		normalize_landmass(n);
		break;
	case ButtonId::LAND_MINUS:
		if (m_landval >= 5)
			m_landval -= 5;
		else
			m_landval = 0;
		normalize_landmass(n);
		break;
	case ButtonId::SWITCH_WORLD:
		++ m_current_world;
		m_current_world %= m_world_descriptions.size();
		m_world->set_title(m_world_descriptions[m_current_world].descrname);
		break;
	case ButtonId::SWITCH_ISLAND_MODE:
		break;
	case ButtonId::WASTE_PLUS:
		if (m_wastelandval < 70)
			m_wastelandval += 5;
		normalize_landmass(n);
		break;
	case ButtonId::WASTE_MINUS:
		if (m_wastelandval >= 5)
			m_wastelandval -= 5;
		else
			m_wastelandval = 0;
		normalize_landmass(n);
		break;
	case ButtonId::SWITCH_RES:
		++ m_res_amount;
		if (m_res_amount == m_res_amounts.size())
			m_res_amount = 0;
		m_res->set_title(m_res_amounts[m_res_amount].c_str());
		break;
	default:
		assert(false);
	}

	if (m_w <  0)                        m_w = 0;
	if (m_w >= NUMBER_OF_MAP_DIMENSIONS) m_w = NUMBER_OF_MAP_DIMENSIONS - 1;
	if (m_h <  0)                        m_h = 0;
	if (m_h >= NUMBER_OF_MAP_DIMENSIONS) m_h = NUMBER_OF_MAP_DIMENSIONS - 1;

	m_width ->set_text((boost::format(_("Width: %u")) % Widelands::MAP_DIMENSIONS[m_w]).str());
	m_height->set_text((boost::format(_("Height: %u")) % Widelands::MAP_DIMENSIONS[m_h]).str());
	m_water->set_text((boost::format(_("Water: %i %%")) % m_waterval).str());
	m_land->set_text((boost::format(_("Land: %i %%")) % m_landval).str());
	m_wasteland->set_text((boost::format(_("Wasteland: %i %%")) % m_wastelandval).str());
	m_mountains->set_text((boost::format(_("Mountains: %i %%")) % m_mountainsval).str());
	m_players->set_text((boost::format(_("Players: %u"))
								% static_cast<unsigned int>(m_pn)).str());

	nr_edit_box_changed();  // Update ID String
}


// If the the sum of our landmass is < 0% or > 100% change the mountain value.
// If the mountain value gets out of range, change the other values.
void MainMenuNewRandomMap::normalize_landmass(ButtonId clicked_button) {
	int32_t sum_without_mountainsval = m_waterval + m_landval + m_wastelandval;

	// Prefer changing mountainsval to keep consistency with old behaviour
	while (sum_without_mountainsval + m_mountainsval > 100) {
			m_mountainsval -= 1;
	}
	while (sum_without_mountainsval + m_mountainsval < 100) {
			m_mountainsval += 1;
	}

	// Compensate if mountainsval got above 100% / below 0%
	while (m_mountainsval < 0) {
		if (clicked_button != ButtonId::WASTE_PLUS && m_wastelandval > 0) {
			m_wastelandval -= 5;
			m_mountainsval += 5;
		}
		if (m_mountainsval < 0 && clicked_button != ButtonId::LAND_PLUS && m_landval > 0) {
			m_landval -= 5;
			m_mountainsval += 5;
		}
		if (m_mountainsval < 0 && clicked_button != ButtonId::WATER_PLUS && m_waterval > 0) {
			m_waterval -= 5;
			m_mountainsval += 5;
		}
	}

	while (m_mountainsval > 100) {
		if (clicked_button != ButtonId::WASTE_MINUS && m_wastelandval < 100) {
			m_wastelandval += 5;
			m_mountainsval -= 5;
		}
		if (m_mountainsval > 100 && clicked_button != ButtonId::LAND_MINUS && m_landval < 100) {
			m_landval += 5;
			m_mountainsval -= 5;
		}
		if (m_mountainsval > 100 && clicked_button != ButtonId::WATER_MINUS && m_waterval < 100) {
			m_waterval += 5;
			m_mountainsval -= 5;
		}
	}
}

void MainMenuNewRandomMap::clicked_create_map() {
	EditorInteractive & eia =
		dynamic_cast<EditorInteractive&>(*get_parent());
	Widelands::EditorGameBase & egbase = eia.egbase();
	Widelands::Map              & map    = egbase.map();
	UI::ProgressWindow loader;

	egbase.cleanup_for_load();

	UniqueRandomMapInfo mapInfo;
	set_map_info(mapInfo);

	std::stringstream sstrm;
	sstrm << "Random generated map\nRandom number = "
		<< mapInfo.mapNumber << "\n"
		<< "Water = " << m_waterval << " %\n"
		<< "Land = " << m_landval << " %\n"
		<< "Wasteland = " << m_wastelandval << " %\n"
		<< "Resources = " << m_res->get_title() << "\n"
		<< "ID = " << m_idEditbox->text() << "\n";

	MapGenerator gen(map, mapInfo, egbase);
	map.create_empty_map(
		egbase.world(),
		mapInfo.w,
		mapInfo.h,
		_("No Name"),
		g_options.pull_section("global").get_string("realname", pgettext("map_name", "Unknown")),
		sstrm.str().c_str());
	loader.step(_("Generating random map..."));
	gen.create_random_map();

	egbase.postload     ();
	egbase.load_graphics(loader);

	map.recalc_whole_map(egbase.world());

	eia.set_need_save(true);
	eia.register_overlays();

	die();
}

void MainMenuNewRandomMap::id_edit_box_changed()
{
	UniqueRandomMapInfo mapInfo;

	std::string str = m_idEditbox->text();

	if (!UniqueRandomMapInfo::set_from_id_string(mapInfo, str))
		m_goButton->set_enabled(false);
	else {
		std::stringstream sstrm;
		sstrm << mapInfo.mapNumber;
		m_nrEditbox->set_text(sstrm.str());

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

		// Update other values in UI as well
		button_clicked(static_cast<ButtonId>(-1));

		m_goButton->set_enabled(true);
	}
}

void MainMenuNewRandomMap::nr_edit_box_changed()
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
			Widelands::UniqueRandomMapInfo::generate_id_string(idStr, mapInfo);

			m_idEditbox->set_text(idStr);

			m_goButton->set_enabled(true);
		} else
			m_goButton->set_enabled(false);
	} catch (...) {
		m_goButton->set_enabled(false);
	}
}

void MainMenuNewRandomMap::set_map_info
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
		<Widelands::UniqueRandomMapInfo::ResourceAmount>
			(m_res_amount);
	mapInfo.world_name = m_world_descriptions[m_current_world].name;
}
