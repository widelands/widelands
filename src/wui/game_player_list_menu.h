/*
 * Copyright (C) 2007-2021 by the Widelands Development Team
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
 */

#ifndef WL_WUI_GAME_PLAYER_LIST_MENU_H
#define WL_WUI_GAME_PLAYER_LIST_MENU_H

#include <map>
#include <memory>
#include <vector>

#include "logic/widelands.h"
#include "ui_basic/box.h"
#include "ui_basic/panel.h"
#include "ui_basic/unique_window.h"

class ParticipantList;

/**
 * A window with a list of all players and spectators of the game.
 */
struct GamePlayerListMenu : public UI::UniqueWindow {
public:
	/**
	 * Creates the window.
	 * @param parent The parent window.
	 * @param registry The window registry.
	 * @param participants A \c ParticipantList to get the player data from.
	 */
	GamePlayerListMenu(Panel& parent,
	                   UI::UniqueWindow::Registry& registry,
	                   ParticipantList* participants);
	/// Destructor
	~GamePlayerListMenu() override;

private:
	/**
	 * Creates the contents of the window.
	 */
	void rebuild();

	/// A pointer to the \c ParticipantList to get the player data from
	ParticipantList* participants_;

	/// A vertical box as main element of the window to keep the entries in
	UI::Box vbox_;
	/// A map of teams and their respective players, including the header lines
	std::map<Widelands::TeamNumber, std::vector<std::unique_ptr<UI::Panel>>> teams_;
	/// The spectators of the game, including the header lines
	std::vector<std::unique_ptr<UI::Panel>> spectators_;
	/// An object to stop listening for participant updates when the window is closed
	boost::signals2::connection update_signal_connection_;
	/// An object to stop listening for rtt updates when the window is closed
	boost::signals2::connection rtt_signal_connection_;
};

#endif  // end of include guard: WL_WUI_GAME_PLAYER_LIST_MENU_H
