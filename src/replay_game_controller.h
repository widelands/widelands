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

#ifndef REPLAY_GAME_CONTROLLER_H
#define REPLAY_GAME_CONTROLLER_H

#include "gamecontroller.h"
#include "logic/cmd_queue.h"
#include "wlapplication.h"

namespace Widelands {
	class ReplayReader;
}

class ReplayGameController : public GameController {
public:
	ReplayGameController(Widelands::Game & game, const std::string & filename);

	void think() override;

	void sendPlayerCommand(Widelands::PlayerCommand &) override;
	int32_t getFrametime() override;
	std::string getGameDescription() override;
	uint32_t realSpeed() override;
	uint32_t desiredSpeed() override;
	void setDesiredSpeed(uint32_t const speed) override;
	bool isPaused() override;
	void setPaused(bool const paused) override;

private:
	struct Cmd_ReplayEnd : public Widelands::Command {
		Cmd_ReplayEnd (int32_t const _duetime) : Widelands::Command(_duetime) {}
		virtual void execute (Widelands::Game & game);
		virtual uint8_t id() const;
	};

	Widelands::Game & m_game;
	std::unique_ptr<Widelands::ReplayReader> m_replayreader;
	int32_t m_lastframe;
	int32_t m_time;
	uint32_t m_speed;
	bool m_paused;
};

#endif
