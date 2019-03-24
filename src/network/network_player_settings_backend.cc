/*
 * Copyright (C) 2010-2019 by the Widelands Development Team
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

#include "network/network_player_settings_backend.h"

#include "ai/computer_player.h"

void NetworkPlayerSettingsBackend::set_player_state(PlayerSlot id, PlayerSettings::State state) {
	if (id >= s->settings().players.size()) {
		return;
	}
	s->set_player_state(id, state);
}

void NetworkPlayerSettingsBackend::set_player_ai(PlayerSlot id,
                                                 const std::string& name,
                                                 bool random_ai) {
	if (id >= s->settings().players.size()) {
		return;
	}
	if (random_ai) {
		const ComputerPlayer::ImplementationVector& impls = ComputerPlayer::get_implementations();
		ComputerPlayer::ImplementationVector::const_iterator it = impls.begin();
		if (impls.size() > 1) {
			do {
				size_t random = (std::rand() % impls.size());  // Choose a random AI
				it = impls.begin() + random;
			} while ((*it)->type == ComputerPlayer::Implementation::Type::kEmpty);
		}
		s->set_player_ai(id, (*it)->name, random_ai);
	} else {
		s->set_player_ai(id, name, random_ai);
	}
}

void NetworkPlayerSettingsBackend::set_player_tribe(PlayerSlot id, const std::string& tribename) {
	const GameSettings& settings = s->settings();
	if (id >= settings.players.size() || tribename.empty()) {
		return;
	}
	if (settings.players.at(id).state != PlayerSettings::State::kShared) {
		s->set_player_tribe(id, tribename, tribename == "random");
	}
}

/// Set the shared in player for the given id
void NetworkPlayerSettingsBackend::set_player_shared(PlayerSlot id,
                                                     Widelands::PlayerNumber shared) {
	const GameSettings& settings = s->settings();
	if (id >= settings.players.size() || shared > settings.players.size())
		return;
	if (settings.players.at(id).state == PlayerSettings::State::kShared) {
		s->set_player_shared(id, shared);
	}
}

/// Sets the initialization for the player slot (Headquarters, Fortified Village etc.)
void NetworkPlayerSettingsBackend::set_player_init(PlayerSlot id, uint8_t initialization_index) {
	if (id >= s->settings().players.size()) {
		return;
	}
	s->set_player_init(id, initialization_index);
}

/// Sets the team for the player slot
void NetworkPlayerSettingsBackend::set_player_team(PlayerSlot id, Widelands::TeamNumber team) {
	if (id >= s->settings().players.size()) {
		return;
	}
	s->set_player_team(id, team);
}
