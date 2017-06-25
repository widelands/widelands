/*
 * Copyright (C) 2017 by the Widelands Development Team
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

#include "logic/game_settings.h"

Widelands::PlayerNumber GameSettings::find_shared(PlayerSlot slot) const {
	Widelands::PlayerNumber result = 1;
	for (;result <= players.size(); ++result) {
		if (PlayerSettings::can_be_shared(players.at(result - 1).state) && (result - 1) != slot) {
			break;
		}
	}
	return result;
}

bool GameSettings::is_shared_usable(PlayerSlot slot, Widelands::PlayerNumber shared) const {
	return shared <= players.size() && (shared - 1) != slot;
}

bool GameSettings::uncloseable(PlayerSlot slot) const {
	return (scenario && !players.at(slot).closeable) || savegame;
}
