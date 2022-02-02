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

#ifndef WL_WUI_INTERACTIVE_SPECTATOR_H
#define WL_WUI_INTERACTIVE_SPECTATOR_H

#include "io/profile.h"
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
	InteractiveSpectator(Widelands::Game&,
	                     Section& global_s,
	                     bool multiplayer = false,
	                     ChatProvider* chat_provider = nullptr);

	Widelands::Player* get_player() const override;

	bool handle_key(bool down, SDL_Keysym) override;
	void draw(RenderTarget& dst) override;
	void draw_map_view(MapView* given_map_view, RenderTarget* dst) override;

private:
	bool player_hears_field(const Widelands::Coords& coords) const override;

	void exit_btn();
	bool can_see(Widelands::PlayerNumber) const override;
	bool can_act(Widelands::PlayerNumber) const override;
	Widelands::PlayerNumber player_number() const override;
	void node_action(const Widelands::NodeAndTriangle<>& node_and_triangle) override;
};

#endif  // end of include guard: WL_WUI_INTERACTIVE_SPECTATOR_H
