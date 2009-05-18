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

#ifndef EDITOR_MAIN_MENU_RANDOM_MAP_H
#define EDITOR_MAIN_MENU_RANDOM_MAP_H

#include "ui_basic/window.h"
#include "ui_basic/editbox.h"
#include "ui_basic/checkbox.h"

#include <vector>

struct Editor_Interactive;
namespace UI {
template <typename T, typename ID> struct IDButton;
struct Textarea;
};

/**
 * This is the new map selection menu. It offers
 * the user to choose the new world and a few other
 * things like size, world ....
*/
struct Main_Menu_New_Random_Map : public UI::Window {
	Main_Menu_New_Random_Map(Editor_Interactive *);

private:
	Editor_Interactive * m_parent;
	UI::Textarea * m_width, * m_height;
	UI::Textarea * m_land, * m_water, * m_mountains, * m_wasteland;
	UI::Callback_IDButton<Main_Menu_New_Random_Map, int32_t> * m_world;
	UI::Checkbox * m_island_mode;
	UI::Callback_Button<Main_Menu_New_Random_Map>* m_goButton;
	int32_t m_w, m_h, m_landval, m_waterval, m_wastelandval;
	uint32_t m_mapNumber;
	std::vector<std::string>::size_type m_currentworld;
	std::vector<std::string> m_worlds;

	UI::EditBox * m_nrEditbox;
	UI::EditBox * m_idEditbox;

	void button_clicked(int32_t);
	void clicked_create_map();
	void id_edit_box_changed();
	void nr_edit_box_changed();
};

#endif
