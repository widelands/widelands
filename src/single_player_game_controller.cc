/*
 * Copyright (C) 2014 by the Widelands Development Team
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

#include "single_player_game_controller.h"

#include "computer_player.h"
#include "logic/game.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "logic/playersmanager.h"
#include "profile/profile.h"
#include "wlapplication.h"


SinglePlayerGameController::SinglePlayerGameController
	(Widelands::Game        &       game,
	 bool                     const useai,
	 Widelands::Player_Number const local)
	: m_game          (game),
	m_useai           (useai),
	m_lastframe       (WLApplication::get()->get_time()),
	m_time            (m_game.get_gametime()),
	m_speed
		(g_options.pull_section("global").get_natural
		 	("speed_of_new_game", 1000)),
	m_paused(false),
	m_player_cmdserial(0),
	m_local           (local)
{
}

SinglePlayerGameController::~SinglePlayerGameController()
{
	for (uint32_t i = 0; i < m_computerplayers.size(); ++i)
		delete m_computerplayers[i];
	m_computerplayers.clear();
}

void SinglePlayerGameController::think()
{
	int32_t const curtime = WLApplication::get()->get_time();
	int32_t frametime = curtime - m_lastframe;
	m_lastframe = curtime;

	// prevent crazy frametimes
	if (frametime < 0)
		frametime = 0;
	else if (frametime > 1000)
		frametime = 1000;

	frametime = frametime * realSpeed() / 1000;

	m_time = m_game.get_gametime() + frametime;

	if (m_useai && m_game.is_loaded()) {
		const Widelands::Player_Number nr_players = m_game.map().get_nrplayers();
		iterate_players_existing(p, nr_players, m_game, plr)
			if (p != m_local) {

				if (p > m_computerplayers.size())
					m_computerplayers.resize(p);
				if (!m_computerplayers[p - 1])
					m_computerplayers[p - 1] =
						Computer_Player::getImplementation
							(plr->getAI())->instantiate(m_game, p);
				m_computerplayers[p - 1]->think();
			}
	}
}

void SinglePlayerGameController::sendPlayerCommand
	(Widelands::PlayerCommand & pc)
{
	pc.set_cmdserial(++m_player_cmdserial);
	m_game.enqueue_command (&pc);
}

int32_t SinglePlayerGameController::getFrametime()
{
	return m_time - m_game.get_gametime();
}

std::string SinglePlayerGameController::getGameDescription()
{
	return "singleplayer";
}

uint32_t SinglePlayerGameController::realSpeed()
{
	if (m_paused)
		return 0;
	else
		return m_speed;
}

uint32_t SinglePlayerGameController::desiredSpeed()
{
	return m_speed;
}

void SinglePlayerGameController::setDesiredSpeed(uint32_t const speed)
{
	m_speed = speed;
}

bool SinglePlayerGameController::isPaused()
{
	return m_paused;
}

void SinglePlayerGameController::setPaused(bool paused)
{
	m_paused = paused;
}

void SinglePlayerGameController::report_result
	(uint8_t p_nr, Widelands::PlayerEndResult result, const std::string & info)
{
	Widelands::PlayerEndStatus pes;
	Widelands::Player* player = m_game.get_player(p_nr);
	assert(player);
	pes.player = player->player_number();
	pes.time = m_game.get_gametime();
	pes.result = result;
	pes.info = info;
	m_game.player_manager()->add_player_end_status(pes);
}
