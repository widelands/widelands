/*
 * Copyright (C) 2007-2008 by the Widelands Development Team
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

#ifndef GAME_SUMMARY_H
#define GAME_SUMMARY_H

#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/table.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"

class Interactive_GameBase;
namespace Widelands
{
class Game;
}

/// Shows an ingame summary window on game end
class GameSummaryScreen : public UI::UniqueWindow {
public:
	GameSummaryScreen
		(Interactive_GameBase * parent, UI::UniqueWindow::Registry * r);

	bool handle_mousepress(Uint8 btn, int32_t mx, int32_t my) override;
private:
	void fill_data();
	void stop_clicked();
	void continue_clicked();
	void player_selected(uint32_t idx);
	std::string parse_player_info(std::string & info);

	Widelands::Game & m_game;
	UI::Textarea *  m_title_area;
	UI::Textarea *  m_gametime_label;
	UI::Textarea *  m_gametime_value;
	UI::Multiline_Textarea *  m_info_area;
	UI::Button *    m_continue_button;
	UI::Button *    m_stop_button;
	UI::Table<uintptr_t const> *     m_players_table;
};

#endif
