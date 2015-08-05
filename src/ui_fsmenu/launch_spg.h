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

#ifndef WL_UI_FSMENU_LAUNCH_SPG_H
#define WL_UI_FSMENU_LAUNCH_SPG_H

#include <string>

#include "logic/constants.h"
#include "ui_basic/button.h"
#include "ui_basic/listselect.h"
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
 *    UI::Button m_select_map  - only shown if the player has the right to
 *                               change the map.
 *
 */
class FullscreenMenuLaunchSPG : public FullscreenMenuBase {
public:
	FullscreenMenuLaunchSPG
		(GameSettingsProvider *, GameController * = nullptr, bool autolaunch = false);
	~FullscreenMenuLaunchSPG();

	void start() override;
	void think() override;

	void refresh();

protected:
	void clicked_ok() override;
	void clicked_back() override;

private:
	LuaInterface * m_lua;

	void select_map();
	void win_condition_clicked();
	void win_condition_update();
	void set_scenario_values();
	void switch_to_position(uint8_t);
	void safe_place_for_host(uint8_t);

	uint32_t    m_butw;
	uint32_t    m_buth;

	UI::Button       m_select_map, m_wincondition, m_back, m_ok;
	UI::Button     * m_pos[MAX_PLAYERS];
	UI::Textarea              m_title, m_mapname;
	UI::Textarea              m_name, m_type, m_team, m_tribe, m_init, m_wincondition_type;
	GameSettingsProvider    * m_settings;
	GameController          * m_ctrl; // optional
	PlayerDescriptionGroup  * m_players[MAX_PLAYERS];
	std::string               m_filename;
	std::string               m_filename_proof; // local var. to check UI state
	std::string               m_player_save_name[MAX_PLAYERS];
	std::string               m_player_save_tribe[MAX_PLAYERS];
	int8_t                    m_nr_players;
	bool                      m_is_scenario;
	std::vector<std::string>  m_win_condition_scripts;
	uint8_t                   m_cur_wincondition;
};


#endif  // end of include guard: WL_UI_FSMENU_LAUNCH_SPG_H
