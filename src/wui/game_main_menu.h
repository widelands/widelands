/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

#ifndef WL_WUI_GAME_MAIN_MENU_H
#define WL_WUI_GAME_MAIN_MENU_H

#include "wui/interactive_player.h"
#include "ui_basic/button.h"

// The GameMainMenu is a rather dumb window with lots of buttons
struct GameMainMenu : public UI::UniqueWindow {
	GameMainMenu
		(InteractivePlayer &,
		 UI::UniqueWindow::Registry &,
		 InteractivePlayer::GameMainMenuWindows &);

	~GameMainMenu();

private:
	InteractivePlayer                      & player_;
	InteractivePlayer::GameMainMenuWindows & windows_;
	UI::Button general_stats;
	UI::Button ware_stats;
	UI::Button building_stats;
	UI::Button stock;

	/** Returns the horizontal/vertical spacing between buttons. */
	uint32_t hspacing() const {return 5;}
	uint32_t vspacing() const {return 5;}

	/** Returns the horizontal/vertical margin between edge and buttons. */
	uint32_t hmargin() const {return 2 * hspacing();}
	uint32_t vmargin() const {return 2 * vspacing();}

	/** Returns the width of a button in a row with nr_buttons buttons. */
	uint32_t buttonw(uint32_t const nr_buttons) const {
		return (get_inner_w() - (nr_buttons + 3) * hspacing()) / nr_buttons;
	}

	/** Returns the height of buttons in a window with nr_rows rows. */
	uint32_t buttonh(uint32_t const nr_rows) const {
		return (get_inner_h() - (nr_rows    + 3) * vspacing()) / nr_rows;
	}

	/// Returns the x coordinate of the (left edge of) button number nr in a row
	/// with nr_buttons buttons.
	uint32_t posx(uint32_t const nr, uint32_t const nr_buttons) const {
		return hmargin() + nr * (buttonw(nr_buttons) + hspacing());
	}

	/// Returns the y coordinate of the (top edge of) a button in row number nr
	/// in a dialog with nr_rows rows.
	uint32_t posy(uint32_t const nr, uint32_t const nr_rows) const {
		return vmargin() + nr * (buttonh(nr_rows) + vspacing());
	}
};

#endif  // end of include guard: WL_WUI_GAME_MAIN_MENU_H
