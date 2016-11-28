/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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
#include <string>

#include "graphic/playercolor.h"
#include "logic/map.h"
#include "ui_basic/button.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "ui_fsmenu/base.h"

struct ChatProvider;
class GameController;
struct GameSettingsProvider;
class LuaInterface;

/**
 * Fullscreen menu for setting map and mapsettings for single and multi player
 * games.
 *
 * The menu has a lot dynamic user-interfaces, that are only shown in specific
 * cases:
 *    UI::Button select_map_  - only shown if the player has the right to
 *                               change the map.
 *
 */
// NOCOM fix comments
class FullscreenMenuLaunchGame : public FullscreenMenuBase {
public:
	FullscreenMenuLaunchGame(GameSettingsProvider*, GameController*);
	~FullscreenMenuLaunchGame();

	void think() override;

	virtual void refresh() = 0;

protected:
	void clicked_ok() override;
	void clicked_back() override;

	LuaInterface* lua_;

	/// Loads all win conditions that can be played with the map into the selection dropdown.
	/// Disables the dropdown if the map is a scenario.
	void update_win_conditions();
	/// Reads the win conditions that are available for the given map and adds the entries to the
	/// dropdown.
	void load_win_conditions(const Widelands::Map& map);
	/// Remembers the win condition that is currently selected in the dropdown.
	virtual void win_condition_selected() = 0;
	/// If the win condition in 'win_condition_script' can be played with the map tags,
	/// parses the win condition and returns it as a std::unique_ptr<LuaTable>.
	/// If this win condition can't be played with the map tags, returns a unique_ptr to nullptr.
	std::unique_ptr<LuaTable> win_condition_if_valid(const std::string& win_condition_script,
																	 std::set<std::string> tags) const;
	void set_scenario_values();

	uint32_t butw_;
	uint32_t buth_;

	UI::Dropdown<std::string> win_condition_dropdown_;
	std::string last_win_condition_;
	UI::Button ok_, back_;
	UI::Textarea title_;
	GameSettingsProvider* settings_;
	GameController* ctrl_;

	Widelands::PlayerNumber nr_players_;
};

#endif  // end of include guard: WL_UI_FSMENU_LAUNCH_GAME_H
