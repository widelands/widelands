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

#ifndef FULLSCREEN_MENU_LAUNCHMPG_H
#define FULLSCREEN_MENU_LAUNCHMPG_H

#include <string>

#include "ui_fsmenu/base.h"
#include "ui_basic/button.h"
#include "ui_basic/helpwindow.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"


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
 * The return values of run() are:
 *    0  - back was pressed
 *    1  - normal game
 *    2  - scenario game
 *    3  - multi player savegame
 *    4  - multi player scenario savegame <- not yet implemented
 */
struct Fullscreen_Menu_LaunchMPG : public Fullscreen_Menu_Base {
	Fullscreen_Menu_LaunchMPG(GameSettingsProvider *, GameController *);
	~Fullscreen_Menu_LaunchMPG();

	void setChatProvider(ChatProvider &);

	void think() override;

	void refresh();

private:
	LuaInterface * m_lua;

	void change_map_or_save();
	void select_map();
	void select_saved_game();
	void back_clicked();
	void start_clicked();
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

	UI::Button       m_change_map_or_save, m_ok, m_back, m_wincondition;
	UI::Button       m_help_button;
	UI::Textarea              m_title, m_mapname, m_clients, m_players, m_map, m_wincondition_type;
	UI::Multiline_Textarea    m_map_info, m_client_info;
	UI::HelpWindow          * m_help;
	GameSettingsProvider    * m_settings;
	GameController          * m_ctrl;
	GameChatPanel           * m_chat;
	MultiPlayerSetupGroup   * m_mpsg;
	std::string               m_filename_proof; // local variable to check state
	int16_t                   m_nr_players;
};


#endif
