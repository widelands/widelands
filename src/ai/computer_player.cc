/*
 * Copyright (C) 2004-2020 by the Widelands Development Team
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

#include "ai/computer_player.h"

#include <SDL_timer.h>

#include "ai/defaultai.h"
#include "base/multithreading.h"

ComputerPlayer::ComputerPlayer(Widelands::Game& g, Widelands::PlayerNumber const pid)
   : game_(g), player_number_(pid) {
}

ComputerPlayer::~ComputerPlayer() {
}

struct EmptyAI : ComputerPlayer {
	EmptyAI(Widelands::Game& g, const Widelands::PlayerNumber pid) : ComputerPlayer(g, pid) {
	}

	void think() override {
	}

	struct EmptyAIImpl : Implementation {
		EmptyAIImpl()
		   : Implementation(
		        "empty",
		        /** TRANSLATORS: This is the name of an AI used in the game setup screens */
		        _("No AI"),
		        "images/ai/ai_empty.png",
		        Implementation::Type::kEmpty) {
		}
		ComputerPlayer* instantiate(Widelands::Game& g,
		                            Widelands::PlayerNumber const pid) const override {
			return new EmptyAI(g, pid);
		}
	};

	static EmptyAIImpl implementation;
};

EmptyAI::EmptyAIImpl EmptyAI::implementation;

const ComputerPlayer::ImplementationVector& ComputerPlayer::get_implementations() {
	static std::vector<ComputerPlayer::Implementation const*> impls;

	if (impls.empty()) {
		impls.push_back(&DefaultAI::normal_impl);
		impls.push_back(&DefaultAI::weak_impl);
		impls.push_back(&DefaultAI::very_weak_impl);
		impls.push_back(&EmptyAI::implementation);
	}

	return impls;
}

const ComputerPlayer::Implementation* ComputerPlayer::get_implementation(const std::string& name) {
	const ImplementationVector& vec = get_implementations();

	for (const ComputerPlayer::Implementation* implementation : vec) {
		if (implementation->name == name) {
			return implementation;
		}
	}
	return vec[0];
}

// The number of milliseconds realtime how frequently an AI wants to be allowed to think()
constexpr uint32_t kThinkDelay = 200;

void* ComputerPlayer::runthread(void* cp) {
	assert(cp);
	ComputerPlayer& ai = *static_cast<ComputerPlayer*>(cp);
	uint32_t next_time = SDL_GetTicks() + kThinkDelay;
	for (;;) {
		const uint32_t time = SDL_GetTicks();
		if (time >= next_time) {
			MutexLock m;
			if (!m.is_valid()) {
				// end of game
				break;
			}
			ai.think();
		}
		const int32_t delay = next_time - SDL_GetTicks();
		if (delay > 0) {
			SDL_Delay(delay);
		}
	}
	pthread_exit(NULL);
	return nullptr;
}
