/*
 * Copyright (C) 2011-2022 by the Widelands Development Team
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

#ifndef WL_NETWORK_NETWORK_PLAYER_SETTINGS_BACKEND_H
#define WL_NETWORK_NETWORK_PLAYER_SETTINGS_BACKEND_H

#include "logic/game_settings.h"

struct NetworkPlayerSettingsBackend {

	explicit NetworkPlayerSettingsBackend(GameSettingsProvider* const settings) : s(settings) {
	}

	void set_player_state(PlayerSlot id, PlayerSettings::State state) const;
	void set_player_ai(PlayerSlot id, const std::string& name, bool random_ai) const;
	void set_player_shared(PlayerSlot id, Widelands::PlayerNumber shared) const;
	void set_player_tribe(PlayerSlot id, const std::string& tribename) const;
	void set_player_init(PlayerSlot id, uint8_t initialization_index) const;
	void set_player_team(PlayerSlot id, Widelands::TeamNumber team) const;
	void set_player_color(PlayerSlot id, const RGBColor& c) const;

	GameSettingsProvider* const s;
};

#endif  // end of include guard: WL_NETWORK_NETWORK_PLAYER_SETTINGS_BACKEND_H
