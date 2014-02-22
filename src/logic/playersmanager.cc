/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#include "logic/playersmanager.h"

#include <cstring>

#include "gamesettings.h"
#include "logic/editor_game_base.h"
#include "logic/player.h"
#include "wexception.h"
#include "wui/interactive_gamebase.h"

namespace Widelands {

Players_Manager::Players_Manager(Editor_Game_Base& egbase) :
m_egbase(egbase),
m_number_of_players(0)
{
	memset(m_players, 0, sizeof(m_players));
}

Players_Manager::~Players_Manager()
{
	cleanup();
}

void Players_Manager::cleanup()
{
	const Player * const * const players_end = m_players + MAX_PLAYERS;
	for (Player * * p = m_players; p < players_end; ++p) {
		delete *p;
		*p = nullptr;
	}
	m_number_of_players = 0;
}


void Players_Manager::remove_player(Player_Number plnum)
{
	assert(1 <= plnum);
	assert(plnum <= MAX_PLAYERS);

	Player * & p = m_players[plnum - 1];
	if (p) {
		delete p;
		p = nullptr;
		if (plnum <= UserSettings::highestPlayernum()) {
			m_number_of_players--;
		}
	}
}

Player* Players_Manager::add_player
	(Player_Number       const player_number,
	 uint8_t             const initialization_index,
	 const std::string &       tribe,
	 const std::string &       name,
	 TeamNumber                team)
{
	assert(1 <= player_number);
	assert(player_number <= MAX_PLAYERS);

	Player * & p = m_players[player_number - 1];
	if (p) {
		delete p;
		if (player_number <= UserSettings::highestPlayernum()) {
			m_number_of_players--;
		}
	}
	p = new Player
		(m_egbase,
		 player_number,
		 initialization_index,
		 m_egbase.manually_load_tribe(tribe),
		 name);
	p->set_team_number(team);
	if (player_number <= UserSettings::highestPlayernum()) {
		m_number_of_players++;
	}
	return p;
}

void Players_Manager::add_player_end_status(const PlayerEndStatus& status)
{
	// Ensure we don't have a status for it yet
	std::vector<PlayerEndStatus>::iterator it;
	for (it = m_players_end_status.begin(); it != m_players_end_status.end(); ++it) {
		PlayerEndStatus pes = *it;
		if (pes.player == status.player) {
			throw wexception("Player End status for player %d already reported", pes.player);
		}
	}
	m_players_end_status.push_back(status);

	// If all results have been gathered, save game and show summary screen
	if (m_players_end_status.size() < m_number_of_players) {
		return;
	}

	if (m_egbase.get_igbase()) {
		m_egbase.get_igbase()->show_game_summary();
	}
}


}  // namespace Widelands
