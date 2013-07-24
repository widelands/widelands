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

#include "game_summary.h"

#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include "graphic/graphic.h"
#include "interactive_gamebase.h"
#include "interactive_player.h"
#include "logic/game.h"
#include "logic/player.h"
#include "timestring.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/table.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"
#include "wlapplication.h"

#define PADDING 4

GameSummaryScreen::GameSummaryScreen
	(Interactive_GameBase * parent, UI::UniqueWindow::Registry * r)
: UI::UniqueWindow(parent, "game_summary", r, 500, 400, _("Game over")),
m_game(parent->game())
{
	// Init boxes
	UI::Box * vbox = new UI::Box(this, 0, 0, UI::Box::Vertical, 0, 0, PADDING);
	m_title_area = new UI::Textarea(vbox, "", UI::Align_HCenter);
	vbox->add(m_title_area, UI::Box::AlignCenter);
	vbox->add_space(PADDING);

	UI::Box * hbox1 = new UI::Box(this, 0, 0, UI::Box::Horizontal);
	m_players_table = new UI::Table<const uintptr_t>(hbox1, 0, 0, 300, 200);
	hbox1->add_space(PADDING);
	hbox1->add(m_players_table, UI::Box::AlignTop);
	hbox1->add_space(PADDING);

	UI::Box * infoBox = new UI::Box(hbox1, 0, 0, UI::Box::Vertical, 0, 0);
	// NOCOM(#cghislai): Gametime sounds weird. Just Time? or Elapsed time?
	m_gametime_label = new UI::Textarea(infoBox, _("Gametime :"));
	infoBox->add(m_gametime_label, UI::Box::AlignLeft);
	m_gametime_value = new UI::Textarea(infoBox);
	infoBox->add(m_gametime_value, UI::Box::AlignRight);
	infoBox->add_space(PADDING);
	hbox1->add(infoBox, UI::Box::AlignTop);
	hbox1->add_space(PADDING);
	vbox->add(hbox1, UI::Box::AlignLeft);
	vbox->add_space(PADDING);

	UI::Box * buttonBox = new UI::Box(this, 0, 0, UI::Box::Horizontal);
	m_continue_button = new UI::Button
		(buttonBox, "continue_button",
		0, 0, 100, 32, g_gr->images().get("pics/but0.png"),
		_("Continue"), _("Continue playing"));
	buttonBox->add(m_continue_button, UI::Box::AlignRight);
	buttonBox->add_space(PADDING);
	m_stop_button = new UI::Button
		(buttonBox, "stop_button",
		0, 0, 100, 32, g_gr->images().get("pics/but0.png"),
		_("Quit"), _("Return to main menu"));
	buttonBox->add(m_stop_button, UI::Box::AlignRight);
	vbox->add(buttonBox, UI::Box::AlignBottom);
	vbox->add_space(PADDING);
	set_center_panel(vbox);

	// Prepare table
	m_players_table->add_column(150, _("Player"));
	m_players_table->add_column(50, _("Team"), UI::Align_HCenter);
	m_players_table->add_column(100, _("Status"), UI::Align_HCenter);
	m_players_table->add_column(100, _("Time"));
	m_players_table->add_column(100, _("Score"), UI::Align_Right);

	// Prepare Elements
	m_title_area->set_textstyle(UI::TextStyle::ui_big());

	// Connections
	m_continue_button->sigclicked.connect
		(boost::bind(&GameSummaryScreen::continue_clicked, this));
	m_stop_button->sigclicked.connect
		(boost::bind(&GameSummaryScreen::stop_clicked, this));
	m_players_table->selected.connect
		(boost::bind(&GameSummaryScreen::player_selection, this, _1));

	// Window
	center_to_parent();
	set_can_focus(true);
	focus();
	fill_data();
}

bool GameSummaryScreen::handle_mousepress(Uint8 btn, int32_t mx, int32_t my)
{
	// Prevent closing with right click
	if (btn == SDL_BUTTON_RIGHT)
		return true;

	return UI::Window::handle_mousepress(btn, mx, my);
}

void GameSummaryScreen::fill_data()
{
	std::vector<Widelands::PlayerEndStatus> players_status
		= m_game.get_players_end_status();
	bool local_in_game = false;
	bool local_won = false;
	Widelands::Player* single_won = NULL;
	uint8_t team_won = 0;
	Interactive_Player* ipl = m_game.get_ipl();

	BOOST_FOREACH(Widelands::PlayerEndStatus pes, players_status) {
		if (ipl && pes.player == ipl->player_number()) {
			local_in_game = true;
			local_won = pes.win;
		}
		Widelands::Player* p = m_game.get_player(pes.player);
		UI::Table<uintptr_t>::Entry_Record & te
			= m_players_table->add(pes.player);
		// Player name & pic
		char buf[256];
		// NOCOM(#cghislai): prefer boost format over snprintf. more typesafe and more performant. It also supports positional arguments which makes localization easier.
		sprintf(buf, "pics/genstats_enable_plr_0%u.png", pes.player);
		const Image* pic = g_gr->images().get(buf);
		te.set_picture(0, pic, p->get_name());
		// Team
		sprintf(buf, "%i", p->team_number());
		te.set_string(1, buf);
		// Status
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
		// Time
		te.set_string(3, gametimestring(pes.time));
		// Points
		sprintf(buf, "%i", pes.points);
		te.set_string(4, buf);
	}

	if (local_in_game) {
		if (local_won) {
			m_title_area->set_text(_("You won!"));
		} else {
			// NOCOM(#cghislai): Why two points?
			m_title_area->set_text(_("You lost.."));
		}
	} else {
		if (team_won <= 0) {
			// NOCOM(#cghislai): why are the strings not tagged for translation?
			m_title_area->set_text
				((boost::format("%s won!") % single_won->get_name()).str());
		} else {
			m_title_area->set_text
				((boost::format("Team %i won!") % team_won).str());
		}
	}
	m_players_table->update();
	m_gametime_value->set_text(gametimestring(m_game.get_gametime()));
}

// NOCOM(#cghislai): is this even needed then?
void GameSummaryScreen::player_selection(uint8_t idx)
{

}

void GameSummaryScreen::continue_clicked()
{
	die();
}

void GameSummaryScreen::stop_clicked()
{
	m_game.get_ibase()->end_modal(0);
}

