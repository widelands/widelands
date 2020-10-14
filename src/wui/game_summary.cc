/*
 * Copyright (C) 2007-2020 by the Widelands Development Team
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

#include <SDL_mouse.h>
#include <boost/algorithm/string.hpp>

#include "base/time_string.h"
#include "graphic/playercolor.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/player.h"
#include "logic/player_end_result.h"
#include "logic/playersmanager.h"
#include "wui/interactive_gamebase.h"
#include "wui/interactive_player.h"

constexpr uint8_t kPadding = 4;

GameSummaryScreen::GameSummaryScreen(InteractiveGameBase* parent, UI::UniqueWindow::Registry* r)
   : UI::UniqueWindow(parent, "game_summary", r, 0, 0, _("Game over")),
     game_(parent->game()),
     desired_speed_(game_.game_controller()->desired_speed()) {
	game_.game_controller()->set_desired_speed(0);
	// Init boxes
	UI::Box* vbox = new UI::Box(this, 0, 0, UI::Box::Vertical, 0, 0, kPadding);
	title_area_ = new UI::Textarea(
	   vbox, "", UI::Align::kCenter, g_style_manager->font_style(UI::FontStyle::kFsMenuTitle));

	vbox->add(title_area_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	vbox->add_space(kPadding);

	UI::Box* hbox1 = new UI::Box(vbox, 0, 0, UI::Box::Horizontal);
	players_table_ = new UI::Table<uintptr_t const>(hbox1, 0, 0, 0, 0, UI::PanelStyle::kWui);
	players_table_->fit_height(game_.player_manager()->get_players_end_status().size());

	info_box_ = new UI::Box(hbox1, 0, 0, UI::Box::Vertical, 0, 0);
	info_area_label_ = new UI::Textarea(info_box_, _("Player Info:"));
	info_area_ = new UI::MultilineTextarea(
	   info_box_, 0, 0, 130,
	   std::max(130, players_table_->get_h() - info_area_label_->get_h() - kPadding),
	   UI::PanelStyle::kWui, "");

	widelands_icon_ = new UI::Icon(hbox1, 0, 0, info_area_->get_w(), info_area_->get_h(),
	                               g_image_cache->get("images/logos/wl-ico-128.png"));

	info_box_->add(info_area_label_);
	info_box_->add(info_area_, UI::Box::Resizing::kFullSize);

	hbox1->add_space(kPadding);

	// At all times, only one of those is visible
	info_box_->set_visible(false);
	widelands_icon_->set_visible(false);
	hbox1->add(info_box_);
	hbox1->add(widelands_icon_);

	hbox1->add_space(3 * kPadding);
	hbox1->add(players_table_);
	hbox1->add_space(kPadding);

	vbox->add(hbox1);

	UI::Box* bottom_box = new UI::Box(vbox, 0, 0, UI::Box::Horizontal);

	bottom_box->add_space(kPadding);

	gametime_label_ = new UI::Textarea(bottom_box, _("Elapsed time:"));
	bottom_box->add(gametime_label_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	bottom_box->add_space(kPadding);
	gametime_value_ = new UI::Textarea(bottom_box, gametimestring(game_.get_gametime()));
	bottom_box->add(gametime_value_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	bottom_box->add_inf_space();

	continue_button_ =
	   new UI::Button(bottom_box, "continue_button", 0, 0, 35, 35, UI::ButtonStyle::kWuiMenu,
	                  g_image_cache->get("images/ui_basic/continue.png"), _("Continue playing"));
	bottom_box->add(continue_button_);
	bottom_box->add_space(kPadding);
	stop_button_ = new UI::Button(bottom_box, "stop_button", 0, 0, 35, 35, UI::ButtonStyle::kWuiMenu,
	                              g_image_cache->get("images/wui/menus/exit.png"), _("Exit Game"));
	bottom_box->add(stop_button_);
	bottom_box->add_space(kPadding);

	vbox->add(bottom_box, UI::Box::Resizing::kFullSize);
	vbox->add_space(kPadding);
	set_center_panel(vbox);

	// Prepare table
	players_table_->add_column(150, _("Player"));
	players_table_->add_column(80, _("Team"), "", UI::Align::kCenter);
	players_table_->add_column(100, _("Status"), "", UI::Align::kCenter);
	players_table_->add_column(100, _("Time"), "", UI::Align::kCenter);

	// Connections
	continue_button_->sigclicked.connect([this]() { continue_clicked(); });
	stop_button_->sigclicked.connect([this]() { stop_clicked(); });
	players_table_->selected.connect([this](uint32_t i) { player_selected(i); });

	// Window
	center_to_parent();
	players_table_->focus();
	fill_data();

	players_table_->set_column_compare(
	   2, [this](uint32_t a, uint32_t b) { return compare_status(a, b); });
	players_table_->set_sort_column(2);
	players_table_->set_sort_descending(false);
	players_table_->sort();

	set_thinks(false);
}

bool GameSummaryScreen::handle_mousepress(uint8_t btn, int32_t mx, int32_t my) {
	// Prevent closing with right click
	if (btn == SDL_BUTTON_RIGHT) {
		return true;
	}

	return UI::Window::handle_mousepress(btn, mx, my);
}

bool GameSummaryScreen::compare_status(const uint32_t index1, const uint32_t index2) const {
	const std::vector<Widelands::PlayerEndStatus>& all_statuses =
	   game_.player_manager()->get_players_end_status();

	const uintptr_t a = (*players_table_)[index1];
	const uintptr_t b = (*players_table_)[index2];

	assert(a < all_statuses.size());
	assert(b < all_statuses.size());

	const Widelands::PlayerEndStatus p1 = all_statuses[a];
	const Widelands::PlayerEndStatus p2 = all_statuses[b];

	if (p1.result == p2.result) {
		// We want to use the time as tie-breaker: The first player to lose sorts last
		return p1.time > p2.time;
	} else if (p1.result == Widelands::PlayerEndResult::kWon) {
		// Winners sort first
		return true;
	} else if (p1.result == Widelands::PlayerEndResult::kResigned) {
		// Resigned players sort last
		return false;
	} else if (p2.result == Widelands::PlayerEndResult::kWon) {
		return false;
	} else if (p2.result == Widelands::PlayerEndResult::kResigned) {
		return true;
	}

	NEVER_HERE();
}

void GameSummaryScreen::fill_data() {
	std::vector<Widelands::PlayerEndStatus> players_status =
	   game_.player_manager()->get_players_end_status();
	bool local_in_game = false;
	bool local_won = false;
	std::string won_name;
	Widelands::TeamNumber team_won = 0;
	InteractivePlayer* ipl = game_.get_ipl();
	// This defines a row to be selected, current player,
	// if not then the first line
	uint32_t current_player_position = 0;

	for (uintptr_t i = 0; i < players_status.size(); i++) {
		Widelands::PlayerEndStatus pes = players_status.at(i);
		if (ipl && pes.player == ipl->player_number()) {
			local_in_game = true;
			local_won = pes.result == Widelands::PlayerEndResult::kWon;
			current_player_position = i;
		}
		Widelands::Player* p = game_.get_player(pes.player);
		UI::Table<uintptr_t const>::EntryRecord& te = players_table_->add(i);
		// Player name & pic
		const Image* player_image =
		   playercolor_image(pes.player - 1, "images/players/genstats_player.png");
		assert(player_image);
		te.set_picture(0, player_image, p->get_name());
		// Team
		std::string teastr_ =
		   p->team_number() == 0 ?
		      "—" :
		      (boost::format("%|1$u|") % static_cast<unsigned int>(p->team_number())).str();
		te.set_string(1, teastr_);
		// Status
		std::string stat_str;
		switch (pes.result) {
		case Widelands::PlayerEndResult::kLost:
			/** TRANSLATORS: This is shown in the game summary for the players who have lost. */
			stat_str = _("Lost");
			break;
		case Widelands::PlayerEndResult::kWon:
			/** TRANSLATORS: This is shown in the game summary for the players who have won. */
			stat_str = _("Won");
			if (won_name.empty()) {
				won_name = p->get_name();
			} else {
				team_won = p->team_number();
			}
			break;
		case Widelands::PlayerEndResult::kResigned:
			/** TRANSLATORS: This is shown in the game summary for the players who have resigned. */
			stat_str = _("Resigned");
			break;
		case Widelands::PlayerEndResult::kUndefined:
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
		if (team_won == 0) {
			title_area_->set_text((boost::format(_("%s won!")) % won_name).str());
		} else {
			title_area_->set_text(
			   (boost::format(_("Team %|1$u| won!")) % static_cast<unsigned int>(team_won)).str());
		}
	}
	if (!players_status.empty()) {
		players_table_->select(current_player_position);
	}
	players_table_->layout();
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
	if (info_str.empty()) {
		widelands_icon_->set_visible(true);
		info_box_->set_visible(false);
	} else {
		widelands_icon_->set_visible(false);
		info_box_->set_visible(true);
	}
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
