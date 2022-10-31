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

#ifndef WL_LOGIC_REPLAY_GAME_CONTROLLER_H
#define WL_LOGIC_REPLAY_GAME_CONTROLLER_H

#include <memory>

#include "logic/cmd_queue.h"
#include "logic/game_controller.h"

namespace Widelands {
class ReplayReader;
}

class ReplayGameController : public GameController {
public:
	explicit ReplayGameController(Widelands::Game& game);

	void think() override;

	void send_player_command(Widelands::PlayerCommand* command) override;
	Duration get_frametime() override;
	GameController::GameType get_game_type() override;
	uint32_t real_speed() override;
	uint32_t desired_speed() override;
	void set_desired_speed(uint32_t const speed) override;
	bool is_paused() override;
	void set_paused(bool const paused) override;

private:
	struct CmdReplayEnd : public Widelands::Command {
		explicit CmdReplayEnd(const Time& init_duetime) : Widelands::Command(init_duetime) {
		}
		void execute(Widelands::Game& game) override;
		Widelands::QueueCommandTypes id() const override;
	};

	Widelands::Game& game_;
	std::unique_ptr<Widelands::ReplayReader> replayreader_;
	int32_t lastframe_;
	Time time_;
	uint32_t speed_;
	bool paused_;
};

#endif  // end of include guard: WL_LOGIC_REPLAY_GAME_CONTROLLER_H
