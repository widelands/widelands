/*
 * Copyright (C) 2010-2012 by the Widelands Development Team
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

#include "gamesettings.h"
#include "i18n.h"
#include "log.h"
#include "logic/player.h"
#include "logic/tribe.h"
#include "profile/profile.h"


/// Toggle through the types
void NetworkPlayerSettingsBackend::toggle_type(uint8_t id) {
	if (id >= s->settings().players.size())
		return;

	s->nextPlayerState(id);
}

/// Toggle through the tribes + handle shared in players
void NetworkPlayerSettingsBackend::toggle_tribe(uint8_t id) {
	const GameSettings & settings = s->settings();

	if (id >= settings.players.size())
		return;

	if (settings.players.at(id).state != PlayerSettings::stateShared) {
		const PlayerSettings & player = settings.players.at(id);
		const std::string & currenttribe = player.tribe;
		std::string nexttribe = settings.tribes.at(0).name;
		uint32_t num_tribes = settings.tribes.size();
		bool random_tribe = false;

		if (player.random_tribe) {
			nexttribe = settings.tribes.at(0).name;
		} else if (player.tribe == settings.tribes.at(num_tribes - 1).name) {
			nexttribe = "Random";
			random_tribe = true;
		} else {
			for (uint32_t i = 0; i < num_tribes - 1; ++i) {
				if (settings.tribes[i].name == currenttribe) {
					nexttribe = settings.tribes.at(i + 1).name;
					break;
				}
			}
		}

		s->setPlayerTribe(id, nexttribe, random_tribe);
	} else {
		// This button is temporarily used to select the player that uses this starting position
		uint8_t sharedplr = settings.players.at(id).shared_in;
		for (; sharedplr < settings.players.size(); ++sharedplr) {
			if
				(settings.players.at(sharedplr).state != PlayerSettings::stateClosed
				 &&
				 settings.players.at(sharedplr).state != PlayerSettings::stateShared)
				break;
		}
		if (sharedplr < settings.players.size()) {
			// We have already found the next player
			s->setPlayerShared(id, sharedplr + 1);
			return;
		}
		sharedplr = 0;
		for (; sharedplr < settings.players.at(id).shared_in; ++sharedplr) {
			if
				(settings.players.at(sharedplr).state != PlayerSettings::stateClosed
				 &&
				 settings.players.at(sharedplr).state != PlayerSettings::stateShared)
				break;
		}
		if (sharedplr < settings.players.at(id).shared_in) {
			// We have found the next player
			s->setPlayerShared(id, sharedplr + 1);
			return;
		} else {
			// No fitting player found
			return toggle_type(id);
		}
	}
}

/// Toggle through the initializations
void NetworkPlayerSettingsBackend::toggle_init(uint8_t id) {
	const GameSettings & settings = s->settings();

	if (id >= settings.players.size())
		return;

	const PlayerSettings & player = settings.players[id];
	container_iterate_const(std::vector<TribeBasicInfo>, settings.tribes, j)
		if (j.current->name == player.tribe)
			return
				s->setPlayerInit
					(id,
					 (player.initialization_index + 1)
					 %
					 j.current->initializations.size());
	assert(false);
}

/// Toggle through the teams
void NetworkPlayerSettingsBackend::toggle_team(uint8_t id) {
	const GameSettings & settings = s->settings();

	if (id >= settings.players.size())
		return;

	Widelands::TeamNumber currentteam = settings.players.at(id).team;
	Widelands::TeamNumber maxteam = settings.players.size() / 2;
	Widelands::TeamNumber newteam;

	if (currentteam >= maxteam)
		newteam = 0;
	else
		newteam = currentteam + 1;

	s->setPlayerTeam(id, newteam);
}

/// Check if all settings for the player are still valid
void NetworkPlayerSettingsBackend::refresh(uint8_t id) {
	const GameSettings & settings = s->settings();

	if (id >= settings.players.size())
		return;

	const PlayerSettings & player = settings.players[id];

	if (player.state == PlayerSettings::stateShared) {
		// ensure that the shared_in player is able to use this starting position
		if (player.shared_in > settings.players.size())
			toggle_tribe(id);
		if
			(settings.players.at(player.shared_in - 1).state == PlayerSettings::stateClosed
			 ||
			 settings.players.at(player.shared_in - 1).state == PlayerSettings::stateShared)
			toggle_tribe(id);

		if (shared_in_tribe[id] != settings.players.at(player.shared_in - 1).tribe) {
			s->setPlayerTribe
				(id, settings.players.at(player.shared_in - 1).tribe,
				 settings.players.at(player.shared_in - 1).random_tribe);
			shared_in_tribe[id] = settings.players.at(id).tribe;
		}
	}
}
