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

#ifndef WL_UI_FSMENU_LAUNCH_MPG_H
#define WL_UI_FSMENU_LAUNCH_MPG_H

#include <memory>
#include <string>

#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "ui_fsmenu/base.h"
#include "ui_fsmenu/helpwindow.h"
#include "wui/suggested_teams_box.h"

struct ChatProvider;
struct GameChatPanel;
class GameController;
struct GameSettingsProvider;
struct MultiPlayerSetupGroup;
class LuaInterface;

/**
 * Fullscreen menu for setting map and mapsettings for single and multi player
 * games.
 *
 */
class FullscreenMenuLaunchMPG : public FullscreenMenuBase {
public:
	FullscreenMenuLaunchMPG(GameSettingsProvider*, GameController*);
	~FullscreenMenuLaunchMPG();

	void set_chat_provider(ChatProvider&);

	void think() override;

	void refresh();

protected:
	void clicked_ok() override;
	void clicked_back() override;

private:
	void fit_to_screen() override;

	LuaInterface* lua_;

	void change_map_or_save();
	void select_map();
	void select_saved_game();
	void win_condition_clicked();
	void win_condition_update();
	void win_condition_load();
	void set_scenario_values();
	void load_previous_playerdata();
	void load_map_info();
	void help_clicked();

	uint32_t butw_;
	uint32_t buth_;
	uint32_t fs_;

	// TODO(GunChleoc): We still need to use these consistently. Just getting them in for now
	// so we can have the SuggestedTeamsBox
	int32_t const padding_;  // Common padding between panels
	int32_t const indent_;   // Indent for elements below labels
	int32_t const label_height_;
	int32_t const right_column_x_;

	UI::Button change_map_or_save_, ok_, back_, wincondition_;
	UI::Button help_button_;
	UI::Textarea title_, mapname_, clients_, players_, map_, wincondition_type_;
	UI::MultilineTextarea map_info_, client_info_;
	std::unique_ptr<UI::FullscreenHelpWindow> help_;
	GameSettingsProvider* settings_;
	GameController* ctrl_;
	GameChatPanel* chat_;
	MultiPlayerSetupGroup* mpsg_;
	std::string filename_proof_;  // local variable to check state
	int16_t nr_players_;

	UI::SuggestedTeamsBox* suggested_teams_box_;
};

#endif  // end of include guard: WL_UI_FSMENU_LAUNCH_MPG_H
