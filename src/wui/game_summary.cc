/*
 * Copyright (C) 2007-2017 by the Widelands Development Team
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

#include "wui/game_summary.h"

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "base/time_string.h"
#include "graphic/graphic.h"
#include "graphic/playercolor.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/player.h"
#include "logic/playersmanager.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/table.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"
#include "wui/interactive_gamebase.h"
#include "wui/interactive_player.h"

#define PADDING 4

GameSummaryScreen::GameSummaryScreen(InteractiveGameBase* parent, UI::UniqueWindow::Registry* r)
   : UI::UniqueWindow(parent, "game_summary", r, 0, 0, _("Game over")),
     game_(parent->game()),
     desired_speed_(game_.game_controller()->desired_speed()) {
	game_.game_controller()->set_desired_speed(0);
	// Init boxes
	UI::Box* vbox = new UI::Box(this, 0, 0, UI::Box::Vertical, 0, 0, PADDING);
	title_area_ = new UI::Textarea(vbox, "", UI::Align::kCenter);
	vbox->add(title_area_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	vbox->add_space(PADDING);

	UI::Box* hbox1 = new UI::Box(this, 0, 0, UI::Box::Horizontal);
	players_table_ = new UI::Table<uintptr_t const>(hbox1, 0, 0, 0, 0);
	players_table_->fit_height(game_.player_manager()->get_players_end_status().size());
	hbox1->add_space(PADDING);
	hbox1->add(players_table_);
	hbox1->add_space(PADDING);

	UI::Box* info_box = new UI::Box(hbox1, 0, 0, UI::Box::Vertical, 0, 0);
	info_area_label_ = new UI::Textarea(info_box, _("Player Info:"));
	info_box->add(info_area_label_);
	info_area_ = new UI::MultilineTextarea(
	   info_box, 0, 0, 130,
	   std::max(130, players_table_->get_h() - info_area_label_->get_h() - PADDING), "");
	info_box->add(info_area_, UI::Box::Resizing::kFullSize);
	info_box->add_space(PADDING);
	hbox1->add(info_box);
	hbox1->add_space(PADDING);
	vbox->add(hbox1);

	UI::Box* bottom_box = new UI::Box(this, 0, 0, UI::Box::Horizontal);

	bottom_box->add_space(PADDING);

	gametime_label_ = new UI::Textarea(bottom_box, _("Elapsed time:"));
	bottom_box->add(gametime_label_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	bottom_box->add_space(PADDING);
	gametime_value_ = new UI::Textarea(bottom_box, gametimestring(game_.get_gametime()));
	bottom_box->add(gametime_value_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	bottom_box->add_inf_space();

	continue_button_ = new UI::Button(
	   bottom_box, "continue_button", 0, 0, 35, 35, g_gr->images().get("images/ui_basic/but4.png"),
	   g_gr->images().get("images/ui_basic/continue.png"), _("Continue playing"));
	bottom_box->add(continue_button_);
	bottom_box->add_space(PADDING);
	stop_button_ = new UI::Button(
	   bottom_box, "stop_button", 0, 0, 35, 35, g_gr->images().get("images/ui_basic/but4.png"),
	   g_gr->images().get("images/wui/menus/menu_exit_game.png"), _("Exit Game"));
	bottom_box->add(stop_button_);
	bottom_box->add_space(PADDING);

	vbox->add(bottom_box, UI::Box::Resizing::kFullSize);
	vbox->add_space(PADDING);
	set_center_panel(vbox);

	// Prepare table
	players_table_->add_column(150, _("Player"));
	players_table_->add_column(80, _("Team"), "", UI::Align::kCenter);
	players_table_->add_column(100, _("Status"), "", UI::Align::kCenter);
	players_table_->add_column(0, _("Time"), "", UI::Align::kRight, UI::TableColumnType::kFlexible);

	// Prepare Elements
	title_area_->set_fontsize(UI_FONT_SIZE_BIG);

	// Connections
	continue_button_->sigclicked.connect(boost::bind(&GameSummaryScreen::continue_clicked, this));
	stop_button_->sigclicked.connect(boost::bind(&GameSummaryScreen::stop_clicked, this));
	players_table_->selected.connect(boost::bind(&GameSummaryScreen::player_selected, this, _1));

	// Window
	center_to_parent();
	set_can_focus(true);
	focus();
	fill_data();

	set_thinks(false);
}

bool GameSummaryScreen::handle_mousepress(uint8_t btn, int32_t mx, int32_t my) {
	// Prevent closing with right click
	if (btn == SDL_BUTTON_RIGHT)
		return true;

	return UI::Window::handle_mousepress(btn, mx, my);
}

void GameSummaryScreen::fill_data() {
	std::vector<Widelands::PlayerEndStatus> players_status =
	   game_.player_manager()->get_players_end_status();
	bool local_in_game = false;
	bool local_won = false;
	Widelands::Player* single_won = nullptr;
	uint8_t teawon_ = 0;
	InteractivePlayer* ipl = game_.get_ipl();
	// This defines a row to be selected, current player,
	// if not then the first line
	uint32_t current_player_position = 0;

	for (uintptr_t i = 0; i < players_status.size(); i++) {
		Widelands::PlayerEndStatus pes = players_status.at(i);
		if (ipl && pes.player == ipl->player_number()) {
			local_in_game = true;
			local_won = pes.result == Widelands::PlayerEndResult::PLAYER_WON;
			current_player_position = i;
		}
		Widelands::Player* p = game_.get_player(pes.player);
		UI::Table<uintptr_t const>::EntryRecord& te = players_table_->add(i);
		// Player name & pic
		const Image* player_image =
		   playercolor_image(pes.player - 1, g_gr->images().get("images/players/genstats_player.png"),
		                     g_gr->images().get("images/players/genstats_player_pc.png"));
		assert(player_image);
		te.set_picture(0, player_image, p->get_name());
		// Team
		std::string teastr_ =
		   (boost::format("%|1$u|") % static_cast<unsigned int>(p->team_number())).str();
		te.set_string(1, teastr_);
		// Status
		std::string stat_str;
		switch (pes.result) {
		case Widelands::PlayerEndResult::PLAYER_LOST:
			/** TRANSLATORS: This is shown in the game summary for the players who have lost. */
			stat_str = _("Lost");
			break;
		case Widelands::PlayerEndResult::PLAYER_WON:
			/** TRANSLATORS: This is shown in the game summary for the players who have won. */
			stat_str = _("Won");
			if (!single_won) {
				single_won = p;
			} else {
				teawon_ = p->team_number();
			}
			break;
		case Widelands::PlayerEndResult::PLAYER_RESIGNED:
			/** TRANSLATORS: This is shown in the game summary for the players who have resigned. */
			stat_str = _("Resigned");
			break;
		case Widelands::PlayerEndResult::UNDEFINED:
			/** TRANSLATORS: This is shown in the game summary when we don't know */
			/** TRANSLATORS: if the player has lost or won. */
			stat_str = pgettext("player_won", "Unknown");
			break;
		}
		te.set_string(2, stat_str);
		// Time
		te.set_string(3, gametimestring(pes.time));
	}

	if (local_in_game) {
		if (local_won) {
			title_area_->set_text(_("You won!"));
		} else {
			title_area_->set_text(_("You lost."));
		}
	} else {
		if (teawon_ <= 0) {
			assert(single_won);
			title_area_->set_text((boost::format(_("%s won!")) % single_won->get_name()).str());
		} else {
			title_area_->set_text(
			   (boost::format(_("Team %|1$u| won!")) % static_cast<unsigned int>(teawon_)).str());
		}
	}
	if (!players_status.empty()) {
		players_table_->select(current_player_position);
	}
}

