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

struct NetworkPlayerSettingsBackend {

	NetworkPlayerSettingsBackend(GameSettingsProvider* const settings) : s(settings) {
		for (uint8_t i = 0; i < kMaxPlayers; ++i)
			shared_in_tribe[i] = std::string();
	}

	void set_player_state(uint8_t id, PlayerSettings::State state);
	void set_player_ai(uint8_t id, const std::string& name, bool random_ai);
	void set_shared_in(uint8_t id, uint8_t shared_in);
	void set_tribe(uint8_t id, const std::string& tribename);

	void set_block_type_selection(bool blocked) {
		type_selection_blocked = blocked;
	}
	void set_block_tribe_selection(bool blocked) {
		tribe_selection_blocked = blocked;
	}

	void toggle_init(uint8_t id);
	void toggle_team(uint8_t id);
	void refresh(uint8_t id);

	GameSettingsProvider* const s;
	std::string shared_in_tribe[kMaxPlayers];
	bool type_selection_blocked;
	bool tribe_selection_blocked;

private:
	void toggle_shared_in(uint8_t id);
};

#endif  // end of include guard: WL_NETWORK_NETWORK_PLAYER_SETTINGS_BACKEND_H
