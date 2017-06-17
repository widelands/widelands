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

void NetworkPlayerSettingsBackend::set_player_state(Widelands::PlayerNumber id, PlayerSettings::State state) {
	if (id >= s->settings().players.size()) {
		return;
	}
	// Do not close a player in savegames or scenarios
	if (state == PlayerSettings::State::kClosed && ((s->settings().scenario && !s->settings().players.at(id).closeable) || s->settings().savegame)) {
		 state = PlayerSettings::State::kOpen;
	}

	s->set_player_state(id, state);
	if (state == PlayerSettings::State::kShared) {
		Widelands::PlayerNumber shared = 0;
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

void NetworkPlayerSettingsBackend::set_player_ai(Widelands::PlayerNumber id, const std::string& name, bool random_ai) {
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

void NetworkPlayerSettingsBackend::set_tribe(Widelands::PlayerNumber id, const std::string& tribename) {
	const GameSettings& settings = s->settings();

	if (id >= settings.players.size() || tribename.empty())
		return;

	if (settings.players.at(id).state != PlayerSettings::State::kShared) {
		s->set_player_tribe(id, tribename, tribename == "random");
	}
}

/// Set the shared in player for the given id
void NetworkPlayerSettingsBackend::set_shared_in(Widelands::PlayerNumber id, Widelands::PlayerNumber shared_in) {
	const GameSettings& settings = s->settings();
	if (id > settings.players.size() || shared_in > settings.players.size())
		return;
	if (settings.players.at(id).state == PlayerSettings::State::kShared) {
		s->set_player_shared(id, shared_in);
	}
}

/// Toggle through shared in players
void NetworkPlayerSettingsBackend::toggle_shared_in(Widelands::PlayerNumber id) {
	const GameSettings& settings = s->settings();

	if (id >= settings.players.size() ||
	    settings.players.at(id).state != PlayerSettings::State::kShared)
		return;

	Widelands::PlayerNumber sharedplr = settings.players.at(id).shared_in;
	for (; sharedplr < settings.players.size(); ++sharedplr) {
		if (settings.players.at(sharedplr).state != PlayerSettings::State::kClosed &&
		    settings.players.at(sharedplr).state != PlayerSettings::State::kShared)
			break;
	}
	if (sharedplr < settings.players.size()) {
		// We have already found the next player
		set_shared_in(id, sharedplr + 1);
		return;
	}
	sharedplr = 0;
	for (; sharedplr < settings.players.at(id).shared_in; ++sharedplr) {
		if (settings.players.at(sharedplr).state != PlayerSettings::State::kClosed &&
		    settings.players.at(sharedplr).state != PlayerSettings::State::kShared)
			break;
	}
	if (sharedplr < settings.players.at(id).shared_in) {
		// We have found the next player
		set_shared_in(id, sharedplr + 1);
		return;
	} else {
		// No fitting player found
		set_player_state(id, PlayerSettings::State::kClosed);
	}
}

/// Toggle through the initializations
void NetworkPlayerSettingsBackend::toggle_init(Widelands::PlayerNumber id) {
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
void NetworkPlayerSettingsBackend::toggle_team(Widelands::PlayerNumber id) {
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
void NetworkPlayerSettingsBackend::refresh(Widelands::PlayerNumber id) {
	const GameSettings& settings = s->settings();

	if (id >= settings.players.size())
		return;

	const PlayerSettings& player = settings.players[id];

	if (player.state == PlayerSettings::State::kShared) {
		// ensure that the shared_in player is able to use this starting position
		if (player.shared_in > settings.players.size())
			toggle_shared_in(id);
		if (settings.players.at(player.shared_in - 1).state == PlayerSettings::State::kClosed ||
		    settings.players.at(player.shared_in - 1).state == PlayerSettings::State::kShared)
			toggle_shared_in(id);

		if (shared_in_tribe[id] != settings.players.at(player.shared_in - 1).tribe) {
			s->set_player_tribe(id, settings.players.at(player.shared_in - 1).tribe,
			                    settings.players.at(player.shared_in - 1).random_tribe);
			shared_in_tribe[id] = settings.players.at(id).tribe;
		}
	}
}
