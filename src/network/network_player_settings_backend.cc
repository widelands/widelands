/*
 * Copyright (C) 2010-2017 by the Widelands Development Team
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
#include "base/i18n.h"
#include "base/log.h"
#include "base/wexception.h"
#include "logic/game_settings.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"

void NetworkPlayerSettingsBackend::set_player_state(PlayerSlot id, PlayerSettings::State state) {
	if (id >= s->settings().players.size()) {
		return;
	}
	// Do not close a player in savegames or scenarios
	if (state == PlayerSettings::State::kClosed && ((s->settings().scenario && !s->settings().players.at(id).closeable) || s->settings().savegame)) {
		 state = PlayerSettings::State::kOpen;
	}

	s->set_player_state(id, state);
	if (state == PlayerSettings::State::kShared) {
		PlayerSlot shared = 0;
		for (; shared < s->settings().players.size(); ++shared) {
			if (s->settings().players.at(shared).state != PlayerSettings::State::kClosed &&
				 s->settings().players.at(shared).state != PlayerSettings::State::kShared)
				break;
		}
		if (shared < s->settings().players.size()) {
			s->set_player_shared(id, shared + 1);
		} else {
			s->set_player_state(id, PlayerSettings::State::kClosed);
		}
	}
}

void NetworkPlayerSettingsBackend::set_player_ai(PlayerSlot id, const std::string& name, bool random_ai) {
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

void NetworkPlayerSettingsBackend::set_tribe(PlayerSlot id, const std::string& tribename) {
	const GameSettings& settings = s->settings();

	if (id >= settings.players.size() || tribename.empty())
		return;

	if (settings.players.at(id).state != PlayerSettings::State::kShared) {
		s->set_player_tribe(id, tribename, tribename == "random");
	}
}

/// Set the shared in player for the given id
void NetworkPlayerSettingsBackend::set_shared_in(PlayerSlot id, Widelands::PlayerNumber shared_in) {
	const GameSettings& settings = s->settings();
	if (id > settings.players.size() || shared_in > settings.players.size())
		return;
	if (settings.players.at(id).state == PlayerSettings::State::kShared) {
		s->set_player_shared(id, shared_in);
	}
}

/// Toggle through shared in players. We don't set them here yet to avoid triggering extra notifications from the server. If '0' is returned, no suitable player was found.
Widelands::PlayerNumber NetworkPlayerSettingsBackend::find_next_shared_in(PlayerSlot id) {
	const GameSettings& settings = s->settings();

	if (id >= settings.players.size() ||
	    settings.players.at(id).state != PlayerSettings::State::kShared)
		return 0;

	Widelands::PlayerNumber sharedplr = settings.players.at(id).shared_in;
	for (; sharedplr < settings.players.size(); ++sharedplr) {
		if (settings.players.at(sharedplr).state != PlayerSettings::State::kClosed &&
		    settings.players.at(sharedplr).state != PlayerSettings::State::kShared)
			break;
	}
	if (sharedplr < settings.players.size()) {
		// We have already found the next player
		return ++sharedplr;
	}
	sharedplr = 0;
	for (; sharedplr < settings.players.at(id).shared_in; ++sharedplr) {
		if (settings.players.at(sharedplr).state != PlayerSettings::State::kClosed &&
		    settings.players.at(sharedplr).state != PlayerSettings::State::kShared)
			break;
	}
	if (sharedplr < settings.players.at(id).shared_in) {
		++sharedplr;
	}
	return sharedplr;
}

/// Toggle through the initializations
void NetworkPlayerSettingsBackend::toggle_init(PlayerSlot id) {
	const GameSettings& settings = s->settings();

	if (id >= settings.players.size())
		return;

	const PlayerSettings& player = settings.players[id];
	for (const TribeBasicInfo& temp_tribeinfo : settings.tribes) {
		if (temp_tribeinfo.name == player.tribe) {
			return s->set_player_init(
			   id, (player.initialization_index + 1) % temp_tribeinfo.initializations.size());
		}
	}
	NEVER_HERE();
}

/// Toggle through the teams
void NetworkPlayerSettingsBackend::toggle_team(PlayerSlot id) {
	const GameSettings& settings = s->settings();

	if (id >= settings.players.size())
		return;

	Widelands::TeamNumber currentteam = settings.players.at(id).team;
	Widelands::TeamNumber maxteam = settings.players.size() / 2;
	Widelands::TeamNumber newteam;

	if (currentteam >= maxteam)
		newteam = 0;
	else
		newteam = currentteam + 1;

	s->set_player_team(id, newteam);
}

/// Check if all settings for the player are still valid
void NetworkPlayerSettingsBackend::refresh(PlayerSlot id) {
	const GameSettings& settings = s->settings();

	if (id >= settings.players.size())
		return;

	const PlayerSettings& player = settings.players[id];
	if (player.state == PlayerSettings::State::kShared) {
		const Widelands::PlayerNumber old_shared_in = player.shared_in;
		Widelands::PlayerNumber new_shared_in = player.shared_in;
		// ensure that the shared_in player is able to use this starting position
		if (new_shared_in == 0 || new_shared_in > settings.players.size()) {
			new_shared_in = find_next_shared_in(id);
			if (new_shared_in == 0) {
				// No fitting player found
				set_player_state(id, PlayerSettings::State::kClosed);
				return;
			}
		}

		if (settings.players.at(new_shared_in - 1).state == PlayerSettings::State::kClosed ||
			 settings.players.at(new_shared_in - 1).state == PlayerSettings::State::kShared) {
			new_shared_in = find_next_shared_in(id);
			if (new_shared_in == 0) {
				// No fitting player found
				set_player_state(id, PlayerSettings::State::kClosed);
				return;
			}
		}

		if (new_shared_in != old_shared_in && settings.players.at(new_shared_in - 1).state != PlayerSettings::State::kClosed && settings.players.at(new_shared_in - 1).state != PlayerSettings::State::kShared) {
			if (shared_in_tribe[id] != settings.players.at(new_shared_in - 1).tribe) {
				s->set_player_tribe(id, settings.players.at(new_shared_in - 1).tribe,
										  settings.players.at(new_shared_in - 1).random_tribe);
				shared_in_tribe[id] = settings.players.at(id).tribe;
			}
			set_shared_in(id, new_shared_in);
		}
	}
}
