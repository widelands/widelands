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

#ifndef WL_LOGIC_SINGLE_PLAYER_GAME_CONTROLLER_H
#define WL_LOGIC_SINGLE_PLAYER_GAME_CONTROLLER_H

#include "ai/computer_player.h"
#include "logic/game_controller.h"


class SinglePlayerGameController : public GameController {
public:
	SinglePlayerGameController
		(Widelands::Game &, bool useai, Widelands::PlayerNumber local);
	~SinglePlayerGameController();

	void think() override;
	void sendPlayerCommand(Widelands::PlayerCommand &) override;
	int32_t getFrametime() override;
	GameController::GameType getGameType() override;

	uint32_t realSpeed() override;
	uint32_t desiredSpeed() override;
	void setDesiredSpeed(uint32_t speed) override;
	bool isPaused() override;
	void setPaused(bool paused) override;
	void report_result(uint8_t player, Widelands::PlayerEndResult result, const std::string & info) override;

private:
	Widelands::Game & m_game;
	bool m_useai;
	int32_t m_lastframe;
	int32_t m_time;
	uint32_t m_speed; ///< current game speed, in milliseconds per second
	bool m_paused;
	uint32_t m_player_cmdserial;
	Widelands::PlayerNumber m_local;
	std::vector<ComputerPlayer *> m_computerplayers;
};

#endif  // end of include guard: WL_LOGIC_SINGLE_PLAYER_GAME_CONTROLLER_H
