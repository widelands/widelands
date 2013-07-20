/*
 * Copyright (C) 2002-2004, 2006-2008, 2010 by the Widelands Development Team
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

#ifndef FULLSCREEN_MENU_GAME_SUMMARY_H
#define FULLSCREEN_MENU_GAME_SUMMARY_H

#include "base.h"

#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "ui_basic/table.h"

namespace Widelands
{
	class Game;
}
/**
 * Game summary after a game endif
 */
struct Fullscreen_Menu_GameSummary : public Fullscreen_Menu_Base {
	Fullscreen_Menu_GameSummary(Widelands::Game* game);

private:
	void fill_data();
	void player_selection(uint8_t idx);
	void continue_clicked();
	void stop_clicked();

	Widelands::Game* m_game;
	uint32_t    m_fs;
	std::string m_fn;
	uint8_t m_butwidth;
	uint8_t m_butheight;
	UI::Textarea *  m_title_area;
	UI::Textarea *  m_gametime_label;
	UI::Textarea *  m_gametime_value;
	UI::Button *    m_stop_button;
	UI::Button *    m_continue_button;
	UI::Table<uintptr_t const> *     m_players_table;
};

#endif

