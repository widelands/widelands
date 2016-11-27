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

#ifndef WL_UI_FSMENU_LAUNCH_SPG_H
#define WL_UI_FSMENU_LAUNCH_SPG_H

#include <memory>
#include <string>

#include "graphic/playercolor.h"
#include "logic/map.h"
#include "ui_basic/button.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "ui_fsmenu/launch_game.h"

struct ChatProvider;
class GameController;
struct GameSettingsProvider;
struct PlayerDescriptionGroup;
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
class FullscreenMenuLaunchSPG : public FullscreenMenuLaunchGame {
public:
	FullscreenMenuLaunchSPG(GameSettingsProvider*, GameController* = nullptr);
	~FullscreenMenuLaunchSPG();

	void start();
	void refresh() override;

protected:
	void clicked_ok() override;
	void clicked_back() override;

private:
	void select_map();
	void win_condition_selected() override;
	void set_scenario_values();
	void switch_to_position(uint8_t);
	void safe_place_for_host(uint8_t);

	UI::Button select_map_;
	UI::Button* pos_[kMaxPlayers];
	UI::Textarea mapname_;
	UI::Textarea name_, type_, team_, tribe_, init_, wincondition_type_;
	PlayerDescriptionGroup* players_[kMaxPlayers];
	std::string filename_;
	std::string filename_proof_;  // local var. to check UI state
	std::string player_save_name_[kMaxPlayers];
	std::string player_save_tribe_[kMaxPlayers];
	bool is_scenario_;
};

#endif  // end of include guard: WL_UI_FSMENU_LAUNCH_SPG_H