void GameSummaryScreen::continue_clicked() {
	game_.game_controller()->set_desired_speed(desired_speed_);
	die();
}

void GameSummaryScreen::stop_clicked() {
	game_.get_ibase()->end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
}

void GameSummaryScreen::player_selected(uint32_t entry_index) {
	const uintptr_t selected_player_index = (*players_table_)[entry_index];
	const Widelands::PlayerEndStatus& player_status =
	   game_.player_manager()->get_players_end_status()[selected_player_index];

	std::string info_str = parse_player_info(player_status.info);
	info_area_->set_text(info_str);
	layout();
}

std::string GameSummaryScreen::parse_player_info(std::string info) {
	using StringSplitIterator = boost::split_iterator<std::string::iterator>;
	if (info.empty()) {
		return info;
	}
	// Iterate through all key=value pairs
	StringSplitIterator substring_it =
	   boost::make_split_iterator(info, boost::first_finder(";", boost::is_equal()));
	std::string info_str;
	while (substring_it != StringSplitIterator()) {
		std::string substring = boost::copy_range<std::string>(*substring_it);
		std::vector<std::string> pair;
		boost::split(pair, substring, boost::is_any_of("="));
		assert(pair.size() == 2);

		std::string key = pair.at(0);
		if (key == "score") {
			info_str += (boost::format("%1% : %2%\n") % _("Score") % pair.at(1)).str();
		} else if (key == "team_score") {
			info_str += (boost::format("%1% : %2%\n") % _("Team Score") % pair.at(1)).str();
		} else if (key == "resign_reason") {
			info_str += (boost::format("%1%\n") % pair.at(1)).str();
		}
		++substring_it;
	}
	return info_str;
}
