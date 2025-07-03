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

#ifndef WL_LOGIC_REPLAY_GAME_CONTROLLER_H
#define WL_LOGIC_REPLAY_GAME_CONTROLLER_H

#include <memory>

#include "commands/command.h"
#include "logic/game_controller.h"

namespace Widelands {
class ReplayReader;
}  // namespace Widelands

class ReplayGameController : public GameController {
public:
	explicit ReplayGameController(Widelands::Game& game);

	void think() override;

	void send_player_command(Widelands::PlayerCommand* command) override;
	Duration get_frametime() override;
	GameController::GameType get_game_type() override;
	uint32_t real_speed() override;
	uint32_t desired_speed() override;
	void set_desired_speed(uint32_t speed) override;
	bool is_paused() override;
	void set_paused(bool paused) override;
	void
	report_result(uint8_t p_nr, Widelands::PlayerEndResult result, const std::string& info) override;
	void set_write_replay(bool /* replay */) override {
		NEVER_HERE();
	}

private:
	Widelands::Game& game_;
	std::unique_ptr<Widelands::ReplayReader> replayreader_;
	int32_t lastframe_;
	Time time_;
	uint32_t speed_{1000};
	bool paused_{false};
};

#endif  // end of include guard: WL_LOGIC_REPLAY_GAME_CONTROLLER_H
