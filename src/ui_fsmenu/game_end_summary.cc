/*
 * Copyright (C) 2002-2012 by the Widelands Development Team
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

#include "game_end_summary.h"

#include "logic/game.h"
#include "logic/player.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "timestring.h"


namespace Columns {enum {Name, Status, Team, Time, Score};}

Fullscreen_Menu_GameSummary::Fullscreen_Menu_GameSummary(Widelands::Game* game)
	:Fullscreen_Menu_Base("choosemapmenu.jpg"),
	m_game(game),
	// Values for alignment and size
	m_butwidth(get_w() / 4),
	m_butheight(get_h() * 9 / 200),
	m_fs   (fs_small()),
	m_fn   (ui_fn()),
	// Elements
	m_title
		(this,
		 get_w() / 2, get_h() * 9 / 50,
		 "", UI::Align_HCenter),
	m_players_table
		(this,
		get_w() *  47 / 2500, get_h() * 3417 / 10000,
		get_w() * 711 / 1250, get_h() * 6083 / 10000),
	m_gametime_label
		(this , get_w() * 7 / 10,  get_h() * 17 / 50,
		_("Game time :")),
	m_gametime_value(this , get_w() * 71 / 100, get_h() * 17 / 50),
	m_back_button
		(this, "back",
		 get_w() * 71 / 100, get_h() * 9 / 10, m_butwidth, m_butheight,
		 g_gr->images().get("pics/but0.png"),
		 _("Back"), std::string(), true, false)
{
	m_back_button.sigclicked.connect
		(boost::bind(&Fullscreen_Menu_GameSummary::end_modal, boost::ref(*this), 0));
	m_back_button.set_font(font_small());
	m_title.set_font(m_fn, fs_big(), UI_FONT_CLR_FG);
	m_gametime_label.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_gametime_value.set_font(m_fn, m_fs, UI_FONT_CLR_FG);

	uint32_t table_width = m_players_table.get_w();
	m_players_table.add_column(table_width * 6 / 20, _("Player name"));
	m_players_table.add_column(table_width * 4 / 20, _("Status"), UI::Align_HCenter);
	m_players_table.add_column(table_width * 3 / 20, _("Team"), UI::Align_HCenter);
	m_players_table.add_column(table_width * 4 / 20, _("Time"));
	m_players_table.add_column(table_width * 3 / 20, _("Score"), UI::Align_Right);
	m_players_table.set_column_compare
		(Columns::Name, boost::bind
		 	(&Fullscreen_Menu_GameSummary::compare_player_status, this, _1, _2));

	m_gametime_label.set_text(m_game->get_players_end_status().front().extra);
}

bool Fullscreen_Menu_GameSummary::compare_player_status(uint32_t rowa, uint32_t rowb)
{
	return rowa < rowb;
}

