/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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

#include "logic/playersmanager.h"

#include "base/wexception.h"
#include "logic/editor_game_base.h"
#include "logic/game_settings.h"
#include "logic/player.h"
#include "wui/interactive_gamebase.h"

namespace Widelands {

PlayersManager::PlayersManager(EditorGameBase& egbase) : egbase_(egbase) {
	memset(players_, 0, sizeof(players_));
}

PlayersManager::~PlayersManager() {
	cleanup();
}

void PlayersManager::cleanup() {
	const Player* const* const players_end = players_ + kMaxPlayers;
	for (Player** p = players_; p < players_end; ++p) {
		delete *p;
		*p = nullptr;
	}
	number_of_players_ = 0;
	players_end_status_.clear();
}

void PlayersManager::remove_player(PlayerNumber plnum) {
	assert(1 <= plnum);
	assert(plnum <= kMaxPlayers);

	Player*& p = players_[plnum - 1];
	if (p != nullptr) {
		delete p;
		p = nullptr;
		if (plnum <= UserSettings::highest_playernum()) {
			number_of_players_--;
		}
	}
}

Player* PlayersManager::add_player(PlayerNumber const player_number,
                                   uint8_t const initialization_index,
                                   const RGBColor& pc,
                                   const std::string& tribe,
                                   const std::string& name,
                                   TeamNumber team) {
	assert(1 <= player_number);
	assert(player_number <= kMaxPlayers);

	Player*& p = players_[player_number - 1];
	if (p != nullptr) {
		delete p;
		if (player_number <= UserSettings::highest_playernum()) {
			number_of_players_--;
		}
	}

	const DescriptionIndex tribe_index = egbase_.mutable_descriptions()->load_tribe(tribe);
	p = new Player(egbase_, player_number, initialization_index, pc,
	               *egbase_.descriptions().get_tribe_descr(tribe_index), name);

	p->set_team_number(team);
	if (player_number <= UserSettings::highest_playernum()) {
		number_of_players_++;
	}
	return p;
}

const PlayerEndStatus* PlayersManager::get_player_end_status(PlayerNumber player) const {
	auto it = players_end_status_.find(player);
	if (it == players_end_status_.end()) {
		return nullptr;
	}
	return &(it->second);
}

void PlayersManager::add_player_end_status(const PlayerEndStatus& status, bool change_existing) {
	assert(status.player > 0);
	const PlayerNumber& pn = status.player;
	auto it = players_end_status_.find(pn);

	if (it == players_end_status_.end()) {
		players_end_status_.emplace(pn, status);
	} else {
		if (!change_existing) {
			throw wexception("Player end status for player %d already reported", status.player);
		}
		it->second = status;
	}

	/* If all results have been gathered, show the summary screen. */
	if (change_existing || egbase_.get_igbase() == nullptr) {
		return;
	}
	if (players_end_status_.size() < number_of_players_) {
		return;
	}
	egbase_.get_igbase()->show_game_summary();
}

}  // namespace Widelands
