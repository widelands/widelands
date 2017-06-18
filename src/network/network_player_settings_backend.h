/*
 * Copyright (C) 2011-2017 by the Widelands Development Team
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

#ifndef WL_NETWORK_NETWORK_PLAYER_SETTINGS_BACKEND_H
#define WL_NETWORK_NETWORK_PLAYER_SETTINGS_BACKEND_H

#include "graphic/playercolor.h"
#include "logic/game_settings.h"
#include "logic/widelands.h"

struct NetworkPlayerSettingsBackend {

	NetworkPlayerSettingsBackend(GameSettingsProvider* const settings) : s(settings) {
		for (PlayerSlot i = 0; i < kMaxPlayers; ++i)
			shared_in_tribe[i] = std::string();
	}

	void set_player_state(PlayerSlot id, PlayerSettings::State state);
	void set_player_ai(PlayerSlot id, const std::string& name, bool random_ai);
	void set_shared_in(PlayerSlot id, Widelands::PlayerNumber shared_in);
	void set_tribe(PlayerSlot id, const std::string& tribename);

	void toggle_init(PlayerSlot id);
	void toggle_team(PlayerSlot id);
	void refresh(PlayerSlot id);

	GameSettingsProvider* const s;
	std::string shared_in_tribe[kMaxPlayers];

private:
	Widelands::PlayerNumber find_next_shared_in(PlayerSlot id);
};

#endif  // end of include guard: WL_NETWORK_NETWORK_PLAYER_SETTINGS_BACKEND_H
