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

#include "base/i18n.h"
#include "base/log.h"
#include "base/wexception.h"
#include "logic/game_settings.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"

/// Toggle through the types
void NetworkPlayerSettingsBackend::toggle_type(uint8_t id) {
	if (id >= s->settings().players.size())
		return;

	s->next_player_state(id);
}

void NetworkPlayerSettingsBackend::set_tribe(uint8_t id, const std::string& tribename) {
	const GameSettings& settings = s->settings();

	if (id >= settings.players.size() || tribename.empty())
		return;

	if (settings.players.at(id).state != PlayerSettings::stateShared) {
		s->set_player_tribe(id, tribename, tribename == "random");
	}
}

/// Set the shared in player for the given id
void NetworkPlayerSettingsBackend::set_shared_in(uint8_t id, uint8_t shared_in) {
	const GameSettings& settings = s->settings();
	if (id > settings.players.size() || shared_in > settings.players.size())
		return;
	if (settings.players.at(id).state == PlayerSettings::stateShared) {
		s->set_player_shared(id, shared_in);
	}
}

/// Toggle through shared in players
void NetworkPlayerSettingsBackend::toggle_shared_in(uint8_t id) {
	const GameSettings& settings = s->settings();

	if (id >= settings.players.size() ||
	    settings.players.at(id).state != PlayerSettings::stateShared)
		return;

	uint8_t sharedplr = settings.players.at(id).shared_in;
	for (; sharedplr < settings.players.size(); ++sharedplr) {
		if (settings.players.at(sharedplr).state != PlayerSettings::stateClosed &&
		    settings.players.at(sharedplr).state != PlayerSettings::stateShared)
			break;
	}
	if (sharedplr < settings.players.size()) {
		// We have already found the next player
		set_shared_in(id, sharedplr + 1);
		return;
	}
	sharedplr = 0;
	for (; sharedplr < settings.players.at(id).shared_in; ++sharedplr) {
		if (settings.players.at(sharedplr).state != PlayerSettings::stateClosed &&
		    settings.players.at(sharedplr).state != PlayerSettings::stateShared)
			break;
	}
	if (sharedplr < settings.players.at(id).shared_in) {
		// We have found the next player
		set_shared_in(id, sharedplr + 1);
		return;
	} else {
		// No fitting player found
		return toggle_type(id);
	}
}

/// Toggle through the initializations
void NetworkPlayerSettingsBackend::toggle_init(uint8_t id) {
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
void NetworkPlayerSettingsBackend::toggle_team(uint8_t id) {
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
void NetworkPlayerSettingsBackend::refresh(uint8_t id) {
	const GameSettings& settings = s->settings();

	if (id >= settings.players.size())
		return;

	const PlayerSettings& player = settings.players[id];

	if (player.state == PlayerSettings::stateShared) {
		// ensure that the shared_in player is able to use this starting position

		if (player.shared_in > settings.players.size())
			toggle_shared_in(id);
		if (settings.players.at(player.shared_in - 1).state == PlayerSettings::stateClosed ||
		    settings.players.at(player.shared_in - 1).state == PlayerSettings::stateShared)
			toggle_shared_in(id);

		if (shared_in_tribe[id] != settings.players.at(player.shared_in - 1).tribe) {
			s->set_player_tribe(id, settings.players.at(player.shared_in - 1).tribe,
			                    settings.players.at(player.shared_in - 1).random_tribe);
			shared_in_tribe[id] = settings.players.at(id).tribe;
		}
	}
}
