/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_LAUNCH_GAME_H
#define WL_UI_FSMENU_LAUNCH_GAME_H

#include <memory>

#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/textarea.h"
#include "ui_basic/window.h"
#include "ui_fsmenu/main.h"
#include "ui_fsmenu/mapdetailsbox.h"

class GameController;
class LuaInterface;

/**
 * Menu for setting map and mapsettings for single- and multiplayer games.
 */
class FullscreenMenuLaunchGame : public UI::Window {
public:
	FullscreenMenuLaunchGame(FullscreenMenuMain&, GameSettingsProvider*, GameController*);
	~FullscreenMenuLaunchGame() override;

	bool handle_key(bool, SDL_Keysym) override;

protected:
	virtual void clicked_ok() = 0;
	virtual void clicked_back() = 0;
	virtual bool clicked_select_map() = 0;

	FullscreenMenuMain& fsmm_;
	LuaInterface* lua_;

	/// Initializes the label and tooltip for the win condition dropdown and returns 'true' if this
	/// is a scenario or a savegame.
	/// Creates a blank label/tooltip and returns 'false' otherwise.
	bool init_win_condition_label();

	/// Enables or disables the peaceful mode checkbox.
	void update_peaceful_mode();
	/// Enables or disables the custom_starting_positions checkbox.
	void update_custom_starting_positions();

	/// Loads all win conditions that can be played with the map into the selection dropdown.
	/// Disables the dropdown if the map is a scenario.
	void update_win_conditions();
	/// Reads the win conditions that are available for the given map tags and adds the entries to
	/// the
	/// dropdown.
	void load_win_conditions(const std::set<std::string>& tags);
	/// Remembers the win condition that is currently selected in the dropdown.
	virtual void win_condition_selected() = 0;
	/// If the win condition in 'win_condition_script' can be played with the map tags,
	/// parses the win condition and returns it as a std::unique_ptr<LuaTable>.
	/// If this win condition can't be played with the map tags, returns a unique_ptr to nullptr.
	std::unique_ptr<LuaTable> win_condition_if_valid(const std::string& win_condition_script,
	                                                 const std::set<std::string>& tags) const;

	void toggle_peaceful();
	void toggle_custom_starting_positions();

	void layout() override;

	uint32_t standard_element_width_;
	uint32_t standard_element_height_;
	uint32_t padding_;

	UI::Box main_box_;
	UI::Box content_box_;
	UI::Box individual_content_box;
	UI::Box map_box_;

	MapDetailsBox map_details;
	UI::Textarea win_condition_type;
	UI::Dropdown<std::string> win_condition_dropdown_;
	UI::Checkbox peaceful_, custom_starting_positions_;
	std::string last_win_condition_;
	UI::Button ok_, back_;
	GameSettingsProvider* settings_;
	GameController* ctrl_;

	bool peaceful_mode_forbidden_;

private:
	void add_all_widgets();
	void add_behaviour_to_widgets();
};

#endif  // end of include guard: WL_UI_FSMENU_LAUNCH_GAME_H
