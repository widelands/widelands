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

#include <string>

#include "logic/constants.h"
#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "ui_fsmenu/base.h"

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
class FullscreenMenuLaunchSPG : public FullscreenMenuBase {
public:
	FullscreenMenuLaunchSPG(GameSettingsProvider*,
	                        GameController* = nullptr,
	                        bool autolaunch = false);
	~FullscreenMenuLaunchSPG();

	void start() override;
	void think() override;

	void refresh();

protected:
	void clicked_ok() override;
	void clicked_back() override;

private:
	void layout() override;

	LuaInterface* lua_;

	void select_map();
	void win_condition_clicked();
	void win_condition_update();
	void win_condition_load();
	void set_scenario_values();
	void switch_to_position(uint8_t);
	void safe_place_for_host(uint8_t);

	uint32_t butw_;
	uint32_t buth_;

	UI::Button select_map_, wincondition_, back_, ok_;
	UI::Button* pos_[MAX_PLAYERS];
	UI::Textarea title_, mapname_;
	UI::Textarea name_, type_, team_, tribe_, init_, wincondition_type_;
	GameSettingsProvider* settings_;
	GameController* ctrl_;  // optional
	PlayerDescriptionGroup* players_[MAX_PLAYERS];
	std::string filename_;
	std::string filename_proof_;  // local var. to check UI state
	std::string player_save_name_[MAX_PLAYERS];
	std::string player_save_tribe_[MAX_PLAYERS];
	int8_t nr_players_;
	bool is_scenario_;
	std::vector<std::string> win_condition_scripts_;
	uint8_t cur_wincondition_;
};

#endif  // end of include guard: WL_UI_FSMENU_LAUNCH_SPG_H
