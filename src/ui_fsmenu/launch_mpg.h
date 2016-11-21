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
#include "ui_basic/dropdown.h"
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
	LuaInterface* lua_;

	void change_map_or_save();
	void select_map();
	void select_saved_game();
	/// Loads all win conditions that can be played with the map into the selection dropdown.
	/// Disables the dropdown if the map is a scenario.
	void update_win_conditions();
	/// Reads the win conditions that are available for the given map and adds the entries to the
	/// dropdown.
	void load_win_conditions(const Widelands::Map& map);
	/// Remembers the win condition that is currently selected in the dropdown.
	void win_condition_selected();
	/// If the win condition in 'win_condition_script' can be played with the map tags,
	/// parses the win condition and returns it as a std::unique_ptr<LuaTable>.
	/// If this win condition can't be played with the map tags, returns a unique_ptr to nullptr.
	std::unique_ptr<LuaTable> win_condition_if_valid(const std::string& win_condition_script,
	                                                 std::set<std::string> tags) const;

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

	UI::Button change_map_or_save_;
	UI::Dropdown<std::string> win_condition_dropdown_;
	std::string last_win_condition_;
	UI::Button ok_, back_;
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
