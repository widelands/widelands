/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include <memory>

#include "base/i18n.h"
#include "graphic/graphic.h"
#include "io/filesystem/filesystem.h"
#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"

/// Select a Map, Saved Game or Replay in Fullscreen Mode.
/// This class defines common coordinates for these UI screens.
/// It also defines common buttons.
FullscreenMenuLoadMapOrGame::FullscreenMenuLoadMapOrGame()
   : FullscreenMenuBase(),

     // Values for alignment and size
     padding_(4),
     indent_(10),
     label_height_(20),
     right_column_margin_(16),

     // Main buttons
     back_(this, "back", 0, 0, 0, 0, g_gr->images().get("images/ui_basic/but0.png"), _("Back")),
     ok_(this, "ok", 0, 0, 0, 0, g_gr->images().get("images/ui_basic/but2.png"), _("OK")) {
	layout();
}

void FullscreenMenuLoadMapOrGame::layout() {
	// UI coordinates and spacers
	tablex_ = get_w() * 47 / 2500;
	tabley_ = get_h() * 17 / 50;
	tablew_ = get_w() * 711 / 1250;
	tableh_ = get_h() * 6083 / 10000;
	right_column_x_ = tablex_ + tablew_ + right_column_margin_;
	buty_ = get_h() * 9 / 10;
	butw_ = (get_w() - right_column_x_ - right_column_margin_) / 2 - padding_;
	buth_ = get_h() * 9 / 200;
	right_column_tab_ = get_w() - right_column_margin_ - butw_;

	// Main buttons
	back_.set_size(butw_, buth_);
	back_.set_pos(Vector2i(right_column_x_, buty_));
	ok_.set_size(butw_, buth_);
	ok_.set_pos(Vector2i(get_w() - right_column_margin_ - butw_, buty_));
}

int32_t FullscreenMenuLoadMapOrGame::get_y_from_preceding(UI::Panel& preceding_panel) {
	return preceding_panel.get_y() + preceding_panel.get_h();
}

int32_t FullscreenMenuLoadMapOrGame::get_right_column_w(int32_t x) {
	return get_w() - right_column_margin_ - x;
}
