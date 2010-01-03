/*
 * Copyright (C) 2002, 2006-2009 by the Widelands Development Team
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

#ifndef FULLSCREEN_MENU_LAUNCHGAME_H
#define FULLSCREEN_MENU_LAUNCHGAME_H

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
struct PlayerDescriptionGroup;

/**
 * Fullscreen menu for setting map and mapsettings for single and multi player
 * games.
 *
 * The menu has a lot dynamic user-interfaces, that are only shown in specific
 * cases:
 *    UI::Button m_select_save - only shown in multiplayer for the host player.
 *    UI::Button m_select_map  - only shown if the player has the right to
 *                               change the map.
 *    GameChatPanel            - only shown in multiplayer maps
 *
 * The return values of run() are:
 *    0  - back was pressed
 *    1  - normal game (either single or multi player)
 *    2  - scenario game (at the moment only single player)
 *    3  - multi player savegame
 *    4  - multi player scenario savegame <- not yet implemented
 */
struct Fullscreen_Menu_LaunchGame : public Fullscreen_Menu_Base {
	Fullscreen_Menu_LaunchGame
		(GameSettingsProvider *, GameController * = 0, bool autolaunch = false);

	void setChatProvider(ChatProvider &);

	void start();
	void think();

	void refresh();

private:
	void select_map();
	void select_savegame();
	void back_clicked();
	void start_clicked();
	void set_scenario_values();
	void switch_to_position(uint8_t);
	void load_previous_playerdata();
	void enable_all_pdgs();
	void safe_place_for_host(uint8_t);

	uint32_t    m_butw;
	uint32_t    m_buth;
	uint32_t    m_fs;
	std::string m_fn;

	UI::Callback_Button<Fullscreen_Menu_LaunchGame> m_select_map, m_select_save;
	UI::Callback_Button<Fullscreen_Menu_LaunchGame> m_back, m_ok;
	UI::Callback_IDButton<Fullscreen_Menu_LaunchGame, uint8_t> *
		m_pos[MAX_PLAYERS];
	UI::Textarea              m_title, m_mapname, m_lobby;
	UI::Textarea              m_name, m_type, m_tribe, m_init, m_ready;
	UI::Multiline_Textarea    m_notes;
	UI::Listselect<int32_t> * m_lobby_list;
	GameSettingsProvider    * m_settings;
	GameController          * m_ctrl; // optional
	GameChatPanel           * m_chat;
	PlayerDescriptionGroup  * m_players[MAX_PLAYERS];
	std::string               m_filename;
	std::string            m_filename_proof; // locale variable to check UI state
	std::string               m_player_save_name[MAX_PLAYERS];
	std::string               m_player_save_tribe[MAX_PLAYERS];
	int8_t                    m_nr_players;
	bool                      m_is_scenario;
	bool                      m_is_savegame;
	bool                      m_autolaunch;
	bool                      m_multiplayer;
};


#endif
