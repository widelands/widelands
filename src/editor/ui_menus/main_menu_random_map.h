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

#ifndef WL_EDITOR_UI_MENUS_MAIN_MENU_RANDOM_MAP_H
#define WL_EDITOR_UI_MENUS_MAIN_MENU_RANDOM_MAP_H

#include <vector>

#include "base/macros.h"
#include "ui_basic/box.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/editbox.h"
#include "ui_basic/spinbox.h"
#include "ui_basic/textarea.h"
#include "ui_basic/window.h"

namespace Widelands {
	struct UniqueRandomMapInfo;
}

class EditorInteractive;
namespace UI {
template <typename T, typename ID> struct IDButton;
}

/**
 * This is the new map selection menu. It offers
 * the user to choose the new world and a few other
 * things like size, world ....
*/
struct MainMenuNewRandomMap : public UI::Window {
	MainMenuNewRandomMap(EditorInteractive &);

	enum class ButtonId: uint8_t {
		kNone,
		kMapSize,
		kWater,
		kLand,
		kWasteland,
		kResources,
		kWorld,
		kIslandMode
	};

private:
	struct WorldDescription {
		std::string name;
		std::string descname;
	};

	void button_clicked(ButtonId);
	void clicked_create_map();
	void clicked_cancel();
	void id_edit_box_changed();
	void nr_edit_box_changed();

	// Ensures that the sum of our landmass is >= 0% and <= 100%, and changes
	// values as necessary.
	// \param clicked_button: The button that was clicked to change the values.
	//                        This function makes sure that after the normalization,
	//                        the value related to the button will still be set
	//                        as requested by the user.
	void normalize_landmass(MainMenuNewRandomMap::ButtonId clicked_button);

	void set_map_info(Widelands::UniqueRandomMapInfo& map_info) const;

	// Helper function to find a map dimension in the global list of available dimensions.
	size_t find_dimension_index(int32_t value);

	// UI elements
	int32_t margin_;
	int32_t box_width_;
	int32_t label_height_;
	UI::Box box_;

	// Size
	UI::SpinBox width_;
	UI::SpinBox height_;

	uint8_t max_players_;
	UI::SpinBox players_;

	// World + Resources
	const std::vector<WorldDescription> world_descriptions_;
	int current_world_;
	std::vector<std::string> resource_amounts_;
	uint32_t resource_amount_;
	UI::Box world_box_, resources_box_;
	UI::Textarea world_label_, resources_label_;
	UI::Button world_, resources_;

	// Land
	int32_t waterval_, landval_, wastelandval_, mountainsval_;
	UI::SpinBox water_, land_, wasteland_;
	UI::Box mountains_box_;
	UI::Textarea mountains_label_, mountains_;

	UI::Checkbox island_mode_;

	// Geeky stuff
	UI::Box map_number_box_;
	uint32_t map_number_;
	UI::Textarea map_number_label_;
	UI::EditBox map_number_edit_;

	UI::Box map_id_box_;
	UI::Textarea map_id_label_;
	UI::EditBox map_id_edit_;

	// Buttons
	UI::Box button_box_;
	UI::Button ok_button_, cancel_button_;

	DISALLOW_COPY_AND_ASSIGN(MainMenuNewRandomMap);
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_MAIN_MENU_RANDOM_MAP_H
