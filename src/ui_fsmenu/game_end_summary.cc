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

#include "ui_basic/unique_window.h"
#include "ui_basic/textarea.h"
#include "ui_basic/table.h"
#include "ui_basic/button.h"

#include "graphic/graphic.h"
#include "logic/game.h"
#include "logic/player.h"
#include "wui/interactive_gamebase.h"
#include "wui/interactive_player.h"
#include "wlapplication.h"
#include "i18n.h"
#include "timestring.h"

#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/signal.hpp>


namespace Columns {enum {Name, Status, Team, Time, Score};}

Fullscreen_Menu_GameSummary::Fullscreen_Menu_GameSummary(Widelands::Game* game)
	:Fullscreen_Menu_Base("choosemapmenu.jpg"),
	// Values for alignment and size
	m_fn   (ui_fn()),
	m_fs   (fs_small()),
	m_butwidth(get_w() * 25 / 100),
	m_butheight(get_h() * 9 / 200)
{
	m_game = game;
	// Elements
	m_title_area = new UI::Textarea
		(this,
		 get_w() / 2, get_h() * 9 / 50,
		 "", UI::Align_HCenter);
	m_gametime_label = new UI::Textarea
		(this, get_w() * 71 / 100,  get_h() * 17 / 50,
		_("Game time :"));
	m_gametime_value = new UI::Textarea(this, get_w() * 81 / 100, get_h() * 17 / 50);
	m_stop_button = new UI::Button
		(this, "back",
		 get_w() * 71 / 100, get_h() * 85 / 100, m_butwidth, m_butheight,
		 g_gr->images().get("pics/but0.png"),
		 _("Back"), "", true, false);
	m_continue_button = new UI::Button
		(this, "back",
		 get_w() * 71 / 100, get_h() * 91 / 100, m_butwidth, m_butheight,
		 g_gr->images().get("pics/but0.png"),
		 _("Continue"), "", true, false);
	m_players_table = new UI::Table<const uintptr_t>
		(this,
		get_w() *  47 / 2500, get_h() * 3417 / 10000,
		get_w() * 711 / 1250, get_h() * 6083 / 10000);
	// Prepare table
	uint32_t table_width = m_players_table->get_w();
	m_players_table->add_column(table_width * 6 / 20, _("Player name"));
	m_players_table->add_column(table_width * 4 / 20, _("Status"), UI::Align_HCenter);
	m_players_table->add_column(table_width * 3 / 20, _("Team"), UI::Align_HCenter);
	m_players_table->add_column(table_width * 4 / 20, _("Time"));
	m_players_table->add_column(table_width * 3 / 20, _("Score"), UI::Align_Right);

	// Prepare Elements
	m_title_area->set_textstyle(UI::TextStyle::ui_big());

	// Connections
	m_stop_button->sigclicked.connect
		(boost::bind(&Fullscreen_Menu_GameSummary::stop_clicked, this));
	m_continue_button->sigclicked.connect
		(boost::bind(&Fullscreen_Menu_GameSummary::continue_clicked, this));
	m_players_table->selected.connect
		(boost::bind(&Fullscreen_Menu_GameSummary::player_selection, this, _1));

	m_continue_button->update();
	fill_data();
}

void Fullscreen_Menu_GameSummary::fill_data()
{
	std::vector<Widelands::PlayerEndStatus> players_status
		= m_game->get_players_end_status();
	bool local_in_game = false;
	bool local_won = false;
	Widelands::Player* single_won = NULL;
	Widelands::TeamNumber team_won = 0;
	Interactive_Player* ipl = m_game->get_ipl();

	BOOST_FOREACH(Widelands::PlayerEndStatus pes, players_status) {
		if (pes.player == ipl->player_number()) {
			local_in_game = true;
			local_won = pes.win;
		}
		Widelands::Player* p = m_game->get_player(pes.player);
		UI::Table<uintptr_t>::Entry_Record & te
			= m_players_table->add(pes.player);
		te.set_picture(0, NULL, p->get_name());
		char buf[64];
		sprintf(buf, "%i", p->team_number());
		te.set_string(1, buf);
		std::string stat_str = _("Resigned");
		if (pes.lost) {
			stat_str = _("Lost");
		} else if (pes.win) {
			stat_str = _("Won");
			if (!single_won) {
				single_won = p;
			} else {
				team_won = p->team_number();
			}
		}
		te.set_string(2, stat_str);
		te.set_string(3, gametimestring(pes.time));
		sprintf(buf, "%i", pes.points);
		te.set_string(4, buf);
	}

	if (local_in_game) {
		if (local_won) {
			m_title_area->set_text(_("You won!"));
		} else {
			m_title_area->set_text(_("You lost.."));
		}
	} else {
		if (team_won <= 0) {
			m_title_area->set_text
				((boost::format("%s won!") % single_won->get_name()).str());
		} else {
			m_title_area->set_text
				((boost::format("Team %i won!") % team_won).str());
		}
	}
	m_players_table->update();
	m_gametime_value->set_text(gametimestring(m_game->get_gametime()));
}

void Fullscreen_Menu_GameSummary::player_selection(uint8_t idx)
{

}

void Fullscreen_Menu_GameSummary::continue_clicked()
{
	end_modal(1);
}

void Fullscreen_Menu_GameSummary::stop_clicked()
{
	end_modal(0);
}

