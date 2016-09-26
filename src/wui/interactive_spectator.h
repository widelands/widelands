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

#ifndef WL_WUI_INTERACTIVE_SPECTATOR_H
#define WL_WUI_INTERACTIVE_SPECTATOR_H

#include <SDL_keyboard.h>

#include "profile/profile.h"
#include "ui_basic/button.h"
#include "wui/interactive_gamebase.h"

namespace Widelands {
class Game;
}

/**
 * This class shows a game for somebody who is only a spectator.
 *
 * It is used for replays and network games.
 *
 * This class provides the UI, runs the game logic, etc.
 */
struct InteractiveSpectator : public InteractiveGameBase {
	InteractiveSpectator(Widelands::Game&, Section& global_s, bool multiplayer = false);

	~InteractiveSpectator();

	Widelands::Player* get_player() const override;

	bool handle_key(bool down, SDL_Keysym) override;

private:
	void exit_btn();
	int32_t calculate_buildcaps(const Widelands::TCoords<Widelands::FCoords>& c) override;
	bool can_see(Widelands::PlayerNumber) const override;
	bool can_act(Widelands::PlayerNumber) const override;
	Widelands::PlayerNumber player_number() const override;
	void node_action() override;

private:
	UI::Button* toggle_chat_;
	UI::Button* exit_;
	UI::Button* save_;
	UI::Button* toggle_options_menu_;
	UI::Button* toggle_statistics_;
	UI::Button* toggle_minimap_;

	UI::UniqueWindow::Registry chat_;
	UI::UniqueWindow::Registry options_;
};

#endif  // end of include guard: WL_WUI_INTERACTIVE_SPECTATOR_H
