/*
 * Copyright (C) 2002, 2006-2010 by the Widelands Development Team
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

#include <string>

#include "ui_fsmenu/base.h"
#include "ui_basic/button.h"
#include "ui_basic/helpwindow.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "ui_fsmenu/suggested_teams_box.h"

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
	FullscreenMenuLaunchMPG(GameSettingsProvider *, GameController *);
	~FullscreenMenuLaunchMPG();

	void set_chat_provider(ChatProvider &);

	void think() override;

	void refresh();

protected:
	void clicked_ok() override;
	void clicked_back() override;

private:
	LuaInterface * m_lua;

	void change_map_or_save();
	void select_map();
	void select_saved_game();
	void win_condition_clicked();
	void win_condition_update();
	void set_scenario_values();
	void load_previous_playerdata();
	void load_map_info();
	void help_clicked();

	uint32_t    m_butw;
	uint32_t    m_buth;
	uint32_t    m_fs;
	std::string m_fn;
	// TODO(GunChleoc): We still need to use these consistently. Just getting them in for now
	// so we can have the SuggestedTeamsBox
	int32_t const m_padding;               // Common padding between panels
	int32_t const m_indent;                // Indent for elements below labels
	int32_t const m_label_height;
	int32_t const m_right_column_x;

	UI::Button       m_change_map_or_save, m_ok, m_back, m_wincondition;
	UI::Button       m_help_button;
	UI::Textarea              m_title, m_mapname, m_clients, m_players, m_map, m_wincondition_type;
	UI::MultilineTextarea    m_map_info, m_client_info;
	UI::HelpWindow          * m_help;
	GameSettingsProvider    * m_settings;
	GameController          * m_ctrl;
	GameChatPanel           * m_chat;
	MultiPlayerSetupGroup   * m_mpsg;
	std::string               m_filename_proof; // local variable to check state
	int16_t                   m_nr_players;

	UI::SuggestedTeamsBox*    m_suggested_teams_box;
};


#endif  // end of include guard: WL_UI_FSMENU_LAUNCH_MPG_H
