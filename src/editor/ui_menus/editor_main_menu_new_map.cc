/*
 * Copyright (C) 2002-2004, 2006-2009, 2013 by the Widelands Development Team
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

#include "editor/ui_menus/editor_main_menu_new_map.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "graphic/graphic.h"
#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "logic/world/world.h"
#include "ui_basic/button.h"
#include "ui_basic/progresswindow.h"
#include "ui_basic/textarea.h"
#include "ui_basic/window.h"

using Widelands::NUMBER_OF_MAP_DIMENSIONS;

MainMenuNewMap::MainMenuNewMap(EditorInteractive & parent)
	:
	UI::Window
		(&parent, "new_map_menu",
		 (parent.get_w() - 180) / 2, (parent.get_h() - 150) / 2, 180, 150,
		 _("New Map"))
{
	int32_t const offsx   =  5;
	int32_t const offsy   = 30;
	int32_t const spacing =  5;
	int32_t const width   = get_inner_w() - offsx * 2;
	int32_t const height  = 20;
	int32_t       posx    = offsx;
	int32_t       posy    = offsy;
	const Widelands::Map & map = parent.egbase().map();
	{
		Widelands::Extent const map_extent = map.extent();
		for (m_w = 0; Widelands::MAP_DIMENSIONS[m_w] < map_extent.w; ++m_w) {}
		for (m_h = 0; Widelands::MAP_DIMENSIONS[m_h] < map_extent.h; ++m_h) {}
	}

	m_width = new UI::Textarea(this, posx + spacing + 20, posy,
										(boost::format(_("Width: %u")) % Widelands::MAP_DIMENSIONS[m_w]).str());

	UI::Button * widthupbtn = new UI::Button
		(this, "width_up",
		 get_inner_w() - spacing - 20, posy, 20, 20,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_up.png"));
	widthupbtn->sigclicked.connect(boost::bind(&MainMenuNewMap::button_clicked, this, 0));

	UI::Button * widthdownbtn = new UI::Button
		(this, "width_down",
		 posx, posy, 20, 20,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_down.png"));
	widthdownbtn->sigclicked.connect(boost::bind(&MainMenuNewMap::button_clicked, this, 1));

	posy += 20 + spacing + spacing;

	m_height = new UI::Textarea(this, posx + spacing + 20, posy,
										 (boost::format(_("Height: %u"))
										  % Widelands::MAP_DIMENSIONS[m_h]).str());

	UI::Button * heightupbtn = new UI::Button
		(this, "height_up",
		 get_inner_w() - spacing - 20, posy, 20, 20,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_up.png"));
	heightupbtn->sigclicked.connect(boost::bind(&MainMenuNewMap::button_clicked, this, 2));

	UI::Button * heightdownbtn = new UI::Button
		(this, "height_down",
		 posx, posy, 20, 20,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/scrollbar_down.png"));
	heightdownbtn->sigclicked.connect(boost::bind(&MainMenuNewMap::button_clicked, this, 3));

	posy += 20 + spacing + spacing;

	posy += height + spacing + spacing + spacing;

	UI::Button * createbtn = new UI::Button
		(this, "create_map",
		 posx, posy, width, height,
		 g_gr->images().get("pics/but5.png"),
		 _("Create Map"));
	createbtn->sigclicked.connect(boost::bind(&MainMenuNewMap::clicked_create_map, this));
}


/**
 * Called, when button get clicked
*/
void MainMenuNewMap::button_clicked(int32_t n) {
	switch (n) {
	case 0: ++m_w; break;
	case 1: --m_w; break;
	case 2: ++m_h; break;
	case 3: --m_h; break;
	default:
		assert(false);
	}

	if (m_w <  0)                        m_w = 0;
	if (m_w >= NUMBER_OF_MAP_DIMENSIONS) m_w = NUMBER_OF_MAP_DIMENSIONS - 1;
	if (m_h <  0)                        m_h = 0;
	if (m_h >= NUMBER_OF_MAP_DIMENSIONS) m_h = NUMBER_OF_MAP_DIMENSIONS - 1;
	m_width ->set_text((boost::format(_("Width: %u")) % Widelands::MAP_DIMENSIONS[m_w]).str());
	m_height->set_text((boost::format(_("Height: %u")) % Widelands::MAP_DIMENSIONS[m_h]).str());
}

void MainMenuNewMap::clicked_create_map() {
	EditorInteractive & eia =
		dynamic_cast<EditorInteractive&>(*get_parent());
	Widelands::EditorGameBase & egbase = eia.egbase();
	Widelands::Map              & map    = egbase.map();
	UI::ProgressWindow loader;

	egbase.cleanup_for_load();

	map.create_empty_map(
				egbase.world(),
				Widelands::MAP_DIMENSIONS[m_w],
				Widelands::MAP_DIMENSIONS[m_h],
				_("No Name"),
				g_options.pull_section("global").get_string("realname", pgettext("map_name", "Unknown")));

	egbase.postload     ();
	egbase.load_graphics(loader);

	map.recalc_whole_map(egbase.world());

	eia.set_need_save(true);

	die();
}
