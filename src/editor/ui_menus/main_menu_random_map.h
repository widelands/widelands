/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_EDITOR_UI_MENUS_MAIN_MENU_RANDOM_MAP_H
#define WL_EDITOR_UI_MENUS_MAIN_MENU_RANDOM_MAP_H

#include "base/macros.h"
#include "editor/ui_menus/map_size_box.h"
#include "ui_basic/box.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/editbox.h"
#include "ui_basic/spinbox.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"

namespace Widelands {
class EditorGameBase;
struct UniqueRandomMapInfo;
}  // namespace Widelands

class EditorInteractive;
struct SinglePlayerGameSettingsProvider;
namespace UI {
template <typename T, typename ID> struct IDButton;
}

class MainMenuNewRandomMapPanel : public UI::Box {
public:
	explicit MainMenuNewRandomMapPanel(
	   UI::Panel& parent, UI::PanelStyle, int32_t inner_w, uint32_t map_w, uint32_t map_h);

	bool do_generate_map(Widelands::EditorGameBase&,
	                     EditorInteractive*,
	                     SinglePlayerGameSettingsProvider*);

	enum class ButtonId : uint8_t {
		kNone,
		kMapSize,
		kWater,
		kLand,
		kWasteland,
		kIslandMode,
		kPlayers
	};

	void set_buttons(UI::Button& o, UI::Button& c) {
		ok_button_ = &o;
		cancel_button_ = &c;
	}

private:
	void button_clicked(ButtonId);
	void id_edit_box_changed();
	void nr_edit_box_changed();

	// Ensures that the sum of our landmass is >= 0% and <= 100%, and changes
	// values as necessary.
	// \param clicked_button: The button that was clicked to change the values.
	//                        This function makes sure that after the normalization,
	//                        the value related to the button will still be set
	//                        as requested by the user.
	void normalize_landmass(MainMenuNewRandomMapPanel::ButtonId clicked_button);

	void set_map_info(Widelands::UniqueRandomMapInfo& map_info) const;

	UI::FontStyle label_style_;

	// UI elements
	int32_t label_height_;

	// Size
	MapSizeBox map_size_box_;

	uint8_t max_players_;
	UI::SpinBox players_;

	// World + Resources
	int current_world_;
	std::vector<std::string> resource_amounts_;
	uint32_t resource_amount_;
	UI::Dropdown<size_t> world_, resources_;

	enum class TerrainDistribution { kDefault, kAlpine, kAtoll, kWasteland, kRandom, kCustom };
	UI::Dropdown<TerrainDistribution> terrains_distribution_;
	void select_terrains_distribution();

	// Land
	int32_t waterval_, landval_, wastelandval_, mountainsval_;
	UI::SpinBox water_, land_, wasteland_;
	UI::Box mountains_box_;
	UI::Textarea mountains_label_, mountains_;

	UI::Checkbox island_mode_;

	// Geeky stuff
	UI::Box map_number_and_id_hbox_, map_number_and_id_vbox_1_, map_number_and_id_vbox_2_;

	uint32_t map_number_;
	UI::Textarea map_number_label_;
	UI::EditBox map_number_edit_;

	UI::Textarea map_id_label_;
	UI::EditBox map_id_edit_;

	UI::Button* ok_button_;
	UI::Button* cancel_button_;

	DISALLOW_COPY_AND_ASSIGN(MainMenuNewRandomMapPanel);
};

class MainMenuNewRandomMap : public UI::UniqueWindow {
public:
	explicit MainMenuNewRandomMap(UI::Panel& parent,
	                              UI::UniqueWindow::Registry&,
	                              uint32_t map_w,
	                              uint32_t map_h);

	bool handle_key(bool down, SDL_Keysym) override;

private:
	UI::Box box_;
	MainMenuNewRandomMapPanel panel_;

	// Buttons
	UI::Box button_box_;
	UI::Button ok_button_, cancel_button_;
	void clicked_ok();
	DISALLOW_COPY_AND_ASSIGN(MainMenuNewRandomMap);
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_MAIN_MENU_RANDOM_MAP_H
