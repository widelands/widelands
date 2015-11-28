/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#ifndef WL_EDITOR_UI_MENUS_EDITOR_MAIN_MENU_RANDOM_MAP_H
#define WL_EDITOR_UI_MENUS_EDITOR_MAIN_MENU_RANDOM_MAP_H

#include <vector>

#include "base/macros.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/editbox.h"
#include "ui_basic/window.h"

namespace Widelands {
	struct UniqueRandomMapInfo;
}

struct EditorInteractive;
namespace UI {
template <typename T, typename ID> struct IDButton;
struct Textarea;
}

/**
 * This is the new map selection menu. It offers
 * the user to choose the new world and a few other
 * things like size, world ....
*/
struct MainMenuNewRandomMap : public UI::Window {
	MainMenuNewRandomMap(EditorInteractive &);

	enum class ButtonId: uint8_t {
		NO_BUTTON,
		MAP_W_PLUS,
		MAP_W_MINUS,
		MAP_H_PLUS,
		MAP_H_MINUS,
		WATER_PLUS,
		WATER_MINUS,
		LAND_PLUS,
		LAND_MINUS,
		WASTE_PLUS,
		WASTE_MINUS,
		PLAYER_PLUS,
		PLAYER_MINUS,
		SWITCH_ISLAND_MODE,
		SWITCH_RES,
		SWITCH_WORLD
	};

private:
	struct WorldDescription {
		std::string name;
		std::string descrname;
	};

	void button_clicked(ButtonId);
	void clicked_create_map();
	void id_edit_box_changed();
	void nr_edit_box_changed();

	// Ensures that the sum of our landmass is >= 0% and <= 100%, and changes
	// values as necessary.
	// \param clicked_button: The button that was clicked to change the values.
	//                        This function makes sure that after the normalization,
	//                        the value related to the button will still be set
	//                        as requested by the user.
	void normalize_landmass(MainMenuNewRandomMap::ButtonId clicked_button);

	void set_map_info(Widelands::UniqueRandomMapInfo & mapInfo) const;

	const std::vector<WorldDescription> m_world_descriptions;
	int m_current_world;
	UI::Textarea * m_width, * m_height, * m_land;
	UI::Textarea * m_water, * m_mountains, * m_wasteland, * m_players;
	UI::Button * m_res;
	UI::Button * m_world;
	UI::Checkbox * m_island_mode;
	UI::Button * m_goButton;
	int32_t m_w, m_h;
	int32_t m_landval, m_waterval, m_wastelandval, m_mountainsval;
	uint8_t m_pn;
	uint32_t m_mapNumber;
	uint32_t m_res_amount;
	std::vector<std::string> m_res_amounts;

	UI::EditBox * m_nrEditbox;
	UI::EditBox * m_idEditbox;

	DISALLOW_COPY_AND_ASSIGN(MainMenuNewRandomMap);
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_EDITOR_MAIN_MENU_RANDOM_MAP_H
