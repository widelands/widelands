/*
 * Copyright (C) 2015-2022 by the Widelands Development Team
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

#include "logic/single_player_game_settings_provider.h"

#include <cstdlib>

#include "ai/computer_player.h"
#include "base/i18n.h"
#include "base/random.h"
#include "base/wexception.h"
#include "graphic/playercolor.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"

SinglePlayerGameSettingsProvider::SinglePlayerGameSettingsProvider() {
	s.tribes = Widelands::get_all_tribeinfos(nullptr);
	s.scenario = false;
	s.multiplayer = false;
	s.playernum = 0;
}

void SinglePlayerGameSettingsProvider::set_scenario(bool const set) {
	s.scenario = set;
}

const GameSettings& SinglePlayerGameSettingsProvider::settings() {
	return s;
}

bool SinglePlayerGameSettingsProvider::can_change_map() {
	return true;
}

bool SinglePlayerGameSettingsProvider::can_change_player_state(uint8_t number) {
	return ((!s.scenario) && (number != s.playernum));
}

bool SinglePlayerGameSettingsProvider::can_change_player_tribe(uint8_t /*number*/) {
	return !s.scenario;
}

bool SinglePlayerGameSettingsProvider::can_change_player_init(uint8_t /*number*/) {
	return !s.scenario;
}

bool SinglePlayerGameSettingsProvider::can_change_player_team(uint8_t /*number*/) {
	return !s.scenario;
}

bool SinglePlayerGameSettingsProvider::can_launch() {
	return !s.mapname.empty() && !s.players.empty();
}

std::string SinglePlayerGameSettingsProvider::get_map() {
	return s.mapfilename;
}

bool SinglePlayerGameSettingsProvider::is_peaceful_mode() {
	return s.peaceful;
}

void SinglePlayerGameSettingsProvider::set_peaceful_mode(bool peace) {
	s.peaceful = peace;
}

bool SinglePlayerGameSettingsProvider::get_custom_starting_positions() {
	return s.custom_starting_positions;
}

void SinglePlayerGameSettingsProvider::set_custom_starting_positions(bool c) {
	s.custom_starting_positions = c;
}

void SinglePlayerGameSettingsProvider::set_map(const std::string& mapname,
                                               const std::string& mapfilename,
                                               const std::string& map_theme,
                                               const std::string& map_bg,
                                               uint32_t const maxplayers,
                                               bool const savegame) {
	s.mapname = mapname;
	s.mapfilename = mapfilename;
	s.savegame = savegame;
	s.map_background = map_bg;
	s.map_theme = map_theme;
	s.players.resize(maxplayers);

	set_player_number(0);
	for (uint32_t player_nr = 0; player_nr < maxplayers; ++player_nr) {
		PlayerSettings& player = s.players[player_nr];
		player.state =
		   (player_nr == 0) ? PlayerSettings::State::kHuman : PlayerSettings::State::kComputer;
		player.tribe = s.tribes.at(0).name;
		player.random_tribe = false;
		player.initialization_index = 0;
		player.name = format(_("Player %u"), (player_nr + 1));
		player.team = 0;
		player.color = kPlayerColors[player_nr];
		// Set default computerplayer ai type
		if (player.state == PlayerSettings::State::kComputer) {
			const AI::ComputerPlayer::ImplementationVector& impls =
			   AI::ComputerPlayer::get_implementations();
			if (impls.size() > 1) {
				player.ai = impls.at(0)->name;
				player.random_ai = false;
			}
			// If AI player then set tribe to random
			if (!s.scenario) {
				set_player_tribe(player_nr, "", true);
			}
		}
	}
}

void SinglePlayerGameSettingsProvider::set_player_state(uint8_t const number,
                                                        PlayerSettings::State state) {
	if (number == s.playernum || number >= s.players.size()) {
		return;
	}

	if (state == PlayerSettings::State::kOpen) {
		state = PlayerSettings::State::kComputer;
	}

	s.players[number].state = state;
}

void SinglePlayerGameSettingsProvider::set_player_ai(uint8_t const number,
                                                     const std::string& ai,
                                                     bool const random_ai) {
	if (number < s.players.size()) {
		s.players[number].ai = ai;
		s.players[number].random_ai = random_ai;
	}
}

