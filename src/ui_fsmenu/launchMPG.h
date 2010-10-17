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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef FULLSCREEN_MENU_LAUNCHMPG_H
#define FULLSCREEN_MENU_LAUNCHMPG_H

#include "base.h"

#include "ui_basic/button.h"
#include "ui_basic/textarea.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/listselect.h"

#include <string>

struct ChatProvider;
struct GameChatPanel;
struct GameController;
struct GameSettingsProvider;
struct MultiPlayerSetupGroup;
struct LuaInterface;

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
	Fullscreen_Menu_LaunchMPG
		(GameSettingsProvider *, GameController *, bool autolaunch = false);
	~Fullscreen_Menu_LaunchMPG();

	void setChatProvider(ChatProvider &);

	void think();

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

	uint32_t    m_butw;
	uint32_t    m_buth;
	uint32_t    m_fs;
	std::string m_fn;

	UI::Callback_Button<Fullscreen_Menu_LaunchMPG> m_change_map_or_save, m_ok;
	UI::Callback_Button<Fullscreen_Menu_LaunchMPG> m_back, m_wincondition;
	UI::Callback_IDButton<Fullscreen_Menu_LaunchMPG, uint8_t> *
		m_pos[MAX_PLAYERS];
	UI::Textarea              m_title, m_mapname;
	UI::Multiline_Textarea    m_map_info, m_client_info;
	GameSettingsProvider    * m_settings;
	GameController          * m_ctrl; // optional
	GameChatPanel           * m_chat;
	MultiPlayerSetupGroup   * m_mpsg;
	std::string               m_filename_proof; // local variable to check state
	int8_t                    m_nr_players;
	bool                      m_autolaunch;
	std::vector<std::string>  m_win_conditions;
	uint8_t                   m_cur_wincondition;
};


#endif
