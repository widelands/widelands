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

#include "logic/replay_game_controller.h"

#include <memory>

#include <SDL_timer.h>

#include "logic/game.h"
#include "logic/replay.h"
#include "ui_basic/messagebox.h"
#include "wui/interactive_base.h"

ReplayGameController::ReplayGameController(Widelands::Game& game, const std::string& filename)
   : game_(game),
     lastframe_(SDL_GetTicks()),
     time_(game_.get_gametime()),
     speed_(1000),
     paused_(false) {
	game_.set_game_controller(std::shared_ptr<ReplayGameController>(this));
	replayreader_.reset(new Widelands::ReplayReader(game_, filename));
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
			replayreader_.reset(nullptr);
			game_.enqueue_command(new CmdReplayEnd(time_ = game_.get_gametime()));
		}
	}
}

void ReplayGameController::send_player_command(Widelands::PlayerCommand*) {
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

void ReplayGameController::CmdReplayEnd::execute(Widelands::Game& game) {
	game.game_controller()->set_desired_speed(0);

	// Need to pull this out into a variable to make the includes script happy
	InteractiveBase* i = game.get_ibase();
	assert(i);
	UI::WLMessageBox mmb(i, UI::WindowStyle::kWui, _("End of Replay"),
	                     _("The end of the replay has been reached and the game has "
	                       "been paused. You may unpause the game and continue watching "
	                       "if you want to."),
	                     UI::WLMessageBox::MBoxType::kOk);
	mmb.run<UI::Panel::Returncodes>();
}

Widelands::QueueCommandTypes ReplayGameController::CmdReplayEnd::id() const {
	return Widelands::QueueCommandTypes::kReplayEnd;
}
