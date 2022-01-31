/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "wui/game_exit_confirm_box.h"

#include "base/i18n.h"

GameExitConfirmBox::GameExitConfirmBox(UI::Panel& p, InteractiveGameBase& i)
   : GameExitConfirmBox(p,
                        i,
                        /** TRANSLATORS: Window label when "Exit game" has been pressed */
                        _("Exit Game Confirmation"),
                        _("Are you sure you wish to exit this game?")) {
}

// TODO(GunChleoc): Arabic: Buttons need more height for Arabic
GameExitConfirmBox::GameExitConfirmBox(UI::Panel& parent,
                                       InteractiveGameBase& gb,
                                       const std::string& title,
                                       const std::string& message)
   : UI::WLMessageBox(&parent, UI::WindowStyle::kWui, title, message, MBoxType::kOkCancel),
     igb_(gb) {
}

void GameExitConfirmBox::clicked_ok() {
	// Call method linked to ok button
	ok();
	igb_.end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
}

void GameExitConfirmBox::clicked_back() {
	cancel();
	die();
}
