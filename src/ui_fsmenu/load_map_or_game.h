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

#ifndef WL_UI_FSMENU_LOAD_MAP_OR_GAME_H
#define WL_UI_FSMENU_LOAD_MAP_OR_GAME_H

#include "base/i18n.h"
#include "ui_basic/button.h"
#include "ui_basic/table.h"
#include "ui_basic/window.h"
#include "ui_fsmenu/main.h"

namespace Widelands {
class Game;
class Map;
}  // namespace Widelands

/// Select a Map, Saved Game or Replay in Fullscreen Mode.
/// This class defines common coordinates for these UI screens.
/// It also defines common buttons.
class FullscreenMenuLoadMapOrGame : public UI::Window {
public:
	explicit FullscreenMenuLoadMapOrGame(FullscreenMenuMain&, const std::string& title);

	WindowLayoutID window_layout_id() const override {
		return UI::Window::WindowLayoutID::kFsMenuDefault;
	}

protected:
	void layout() override;

	// Updates the information display on the right-hand side.
	// Call this function when a different entry in the table gets selected.
	virtual void entry_selected() {
	}
	virtual void fill_table() {
	}

	virtual void clicked_back() {
		end_modal<MenuTarget>(MenuTarget::kBack);
	}
	virtual void clicked_ok() {
		end_modal<MenuTarget>(MenuTarget::kOk);
	}

	// Returns a y coordinate that can be used to position a Panel below the Panel directly above it
	int32_t get_y_from_preceding(UI::Panel& preceding_panel);

	// Returns the width that a Panel in the right column should have, depending on its x position
	int32_t get_right_column_w(int32_t x);

	// UI coordinates and spacers
	const int32_t padding_ = 4;  // Common padding between panels
	const int32_t indent_ = 10;  // Indent for elements below labels
	const int32_t label_height_ = 20;
	const int32_t right_column_margin_ = 16;  // X margins of the right column
	// These are set by the layout() function.
	int32_t tablex_, tabley_, tablew_, tableh_ = 0;
	int32_t right_column_x_ = 0;
	int32_t buty_, butw_, buth_ = 0;  // Button dimensions
	int32_t right_column_tab_ = 0;

	// Main buttons
	UI::Button back_;
	UI::Button ok_;
};

#endif  // end of include guard: WL_UI_FSMENU_LOAD_MAP_OR_GAME_H