void SinglePlayerGameSettingsProvider::next_player_state(uint8_t const number) {
	if (number == s.playernum || number >= s.players.size()) {
		return;
	}

	const AI::ComputerPlayer::ImplementationVector& impls =
	   AI::ComputerPlayer::get_implementations();
	if (impls.size() > 1) {
		AI::ComputerPlayer::ImplementationVector::const_iterator it = impls.begin();
		do {
			++it;
			if ((*(it - 1))->name == s.players[number].ai) {
				break;
			}
		} while (it != impls.end());
		if (s.players[number].random_ai) {
			s.players[number].random_ai = false;
			it = impls.begin();
		} else if (it == impls.end()) {
			s.players[number].random_ai = true;
			do {
				// Choose a random AI
				uint8_t random = RNG::static_rand(impls.size());
				it = impls.begin() + random;
			} while ((*it)->type == AI::ComputerPlayer::Implementation::Type::kEmpty);
		}
		s.players[number].ai = (*it)->name;
	}

	s.players[number].state = PlayerSettings::State::kComputer;
}

void SinglePlayerGameSettingsProvider::set_player_tribe(uint8_t const number,
                                                        const std::string& tribe,
                                                        bool random_tribe) {
	if (number >= s.players.size()) {
		return;
	}

	std::string actual_tribe = tribe;
	PlayerSettings& player = s.players[number];
	player.random_tribe = random_tribe;

	while (random_tribe) {
		uint8_t num_tribes = s.tribes.size();
		uint8_t random = RNG::static_rand(num_tribes);
		actual_tribe = s.tribes.at(random).name;
		if (player.state != PlayerSettings::State::kComputer ||
		    s.get_tribeinfo(actual_tribe).suited_for_ai) {
			break;
		}
	}

	for (const Widelands::TribeBasicInfo& tmp_tribe : s.tribes) {
		if (tmp_tribe.name == player.tribe) {
			s.players[number].tribe = actual_tribe;
			if (tmp_tribe.initializations.size() <= player.initialization_index) {
				player.initialization_index = 0;
			}
		}
	}
}

void SinglePlayerGameSettingsProvider::set_player_init(uint8_t const number, uint8_t const index) {
	if (number >= s.players.size()) {
		return;
	}

	for (const Widelands::TribeBasicInfo& tmp_tribe : s.tribes) {
		if (tmp_tribe.name == s.players[number].tribe) {
			if (index < tmp_tribe.initializations.size()) {
				s.players[number].initialization_index = index;
			}
			return;
		}
	}
	NEVER_HERE();
}

void SinglePlayerGameSettingsProvider::set_player_team(uint8_t number, Widelands::TeamNumber team) {
	if (number < s.players.size()) {
		s.players[number].team = team;
	}
}

void SinglePlayerGameSettingsProvider::set_player_color(const uint8_t number, const RGBColor& c) {
	if (number < s.players.size()) {
		s.players[number].color = c;
	}
}

void SinglePlayerGameSettingsProvider::set_player_closeable(uint8_t /*number*/,
                                                            bool /*closeable*/) {
	// nothing to do
}

void SinglePlayerGameSettingsProvider::set_player_shared(PlayerSlot /*number*/,
                                                         Widelands::PlayerNumber /*shared*/) {
	// nothing to do
}

void SinglePlayerGameSettingsProvider::set_player_name(uint8_t const number,
                                                       const std::string& name) {
	if (number < s.players.size()) {
		s.players[number].name = name;
	}
}

void SinglePlayerGameSettingsProvider::set_player(uint8_t const number, const PlayerSettings& ps) {
	if (number < s.players.size()) {
		s.players[number] = ps;
	}
}

void SinglePlayerGameSettingsProvider::set_player_number(uint8_t const number) {
	if (number >= s.players.size()) {
		return;
	}
	PlayerSettings const position = settings().players.at(number);
	// Ensure that old player number isn't out of range when we switch to a map with less players
	PlayerSettings const player = settings().players.at(
	   settings().playernum < static_cast<int>(settings().players.size()) ? settings().playernum :
                                                                           0);
	if (number < settings().players.size() && (position.state == PlayerSettings::State::kOpen ||
	                                           position.state == PlayerSettings::State::kClosed ||
	                                           position.state == PlayerSettings::State::kComputer)) {

		// swap player but keep player name
		set_player(number, player);
		set_player_name(number, position.name);

		set_player(settings().playernum, position);
		set_player_name(settings().playernum, player.name);
		s.playernum = number;
	}
}

std::string SinglePlayerGameSettingsProvider::get_win_condition_script() {
	return s.win_condition_script;
}

void SinglePlayerGameSettingsProvider::set_win_condition_script(const std::string& wc) {
	s.win_condition_script = wc;
}

int32_t SinglePlayerGameSettingsProvider::get_win_condition_duration() {
	return s.win_condition_duration;
}

void SinglePlayerGameSettingsProvider::set_win_condition_duration(const int32_t duration) {
	s.win_condition_duration = duration;
}
