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

#ifndef WL_UI_FSMENU_LAUNCH_MPG_H
#define WL_UI_FSMENU_LAUNCH_MPG_H

#include <memory>

#include "logic/game_settings.h"
#include "ui_basic/button.h"
#include "ui_fsmenu/helpwindow.h"
#include "ui_fsmenu/launch_game.h"
#include "ui_fsmenu/multiplayersetupgroup.h"
#include "wui/game_chat_panel.h"

struct ChatProvider;

/**
 * Fullscreen menu for setting map and mapsettings for single and multi player
 * games.
 *
 */
class FullscreenMenuLaunchMPG : public FullscreenMenuLaunchGame {
public:
	FullscreenMenuLaunchMPG(FullscreenMenuMain&,
	                        GameSettingsProvider*,
	                        GameController*,
	                        ChatProvider&,
	                        Widelands::EditorGameBase& egbase);
	~FullscreenMenuLaunchMPG() override;

	void think() override;
	void refresh();

protected:
	void clicked_ok() override;
	void clicked_back() override;

private:
	void layout() override;
	bool clicked_select_map() override;
	void select_map();
	void select_saved_game();
	void win_condition_selected() override;

	void set_scenario_values();
	void load_previous_playerdata();
	void load_map_info();
	void help_clicked();
	void map_changed();

	UI::Button help_button_;

	std::unique_ptr<UI::FullscreenHelpWindow> help_;
	MultiPlayerSetupGroup mpsg_;
	GameChatPanel chat_;
	Widelands::EditorGameBase& egbase_;  // Not owned

	std::unique_ptr<Notifications::Subscriber<NoteGameSettings>> subscriber_;
};

#endif  // end of include guard: WL_UI_FSMENU_LAUNCH_MPG_H
