/*
 * Copyright (C) 2011 by the Widelands Development Team
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

#ifndef NETWORK_PLAYER_SETTINGS_BACKEND_H
#define NETWORK_PLAYER_SETTINGS_BACKEND_H

#include "constants.h"
#include "gamesettings.h"

struct NetworkPlayerSettingsBackend {

	NetworkPlayerSettingsBackend(GameSettingsProvider * const settings)
	:
	s(settings)
	{
		for (uint8_t i = 0; i < MAX_PLAYERS; ++i)
			shared_in_tribe[i] = std::string();
	};

	void toggle_type (uint8_t id);
	void toggle_tribe(uint8_t id);
	void toggle_init (uint8_t id);
	void toggle_team (uint8_t id);
	void refresh     (uint8_t id);

	GameSettingsProvider * const s;
	std::string            shared_in_tribe[MAX_PLAYERS];
};

#endif
