/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "ui_fsmenu/load_map_or_game.h"

#include "base/i18n.h"
#include "io/filesystem/filesystem.h"
#include "ui_basic/button.h"

/// Select a Map, Saved Game or Replay in Fullscreen Mode.
/// This class defines common coordinates for these UI screens.
/// It also defines common buttons.
FullscreenMenuLoadMapOrGame::FullscreenMenuLoadMapOrGame(FullscreenMenuMain& fsmm,
                                                         const std::string& title)
   : UI::Window(&fsmm,
                UI::WindowStyle::kFsMenu,
                "load_map_or_game",
                fsmm.calc_desired_window_x(UI::Window::WindowLayoutID::kFsMenuDefault),
                fsmm.calc_desired_window_y(UI::Window::WindowLayoutID::kFsMenuDefault),
                fsmm.calc_desired_window_width(UI::Window::WindowLayoutID::kFsMenuDefault),
                fsmm.calc_desired_window_height(UI::Window::WindowLayoutID::kFsMenuDefault),
                title),
     // Main buttons
     back_(this, "back", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary, _("Back")),
     ok_(this, "ok", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuPrimary, _("OK")) {
	do_not_layout_on_resolution_change();
	layout();
}

void FullscreenMenuLoadMapOrGame::layout() {
	UI::Window::layout();

	// UI coordinates and spacers
	// TODO(Nordfriese): Magic numbers everywhere. Box layout please…
	tablex_ = get_inner_w() * 47 / 2500;
	tabley_ = get_inner_h() * 17 / 50;
	tablew_ = get_inner_w() * 711 / 1250;
	tableh_ = get_inner_h() * 6083 / 10000;
	right_column_x_ = tablex_ + tablew_ + right_column_margin_;
	buty_ = get_inner_h() * 9 / 10;
	butw_ = (get_inner_w() - right_column_x_ - right_column_margin_) / 2 - padding_;
	buth_ = get_inner_h() * 9 / 200;
	right_column_tab_ = get_inner_w() - right_column_margin_ - butw_;

	// Main buttons
	back_.set_size(butw_, buth_);
	back_.set_pos(Vector2i(right_column_x_, buty_));
	ok_.set_size(butw_, buth_);
	ok_.set_pos(Vector2i(get_inner_w() - right_column_margin_ - butw_, buty_));
}

int32_t FullscreenMenuLoadMapOrGame::get_y_from_preceding(const UI::Panel& preceding_panel) {
	return preceding_panel.get_y() + preceding_panel.get_h();
}

int32_t FullscreenMenuLoadMapOrGame::get_right_column_w(int32_t x) {
	return get_inner_w() - right_column_margin_ - x;
}
