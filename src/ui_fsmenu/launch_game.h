/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_LAUNCH_GAME_H
#define WL_UI_FSMENU_LAUNCH_GAME_H

#include <memory>

#include "ui_basic/checkbox.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/spinbox.h"
#include "ui_basic/textarea.h"
#include "ui_fsmenu/mapdetailsbox.h"
#include "ui_fsmenu/menu.h"

class GameController;
class LuaInterface;

namespace FsMenu {

static constexpr double scale_factor = 1.3;
/**
 * Menu for setting map and mapsettings for single- and multiplayer games.
 */
class LaunchGame : public TwoColumnsFullNavigationMenu {
public:
	LaunchGame(MenuCapsule&, GameSettingsProvider&, GameController*, bool preconfigured, bool mpg);
	~LaunchGame() override;

	GameSettingsProvider& settings() const {
		return settings_;
	}

	/// Enables or disables the custom_starting_positions checkbox.
	void update_custom_starting_positions();
	void update_fogless();

protected:
	std::unique_ptr<LuaInterface> lua_;

	virtual void clicked_select_map() = 0;
	virtual void clicked_select_savegame() = 0;

	/// Initializes the label and tooltip for the win condition dropdown and returns 'true' if this
	/// is a scenario or a savegame.
	/// Creates a blank label/tooltip and returns 'false' otherwise.
	bool init_win_condition_label();

	/// Enables or disables the peaceful mode checkbox.
	void update_peaceful_mode();
	/// Hides or shows the desync warning.
	void update_warn_desyncing_addon();

	/// Loads all win conditions that can be played with the map into the selection dropdown.
	/// Disables the dropdown if the map is a scenario.
	void update_win_conditions();
	/// Reads the win conditions that are available for the given map tags and adds the entries to
	/// the
	/// dropdown.
	void load_win_conditions(const std::set<std::string>& tags);
	/// Remembers the win condition that is currently selected in the dropdown.
	virtual void win_condition_selected() = 0;
	/// The time limit for the win condition was changed.
	void win_condition_duration_changed();
	/// If the win condition in 'win_condition_script' can be played with the map tags,
	/// parses the win condition and returns it as a std::unique_ptr<LuaTable>.
	/// If this win condition can't be played with the map tags, returns a unique_ptr to nullptr.
	std::unique_ptr<LuaTable> win_condition_if_valid(const std::string& win_condition_script,
	                                                 const std::set<std::string>& tags) const;

	void toggle_peaceful();
	void toggle_fogless();
	void toggle_custom_starting_positions();
	bool should_write_replay() const;

	void layout() override;

	MapDetailsBox map_details_;
	UI::Textarea configure_game_;
	UI::Checkbox write_replay_;
	UI::MultilineTextarea warn_desyncing_addon_;
	UI::Dropdown<std::string> win_condition_dropdown_;
	UI::SpinBox win_condition_duration_;
	UI::Checkbox peaceful_, fogless_, custom_starting_positions_;
	UI::Button* choose_map_;
	UI::Button* choose_savegame_;
	std::string last_win_condition_;

	GameSettingsProvider& settings_;
	GameController* ctrl_;

	bool peaceful_mode_forbidden_{false};

private:
	void add_all_widgets();
};
}  // namespace FsMenu
#endif  // end of include guard: WL_UI_FSMENU_LAUNCH_GAME_H
