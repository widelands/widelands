/*
 * Copyright (C) 2015-2025 by the Widelands Development Team
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

#include "logic/replay_game_controller.h"

#include <memory>

#include <SDL_timer.h>

#include "commands/cmd_replay_end.h"
#include "logic/game.h"
#include "logic/playersmanager.h"
#include "logic/replay.h"
#include "wui/interactive_base.h"

ReplayGameController::ReplayGameController(Widelands::Game& game)
   : game_(game), lastframe_(SDL_GetTicks()), time_(game_.get_gametime()) {
	replayreader_.reset(new Widelands::ReplayReader(game_, game_.replay_filename()));
}

void ReplayGameController::think() {
	uint32_t curtime = SDL_GetTicks();
	int32_t frametime = curtime - lastframe_;
	lastframe_ = curtime;

	// prevent crazy frametimes
	if (frametime < 0) {
		frametime = 0;
	} else if (frametime > 1000) {
		frametime = 1000;
	}

	frametime = frametime * real_speed() / 1000;

	time_ = game_.get_gametime() + Duration(frametime);

	if (replayreader_) {
		while (Widelands::Command* const cmd = replayreader_->get_next_command(time_)) {
			game_.enqueue_command(cmd);
		}

		if (replayreader_->end_of_replay()) {
			time_ = game_.get_gametime();
			replayreader_.reset(nullptr);
			game_.enqueue_command(new Widelands::CmdReplayEnd(time_));
		}
	}
}

void ReplayGameController::send_player_command(Widelands::PlayerCommand* /* command */) {
	throw wexception("Trying to send a player command during replay");
}

Duration ReplayGameController::get_frametime() {
	return time_ - game_.get_gametime();
}

GameController::GameType ReplayGameController::get_game_type() {
	return GameController::GameType::kReplay;
}

uint32_t ReplayGameController::real_speed() {
	return paused_ ? 0 : speed_;
}

uint32_t ReplayGameController::desired_speed() {
	return speed_;
}

void ReplayGameController::set_desired_speed(uint32_t const speed) {
	speed_ = speed;
}

bool ReplayGameController::is_paused() {
	return paused_;
}

void ReplayGameController::set_paused(bool const paused) {
	paused_ = paused;
}

void ReplayGameController::report_result(uint8_t p_nr,
                                         Widelands::PlayerEndResult result,
                                         const std::string& info) {
	Widelands::PlayerEndStatus pes;
	Widelands::Player* player = game_.get_player(p_nr);
	assert(player != nullptr);
	pes.player = player->player_number();
	pes.time = game_.get_gametime();
	pes.result = result;
	pes.info = info;
	game_.player_manager()->add_player_end_status(pes);
}
