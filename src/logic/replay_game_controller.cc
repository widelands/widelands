/*
 * Copyright (C) 2015 by the Widelands Development Team
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

#include "logic/replay_game_controller.h"

#include "logic/game.h"
#include "logic/replay.h"
#include "ui_basic/messagebox.h"
#include "wlapplication.h"
#include "wui/interactive_base.h"


ReplayGameController::ReplayGameController(Widelands::Game & game, const std::string & filename) :
	m_game(game),
	m_lastframe(WLApplication::get()->get_time()),
	m_time(m_game.get_gametime()),
	m_speed(1000),
	m_paused(false)
{
	m_game.set_game_controller(this);

	// We have to create an empty map, otherwise nothing will load properly
	game.set_map(new Widelands::Map);
	m_replayreader.reset(new Widelands::ReplayReader(m_game, filename));
}

void ReplayGameController::think() {
	int32_t curtime = WLApplication::get()->get_time();
	int32_t frametime = curtime - m_lastframe;
	m_lastframe = curtime;

	// prevent crazy frametimes
	if (frametime < 0)
		frametime = 0;
	else if (frametime > 1000)
		frametime = 1000;

	frametime = frametime * real_speed() / 1000;

	m_time = m_game.get_gametime() + frametime;

	if (m_replayreader) {
		while
			(Widelands::Command * const cmd =
				m_replayreader->get_next_command(m_time))
			m_game.enqueue_command(cmd);

		if (m_replayreader->end_of_replay()) {
			m_replayreader.reset(nullptr);
			m_game.enqueue_command
				(new CmdReplayEnd(m_time = m_game.get_gametime()));
		}
	}
}

void ReplayGameController::send_player_command(Widelands::PlayerCommand &) {
	throw wexception("Trying to send a player command during replay");
}

int32_t ReplayGameController::get_frametime() {
	return m_time - m_game.get_gametime();
}

GameController::GameType ReplayGameController::get_game_type() {
	return GameController::GameType::REPLAY;
}

uint32_t ReplayGameController::real_speed() {
	return m_paused ? 0 : m_speed;
}

uint32_t ReplayGameController::desired_speed() {
	return m_speed;
}

void ReplayGameController::set_desired_speed(uint32_t const speed) {
	m_speed = speed;
}

bool ReplayGameController::is_paused() {
	return m_paused;
}

void ReplayGameController::set_paused(bool const paused) {
	m_paused = paused;
}

void ReplayGameController::CmdReplayEnd::execute (Widelands::Game & game) {
	game.game_controller()->set_desired_speed(0);
	UI::WLMessageBox mmb
		(game.get_ibase(),
		 _("End of replay"),
		 _("The end of the replay has been reached and the game has "
			"been paused. You may unpause the game and continue watching "
			"if you want to."),
		 UI::WLMessageBox::MBoxType::kOk);
	mmb.run<UI::Panel::Returncodes>();
}

uint8_t ReplayGameController::CmdReplayEnd::id() const {
	return QUEUE_CMD_REPLAYEND;
}
